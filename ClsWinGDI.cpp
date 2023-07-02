#include "ClsWinGDI.h"

using namespace std;

namespace GDI
{
	/// <summary>
	/// Constructor
	/// </summary>
	ClsWinGDI::ClsWinGDI()
	{
		//m_bIsWnd = FALSE;
		m_hFileData = NULL;
		m_hMemDC = NULL;
		m_hDisplayDC = NULL;
		m_hBitmap = NULL;
		m_uiWindowFlag = 0;
		//m_myCpyMethod = DEFCPYMETHOD;
		m_strTitle = NULL;
		m_strBmpFileName = NULL;
		m_pFrameData = NULL;
	}
	/// <summary>
	/// Destructor
	/// </summary>
	ClsWinGDI::~ClsWinGDI()
	{
		ClearObjects();
	}//END-CONS
	/**********Get-Set-Methoden*******/
	ClsScreenshot& ClsWinGDI::ScreenShot()
	{
		return m_myClsScreenShot;
	}//END-FUNC
	/*void ClsWinGDI::SetCpyMethod(CopyMethod& myCpyMethod)
	{
		m_myCpyMethod = myCpyMethod;
	}//END-FUNC*/
	void ClsWinGDI::SetFrameData(FrameData** ppFrameData)
	{
		m_pFrameData = *ppFrameData;
		ScreenShot().SetFrameData(ppFrameData);
		//m_pFrameData->pClsWndHandle = &m_MyClsWndHandle;
	}//END-FUNC
	void ClsWinGDI::SetSrcWndTitle(const char* strTitle)
	{
		m_strTitle = strTitle;
	}//END-FUNC
	void ClsWinGDI::SetWndAsSrc(BOOL bWnd)
	{
		//m_bIsWnd = bWnd;
		m_pFrameData->bWndAsSrc = bWnd;
	}
	/*********************************/
	/// <summary>
	/// Sets the Window-handle depending on the Window-Title
	/// It is the Window that we want to record
	/// Will be ignored if CpyMethod::DesktopDupl
	/// CalledBy: Superclass ClsD3D11Recording::Init3DWindow()
	/// </summary>
	/// <returns></returns>
	HRESULT ClsWinGDI::FindSetWindow()
	{
		HRESULT hr = NULL;
		RECT myClientRectSrcWnd = {};							// RECT for ResolutionInfo

		ClearObjects();

		if (*m_pFrameData->pCpyMethod != CopyMethod::DesktopDupl)
			m_uiWindowFlag = m_uiWindowFlag | 0b0001;

		// DesktopDupl
		if (m_uiWindowFlag == 0)
		{
			m_MyClsWndHandle.SetWndTitle(DESKTOP);			// Title of the Window
			return S_OK;
		}
		if (IsWnd())
		{
			m_uiWindowFlag = m_uiWindowFlag | 0b0010;		// Bitflag for Window
		}//END-iF
		if (IsWndTitle())
		{
			m_uiWindowFlag = m_uiWindowFlag | 0b0100;		// Bitflag for WindowTitle
		}//END-IF
		/*
		* Will enter the if case with
		* 1 = No Wnd, No Title
		* 3 = Wnd, No Title
		* 5 = No Wnd, Title
		*/
		if (m_uiWindowFlag < WNDTITLESET)
		{
			m_MyClsWndHandle.SetWndTitle(DESKTOP);			// DESKTOP
			m_hDisplayDC = GetDC(NULL);						// DC of the Window, whole Desktop
			HR_RETURN_ON_NULL_ERR(m_hDisplayDC);
			HR_RETURN_ON_ERR(hr, AllocateMemDC());

			return S_OK;
		}//END-IF
		if (m_uiWindowFlag == WNDTITLESET)					// Looking for Window with set Title
		{
			m_MyClsWndHandle.SetWndTitle(m_strTitle);		// Titel des Fensters
			// Find the WndHandle that has m_strTitle
			// EnumWindowsProc will return FALSE to EnumWindows if the title was found
			// If EnumWindowsProc returns FALSE EnumWindows will leave the Loop and will return false
			if (!EnumWindows(
				EnumWindowsProc,
				reinterpret_cast<LPARAM>(&m_MyClsWndHandle)))
				m_uiWindowFlag = m_uiWindowFlag | 0b1000;	// Bitflag for Window found
		}//END-IF WNDTITLESET
		if (m_uiWindowFlag == WNDFOUND)
		{
			m_hDisplayDC = GetDC(m_MyClsWndHandle.GetHandle());	// DC of the WndHandle
			HR_RETURN_ON_NULL_ERR(m_hDisplayDC);
			HR_RETURN_ON_ERR(hr, AllocateMemDC());			// compatible MemDC with a HBitmap that has the specific destination size

			HR_RETURN_ON_INT_ERR(GetClientRect(
				m_MyClsWndHandle.GetHandle(), &myClientRectSrcWnd));	// keep the size of the window
			m_pFrameData->uiWidthSrc = myClientRectSrcWnd.right - myClientRectSrcWnd.left;
			m_pFrameData->uiHeightSrc = myClientRectSrcWnd.bottom - myClientRectSrcWnd.top;
			m_pFrameData->uiTop = 0;
			m_pFrameData->uiLeft = 0;
		}//END-IF Wnd-Handle
		else 
		{
			m_MyClsWndHandle.SetWndTitle(DESKTOP);			// title of the window
			m_hDisplayDC = GetDC(NULL);						// DC of the Window, whole Desktop
			HR_RETURN_ON_NULL_ERR(m_hDisplayDC);
			HR_RETURN_ON_ERR(hr, AllocateMemDC());

			return S_OK;
		}//END-ELSE no valid WndHandle
		return hr;
	}//END-FUNC
	/// <summary>
	/// Ermittelt Pixeldaten aus einem MemDC, bzw. aus dessen UnterObjekt HBitmap
	/// CalledBy: SuperClass ClsD3D11Recording::Recording()
	/// </summary>
	/// <returns>HRESULT</returns>
	HRESULT ClsWinGDI::GetBitBltDataFromWindow()
	{
		HRESULT hr = NULL;
		UINT& uiPixelDataSize = m_pFrameData->uiPixelDataSize;
		if (m_uiWindowFlag == 0)						// DesktopDupl
			return S_OK;
		if (m_uiWindowFlag >= NODESKDUPL)				// GDI Mapping: DesktopCpy or WndCpy
		{
			//HANDLE hImgData = NULL;
			BYTE* pImgData = NULL;

			HR_RETURN_ON_ERR(hr, CopyBitmapDataToMemDC());
			pImgData = (BYTE*)malloc(m_pFrameData->uiPixelDataSize);
			if (!pImgData)
			{
				printf("pImgData failed: last error is %u\n", GetLastError());
				return E_FAIL;
			}
			// Pixeldaten aus hBitmap auslesen
			HR_RETURN_ON_NULL_ERR(GetBitmapBits(
				m_hBitmap, uiPixelDataSize, pImgData));
			// Pixeldaten in buffer kopieren
			memcpy_s(m_pFrameData->pData, uiPixelDataSize, pImgData, uiPixelDataSize);
			free(pImgData);
		}
		return hr;
	}//END-FUNC
	/// <summary>
	/// Erstellt einen Screenshot aus den aktuellen PixelDaten
	/// </summary>
	/// <returns>HRESULT</returns>
	HRESULT ClsWinGDI::TakeScreenshot()
	{
		HRESULT hr = NULL;
		HR_RETURN_ON_ERR(hr, m_myClsScreenShot.BitBltToFile(m_hMemDC, m_hBitmap, m_uiWindowFlag));

		return hr;
	}//END-FUNC
/*************************************************************************************************
**************************************PRIVATE-METHODES********************************************
*************************************************************************************************/
	
