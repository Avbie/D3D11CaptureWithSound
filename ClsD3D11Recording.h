#pragma once
#include "framework.h"
//#include "FrameData.h"
#include "ClsSinkWriter.h"
#include "ClsWinGDI.h"
#include "ClsD3D11.h"
#include "ClsFpsSync.h"

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
	//const char* strWndTitle;
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

struct PerformanceCounter
{
	LARGE_INTEGER start = { 0 };
	LARGE_INTEGER end = { 0 };
	LARGE_INTEGER interval = { 0 };
	LARGE_INTEGER freq = { 0 };
};

/// <summary>
/// Used in the Enumation of Monitors in the MonitorEnum-CallbackFunction.
/// In the Callback this struct will be set with the Data of a Monitor.
/// It will be pushed into an Vector Array ClsD3D11Recording::m_vMonitors
/// One VectorElement is one MonitorInfo Struct
/// </summary>
struct MonitorInfo
{
private:
	BOOL m_bPrimary;
	UINT m_uiWidth;
	UINT m_uiHeight;
	UINT m_uiTop;
	UINT m_uiLeft;
public:
	// Höhe des Monitors
	void SetHeight(UINT uiHeight)
	{
		m_uiHeight = uiHeight;
	}
	// Breite des Monitors
	void SetWidth(UINT uiWidth)
	{
		m_uiWidth = uiWidth;
	}
	// Wann der Monitor innerhalb des virtuellen Monitors beginnt
	void SetTop(UINT uiTop)
	{
		m_uiTop = uiTop;
	}
	// Wann der Monitor innerhalb des virtuellen Monitors beginnt
	void SetLeft(UINT uiLeft)
	{
		m_uiLeft = uiLeft;
	}
	// Status des Monitors: Hauptmonitor oder nicht
	void SetPrimaryStatus(DWORD dwPrimaryFlag)
	{
		if (dwPrimaryFlag == 1)
			m_bPrimary = true;
		else
			m_bPrimary = false;
	}
	UINT GetHeight() { return m_uiHeight; }
	UINT GetWidth() { return m_uiWidth; }
	UINT GetTop() { return m_uiTop; }
	UINT GetLeft() { return m_uiLeft; }
};

// SuperClass
class ClsD3D11Recording
{
public:
	friend class ClsWnd;
	
private:
	HWND m_hWnd;
	static UINT m_uiMaxMonitors;
	UINT m_uiPickedMonitor;
	int m_iLeft;
	int m_iTop;
	int m_iXPos;
	int m_iYPos;
	
	RECT m_myClientRect;			// Client Area
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
	ClsD3D11Recording(VideoDescriptor* pVidDesc);
	~ClsD3D11Recording();

	void AdjustRatio();
	void CreateWindowList();
	void Finalize();
	vector<GDI::ActiveWnd>* GetWindowList();
	void Init3DWindow();
	void Loop();
	void PrepareRecording();
	void SetActiveWindow(UINT uiWndNr);
	void SetAudio(BOOL bAudio);
	void SetFPS(UINT uiFPS);
	void SetRecordingStatus(BOOL bRecord);
	void SetWndAsSrc();
	void SetMonitorAsSrc(UINT uiMonitorID);
	void ZoomInOrOut(float fPercentage);
	BOOL IsRecording();
private:
	D3D::ClsD3D11& D3D11();
	ClsSinkWriter& SinkWriter();
	ClsFPSSync& SyncFPS();
	GDI::ClsWinGDI& WinGDI();
	
	void SetCpyMethod(CopyMethod* myCpyMethod);
	void SetDestResolution(UINT uiWidthDest, UINT uiHeightDest);
	void SetHWND(HWND hWnd);
	void SetSrcDisplay(UINT uiMonitorID);
	void SetSrcResolution();
	void SetWindowPosition();
	void SetWindowRect();
	BOOL IsDesktopDupl();
	static BOOL CALLBACK MonitorEnum(HMONITOR hMonitor, HDC hdc, LPRECT lprcMonitor, LPARAM pData);
	UINT GetWndXSize();
	UINT GetWndYSize();
	UINT GetXPos();
	UINT GetYPos();
};

inline UINT ClsD3D11Recording::m_uiMaxMonitors = 0;