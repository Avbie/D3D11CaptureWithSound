// MyWindowPrj.cpp : Definiert den Einstiegspunkt für die Anwendung.
#include "framework.h"
#include "MyWindowPrj.h"
// Singleton in ClsWndProc
#include "ClsWndProc.h"
#include "ClsWnd.h"

const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
const IID IID_IAudioClient = __uuidof(IAudioClient);
const IID IID_IAudioCaptureClient = __uuidof(IAudioCaptureClient);
const IID IID_IAudioSessionManager2 = __uuidof(IAudioSessionManager2);
const IID IID_IAudioRenderClient = __uuidof(IAudioRenderClient);
const IID IID_ID2DBitmap = __uuidof(ID2D1Bitmap);


UINT ClsD3D11Recording::m_uiMaxMonitors = 0;

/// <summary>
/// WinMain Funktion
/// </summary>
/// <param name="hInstance">Handle zur Anwendung, idendifier für das OS </param>
/// <param name="hPrevInstance">Prüft ob Instanz bereits vorhanden, bis Win 3.1</param>
/// <param name="lpCmdLine">Pointer auf Cmdline Cmds</param>
/// <param name="nCmdShow">Flag für minimized, maximized</param>
/// <returns>NULL</returns>
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
	HRESULT hResult;										// Rückgabewert für Win32Api Funktionen
    UNREFERENCED_PARAMETER(hPrevInstance);					// Nicht referenzierte Parameter Warning unterdrücken
    UNREFERENCED_PARAMETER(lpCmdLine);
	char strWndTitle[256] = "";

	strcpy_s(strWndTitle, "Editor");

	VideoDescriptor myVideoDescriptor;
	/*
	* should be DesktopDupl, its really fast compared to all other Methods.
	* All other Methods have to read the PicData per GDI.
	* DesktopDupl can do around 60 FPS with sound and 120 FPS without sound.
	* AudioHardwareBuffer slows down the process.
	*/
	myVideoDescriptor.strFileName = "Output.mp4";
	myVideoDescriptor.bIsAudio = false;
	myVideoDescriptor.strWndTitle = "Editor";
	myVideoDescriptor.uiFPS = 25;
	myVideoDescriptor.myCpyMethod = CopyMethod::D2D1Surface;
	myVideoDescriptor.uiMonitorID = MAINMONITOR;

	ClsD3D11Recording myClsD3D11Recording(&myVideoDescriptor);

	ClsWnd oMyWnd(											// führt CreateWindowEx aus, d.h. erzeugt Fenster die reg. sind. 
															// WNDCLASS und Register wird bereits beim Start des Programms per Singleton ausgeführt
		L"MyProgramm", &myClsD3D11Recording);				// Titel

	oMyWnd.SetVisibility(true);								// Fenster sichtbar machen
	

	myClsD3D11Recording.Init3DWindow();
	myClsD3D11Recording.PrepareRecording();

	while (oMyWnd.RunMsgLoop())								// WndMainLoop
	{
		myClsD3D11Recording.Recording();
	}
	myClsD3D11Recording.StopRecording();
	CoUninitialize();
	return 0;
}//END-FUNC Main

