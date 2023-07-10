#include "ClsWndHandle.h"

namespace GDI
{
	/// <summary>
	/// Constructor
	/// </summary>
	ClsWndHandle::ClsWndHandle()
	{
		//m_bIsWndSet = FALSE;
		m_myActiveWnd = {};
	}//END-CONS
	/// <summary>
	/// Sets the active Window
	/// CalledBy: ClsWinGDI::SetActiveWindow
	/// </summary>
	/// <param name="myActivWnd">keeps the active window. Structs includes the Handle, Name, Pid </param>
	void ClsWndHandle::SetActiveWnd(const ActiveWnd myActivWnd)
	{
		m_myActiveWnd = myActivWnd;
	}//END-FUNC
	/// <summary>
	/// Returns the current window Handle, that we want  to record
	/// CalledBy: ClsWinGDI::FindSetWindow()
	/// </summary>
	/// <returns>WindowHandle</returns>
	HWND ClsWndHandle::GetWndHandle()
	{
		return m_myActiveWnd.hWnd;
	}//END-FUNC
	/// <summary>
	/// Returns the Pid of the selected active window
	/// CalledBy: ClsWinGDI::EnumWindowsProc
	/// </summary>
	/// <returns>ProcessID</returns>
	const DWORD ClsWndHandle::GetProcessID()
	{
		return m_myActiveWnd.dwProcessID;
	}//END-FUNC
}//END NAMESPACE GDI
