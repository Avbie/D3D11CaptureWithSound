#include "ClsWndHandle.h"

namespace GDI
{
	/// <summary>
	/// Constructor
	/// </summary>
	ClsWndHandle::ClsWndHandle()
	{
		m_bIsWndSet = FALSE;
		m_myActiveWnd = {};
	}//END-CONS
	/// <summary>
	/// Sets the active Window
	/// CalledBy: ClsWinGDI::SetActiveWindow
	/// </summary>
	/// <param name="myActivWnd">Window Number in array of the WindowList</param>
	void ClsWndHandle::SetActiveWnd(ActiveWnd myActivWnd)
	{
		m_myActiveWnd = myActivWnd;
		m_bIsWndSet = TRUE;
	}//END-FUNC
	/// <summary>
	/// Sets a invalid Handle if the source is the Desktop and no window.
	/// CalledBy: ClsWinGDI::FindSetWindow()
	/// </summary>
	void ClsWndHandle::SetDesktop()
	{
		m_myActiveWnd.dwProcessID = -1;
		m_myActiveWnd.hWnd = NULL;
		m_myActiveWnd.sTitle = DESKTOP;
		m_bIsWndSet = FALSE;
	}//END-FUNC
	/// <summary>
	/// Returns true if the handle is a valid window handle
	/// CalledBy: ClsWinGDI::FindSetWindow()
	/// </summary>
	/// <returns>true if its a valid window handle</returns>
	BOOL ClsWndHandle::IsWndSet()
	{
		return m_bIsWndSet;
	}//END-FUNC
	/// <summary>
	/// Returns the current window Handle, that we want  to record
	/// CalledBy: ClsWinGDI::FindSetWindow()
	/// </summary>
	/// <returns>WindowHandle</returns>
	HWND ClsWndHandle::GetHandle()
	{
		return m_myActiveWnd.hWnd;
	}//END-FUNC
	/// <summary>
	/// Returns the Pid of the selected active window
	/// CalledBy: ClsWinGDI::EnumWindowsProc
	/// </summary>
	/// <returns>ProcessID</returns>
	DWORD ClsWndHandle::GetProcessID()
	{
		return m_myActiveWnd.dwProcessID;
	}//END-FUNC
}//END NAMESPACE GDI
