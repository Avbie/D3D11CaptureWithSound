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
	m_hWnd = NULL;
	m_iXPos = 0;
	m_iYPos = 0;
	m_iTop = 0;
	m_iLeft = 0;
	m_uiPickedMonitorIndex = 0;
	m_iPickedResIndex = DEFRES;
	m_fZoomPercentage = 100.0f;
	m_myBitReading = PicDataBitReading::Standard;
	//m_myClientRect = {};
	m_myWndRect = {};
	m_vMonitors.clear();
	m_pFrameData = new FrameData();
	m_pFrameData->pCpyMethod = &m_pVidDesc->myCpyMethod;
	m_pFrameData->uiFPS = m_pVidDesc->uiFPS;
	m_pFrameData->pIsAudio = &m_pVidDesc->bIsAudio;

	SinkWriter().SetFrameData(&m_pFrameData);
	D3D11().SetFrameData(&m_pFrameData);

	ClsMonitor myMonitor(&m_vMonitors, &m_uiMaxMonitors);
	
	if (!SetSrcDisplay(m_pVidDesc->uiMonitorID))
		return;

	SetDestResFromList();
	
	if (!ReCreateFrameBuffer())
		return;
	SetWindowRect();
	SetWindowPosition();
	
	SinkWriter().CalcDurationVid();
	SinkWriter().PrepareAudio();
	WinGDI().SetFrameData(&m_pFrameData);
	HR2(WinGDI().CreateWindowList());
	
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
	}//END-IF
	if (m_pFrameData)
	{
		delete m_pFrameData;
		m_pFrameData = nullptr;
	}//END-IF
	for (auto Monitor : m_vMonitors)
	{
		if (Monitor)
		{
			delete Monitor;
			Monitor = nullptr;
		}//END-IF
	}//END-FOR
}//END-DESTR
/*************************************************************************************************
**************************************GET-METHODS*************************************************
*************************************************************************************************/

