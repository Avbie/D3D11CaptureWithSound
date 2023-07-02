// header.h: Includedatei für Include-Standardsystemdateien
// oder projektspezifische Includedateien.
//

#pragma once

//using namespace std;

//#include "targetver.h"
#include "dxerr.h"
#define WIN32_LEAN_AND_MEAN             // Selten verwendete Komponenten aus Windows-Headern ausschließen
// Windows-Headerdateien

#define EXCEP(expr) \
{ \
 try{expr;} \
 catch(exception &e) \
	{ \
		MessageBoxW(NULL, CA2W(e.what()),  _T(""), MB_OK); \
	} \
}

enum class CopyMethod { Mapping, D2D1Surface, SubResource, DesktopDupl };
enum class PicDataBitReading { Invert, Standard };

//#include "ClsWndHandle.h"
//forward declaration
//namespace GDI
//{
//	class ClsWndHandle;
//}
/// <summary>
/// Container for transfering Data into Subclasses of the Superclass
/// </summary>


template <class T>
void SafeRelease(T** ppT)
{
	if (*ppT)
	{
		(*ppT)->Release();
		*ppT = NULL;
	}
}

// Defines for CoreAudio
#define REFTIMES_PER_MS  (10000)
#define M_PI (3.14159265358979323846)
/* hrTemp: HResult of Function
*  exp: The Function/Return of Function
*  Zuweisung von exp an hrTemp
*  Wenn hrTemp < 0 dann ERROR -> akt. Funktion wird mit HRESULT beendet
*/

#define WINSTYLE WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SIZEBOX
#define NODESKDUPL 1
#define SETWND 3
#define WNDTITLESET 7
#define WNDFOUND 15

// Default Audio HardwareBuffer
#define DEFAUDIOHWBUFFERSIZE 500000 
// Default Resolution
#define DEFWIDTH 2560
#define DEFHEIGHT 1440
#define DEFFPS 25
// Default Monitor
#define MAINMONITOR 0
// Default CopyMethod of Source Picture/Window
#define DEFCPYMETHOD CopyMethod::DesktopDupl
// ScreenshotName
#define BMPFILENAME "Screenshot.bmp"

#define BITDEPTH 4
#define MAXSIZE 256
#define DESKTOP "desktop"
#define MULTIPL100NS 10000000
/*********Macros for HRESULT Returns**/
#define HR_RETURN_ON_ERR(hrTemp, exp) if(FAILED(hrTemp = exp)){return hrTemp;};
#define GETHRESULT return HRESULT_FROM_WIN32(GetLastError());
#define HR_RETURN_ON_INT_ERR(exp) if(exp==0){GETHRESULT};
#define HR_RETURN_ON_NULL_ERR(exp) if(exp==NULL){GETHRESULT};
#define HR_RETURN_ON_HANDLE_ERR(exp) if(exp == INVALID_HANDLE_VALUE){GETHRESULT};
#ifdef _DEBUG
#ifndef HR
/*
* Mehrzeiliges Makro HR mit Parameter x
* Empfängt Parameter/HResult von HR_RETURN_ON_ERR
* IF FAILED: Wenn VorzeichenBit vorhanden, dann Fehler. Wenn 0 Dann kein Fehler
* Parameter von DXTraceW:
* __FILE__ Name der QuellcodeDatei
* __LINE__ Zeile in Quellcodedatei
* # Formatierung zur Zeichenkette: 0x000443 wird zu "0x000443". mit L wird "0x00443" zum WCHAR
* TRUE mit MsgBox
*/
#define HR(x) \
{ \
 HRESULT hr = x; \
 if(FAILED(hr)) \
 { \
	DXTraceW(__FILEW__,__LINE__,hr,L#x, TRUE); \
 } \
}
#endif
#endif
#ifndef HR
#define HR(x) x;
#endif
/*************************************/
#include <windows.h>
// C RunTime-Headerdateien
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <d3d12.h>
#include <d3d11.h>
#include <dxgi.h>
#include <dxgi1_2.h>
//Multithreading
#include <thread>
// d2d
#include <d2d1.h>
#include <d2d1_2.h>
#include <d2d1_1helper.h>

#include <d3dcommon.h>
#include <d3dcompiler.h>
#include <dwmapi.h>  // dwm map window
#include <cmath>
#include <string.h>
// Windows ComPtr
#include <wrl.h>
#include <wrl/client.h>
//sinkwriter
#include <mfapi.h>
#include <mfidl.h>
#include <Mfreadwrite.h>
#include <mferror.h>
// ATL-Lib
#include "atlstr.h" // A2T-Macro  ANSI to Unicode
//CoreAudio
#include <mmsystem.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <audiopolicy.h>
// directx tool kit helper classes
#include "BufferHelpers.h"
#include "CommonStates.h"
#include "DDSTextureLoader.h"
#include "DirectXHelpers.h"
#include "Effects.h"
#include "GamePad.h"
#include "GeometricPrimitive.h"
#include "GraphicsMemory.h"
#include "Keyboard.h"
#include "Model.h"
#include "Mouse.h"
#include "PostProcess.h"
#include "PrimitiveBatch.h"
#include "ScreenGrab.h"
#include "SimpleMath.h"
#include "SpriteBatch.h"
#include "SpriteFont.h"
#include "VertexTypes.h"
#include "WICTextureLoader.h"
#include "ClsTimer.h"

//using namespace Microsoft::WRL;
//using namespace ABI::Windows::Foundation;
//using Microsoft::WRL::ComPtr;