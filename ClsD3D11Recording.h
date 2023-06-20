#pragma once
#include "framework.h"
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
	const char* strWndTitle;
	const char* strFileName;
public:
	VideoDescriptor()
	{
		bIsAudio = false;
		strWndTitle = NULL;
		strFileName = NULL;
		uiFPS = DEFFPS;
		uiHeightDest = DEFHEIGHT;
		uiMonitorID = MAINMONITOR;
		uiWidthDest = DEFWIDTH;
		myInputFormat = MFVideoFormat_ARGB32;
		myOutputFormat = MFVideoFormat_H264;
		myBitReading = PicDataBitReading::Standard;
		myCpyMethod = CopyMethod::DesktopDupl;
	}
};
struct MonitorInfo
{
private:
	BOOL m_bPrimary;
	UINT m_uiWidth;
	UINT m_uiHeight;
	UINT m_uiTop;
	UINT m_uiLeft;
public:
	// Breite des Monitors
	void SetWidth(UINT uiWidth)
	{
		m_uiWidth = uiWidth;
	}
	// Höhe des Monitors
	void SetHeight(UINT uiHeight)
	{
		m_uiHeight = uiHeight;
	}
	// Status des Monitors: Hauptmonitor oder nicht
	void SetPrimaryStatus(DWORD dwPrimaryFlag)
	{
		if (dwPrimaryFlag == 1)
			m_bPrimary = true;
		else
			m_bPrimary = false;
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
	int m_iXPos;
	int m_iYPos;
	UINT m_uiTop;
	UINT m_uiLeft;
	UINT m_uiPickedMonitor;
	RECT m_myWndRect;
	RECT m_myClientRect;			// Client Area
	CopyMethod m_myCpyMethod;
	PicDataBitReading m_myBitReading;
	FrameData* m_pFrameData;
	std::vector<MonitorInfo*>   m_vMonitors;

	static UINT m_uiMaxMonitors;
private:
	D3D::ClsD3D11 m_myClsD3D11;
	ClsSinkWriter m_myClsSinkWriter;
	GDI::ClsWinGDI m_myClsWinGDI;
	ClsFPSSync m_myClsSyncFPS;
private:
	D3D::ClsD3D11& D3D11()
	{
		return m_myClsD3D11;
	}
	ClsSinkWriter& SinkWriter()
	{
		return m_myClsSinkWriter;
	}
	GDI::ClsWinGDI& WinGDI()
	{
		return m_myClsWinGDI;
	}
	ClsFPSSync& SyncFPS()
	{
		return m_myClsSyncFPS;
	}
private:
	static BOOL CALLBACK MonitorEnum(HMONITOR hMonitor, HDC hdc, LPRECT lprcMonitor, LPARAM pData)
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
	}
public:
	ClsD3D11Recording(VideoDescriptor* pVidDesc)
	{
		m_hWnd = NULL;
		m_iXPos = 0;
		m_iYPos = 0;
		m_uiTop = 0;
		m_uiLeft = 0;
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
		SetSrcResolution();									// Sets Values in m_pFrameData
		SetDestResolution(pVidDesc->uiWidthDest, pVidDesc->uiHeightDest);
		// Set Window Size and Position with the MonitorData. MonitorData by EnumDisplayMonitors() earlier
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
	}
	void Init3DWindow()
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
	}
	void PrepareRecording()
	{
		SinkWriter().PrepareInputOutput();
		SinkWriter().StartRecording();
		SyncFPS().Start();
	}
	void Recording()
	{
		++SyncFPS();
		if (!IsDesktopDupl())
			WinGDI().GetBitBltDataFromWindow();

		SinkWriter().StartReadAudioHWBufferThread();
		D3D11().BitBltDataToRT();
		//D3D11().SetConstantBuffer();
		WinGDI().TakeScreenshot();
		SinkWriter().LoopRecording();
		D3D11().PresentTexture();
		SyncFPS().SleepUntilNextFrame();
	}
	void StopRecording()
	{
		SinkWriter().StopRecording();
	}
	~ClsD3D11Recording()
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
	}
private:
	void SetCpyMethod(CopyMethod myCpyMethod)
	{
		m_myCpyMethod = myCpyMethod;
	}
	void SetWindowPosition()
	{
		m_iXPos = m_pFrameData->uiWidthDest / 2 - m_myWndRect.right / 2;
		m_iYPos = m_pFrameData->uiHeightDest / 2 - m_myWndRect.bottom / 2;
	}
	void SetWindowRect()
	{
		m_myWndRect.top = 0;
		m_myWndRect.bottom = 800;
		m_myWndRect.left = 0;
		m_myWndRect.right = 800;
		AdjustWindowRect(&m_myWndRect, WS_BORDER | WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, FALSE);
	}
	UINT GetWndXSize() { return m_myWndRect.right - m_myWndRect.left; }
	UINT GetWndYSize() { return m_myWndRect.bottom - m_myWndRect.top; }
	UINT GetXPos() { return m_iXPos; }
	UINT GetYPos() { return m_iYPos; }

	void SetHWND(HWND hWnd)
	{
		m_hWnd = hWnd;
		D3D11().SetWnd(hWnd);
	}
	BOOL IsDesktopDupl()
	{
		if (m_myCpyMethod == CopyMethod::DesktopDupl)
			return true;
		return false;
	}
	RECT GetMyClientRect()
	{
		GetClientRect(m_hWnd, &m_myClientRect);
		return m_myClientRect;
	}
private:
	void SetSrcResolution()
	{
		m_pFrameData->uiWidthSrc = m_vMonitors[m_uiPickedMonitor]->GetWidth();
		m_pFrameData->uiHeightSrc = m_vMonitors[m_uiPickedMonitor]->GetHeight();
		m_pFrameData->uiTop = m_vMonitors[m_uiPickedMonitor]->GetTop();
		m_pFrameData->uiLeft = m_vMonitors[m_uiPickedMonitor]->GetLeft();
		m_pFrameData->uiBpp = BITDEPTH;
	}
	void SetSrcDisplay(UINT uiMonitorID)
	{
		if (uiMonitorID > m_uiMaxMonitors)
			m_uiPickedMonitor = 0;
		else
			m_uiPickedMonitor = uiMonitorID;
	}
	void SetDestResolution(UINT uiWidthDest, UINT uiHeightDest)
	{
		// DesktopDupl cannot to be scaled
		if (m_myCpyMethod == CopyMethod::DesktopDupl )
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
	}

};