/// <summary>
/// Gets the handle of the current active Window that is set as source.
/// </summary>
/// <returns>Window handle</returns>
HWND ClsD3D11Recording::GetActiveWindow()
{
	return WinGDI().WndHandle().GetWndHandle();
}//END-FUNC
/// <summary>
/// Gets a Array of valid resolutions of all installed monitors.
/// Array is filled in the Constructor ClsD3D11Recording::ClsD3D11Recording by calling ClsMonitor
/// </summary>
/// <returns>List with valid resolutions</returns>
const std::vector<SupportedResolution>& ClsD3D11Recording::GetAllSupportedRes()
{
	return ClsSupportedRes::GetAllSupportedRes();
}//END-FUNC
/// <summary>
/// Gets the FPS that are currently set for recording
/// </summary>
/// <returns>FPS</returns>
const UINT ClsD3D11Recording::GetFPS()
{
	return m_pFrameData->uiFPS;
}//END-FUNC
/// <summary>
/// Gets an Array of installed Monitors
/// </summary>
/// <returns>Installed Monitors</returns>
const std::vector<MonitorInfo*>& ClsD3D11Recording::GetMonitors()
{
	return m_vMonitors;
}//END-FUNC
/// <summary>
/// Gets the monitor that is currently selected for recording.
/// </summary>
/// <returns>Picked Monitor as ID</returns>
const UINT ClsD3D11Recording::GetSelectedIndexOfMonitorList()
{
	return m_uiPickedMonitorIndex;
}
/// <summary>
/// Gets the monitor that is currently selected for recording.
/// </summary>
/// <returns>Picked Monitor as ID</returns>
const INT64 ClsD3D11Recording::GetSelectedIndexOfResList()
{
	return m_iPickedResIndex;
}//END-FUNC
/// <summary>
/// Gets the current List of window that are visible and open
/// </summary>
/// <returns>List of Windows</returns>
const vector<GDI::ActiveWnd>& ClsD3D11Recording::GetWindowList()
{
	return WinGDI().GetWindowList();
}//END-FUNC
/// <summary>
/// Gets the ZoomFactor that is currently set for the PreView
/// </summary>
/// <returns>Zoom in percentage</returns>
const FLOAT ClsD3D11Recording::GetZoomFactor()
{
	return m_fZoomPercentage;
}//END-IF
/// <summary>
/// Gets the Status of if the audio is set or not
/// </summary>
/// <returns>true, audio is active</returns>
BOOL ClsD3D11Recording::IsAudio()
{
	return *m_pFrameData->pIsAudio;
}//END-FUNC
/*************************************************************************************************
**************************************SET-METHODS*************************************************
*************************************************************************************************/
/// <summary>
/// Enables or disables the audio
/// CalledBy: User
/// </summary>
/// <param name="bAudio">TRUE: Audio enabled</param>
void ClsD3D11Recording::SetAudio(BOOL bAudio)
{
	*m_pFrameData->pIsAudio = bAudio;
}//END-FUNC
/// <summary>
/// Sets the FPS limit
/// CalledBy: User
/// </summary>
/// <param name="uiFPS">Number of FPS</param>
void ClsD3D11Recording::SetFPS(const UINT uiFPS)
{
	m_pFrameData->uiFPS = uiFPS;
	SinkWriter().CalcDurationVid();
}//END-FUNC
/// <summary>
/// Sets a specific window as source for capturing
/// CalledBy: User
/// </summary>
/// <param name="uiWndNr">Select window from list</param>
/// <returns>TRUE: Window selected, 
///			 FALSE: no window selected 
///			 FALSE: window selected but something went wrong + ErrMsg
/// </returns>
BOOL ClsD3D11Recording::SetActiveWindow(const UINT uiWndNr)
{
	SetCpyMethod(CopyMethod::Mapping);						// Mapping method is the way for capturing a specific window
	if (WinGDI().SetActiveWindow(uiWndNr))						// select a window from the List
	{
		// May befor CopyMethod DesktopDupl was selected that not allow custom resolution
		//RefreshDestResolution();// (m_pVidDesc->uiWidthDest, m_pVidDesc->uiHeightDest);
		//ReCreateFrameBuffer();
		//SinkWriter().SetBitRate();
		HR(D3D11().CreateD3D11Texture());							// Recreate, maybe Access Rights changed: Mapping <-> DesktopDupl
		HR(D3D11().CreateShaderView());
		HR(WinGDI().FindSetWindow());
		HR(AdjustRatio());
		return TRUE;
	}
	return FALSE;
}//END-FUNC
/// <summary>
/// Sets the Resolution depending on Videodescriptors Parameters. Videodescriptor set by user.
/// If the CopyMethod is the DesktopDupl the DestRes is set to the SourceRes.
/// CalledBy: User and Constructor. Constructor gets Resolution by User per Videodescriptor
/// </summary>
/// <param name="uiWidthDest">Width</param>
/// <param name="uiHeightDest">Height</param>
/// <returns>TRUE:	Resolution selected
///			FALSE:  No resolution selected, 
///			FALSE:  Resolution selected but somethinf went wrong
/// </returns>
BOOL ClsD3D11Recording::SetDestResFromList(int iResIndexNr)
{
	BOOL bSelected = FALSE;

	bSelected=RefreshDestResolution(iResIndexNr);
	m_iPickedResIndex = ClsSupportedRes::GetIndexOfSelectedRes(m_pFrameData->uiWidthDest, m_pFrameData->uiHeightDest);
	if (D3D11().IsDevice())
	{
		if (!ReCreateFrameBuffer())
			return FALSE;
		SinkWriter().SetBitRate();								// new DestRes, update the bitrate per frame
		HR(D3D11().CreateD3D11Texture());							// Recreates new Texture with DestRes
		HR(D3D11().CreateShaderView());								// have to rebind the new Texture to the Shaderview
		HR(WinGDI().FindSetWindow());								// Set DestBitmapSize with new Resolution
		HR(AdjustRatio());											// Adjust the ratio for the ViewPort
	}//END-IF valid D3DDevice
	
	return bSelected;
}//END-FUNC
/// <summary>
/// Swaps the Source Monitor for Capturing. Effected only when DesktopDupl will be used.
/// May the Resolution will change because with CopyMethod-DesktopDupl the DestRes must be the same as the SrcRes.
/// Thats why we have to init. the D3D11Texture again.
/// Besides that we have to bind the D3D11Texture to the ShaderView again.
/// The initialization of the CPUAccess-D3D11Texture in PreparePresentation must also be recreated.
/// The check of the right parameters for the Sinkwriter will be checked every time when recording will start.
/// CalledBy: User
/// </summary>
/// <param name="uiMonitorID">Monitor ID that we want to use</param>
/// <returns>TRUE:	Monitor selected
///			FALSE:  No Monitor selected, FALSE: 
///					No Monitor selected and ErrorMsg
/// </returns>
BOOL ClsD3D11Recording::SetMonitor(const UINT uiMonitorID)
{
	SetCpyMethod(CopyMethod::DesktopDupl);					// Take the DesktopDupl when we use the whole monitor as a source
	if (SetSrcDisplay(uiMonitorID))								// identify the Monitor
	{
		// Make the DestResolution equal to the SrcResolution, no scaling possible with DesktopDupl
		MakeEqualResolution();									// Set the DestRes to the SrcRes
		if (!ReCreateFrameBuffer())
			return FALSE;
		SinkWriter().SetBitRate();								// new DestRes, update the bitrate per frame

		HR(D3D11().CreateD3D11Texture());							// Creates D3D11Texture with new DestRes
		HR(D3D11().CreateShaderView());								// have to rebind the new Texture to the Shaderview
		//WinGDI().FindSetWindow();
		HR(AdjustRatio());											// Adjust the ratio for the ViewPort
		return TRUE;
	}
	return FALSE;
}//END-FUNC
/// <summary>
/// Zoom in or out, its depends on the Parameter
/// CalledBy: User
/// </summary>
/// <param name="uiPercentage">Percentage of the zoom</param>
/// <returns>TRUE:	Zoom selected
/// </returns>
BOOL ClsD3D11Recording::SetZoomFactor(const float fPercentage)
{
	m_fZoomPercentage = fPercentage;
	HR(D3D11().SetZoomPercentage(fPercentage));

	return TRUE;
}//END-FUNC
/*************************************************************************************************
**************************************RECORDING-METHODS*******************************************
*************************************************************************************************/
/// <summary>
/// This Method have to be called in the FPS-loop
/// Capturing 1 Frame per 1 LoopPassage
/// <returns>TRUE: no Error
///			 FALSE: something went wrong
/// </returns>
/// </summary>
BOOL ClsD3D11Recording::Loop()
{
	++SyncFPS();
	if (!IsDesktopDupl())
		HR(WinGDI().GetBitBltDataFromWindow());
	HR(SinkWriter().StartReadAudioHWBufferThread());
	HR(D3D11().BitBltDataToRT());
	//WinGDI().TakeScreenshot();
	HR(SinkWriter().LoopRecording());
	HR(D3D11().PresentTexture());
	SyncFPS().SleepUntilNextFrame();

	return TRUE;
}//END-FUNC
/// <summary>
/// Preparing the Sinkwriter for Recording a Video with/without Audio
/// - Set Input and Putput Format
/// - Sets the Status of the IMFSinkwriter to ready for capturing
/// - synchronize the FPS - set TimeStamps 
/// </summary>
/// /// <returns>TRUE: Recording started, 
///			 FALSE: Recording did not start
///			 FALSE: Recording started but something went wrong
/// </returns>
BOOL ClsD3D11Recording::StartRecording()
{
	if (!IsRecording())
	{
		SinkWriter().CalcDurationVid();
		SinkWriter().SetFormats(m_pVidDesc->myInputFormat, m_pVidDesc->myOutputFormat);
		SinkWriter().SetBitReading(m_pVidDesc->myBitReading);
		SinkWriter().SetBitRate();
		SinkWriter().SetFileName(m_pVidDesc->strFileName);
		SyncFPS().SetFrameDuaration(SinkWriter().GetVideoFrameDuration());
		HR(SinkWriter().PrepareInputOutput());
		HR(SinkWriter().StartRecording());
		SyncFPS().Start();
		SetRecordingStatus(true);
	}//END-IF recording
	return TRUE;
}//END-FUNC
/// <summary>
/// Tells the Sinkwriter to stop recording Frames
/// </summary>
/// /// <returns>TRUE: recording stopped, 
///			 FALSE: recording did not stopped 
/// </returns>
BOOL ClsD3D11Recording::StopRecording()
{
	if (IsRecording())
	{
		HR(SinkWriter().StopRecording());
		SetRecordingStatus(false);
	}//END-IF
	return TRUE;
}//END-FUNC
/*************************************************************************************************
**************************************INITIALIZATION**********************************************
*************************************************************************************************/
/// <summary>
/// Creates a List of Window that are visible and open
/// CalledBy: Constructor
/// </summary>
/// <returns>TRUE: WindowList created, 
///			 FALSE: WindowList was not created
/// </returns>
BOOL ClsD3D11Recording::CreateWindowList()
{
	HR(WinGDI().CreateWindowList());
	return TRUE;
}//END-FUNC
/// <summary>
/// Init. the D3D11-Window
/// CalledBy: Constructor
/// <returns>TRUE: init. succeeded, 
///			 FALSE: something went wrong + ErrMsg
/// </returns>
/// </summary>
BOOL ClsD3D11Recording::Init3DWindow()
{
	HR(D3D11().CreateDevice());
	HR(D3D11().CreateSwapChain());
	HR(D3D11().CreateSwapChainBuffer());
	HR(D3D11().CreateBuffers());
	HR(D3D11().CreateAndSetSampler());
	HR(D3D11().CreateD3D11Texture());
	HR(D3D11().CreateShaderView());
	HR(D3D11().CreateAndSetShader());
	HR(D3D11().CreateInputLayout());
	HR(D3D11().SetConstantBuffer());
	HR(WinGDI().FindSetWindow());
	HR(AdjustRatio());

	return TRUE;
}//END-FUNC
/// <summary>
/// Adjust the Viewport withhin the Window
/// </summary>
/// <returns>TRUE: Adjustment succeeded, 
///			 FALSE: something went wrong + ErrMsg
/// </returns>
HRESULT ClsD3D11Recording::AdjustRatio()
{
	HRESULT hr = NULL;
	HR_RETURN_ON_ERR(hr, D3D11().AdjustD3D11Ratio());

	return TRUE;
}//END-FUNC
/*************************************************************************************************
**************************************PRIVATE-SUB-OBJECTS*****************************************
*************************************************************************************************/
/*Returning the SubObjects that will be used in this Superclass*/
D3D::ClsD3D11& ClsD3D11Recording::D3D11()
{
	return m_myClsD3D11;
}//END-FUNC
ClsSinkWriter& ClsD3D11Recording::SinkWriter()
{
	return m_myClsSinkWriter;
}//END-FUNC
ClsFPSSync& ClsD3D11Recording::SyncFPS()
{
	return m_myClsSyncFPS;
}//END-FUNC
GDI::ClsWinGDI& ClsD3D11Recording::WinGDI()
{
	return m_myClsWinGDI;
}//END-FUNC
/*************************************************************************************************
**************************************PRIVATE-METHODES********************************************
*************************************************************************************************/
/// <summary>
/// Sets the destination resolution to the source resolution
/// CalledBy: SetMonitor(...),SetDestResolution(...)
/// </summary>
void ClsD3D11Recording::MakeEqualResolution()
{
	m_pFrameData->uiWidthDest = m_pFrameData->uiWidthSrc;
	m_pFrameData->uiHeightDest = m_pFrameData->uiHeightSrc;
}//END-FUNC
/// <summary>
/// Sets the CopyMethod
/// CalledBy: SetActiveWindow(...) SetMonitor(...)
/// </summary>
/// <param name="myCpyMethod">Enum of the CopyMethod</param>
void ClsD3D11Recording::SetCpyMethod(CopyMethod myCpyMethod)
{
	*m_pFrameData->pCpyMethod = myCpyMethod;
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
/// Enables the Recording itself.
/// Requested in MainLoop
/// CalledBy: User
/// </summary>
/// <param name="bRecord">TRUE: recording enabled</param>
void ClsD3D11Recording::SetRecordingStatus(BOOL bRecord)
{
	m_pFrameData->bIsRecording = bRecord;
}//END-FUNC
/// <summary>
/// - Set the position of the Window of this App, depending on the Monitor Resolution
/// CalledBy: Constructor
/// </summary>
void ClsD3D11Recording::SetWindowPosition()
{
	m_iXPos = m_pFrameData->uiWidthDest / 2 - m_myWndRect.right / 2;
	m_iYPos = m_pFrameData->uiHeightDest / 2 - m_myWndRect.bottom / 2;
}//END-FUNC
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
	AdjustWindowRect(&m_myWndRect, WINSTYLE, TRUE);
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
/// Gets the current Status if its recording or not
/// </summary>
/// <returns></returns>
BOOL ClsD3D11Recording::IsRecording()
{
	return m_pFrameData->bIsRecording;
}//END-FUNC
/// <summary>
/// Recreates the size of the Framebuffer, must be called when resolution changed.
/// CalledBy: Constructor(), SetMonitor(...), SetActiveWindow(...)
/// </summary>
BOOL ClsD3D11Recording::ReCreateFrameBuffer()
{
	m_pFrameData->uiPixelDataSize = m_pFrameData->uiWidthDest * m_pFrameData->uiHeightDest * m_pFrameData->uiBpp;
	if (m_pFrameData->pData)
		delete m_pFrameData->pData;
	// DatenBuffer für PixelDaten; GetBits = Width*Height*Bpp
	m_pFrameData->pData = new unsigned char[m_pFrameData->uiPixelDataSize];

	memset(m_pFrameData->pData, 0, m_pFrameData->uiPixelDataSize);

	if (!m_pFrameData->pData)
		return FALSE;
	return TRUE;
}//END-FUNC
/// <summary>
/// Sets the DestRes from a List or Resolutions or set it to the SrcRes.
/// CalledBy: SetDestResFromList
/// </summary>
/// <param name="myCpyMethod">CopyMethod</param>
BOOL ClsD3D11Recording::RefreshDestResolution(int iResIndexNr)
{
	BOOL bSelected = FALSE;
	DWORD dwHeight = 0;
	DWORD dwWidth = 0;

	if (*m_pFrameData->pCpyMethod == CopyMethod::DesktopDupl)
	{
		MakeEqualResolution();
		return bSelected;
	}//END-IF copy method

	bSelected = ClsSupportedRes::SelectDestResolution(iResIndexNr, &dwWidth, &dwHeight);

	m_pFrameData->uiWidthDest = dwWidth;
	m_pFrameData->uiHeightDest = dwHeight;

	return bSelected;
}//END-FUNC
/// <summary>
/// - Sets the Monitor that we want to capture
///   - used by DesktopDupl
/// - Enumation of Monitors still executed
/// - Will use the MainMonitor if the input is invalid
/// CalledBy: Constructor, SetWndAsSrc(), SetMonAsSrc(), PrepareRecording()
/// </summary>
/// <param name="uiMonitorID">selected Monitor</param>
/// <returns>TRUE: SrcRes selected, 
///			 FALSE: no SrcRes selected
/// </returns>
BOOL ClsD3D11Recording::SetSrcDisplay(const UINT uiMonitorID)
{
	if (uiMonitorID > m_uiMaxMonitors)
	{
		m_uiPickedMonitorIndex = 0;
		m_pVidDesc->uiMonitorID = m_uiPickedMonitorIndex;
	}//END-IF
	else
	{
		m_uiPickedMonitorIndex = uiMonitorID;
		m_pVidDesc->uiMonitorID = uiMonitorID;
	}//END-ELSE
	m_pFrameData->pPickedMonitor = &m_uiPickedMonitorIndex;

	return SetSrcResolution();
}//END-FUNC
/// <summary>
/// - Sets the Source Resolution depending on the picked Monitor
/// CalledBy: Constructor
/// </summary>
/// <returns>TRUE: SrcRes selected, 
///			 FALSE: no SrcRes selected
/// </returns>
BOOL ClsD3D11Recording::SetSrcResolution()
{
	m_pFrameData->uiWidthSrc = m_vMonitors[m_uiPickedMonitorIndex]->GetWidth();
	m_pFrameData->uiHeightSrc = m_vMonitors[m_uiPickedMonitorIndex]->GetHeight();
	m_pFrameData->uiTop = m_vMonitors[m_uiPickedMonitorIndex]->GetTop();
	m_pFrameData->uiLeft = m_vMonitors[m_uiPickedMonitorIndex]->GetLeft();
	m_pFrameData->uiBpp = BITDEPTH;

	if (m_pFrameData->uiWidthSrc <= 0 || m_pFrameData->uiHeightSrc <= 0)
		return FALSE;

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