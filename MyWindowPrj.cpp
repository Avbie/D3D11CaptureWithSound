// MyWindowPrj.cpp : Definiert den Einstiegspunkt für die Anwendung.
#include "framework.h"
#include "MyWindowPrj.h"
// Singleton in ClsWndProc
#include "ClsWndProc.h"
#include "ClsWnd.h"

//UINT ClsD3D11Recording::m_uiMaxMonitors = 0;
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
    UNREFERENCED_PARAMETER(hPrevInstance);					// Nicht referenzierte Parameter Warning unterdrücken
    UNREFERENCED_PARAMETER(lpCmdLine);

	VideoDescriptor myVideoDescriptor;
	/*should be DesktopDupl, its really fast compared to all other Methods.
	* All other Methods have to read the PicData per GDI.
	* DesktopDupl can do around 60 FPS with sound and 120 FPS without sound.
	* AudioHardwareBuffer slows down the process.
	* the Piep sound is used for testing some stuff, and can be disabled in
	* ClsSilence::LoadAudioBuffer(..) dFrequency  = 0 
	* Bug: with resolution 1680x1050, Sinkwriter gets a other width and recorded video is broken
	*/
	// todo button Record and Stop
	// todo compilerwarnungen durchgehen
	myVideoDescriptor.strFileName = "Output.mp4";
	myVideoDescriptor.bIsAudio = true;
	myVideoDescriptor.strWndTitle = "Editor";
	myVideoDescriptor.uiFPS = 60;
	myVideoDescriptor.myCpyMethod = CopyMethod::DesktopDupl;
	myVideoDescriptor.uiMonitorID = 1;

	ClsD3D11Recording myClsD3D11Recording(&myVideoDescriptor);
	ClsWnd oMyWnd(											// führt CreateWindowEx aus, d.h. erzeugt Fenster die reg. sind. 
															// WNDCLASS und Register wird bereits beim Start des Programms per Singleton ausgeführt
		L"MyProgramm", &myClsD3D11Recording);				// Titel
	

	oMyWnd.SetVisibility(true);								// Fenster sichtbar machen
	myClsD3D11Recording.Init3DWindow();
	//myClsD3D11Recording.SetRecordingStatus(true);
	//myClsD3D11Recording.PrepareRecording();
	while (oMyWnd.RunMsgLoop())								// WndMainLoop
	{
		myClsD3D11Recording.Loop();
	}
	//myClsD3D11Recording.SetRecordingStatus(false);
	//myClsD3D11Recording.Finalize();
	CoUninitialize();
	return 0;
}//END-FUNC Main

