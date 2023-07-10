#pragma once
#include "framework.h"
//#include "FrameData.h"
#include "ClsSinkWriter.h"
#include "ClsWinGDI.h"
#include "ClsD3D11.h"
#include "ClsFpsSync.h"
#include "ClsSupportedRes.h"
#include "ClsMonitor.h"


/// <summary>
/// Enthält Metadaten für das Fenster: Höhe, Breite, Bpp, Handle, Titelname
/// Enthält Filename für BMP-Datei (für Screenshot)
/// Bekommt Daten des Fensterinhalts über m_pData
/// </summary>

/// <summary>
/// Allows the User to discripe the Superclass
/// Parameter for the Constructor
/// </summary>
struct VideoDescriptor
{
public:
	BOOL bIsAudio;
	UINT uiFPS;
	UINT uiHeightDest;
	UINT uiMonitorID; 
	UINT uiWidthDest; 
	GUID myInputFormat;
	GUID myOutputFormat;
	CopyMethod myCpyMethod;
	PicDataBitReading myBitReading;
	const char* strFileName;
public:
	VideoDescriptor()
	{
		bIsAudio = false;
		//strWndTitle = NULL;
		strFileName = NULL;
		uiFPS = DEFFPS;
		uiHeightDest = DEFHEIGHT;
		uiMonitorID = MAINMONITOR;
		uiWidthDest = DEFWIDTH;
		myInputFormat = MFVideoFormat_ARGB32;
		myOutputFormat = MFVideoFormat_H264;
		myCpyMethod = CopyMethod::DesktopDupl;
		myBitReading = PicDataBitReading::Standard;
	}
};

struct CpyMethodDescription
{
	CopyMethod m_myCpyMethod;
	std::string m_sName;
	std::string m_sDescription;
};
// SuperClass
class ClsD3D11Recording
{
public:
	friend class ClsWnd;
private:
	HWND m_hWnd;
	static UINT m_uiMaxMonitors;
	UINT m_uiPickedMonitorIndex;
	INT64 m_iPickedResIndex;
	int m_iLeft;
	int m_iTop;
	int m_iXPos;
	int m_iYPos;
	float m_fZoomPercentage;
	
	//RECT m_myClientRect;			// Client Area
	RECT m_myWndRect;
	PicDataBitReading m_myBitReading;
	VideoDescriptor* m_pVidDesc;
	FrameData* m_pFrameData;
	std::vector<MonitorInfo*> m_vMonitors;

private:
	D3D::ClsD3D11 m_myClsD3D11;
	ClsSinkWriter m_myClsSinkWriter;
	ClsFPSSync m_myClsSyncFPS;
	GDI::ClsWinGDI m_myClsWinGDI;
public:
	/**********PUBLIC-METHODS*********/
	ClsD3D11Recording(VideoDescriptor* pVidDesc);
	~ClsD3D11Recording();
	/**********GET-METHODS************/
	HWND GetActiveWindow();
	const std::vector<SupportedResolution>& GetAllSupportedRes();
	const UINT GetFPS();
	const std::vector<MonitorInfo*>& GetMonitors();
	const UINT GetSelectedIndexOfMonitorList();
	const INT64 GetSelectedIndexOfResList();
	const vector<GDI::ActiveWnd>& GetWindowList();
	const FLOAT GetZoomFactor();
	BOOL IsAudio();
	/**********SET-METHODS************/
	void SetAudio(BOOL bAudio);
	void SetFPS(const UINT uiFPS);
	BOOL SetActiveWindow(const UINT uiWndNr);
	BOOL SetDestResFromList(int iResIndexNr=DEFRES);
	BOOL SetMonitor(const UINT uiMonitorID);
	BOOL SetZoomFactor(const float fPercentage);
	/**********RECORDING-METHODS******/
	BOOL Loop();
	BOOL StartRecording();
	BOOL StopRecording();
	/**********INITIALIZATION********/
	BOOL CreateWindowList();
	BOOL Init3DWindow();
	HRESULT AdjustRatio(); // triggered by resizing the Window
private:
	/**********PRIVATE-METHODS********/
	/**********SUB-OBJECTS************/
	D3D::ClsD3D11& D3D11();
	ClsSinkWriter& SinkWriter();
	ClsFPSSync& SyncFPS();
	GDI::ClsWinGDI& WinGDI();
	/*********************************/
	
	//void CompareDestResWithResArray();
	void MakeEqualResolution();
	
	void SetCpyMethod(CopyMethod myCpyMethod);
	void SetHWND(HWND hWnd);
	void SetRecordingStatus(BOOL bRecord);
	
	void SetWindowPosition();
	void SetWindowRect();

	BOOL IsDesktopDupl();
	BOOL IsRecording();
	BOOL ReCreateFrameBuffer();
	BOOL RefreshDestResolution(int iResIndexNr = DEFRES);
	BOOL SetSrcDisplay(const UINT uiMonitorID);
	BOOL SetSrcResolution();
	
	UINT GetWndXSize();
	UINT GetWndYSize();
	UINT GetXPos();
	UINT GetYPos();
};

inline UINT ClsD3D11Recording::m_uiMaxMonitors = 0;