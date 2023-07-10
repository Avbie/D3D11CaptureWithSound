#include "ClsMonitor.h"

/// <summary>
/// - Uses the Parameter as input and filling it with valid MonitorData
/// - Its a pointer to an VectorArray which consists of elements that are pointers to MonitorInfo
/// - calls EnumDisplayMonitors and uses ClsMonitor::MonitorEnum for iterating each monitor.
/// CalledBy: ClsD3D11Recording::ClsD3D11Recording
/// </summary>
/// <param name="vMonitors">ClsD3D11Recording::m_vMonitors</param>
ClsMonitor::ClsMonitor(std::vector<MonitorInfo*>* vMonitors, UINT* pMaxMonitors)
{
	// Get DC of Graphiccard (incudes all Monitors as a virtual Desktop)
	HDC DevHC = GetDC(NULL);
	if (!EnumDisplayMonitors(DevHC, 0, ClsMonitor::MonitorEnum, (LPARAM)vMonitors))
		HRESULT_FROM_WIN32(GetLastError());
	*pMaxMonitors = (UINT)vMonitors->size();
}//END-FUNC
/// <summary>
/// Creates list of all supported resolution that are possible for all installed monitors.
/// </summary>
/// <param name="vMonitors">List of resolution for a installed monitor</param>
void ClsMonitor::CreateListOfAllMonitorRes(const std::vector<MonitorInfo*>* vMonitors)
{
	for (auto Monitor : *vMonitors)
	{
		//vector<SupportedResolution> g; // Parameter in funktion
		//vector<SupportedResolution>& r = g; // der rückgabe wird als referenz
		//vector<SupportedResolution> g = r; // zuweisung des rückgabe referenz, impl. konvertierung von lvalue zu rvalue
		m_vListOfAllMonitorRes.insert(m_vListOfAllMonitorRes.begin(), Monitor->GetSupportedRes().begin(), Monitor->GetSupportedRes().end());
	}//END-FOR
}//END-FUNC
/// <summary>
/// - overwritten Callback Function for enum Monitors.
/// - Set in EnumDisplayMonitors, called in the Constructor
/// </summary>
/// <param name="hMonitor">Handle of the Monitor</param>
/// <param name="hdc">Handle to the DeviceContext (Graficcard)</param>
/// <param name="lprcMonitor">Rect with Resolution Info (we used GetMonitorInfo instead)</param>
/// <param name="pData">Custom Parameter, ClsD3D11Recording::m_vMonitors</param>
/// <returns>always TRUE</returns>
BOOL CALLBACK ClsMonitor::MonitorEnum(HMONITOR hMonitor, HDC hdc, LPRECT lprcMonitor, LPARAM pData)
{
	MONITORINFOEX myMonitorInfo;
	myMonitorInfo.cbSize = sizeof(MONITORINFOEX);
	GetMonitorInfo(hMonitor, &myMonitorInfo);
	//pMonitorInfo->myMonInfo = *lprcMonitor; // other method for getting the myMonitorInfo

	// Pointer of an VactorArray that includes pointers of MonitorInfo
	vector<MonitorInfo*>* vMonitors = reinterpret_cast<vector<MonitorInfo*>*>(pData);
	MonitorInfo* pMonitorInfo = new MonitorInfo;

	ClsSupportedRes mySupportedRes(myMonitorInfo.szDevice);
	pMonitorInfo->SetName(myMonitorInfo.szDevice);
	pMonitorInfo->SetPrimaryStatus(myMonitorInfo.dwFlags);
	pMonitorInfo->SetTop(myMonitorInfo.rcMonitor.top);
	pMonitorInfo->SetLeft(myMonitorInfo.rcMonitor.left);
	pMonitorInfo->SetWidth(myMonitorInfo.rcMonitor.right - myMonitorInfo.rcMonitor.left);
	pMonitorInfo->SetHeight(myMonitorInfo.rcMonitor.bottom - myMonitorInfo.rcMonitor.top);
	pMonitorInfo->SetSupportedRes(mySupportedRes.GetSupportedRes());
	vMonitors->push_back(pMonitorInfo);

	return TRUE;
}//END-FUNC