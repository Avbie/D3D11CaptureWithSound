#include "ClsD3D11Recording.h"

/*************************************************************************************************
**************************************PUBLIC-METHODES*********************************************
*************************************************************************************************/

/// <summary>
/// Constructor, defined by User
/// </summary>
/// <param name="pVidDesc">VideoDescription, must be set by User</param>
ClsD3D11Recording::ClsD3D11Recording(VideoDescriptor* pVidDesc)
{
	m_bIsAudio = pVidDesc->bIsAudio;
	m_hWnd = NULL;
	m_iXPos = 0;
	m_iYPos = 0;
	m_iTop = 0;
	m_iLeft = 0;
	m_uiPickedMonitor = 0;
	m_myCpyMethod = pVidDesc->myCpyMethod;
	m_myBitReading = PicDataBitReading::Standard;
	m_myClientRect = {};
	m_myWndRect = {};
	m_vMonitors.clear();
	m_pFrameData = new FrameData();

	// Get DC of Graphiccard (incudes all Monitors as a virtual Desktop)
	HDC DevHC = GetDC(NULL);
	EnumDisplayMonitors(DevHC, 0, MonitorEnum, (LPARAM)this);
	SetSrcDisplay(pVidDesc->uiMonitorID);
	// Sets Values in m_pFrameData
	SetSrcResolution();										
	SetDestResolution(pVidDesc->uiWidthDest, pVidDesc->uiHeightDest);
	// Set Window Size and Position with the MonitorData. 
	// MonitorData by EnumDisplayMonitors() earlier
	SetWindowRect();
	SetWindowPosition();

	SinkWriter().SetFrameData(&m_pFrameData);
	SinkWriter().SetBitReading(PicDataBitReading::Standard);// m_myBitReading);

	WinGDI().SetFrameData(&m_pFrameData);
	WinGDI().SetSrcWndTitle(pVidDesc->strWndTitle);
	WinGDI().SetCpyMethod(m_myCpyMethod);

	D3D11().SetFrameData(&m_pFrameData);
	D3D11().SetCpyMethod(m_myCpyMethod);
	D3D11().SetPickedMonitor(m_uiPickedMonitor);

	SinkWriter().SetFPS(pVidDesc->uiFPS);
	SinkWriter().SetFormats(pVidDesc->myInputFormat, pVidDesc->myOutputFormat);
	SinkWriter().SetBitReading(pVidDesc->myBitReading);
	SinkWriter().SetFileName(pVidDesc->strFileName);
	SinkWriter().SetAudio(pVidDesc->bIsAudio);

	SyncFPS().SetFrameDuaration(SinkWriter().GetVideoFrameDuration());
}//END-CONS
/// <summary>
/// Destructor
/// </summary>
ClsD3D11Recording::~ClsD3D11Recording()
{
	if (m_pFrameData->pData)
	{
		delete m_pFrameData->pData;
		m_pFrameData->pData = nullptr;
	}
	if (m_pFrameData)
	{
		delete m_pFrameData;
		m_pFrameData = nullptr;
	}
}//END-DESTR
/// <summary>
/// Init. the D3D11-Window
/// </summary>
void ClsD3D11Recording::Init3DWindow()
{
	D3D11().CreateDevice();
	D3D11().CreateSwapChain();
	D3D11().CreateSwapChainBuffer();
	D3D11().CreateBuffers();
	D3D11().CreateAndSetSampler();
	D3D11().CreateD3D11Texture();
	D3D11().CreateShaderView();
	D3D11().CreateAndSetShader();
	D3D11().CreateInputLayout();
	D3D11().SetClientRect(GetMyClientRect());
	D3D11().PreparePresentation();
	WinGDI().FindSetWindow();
}//END-FUNC
/// <summary>
/// Preparing the Sinkwriter for Recording a Video with/without Audio
/// - Set Input and Putput Format
/// - Sets the Status of the IMFSinkwriter to ready for capturing
/// - synchronize the FPS - set TimeStamps 
/// </summary>
void ClsD3D11Recording::PrepareRecording()
{
	SinkWriter().PrepareInputOutput();
	SinkWriter().StartRecording();
	SyncFPS().Start();
}//END-FUNC
/// <summary>
/// This Method have to be called in the FPS-loop
/// Capturing 1 Frame per 1 LoopPassage
/// </summary>
void ClsD3D11Recording::Recording()
{
	++SyncFPS();
	if (!IsDesktopDupl())
		WinGDI().GetBitBltDataFromWindow();

	if (m_bIsAudio)
		SinkWriter().StartReadAudioHWBufferThread();
	D3D11().BitBltDataToRT();
	//D3D11().SetConstantBuffer();
	//WinGDI().TakeScreenshot();
	SinkWriter().LoopRecording();
	D3D11().PresentTexture();
	SyncFPS().SleepUntilNextFrame();
}//END-FUNC
/// <summary>
/// Tells the Sinkwriter to stop recording Frames
/// </summary>
void ClsD3D11Recording::StopRecording()
{
	SinkWriter().StopRecording();
}//END-FUNC
/*************************************************************************************************
**************************************PRIVATE-GET-METHODES****************************************
*************************************************************************************************/
/*Returning the SubObjects that will be used in this Superclass*/
D3D::ClsD3D11& ClsD3D11Recording::D3D11()
{
	return m_myClsD3D11;
}
ClsSinkWriter& ClsD3D11Recording::SinkWriter()
{
	return m_myClsSinkWriter;
}
ClsFPSSync& ClsD3D11Recording::SyncFPS()
{
	return m_myClsSyncFPS;
}
GDI::ClsWinGDI& ClsD3D11Recording::WinGDI()
{
	return m_myClsWinGDI;
}
/*************************************************************************************************
**************************************PRIVATE-METHODES********************************************
*************************************************************************************************/

