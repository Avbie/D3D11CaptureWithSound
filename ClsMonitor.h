#pragma once

#include "framework.h"
#include "FrameData.h"
#include "ClsSupportedRes.h"

using namespace std;

/// <summary>
/// Used in the Enumation of Monitors in the MonitorEnum-CallbackFunction.
/// In the Callback this struct will be set with the Data of a Monitor.
/// It will be pushed into an Vector Array ClsD3D11Recording::m_vMonitors
/// One VectorElement is one MonitorInfo Struct
/// </summary>
struct MonitorInfo
{
public:
	MonitorInfo()
	{
		m_bPrimary = FALSE;
		m_uiWidth = 0;
		m_uiHeight = 0;
		m_uiTop = 0;
		m_uiLeft = 0;
		wcsncpy_s(m_wsName,32, L"", 32);
		m_pFrameData = NULL;
	}
private:
	BOOL m_bPrimary;
	UINT m_uiWidth;
	UINT m_uiHeight;
	UINT m_uiTop;
	UINT m_uiLeft;
	WCHAR m_wsName[32];
	FrameData* m_pFrameData;
public:
	vector<SupportedResolution> m_vSupportedRes;
public:
	vector<SupportedResolution>& GetSupportedRes()
	{
		return m_vSupportedRes;
	}
	// Höhe des Monitors
	void SetHeight(const UINT uiHeight)
	{
		m_uiHeight = uiHeight;
	}
	// Breite des Monitors
	void SetWidth(const UINT uiWidth)
	{
		m_uiWidth = uiWidth;
	}
	// Wann der Monitor innerhalb des virtuellen Monitors beginnt
	void SetTop(const UINT uiTop)
	{
		m_uiTop = uiTop;
	}
	// Wann der Monitor innerhalb des virtuellen Monitors beginnt
	void SetLeft(const UINT uiLeft)
	{
		m_uiLeft = uiLeft;
	}
	void SetName(const WCHAR* wsName)
	{
		wcsncpy_s(m_wsName,32, wsName, 32);
	}
	// Status des Monitors: Hauptmonitor oder nicht
	void SetPrimaryStatus(const DWORD dwPrimaryFlag)
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

	void SetSupportedRes(const vector<SupportedResolution>& vSupportedRes)
	{
		m_vSupportedRes = vSupportedRes;
	}
	//vector<SupportedResolution> GetSupported
};

class ClsMonitor
{
private:
	vector<SupportedResolution> m_vListOfAllMonitorRes;
	//UINT m_uiMaxMonitors = 0;
public:
	ClsMonitor(std::vector<MonitorInfo*>* vMonitors, UINT* pMaxMonitors);
public:
	//UINT CountMonitors();
private:
	void CreateListOfAllMonitorRes(const std::vector<MonitorInfo*>* vMonitors);
	static BOOL CALLBACK MonitorEnum(HMONITOR hMonitor, HDC hdc, LPRECT lprcMonitor, LPARAM pData);
	
};