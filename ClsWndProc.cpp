#include "ClsWndProc.h"

LPCWSTR ClsWndProc::m_lpClassName = L"GfxWindowCls";
ClsWndProc ClsWndProc::SingletonInstance = ClsWndProc();

/// <summary>
/// Wird beim Start des Programms ausgeführt, auf Grund der Singleton-Definition
/// </summary>
ClsWndProc::ClsWndProc()
{
	m_hInstance = GetModuleHandle(NULL);					// Get hinstance

	WNDCLASSEX WndDescriptor;								// Describe window class
	ZeroMemory(&WndDescriptor, sizeof(WNDCLASSEX));			// Reset Memory
	WndDescriptor.cbSize = sizeof(WNDCLASSEX);				// Set Size of Descriptor

	WndDescriptor.style = CS_OWNDC;
	WndDescriptor.lpfnWndProc = &ClsWndProc::MsgProcSetup;	// FunctionPointer to our WndProc (static ClassFunction)
	WndDescriptor.cbClsExtra = 0;							// Extra-Bytes for us
	WndDescriptor.cbWndExtra = 0;							// Link a other Window/DlgBox
	WndDescriptor.hInstance = m_hInstance;					// Instance of Winapi
	WndDescriptor.hIcon = NULL;								// Icon
	WndDescriptor.hIconSm = NULL;							// Small Icon	
	WndDescriptor.hCursor = LoadCursor(m_hInstance, IDC_ARROW);
	WndDescriptor.hbrBackground = NULL;						// Background
	WndDescriptor.lpszMenuName = NULL;						// Pointer to the MenuClass
	WndDescriptor.lpszClassName = m_lpClassName;			// Name of Class, needed as Ident (CreateWindowEx reads from Registred)

	// Register window class
	RegisterClassEx(&WndDescriptor);						// Register - "put it in a list"
}// END-CONSTR


/// <summary>
/// Überschreibung der CallbackFunktion.
/// Erster und einziger Aufruf erfolgt durch CreateWindowEx in ClsWnd-Konstruktor
/// Festgelegt als Funktionspointer (WindowProc) in der WindowDescription im Konstruktor
/// 
/// Hier erfolgt eine neue Festlegung der WindowProc: von MsgProcSetup zu MsgProcRun:
/// Alle anderen Trigger gehen nun durch MsgProcRun
/// Merken hier unsere ClsWnd-Instanz per SetLongPtr (Zugriff dieser Instanz in ClsWndProc::MsgProcRun)
/// </summary>
/// <param name="hWnd">Handle zum Window was erstellt wird/wurde</param>
/// <param name="uiMsg">Ereignis was gesendet wurde</param>
/// <param name="wParam"></param>
/// <param name="lParam">Struct von CreateWindowEx in ClsWnd-Konstr als VoidPointer</param>
/// <returns>Verarbeitete Msgs</returns>
LRESULT ClsWndProc::MsgProcSetup(HWND hWnd, UINT uiMsg, WPARAM wParam, LPARAM lParam)
{
	if (uiMsg == WM_NCCREATE)
	{
		CREATESTRUCT* pCreateStruct = (CREATESTRUCT*)lParam;
		ClsWnd* pClsWnd = (ClsWnd*)pCreateStruct->lpCreateParams;
		SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pClsWnd);
		SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG_PTR)ClsWndProc::MsgProcRun);

		//return pClsWnd
	}
	return DefWindowProc(hWnd, uiMsg, wParam, lParam);
}// END WINDOW-PROC

/// <summary>
/// Festgelegt dieser WndProc durch WndProcSetup
/// Getriggert durch DispatchMessage() in ClsWnd::RunMsgLoop()
/// Übergibt unserer WndClass die Parameter für das Handling der inc. Msgs
/// </summary>
/// <param name="hWnd">aktuelles Fenster</param>
/// <param name="uiMsg">Msg die vom Fenster gesendet wird. Z.B. minimieren</param>
/// <param name="wParam">zusätzliche MsgInfos. kommt auf Call drauf an</param>
/// <param name="lParam">zusätzliche MsgInfos. kommt auf Call drauf an. Bei CREATE ist es der WindowCreateEx-Struct</param>
/// <returns>Verarbeitete Msgs</returns>
LRESULT ClsWndProc::MsgProcRun(HWND hWnd, UINT uiMsg, WPARAM wParam, LPARAM lParam) {
	// Extract IMessageReciver
	ClsWnd* pClsWnd = (ClsWnd*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
	// Call reciver
	return pClsWnd->ProcessTriggeredMsg(hWnd, uiMsg, wParam, lParam);
}
/// <summary>
/// Returns the ClassName:
/// Needed for RegisterClass in Constructor
/// Ident in Window-Descriptor in WindProc (set) per Singleton, 
/// ident in Creating Window in ClsWnd (get) called in WinMain
/// </summary>
/// <returns>Ident</returns>
LPCWSTR ClsWndProc::GetMyClassName()
{
	return m_lpClassName;
}//END GetMyClassName

/// <summary>
/// Beim beenden des Programms getriggert
/// Ident erfolgt über m_lpClassName
/// Löscht das Fenster aus der internen Liste der WINAPI
/// </summary>
ClsWndProc::~ClsWndProc() {
	// Unregister class
	UnregisterClass(m_lpClassName, m_hInstance);
}//END-DESTR