#include "ClsWinGDI.h"

using namespace std;

namespace GDI
{
	/// <summary>
	/// Constructor
	/// </summary>
	ClsWinGDI::ClsWinGDI()
	{
		m_hFileData = NULL;
		m_hMemDC = NULL;
		m_hDisplayDC = NULL;
		m_hBitmap = NULL;
		m_uiWindowFlag = 0;
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
	void ClsWinGDI::SetFrameData(FrameData** ppFrameData)
	{
		m_pFrameData = *ppFrameData;
		ScreenShot().SetFrameData(ppFrameData);
	}//END-FUNC
	BOOL ClsWinGDI::SetActiveWindow(const UINT uiWndNr)
	{
		size_t uiSize = m_vWndCollection.size();


		if (uiSize <= 0 || (uiWndNr < 0 || uiWndNr > uiSize))
		{
			//m_myClsWndHandle.SetActiveWnd(m_vWndCollection[0]);
			return FALSE;
		}//END-IF window not set
		else
		{
			m_myClsWndHandle.SetActiveWnd(m_vWndCollection[uiWndNr]);
			return TRUE;
		}//END-ELSE  window set
	}//END-FUNC
	ClsScreenshot& ClsWinGDI::ScreenShot()
	{
		return m_myClsScreenShot;
	}//END-FUNC
	const vector<ActiveWnd>& ClsWinGDI::GetWindowList()
	{
		return m_vWndCollection;
	}
	ClsWndHandle& ClsWinGDI::WndHandle()
	{
		return m_myClsWndHandle;
	}
	/*********************************/
	HRESULT ClsWinGDI::CreateWindowList()
	{
		m_vWndCollection.clear();
		HR_RETURN_ON_NULL_ERR(EnumWindows(CreateWindowListProc, reinterpret_cast<LPARAM>(&m_vWndCollection)));

		return S_OK;
	}//END_FUNC
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
			return S_OK;
		if (m_uiWindowFlag == NODESKDUPL)					// Looking for Window with set Title
		{
			// compares the processID of our set window with the valid windows
			// If EnumWindowsProc returns FALSE (processID is valid)
			// EnumWindows will leave the Loop and will return false
			if (!EnumWindows(
				EnumWindowsProc,
				reinterpret_cast<LPARAM>(&m_myClsWndHandle)))
				m_uiWindowFlag = m_uiWindowFlag | 0b0010;	// Bitflag for Window found
		}//END-IF WNDTITLESET
		if (m_uiWindowFlag == WNDFOUND)
		{
			m_hDisplayDC = GetDC(m_myClsWndHandle.GetWndHandle());	// DC of the WndHandle
			HR_RETURN_ON_NULL_ERR(m_hDisplayDC);
			HR_RETURN_ON_ERR(hr, AllocateMemDC());			// compatible MemDC with a HBitmap that has the specific destination size

			HR_RETURN_ON_INT_ERR(GetClientRect(
				m_myClsWndHandle.GetWndHandle(), &myClientRectSrcWnd));	// keep the size of the window
			m_pFrameData->uiWidthSrc = myClientRectSrcWnd.right - myClientRectSrcWnd.left;
			m_pFrameData->uiHeightSrc = myClientRectSrcWnd.bottom - myClientRectSrcWnd.top;
			m_pFrameData->uiTop = 0;
			m_pFrameData->uiLeft = 0;
		}//END-IF Wnd-Handle
		else 
		{
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
		ReleaseDC(m_myClsWndHandle.GetWndHandle(), m_hDisplayDC);
		ReleaseDC(NULL, m_hDisplayDC);
		m_uiWindowFlag = 0;
	}//END-FUNC
	/// <summary>
	/// - Callback Function used in CollectOpenWnd() in EnumWindowProc 
	///   for collect all valid windows
	/// - This function will be called in a loop in EnumWindowProc, for every Window that is currently open
	/// </summary>
	/// <param name="hWnd">current hwnd by EnumWindowProc</param>
	/// <param name="lparam">not used</param>
	/// <returns>always true</returns>
	BOOL CALLBACK ClsWinGDI::CreateWindowListProc(HWND hWnd, LPARAM lParam)
	{
		DWORD dwProcessID = 0;
		TCHAR wstrCurTitle[WNDTITLESIZE]{};
		wstring wsCurTitle;
		string sCurTitle;

		vector<ActiveWnd>* pvWndCollection = reinterpret_cast<vector<ActiveWnd>*>(lParam);

		if (!IsWindowVisible(hWnd))
			return TRUE;

		if (GetWindowTextLength(hWnd) == 0)
			return TRUE;
		GetWindowText(hWnd, wstrCurTitle, WNDTITLESIZE);
		wsCurTitle.assign(wstrCurTitle);
		sCurTitle.assign(wsCurTitle.begin(), wsCurTitle.end());

		GetWindowThreadProcessId(hWnd, &dwProcessID);

		ActiveWnd myActiveWnd;
		myActiveWnd.dwProcessID = dwProcessID;
		myActiveWnd.hWnd = hWnd;
		myActiveWnd.sTitle = sCurTitle;

		pvWndCollection->push_back(myActiveWnd);

		return TRUE;
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
		DWORD dwProcessID = -1;		
		//USES_CONVERSION;
		ClsWndHandle* pClsWndHandle = reinterpret_cast<ClsWndHandle*>(lParam);
		
		GetWindowThreadProcessId(hWnd, &dwProcessID);
		if(pClsWndHandle->GetProcessID() == dwProcessID)
			return FALSE;												// Pid found, leave loop in EnumWindowProc
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
			return TRUE;
		if (m_pFrameData->uiWidthDest != m_pFrameData->uiWidthSrc)
			return TRUE;
		return FALSE;
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
}//END-NAMESPACE GDI