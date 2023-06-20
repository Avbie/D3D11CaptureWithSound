#include "ClsWndHandle.h"

namespace GDI
{
	ClsWndHandle::ClsWndHandle()
	{
		m_bIsWnd = false;
		m_hWnd = NULL;
		m_dwPid = 0;
		m_strTitle = NULL;
	}
	/// <summary>
	/// - Sets the Window Handle
	/// - Depending on the Title
	/// CalledBy: ClsWinGDI::EnumWindowsProc(...)
	/// </summary>
	/// <param name="hWnd">Window Handle</param>
	void ClsWndHandle::SetHandle(HWND hWnd)
	{
		m_hWnd = hWnd;
		GetWindowThreadProcessId(m_hWnd, &m_dwPid);
		m_bIsWnd = true;
	}//END-FUNC
	/// <summary>
	/// - Sets the Title defined by User
	/// - Whole ClsWndHandle object will be used in
	/// </summary>
	/// <param name="strTitle"></param>
	void ClsWndHandle::SetWndTitle(const char* strTitle)
	{
		//strcpy_s(m_strTitle, strTitle);
		m_strTitle = (char*)strTitle;
	}//END-FUNC
	/// <summary>
	/// Not used yet
	/// </summary>
	/// <returns></returns>
	BOOL ClsWndHandle::CheckWnd()
	{
		if (IsWindow(m_hWnd))
		{
			DWORD dwNewId = NULL;
			GetWindowThreadProcessId(m_hWnd, &dwNewId);
			if (m_dwPid == dwNewId)
			{
				m_bIsWnd = true;
				return true;
			}
			else
			{
				m_hWnd = NULL;
				m_dwPid = 0;
				m_bIsWnd = false;
				return false;
			}
		}
		else
		{
			m_hWnd = NULL;
			m_dwPid = 0;
			m_bIsWnd = false;
			return false;
		}
	}//END-FUNC
	/// <summary>
	/// Returns the current Window Handle, that we want  to record
	/// </summary>
	/// <returns>WindowHandle</returns>
	HWND ClsWndHandle::GetHandle()
	{
		return m_hWnd;
	}//END-FUNC
	/// <summary>
	/// Returns the current Title of the Window that we want to record
	/// </summary>
	/// <returns>Title</returns>
	char* ClsWndHandle::GetWndTitle()
	{
		return m_strTitle;
	}//END-FUNC
}//END NAMESPACE GDI