	/// <summary>
	/// Clear all objects and freeing all memory.
	/// CalledBy: Destructor and SetFreeWindow()
	///  - SetFreeWindow freeing all objects right before reuse it.
	/// </summary>
	void ClsWinGDI::ClearObjects()
	{
		DeleteObject(m_hMemDC);
		DeleteObject(m_hBitmap);
		ReleaseDC(m_MyClsWndHandle.GetHandle(), m_hDisplayDC);
		ReleaseDC(NULL, m_hDisplayDC);
		m_uiWindowFlag = 0;
	}
	/// <summary>
	/// - Callback Function used in FindSetWindow() in EnumWindowProc 
	///   for specifing the right Source Window depending on the Title
	/// - This function will be called in a loop in EnumWindowProc, for every Window that is currently open
	/// - If the Title is the same as requested, we will keep the current WindowHandle
	/// </summary>
	/// <param name="hWnd">WindowHandle of one open Window</param>
	/// <param name="lParam">"zipped" ClsWndHandle Object that will keep the right WindowHandle</param>
	/// <returns>FALSE: breaks the loop in EnumWindowProc, window found</returns>
	/// <returns>TRUE: continue loop, no specific window found</returns>
	BOOL CALLBACK ClsWinGDI::EnumWindowsProc(HWND hWnd, LPARAM lParam) 
	{
		TCHAR wstrCurTitle[1024]{};
		wstring wsCurTitle;
		string sCurTitle;
		string sTitleLookingFor;
		
		//USES_CONVERSION;
		ClsWndHandle* pClsWndHandle = reinterpret_cast<ClsWndHandle*>(lParam);
		//GetClassName(hWnd, buf, 100);
		GetWindowText(hWnd, wstrCurTitle, 1024);
		wsCurTitle.assign(wstrCurTitle);
		sCurTitle.assign(wsCurTitle.begin(), wsCurTitle.end());

		sTitleLookingFor.assign(pClsWndHandle->GetWndTitle());

		if (sCurTitle.find(sTitleLookingFor) != string::npos)
		{
			pClsWndHandle->SetHandle(hWnd);
			return FALSE;												// Title found, leave loop in EnumWindowProc
		}//END-IF
		return TRUE;
	}//END-FUNC
	/// <summary>
	/// Prüft ob Quelle und Target selbe Auflösung haben
	/// CalledBy: CopyBitmapDataToMemDC
	/// </summary>
	/// <returns>True wenn Scaled bzw. Auflösung unterschiedlich</returns>
	BOOL ClsWinGDI::IsScaled()
	{
		if (m_pFrameData->uiHeightDest != m_pFrameData->uiHeightSrc)
			return true;
		if (m_pFrameData->uiWidthDest != m_pFrameData->uiWidthSrc)
			return true;
		return false;
	}//END-FUNC
	BOOL ClsWinGDI::IsWnd()
	{
		return m_pFrameData->bWndAsSrc;
	}
	/// <summary>
	/// Checks if the Title is set
	/// CalledBy: FindSetWindow()
	/// </summary>
	/// <returns>True if Title is set</returns>
	BOOL ClsWinGDI::IsWndTitle()
	{
		if (m_strTitle == NULL || strcmp(m_strTitle, "\0") == 0)
			return false;
		else
			return true;
	}//END-FUNC
	/// <summary>
	/// Allocate Memory für ein MemoryDC, allocate a hBitmap
	/// Erneuter Aufruf wird nötig falls die Bedingungen sich ändern: Anderes Fenster, andere Größe, andere ZielAuflösung
	/// Erstellt ein neues und leeres Bitmap mit passender Größe
	/// Bindet mit SelectObject das m_hBitmap an m_hMemDC
	/// CalledBy: FindSetWindow
	/// </summary>
	/// <returns>HRESULT</returns>
	HRESULT ClsWinGDI::AllocateMemDC()
	{
		HRESULT hr = NULL;

		m_hMemDC = CreateCompatibleDC(m_hDisplayDC);		// MemoryDC aus DisplayDC erstellen und Handle darauf zurückbekommen
		m_hBitmap = CreateCompatibleBitmap(					// Hier werden nur Metadaten gesetzt, keine Pixeldaten kopiert
			m_hDisplayDC,									// Memory Bitmap erstellen, aus Bitmap  des DisplayDC. 
			m_pFrameData->uiWidthDest,						// aber mit größerer Auflösung
			m_pFrameData->uiHeightDest
		);

		HR_RETURN_ON_NULL_ERR(m_hMemDC);
		HR_RETURN_ON_NULL_ERR(m_hBitmap);
		SelectObject(m_hMemDC, m_hBitmap);					// Ersetzen des 1x1Px Platzhalters im MemDC durch mein Bitmap

		return hr;
	}//END-FUNC
	/// <summary>
	/// Aufgerufen wenn GDI-Mapping:
	///		- Window gefunden (SrcRes: Wnd-Resolution)
	///		- Window nicht gefunden (SrcRes: Desktop-Resolution)
	/// kopiert UnterObjekt (Bitmap) von m_hDisplayDC in das Unterobjekt (Bitmap) von m_hMemDc
	/// m_hBitmap ist an m_hMemDC gebunden
	/// CalledBy: GetBitBltDataFromWindow
	/// </summary>
	/// <returns>HRESULT</returns>
	HRESULT ClsWinGDI::CopyBitmapDataToMemDC()
	{
		HRESULT hr = NULL;
		BOOL bReturn = FALSE;

		if (IsScaled())										// SrcRes have to be scale to the dest. resolution
		{
			//HR_RETURN_ON_INT_ERR(StretchBlt(				// Kopieren der BitmapDaten die als Bitmap-Unterobj in den DCs existieren
			HR_RETURN_ON_INT_ERR(StretchBlt(				// Kopieren der BitmapDaten die als Bitmap-Unterobj in den DCs existieren
				m_hMemDC,									// Ziel: MemoryDC
				0,											// X (0,0 ist oben links)
				0,											// Y (0,0 ist oben links) Wenn 500, ersten 500 pixel von oben aus wären dann schwarz
				m_pFrameData->uiWidthDest,					// Breite ab X
				m_pFrameData->uiHeightDest,					// Höhe ab Y
				m_hDisplayDC,								// Source hDisplayDC, also der DisplayDeviceContext
				m_pFrameData->uiLeft,						// X (0,0 ist oben links)
				m_pFrameData->uiTop,						// Y (0,0 ist oben links)
				m_pFrameData->uiWidthSrc,					// Breite ab X
				m_pFrameData->uiHeightSrc,					// Höhe ab Y
				SRCCOPY
			));												// Flags: SRCOPY: kopiert Viereck der Quelle ins Viereck des Ziels
															// -> habe nun fertiges MemoryDC mit fertigem, gefülltem Bitmap 
															//ReleaseDC(MyClsWndHandle.GetHandle(), m_hDisplayDC); 
		}
		else
		{
			HR_RETURN_ON_INT_ERR(BitBlt(
				m_hMemDC,									// Ziel: MemoryDC
				0,											// Dest: X (0,0 ist oben links)
				0,											// Dest: Y (0,0 ist oben links)
				m_pFrameData->uiWidthDest,					// Breite ab X
				m_pFrameData->uiHeightDest,					// Höhe ab Y
				m_hDisplayDC,								// Source hDisplayDC, also der DisplayDeviceContext
				0,											// Src: X (0,0 ist oben links)
				0,											// Src: Y (0,0 ist oben links)
				SRCCOPY));
		}//END-IF IsScaled
		return hr;
	}//END-FUNC
	/// <summary>
	/// - Get the Resolution of the recording Window-Handle
	/// - If the Handle is invalid it takes the whole Desktop
	/// - safes the Size of Window/Desktop into the ClsDataContainer
	/// CalledBy: FindSetWindow
	/// </summary>
	/// <returns>HResult</returns>
	HRESULT  ClsWinGDI::GetResolutionOfWndHandle()
	{
		HRESULT hr = NULL;
		RECT WindowSrcRect = {};							// RECT for ResolutionInfo
		
		if (m_uiWindowFlag == WNDFOUND)
		{
			HR_RETURN_ON_INT_ERR(GetClientRect(
			m_MyClsWndHandle.GetHandle(), &WindowSrcRect));	// Auflösung des Fensters
			//m_pFrameData->pClsWndHandle = &m_MyClsWndHandle;
			m_pFrameData->uiWidthSrc =  WindowSrcRect.right - WindowSrcRect.left;			// Breite
			m_pFrameData->uiHeightSrc = WindowSrcRect.bottom - WindowSrcRect.top;
		}//END-IF valid Wnd-Title (Window for recording)
		else
		{
			m_MyClsWndHandle.SetWndTitle("Desktop");		// Titel des Fensters
			m_pFrameData->uiHeightSrc = GetSystemMetrics(SM_CYSCREEN);
			m_pFrameData->uiWidthSrc = GetSystemMetrics(SM_CXSCREEN);
			int iX = GetSystemMetrics(SM_XVIRTUALSCREEN);
			int iY = GetSystemMetrics(SM_YVIRTUALSCREEN);
		}//END-ELSE take whole Desktop instead of Window
		return hr;
	}//END-FUNC
}//END-NAMESPACE GDI