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
	m_pVidDesc = pVidDesc;
	m_bIsRecording = false;
	m_bIsAudio = m_pVidDesc->bIsAudio;
	m_hWnd = NULL;
	m_iXPos = 0;
	m_iYPos = 0;
	m_iTop = 0;
	m_iLeft = 0;
	m_uiPickedMonitor = 0;
	//m_myCpyMethod = m_pVidDesc->myCpyMethod;
	m_myBitReading = PicDataBitReading::Standard;
	m_myClientRect = {};
	m_myWndRect = {};
	m_vMonitors.clear();
	m_pFrameData = new FrameData();
	m_pFrameData->pCpyMethod = &m_pVidDesc->myCpyMethod;

	// Get DC of Graphiccard (incudes all Monitors as a virtual Desktop)
	HDC DevHC = GetDC(NULL);
	EnumDisplayMonitors(DevHC, 0, MonitorEnum, (LPARAM)this);
	SetSrcDisplay(m_pVidDesc->uiMonitorID);
	// Sets Values in m_pFrameData
	SetSrcResolution();										
	SetDestResolution(m_pVidDesc->uiWidthDest, m_pVidDesc->uiHeightDest);
	// Set Window Size and Position with the MonitorData. 
	// MonitorData by EnumDisplayMonitors() earlier
	SetWindowRect();
	SetWindowPosition();

	SinkWriter().SetAudio(m_pVidDesc->bIsAudio);
	SinkWriter().PrepareAudio();
	SinkWriter().SetFrameData(&m_pFrameData);
	//SinkWriter().SetBitReading(PicDataBitReading::Standard);// m_myBitReading);

	WinGDI().SetFrameData(&m_pFrameData);
	WinGDI().SetSrcWndTitle(m_pVidDesc->strWndTitle);
	//WinGDI().SetCpyMethod(m_myCpyMethod);
	WinGDI().SetWndAsSrc(FALSE);

	D3D11().SetFrameData(&m_pFrameData);
	//D3D11().SetCpyMethod(m_myCpyMethod);
	//D3D11().SetPickedMonitor(m_uiPickedMonitor);

	SinkWriter().SetFPS(m_pVidDesc->uiFPS);
	SinkWriter().SetFormats(m_pVidDesc->myInputFormat, m_pVidDesc->myOutputFormat);
	SinkWriter().SetBitReading(m_pVidDesc->myBitReading);
	SinkWriter().SetBitRate();
	SinkWriter().SetFileName(m_pVidDesc->strFileName);

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
/// Tells the Sinkwriter to stop recording Frames
/// </summary>
void ClsD3D11Recording::Finalize()
{
	if (GetRecordingStatus())
	{
		SinkWriter().StopRecording();
		SetRecordingStatus(false);
	}
}//END-FUNC
/// <summary>
/// Gets the current Status if its recording or not
/// </summary>
/// <returns></returns>
BOOL ClsD3D11Recording::GetRecordingStatus()
{
	return m_bIsRecording;
}
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
	//D3D11().SetClientRect(GetMyClientRect());
	
	WinGDI().FindSetWindow();
	D3D11().ReSizeD3D11Window();
}//END-FUNC
/// <summary>
/// This Method have to be called in the FPS-loop
/// Capturing 1 Frame per 1 LoopPassage
/// </summary>
void ClsD3D11Recording::Loop()
{
	++SyncFPS();
	if (!IsDesktopDupl())
		WinGDI().GetBitBltDataFromWindow();

	if (m_bIsAudio && m_bIsRecording)
		SinkWriter().StartReadAudioHWBufferThread();
	D3D11().BitBltDataToRT();
	//D3D11().SetConstantBuffer();
	//WinGDI().TakeScreenshot();
	if (m_bIsRecording)
		SinkWriter().LoopRecording();
	D3D11().PresentTexture();
	SyncFPS().SleepUntilNextFrame();
}//END-FUNC
/// <summary>
/// Preparing the Sinkwriter for Recording a Video with/without Audio
/// - Set Input and Putput Format
/// - Sets the Status of the IMFSinkwriter to ready for capturing
/// - synchronize the FPS - set TimeStamps 
/// </summary>
void ClsD3D11Recording::PrepareRecording()
{
	if (!GetRecordingStatus())
	{
		SetSrcDisplay(m_pVidDesc->uiMonitorID);
		SetSrcResolution();
		SetDestResolution(m_pVidDesc->uiWidthDest, m_pVidDesc->uiHeightDest);

		SinkWriter().SetAudio(m_pVidDesc->bIsAudio);
		
		//D3D11().SetCpyMethod(m_pVidDesc->myCpyMethod);
		D3D11().SetPickedMonitor(m_uiPickedMonitor);
		D3D11().CreateD3D11Texture(); // new Texture may the Resulotion/Monitor changed
		D3D11().CreateShaderView(); // need new bind to the new created Texture
		D3D11().ReSizeD3D11Window(); // may the Kind of Presentation changed (CopyMethod)
		
		//WinGDI().SetCpyMethod(m_pVidDesc->myCpyMethod);
		WinGDI().SetSrcWndTitle(m_pVidDesc->strWndTitle);
		WinGDI().FindSetWindow(); // may the Window changed

		SinkWriter().SetFPS(m_pVidDesc->uiFPS);
		SinkWriter().SetFormats(m_pVidDesc->myInputFormat, m_pVidDesc->myOutputFormat);
		SinkWriter().SetBitReading(m_pVidDesc->myBitReading);
		SinkWriter().SetBitRate();
		SinkWriter().SetFileName(m_pVidDesc->strFileName);

		SyncFPS().SetFrameDuaration(SinkWriter().GetVideoFrameDuration());
		
		SinkWriter().PrepareInputOutput();
		SinkWriter().StartRecording();
		SyncFPS().Start();
		SetRecordingStatus(true);
	}
}//END-FUNC
/// <summary>
/// True: the recording starts
/// </summary>
/// <param name="bRecord">TRUE: recording</param>
void ClsD3D11Recording::SetRecordingStatus(BOOL bRecord)
{
	m_bIsRecording = bRecord;
}//END-FUNC
void ClsD3D11Recording::SetWndAsSrc()
{
	SetSrcDisplay(m_pVidDesc->uiMonitorID);
	SetSrcResolution();
	SetDestResolution(m_pVidDesc->uiWidthDest, m_pVidDesc->uiHeightDest);
	SinkWriter().SetBitRate();

	D3D11().SetPickedMonitor(m_uiPickedMonitor);
	D3D11().CreateD3D11Texture(); // create D3d11texture with native Monitor resolution
	D3D11().CreateShaderView();
	
	WinGDI().SetWndAsSrc(TRUE);
	WinGDI().FindSetWindow();
	D3D11().ReSizeD3D11Window(); // WindowSize may changed, update the D3D11Viewport
}
/// <summary>
/// Swaps the Source Monitor of Recording. Effects only when DesktopDupl will be used.
/// May the Resolution will change. Thats why we have to init. the D3D11Texture again.
/// Besides that we have to bind the D3D11Texture to the ShaderView again.
/// The initialization of the CPUAccess-D3D11Texture in PreparePresentation must also be recreated.
/// 
/// The check of the right parameters for the Sinkwriter will be checked every time when recording will start.
/// </summary>
/// <param name="uiMonitorID">Monitor ID that we want to use</param>
void ClsD3D11Recording::SetMonitorAsSrc(UINT uiMonitorID)
{
		SetSrcDisplay(uiMonitorID);
		SetSrcResolution();
		SetDestResolution(m_pVidDesc->uiWidthDest, m_pVidDesc->uiHeightDest);
		SinkWriter().SetBitRate();
		
		D3D11().SetPickedMonitor(m_uiPickedMonitor);
		D3D11().CreateD3D11Texture(); // create D3d11texture with native Monitor resolution
		D3D11().CreateShaderView();
		
		WinGDI().SetWndAsSrc(FALSE);
		WinGDI().FindSetWindow();
		D3D11().ReSizeD3D11Window(); // WindowSize may changed, update the D3D11Viewport
}//END-FUNC
/// <summary>
/// This have to be called when the Size of the Window changed.
/// - Window: Window of this App, where the D3D11 will Render our stuff.
/// - E.g. DesktopDupl or other App-Window
/// </summary>
void ClsD3D11Recording::ResizeWindow()
{
	D3D11().ReSizeD3D11Window();
}
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
void ClsD3D11Recording::SetCpyMethod(CopyMethod* myCpyMethod)
{
	m_pFrameData->pCpyMethod = myCpyMethod;
	//WinGDI().SetCpyMethod(m_myCpyMethod);
	//D3D11().SetCpyMethod(m_myCpyMethod);
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
	if (*m_pFrameData->pCpyMethod == CopyMethod::DesktopDupl)
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
	if (m_pFrameData->pData)
		delete m_pFrameData->pData;
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
	m_pVidDesc->uiMonitorID = m_uiPickedMonitor;
	D3D11().SetPickedMonitor(m_uiPickedMonitor);
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
	if (*m_pFrameData->pCpyMethod == CopyMethod::DesktopDupl)
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