/// <summary>
/// Sets the CopyMethod in the capturingProcess for the Frames
/// - DesktopDuplication: Frame will be readed directly in GPU and copied to a texture with CPU write access
///		- no dirty copies yet, just complete Image
/// - Mapping: copying a specific Window by WndHandle per GDI
/// - D2D1Surface: copying from GDI Data to D2D1Surface - Surface is over the D3DArea
/// - SubResource: will be updated by CPU when GPU-VRAM is not locked by GPU (sometimes slow, cpu have to wait)
/// CalledBy: User
/// </summary>
/// <param name="myCpyMethod">CopyMethod</param>
void ClsD3D11Recording::SetCpyMethod(CopyMethod myCpyMethod)
{
	m_myCpyMethod = myCpyMethod;
	WinGDI().SetCpyMethod(m_myCpyMethod);
	D3D11().SetCpyMethod(m_myCpyMethod);
}
/// <summary>
/// Sets the Resolution that from the Videodescriptor set by user.
/// If the CopyMethod is the DesktopDupl the DestRes is set to the SourceRes.
/// CalledBy: Descriptor
/// </summary>
/// <param name="uiWidthDest">Width</param>
/// <param name="uiHeightDest">Height</param>
void ClsD3D11Recording::SetDestResolution(UINT uiWidthDest, UINT uiHeightDest)
{
	// DesktopDupl cannot to be scaled
	if (m_myCpyMethod == CopyMethod::DesktopDupl)
	{
		m_pFrameData->uiWidthDest = m_pFrameData->uiWidthSrc;
		m_pFrameData->uiHeightDest = m_pFrameData->uiHeightSrc;
	}
	else
	{
		m_pFrameData->uiWidthDest = uiWidthDest;
		m_pFrameData->uiHeightDest = uiHeightDest;
	}
	m_pFrameData->uiPixelDataSize = m_pFrameData->uiWidthDest * m_pFrameData->uiHeightDest * m_pFrameData->uiBpp;
	m_pFrameData->pData = new unsigned char[m_pFrameData->uiPixelDataSize];					// DatenBuffer für PixelDaten; GetBits = Width*Height*Bpp
	memset(m_pFrameData->pData, 0, m_pFrameData->uiPixelDataSize);
}//END-FUNC
/// <summary>
/// Sets the Window of this Application.
///  - D3D11 needs a target where the D3D11 rect will be displayed
/// </summary>
/// <param name="hWnd">WindowHandle of this App</param>
void ClsD3D11Recording::SetHWND(HWND hWnd)
{
	m_hWnd = hWnd;
	D3D11().SetWnd(hWnd);
}//END-FUNC
/// <summary>
/// - Sets the Monitor that we want to capture
///   - used by DesktopDupl
/// - Enumation of Monitors still executed
/// - Will use the MainMonitor if the input is invalid
/// CalledBy: Constructor
/// </summary>
/// <param name="uiMonitorID">selected Monitor</param>
void ClsD3D11Recording::SetSrcDisplay(UINT uiMonitorID)
{
	if (uiMonitorID > m_uiMaxMonitors)
		m_uiPickedMonitor = 0;
	else
		m_uiPickedMonitor = uiMonitorID;
}//END-FUNC
/// <summary>
/// - Sets the Source Resolution depending on the picked Monitor
/// CalledBy: Constructor
/// </summary>
void ClsD3D11Recording::SetSrcResolution()
{
	m_pFrameData->uiWidthSrc = m_vMonitors[m_uiPickedMonitor]->GetWidth();
	m_pFrameData->uiHeightSrc = m_vMonitors[m_uiPickedMonitor]->GetHeight();
	m_pFrameData->uiTop = m_vMonitors[m_uiPickedMonitor]->GetTop();
	m_pFrameData->uiLeft = m_vMonitors[m_uiPickedMonitor]->GetLeft();
	m_pFrameData->uiBpp = BITDEPTH;
}//END-FUNC
/// <summary>
/// - Set the position of the Window of this App, depending on the Monitor Resolution
/// CalledBy: Constructor
/// </summary>
void ClsD3D11Recording::SetWindowPosition()
{
	m_iXPos = m_pFrameData->uiWidthDest / 2 - m_myWndRect.right / 2;
	m_iYPos = m_pFrameData->uiHeightDest / 2 - m_myWndRect.bottom / 2;
}
/// <summary>
/// Set the Size of the Window o this App
/// CalledBy: Constructor
/// </summary>
void ClsD3D11Recording::SetWindowRect()
{
	m_myWndRect.top = 0;
	m_myWndRect.bottom = 800;
	m_myWndRect.left = 0;
	m_myWndRect.right = 800;
	AdjustWindowRect(&m_myWndRect, WS_BORDER | WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, FALSE);
}//END-FUNC
/// <summary>
/// Checks if CopyMethod DesktopDupl is used
/// CalledBy: Recording()
/// </summary>
/// <returns>TRUE if its DesktopDupl</returns>
BOOL ClsD3D11Recording::IsDesktopDupl()
{
	if (m_myCpyMethod == CopyMethod::DesktopDupl)
		return true;
	return false;
}//END-FUNC
/// <summary>
/// - overwritten Callback Function for enum Monitors.
/// - Set in EnumDisplayMonitors, called in the Constructor
/// </summary>
/// <param name="hMonitor">Handle of the Monitor</param>
/// <param name="hdc">Handle to the DeviceContext (Graficcard)</param>
/// <param name="lprcMonitor">Rect with Resolution Info (we used GetMonitorInfo instead)</param>
/// <param name="pData">Custom Parameter, Classpointer ClsD3D11Recording</param>
/// <returns>always TRUE</returns>
BOOL CALLBACK ClsD3D11Recording::MonitorEnum(HMONITOR hMonitor, HDC hdc, LPRECT lprcMonitor, LPARAM pData)
{
	m_uiMaxMonitors++;
	MONITORINFOEX myMonitorInfo;
	myMonitorInfo.cbSize = sizeof(MONITORINFOEX);
	GetMonitorInfo(hMonitor, &myMonitorInfo);

	ClsD3D11Recording* pThis = reinterpret_cast<ClsD3D11Recording*>(pData);
	MonitorInfo* pMonitorInfo = new MonitorInfo;

	//pMonitorInfo->myMonInfo = *lprcMonitor;
	pMonitorInfo->SetPrimaryStatus(myMonitorInfo.dwFlags);
	pMonitorInfo->SetTop(myMonitorInfo.rcMonitor.top);
	pMonitorInfo->SetLeft(myMonitorInfo.rcMonitor.left);
	pMonitorInfo->SetWidth(myMonitorInfo.rcMonitor.right - myMonitorInfo.rcMonitor.left);
	pMonitorInfo->SetHeight(myMonitorInfo.rcMonitor.bottom - myMonitorInfo.rcMonitor.top);
	pThis->m_vMonitors.push_back(pMonitorInfo);

	return TRUE;
}//END-FUNC
/// <summary>
/// - Methods for getting the X/Y Size of the Window of this App. 
///   (const default Size)
/// - Methods for getting the X/Y Position of the top-left edge of the Window of this App. 
///   (depending on Monitor Resolution)
/// CalledBy: CreateWindowEx in ClsWnd::ClsWnd
/// </summary>
UINT ClsD3D11Recording::GetWndXSize() { return m_myWndRect.right - m_myWndRect.left; }
UINT ClsD3D11Recording::GetWndYSize() { return m_myWndRect.bottom - m_myWndRect.top; }
UINT ClsD3D11Recording::GetXPos() { return m_iXPos; }
UINT ClsD3D11Recording::GetYPos() { return m_iYPos; }

/// <summary>
/// Gets the Client Rect of the Window of this App (without Menu, Bar etc.)
/// </summary>
/// <returns>ClientRect</returns>
RECT ClsD3D11Recording::GetMyClientRect()
{
	GetClientRect(m_hWnd, &m_myClientRect);
	return m_myClientRect;
}//END-FUNC