// MyWindowPrj.cpp : Definiert den Einstiegspunkt für die Anwendung.
//
#include <wrl.h>
#include <wrl/client.h>

#include <vector>
#include <fstream>  // ifstream
#include <iterator> // istream_iterator
#include <filesystem>
#include <iostream>
#include <DirectXMath.h>
#include <winstring.h>
#include <roapi.h>
#include <windows.graphics.capture.h>
#include <windows.graphics.capture.interop.h>
#include <windows.graphics.directx.direct3d11.h>
#include <windows.graphics.directx.direct3d11.interop.h>
#include <string.h>
#include "atlstr.h"

#include "framework.h"
#include "MyWindowPrj.h"
#include "ClsWndProc.h"
#include "ClsWnd.h"
#include "dxerr.h"
//#include "ChiliTimer.h"
#define STB_IMAGE_IMPLEMENTATION
#define BITDEPTH 4
#include "stb_image.h"
#include "SinkWriter.h"
#include "ClsD3D11.h"
#include "ClsBitBlt.h"

/* *INDENT-OFF* */
using namespace Microsoft::WRL;
using namespace ABI::Windows::Foundation;
using namespace ABI::Windows::Graphics;
using namespace ABI::Windows::Graphics::Capture;
using namespace ABI::Windows::Graphics::DirectX;
using namespace ABI::Windows::Graphics::DirectX::Direct3D11;
using namespace Windows::Graphics::DirectX::Direct3D11;
using Microsoft::WRL::ComPtr;

HWND g_hDuplSrcWnd;
HWND g_hWndApp;
ClsTimer timer;
struct ImgWndData;

BOOL CALLBACK EnumWindowsProc(HWND hWnd, LPARAM lParam);
void BitBltToD3D11SubResource(ImgWndData& MyImgWndData);
void BitBltToFile(ImgWndData& MyImgWndData);



ComPtr<ID3D11Device> pD3dDevice;
ComPtr<ID3D11DeviceContext> pD3dContext;

struct ImgWndData
{
public:
	ImgWndData(int iWidth, int iHeight, int iBpp)
	{
		this->iHeight = iHeight;
		this->iWidth = iWidth;
		strBmpFileName = "Blub.bmp";
		this->iBpp = iBpp;
	}
public:
	int iWidth = 0;
	int iHeight = 0;
	int iBpp = 0;
	unsigned char* pData = NULL;
	const char* strBmpFileName;
	const char strTitle[200] = "Editor";
	RGBQUAD* pRGBQUAD = NULL;
	int GetBits()
	{
		return iHeight * iWidth * iBpp;
	}
	int GetPx()
	{
		return iHeight * iWidth;
	}
	ComPtr<ID3D11Texture2D> pTexture;
	D3D11_SUBRESOURCE_DATA SubResData = {};					// Container für Texture Daten
};
/*
struct BitMapData
{
public:
	BitMapData(unsigned char* pData, int iWidth, int iHeight)
	{
		iBmpStretchHeight = iHeight;
		iBmpStretchWidth = iWidth;
		pMemOfSubResource = pData;
		strBmpFileName = "Blub.bmp";
	}
	int iBmpStretchHeight;
	int iBmpStretchWidth;
	const char* strBmpFileName;
	unsigned char* pMemOfSubResource;
	const char strTitle[200] = "Editor";

};*/
/// <summary>
/// Erstellt eine gültiges Device und DeviceContext.
/// Liefert somit gültige Adresse zum Pointer der wiederum auf ein erstelltes ComObj zeigt.
/// Dies wird für Device und DeviceContext erledigt
/// </summary>
void CreateDevice()
{
	HRESULT hResult = NULL;									// Rückgabewert von ApiFkt. für Errorhandling
	ComPtr<IDXGIDevice2> dxgiDevice;						// Smartpointer zum COM-DXGIDevice-Interface-Obj
	D3D_FEATURE_LEVEL pSelectedFeatureLvl;					// will recieve the FeatureLvl set by GPU
	D3D_FEATURE_LEVEL aFeatureLevels[] =					// FeatureLvlArray, bricht ab nachdem er eins erfolgreich bestätigt hat
	{
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_0
	};

	UINT iSizeFeaturLvl = ARRAYSIZE(aFeatureLevels);		// Size of FeatureLvl Array
	/*Übergibt alle gesetzten Parameter und speichert im pD3dDevice eine Adresse zum Pointer des DeviceObj was über das COM-IF erstellt wurde
	* Device: Obj für die Erstellung sämtliche Obj bzgl DX
	* Context: Zur Manipulation sämtlicher erstellten Obj von Device*/
	hResult = D3D11CreateDevice(
		NULL,												// Adapter, auszulesen über D3D11Adapter->EnumAdapters oder NULL: DefaultAdapter
		D3D_DRIVER_TYPE_HARDWARE,							// Hardwarebeschleunigung der GPU nutzen
		NULL,												// Handle falls Software genutzt wird (CPU statt GPU)
		D3D11_CREATE_DEVICE_BGRA_SUPPORT |					// Mit BGRA support (Blue,Green,red,alpha) 
		D3D11_CREATE_DEVICE_DEBUG,							// zusätzliche DebugMsgs im Compilerfenster
		aFeatureLevels,										// Array mit FeatureLvl,von 0 bis n, bricht bei Erfolg ab
		ARRAYSIZE(aFeatureLevels),							// Größe des FeatureLvlArrays
		D3D11_SDK_VERSION,									// Installierte DX SDK
		&pD3dDevice,										// wird Adresse zum Pointer des DeviceIFObjs speichern
		&pSelectedFeatureLvl,								// recieved FeatureLvl
		&pD3dContext										// wird Adresse zum Pointer des DeviceContextIFObjs speichern
	);
}//END-FUNC CreateDevice

/// <summary>
/// - Erstellt eine Swapchain1 anstatt Swapchain
/// - Für Swapchain1 ist eine Factory notwendig: 
///		- pD3dDevice auf dxgiDevice. Polymorphie des Ziels unseres Pointers von pD3dDevice
///		- Erstellt über dxgiDevice ein COM-IF-OBj für Adpater
///		- Parent von dxgiAdapter ist dxgiFactory
///  mit übergebenen Device
/// </summary>
/// <param name="dxgiAdapter">Referenz zum leeren ComPtr für den Adapter</param>
/// <param name="pDxgiSwapChain">Referenz zum leeren ComPtr für Swapchain</param>
void CreateSwapChain(ComPtr<IDXGIAdapter>& pdxgiAdapter, ComPtr<IDXGISwapChain1>& pDxgiSwapChain)
{
	HRESULT hResult = NULL;										// Rückgabewert von ApiFkt. für Errorhandling
	ComPtr<IDXGIDevice2> dxgiDevice;							// Smartpointer bzw Adresse zum COM-Factory-Interface-Obj
	ComPtr<IDXGIFactory2> dxgiFactory;							// Smartpointer zum COM-Factory-Interface-Obj

	DXGI_SWAP_CHAIN_DESC1 d3d11SwapChainDesc = {};				// typed Struct für die SwapchainDescription

	d3d11SwapChainDesc.Width = 0;								// use window width = 0
	d3d11SwapChainDesc.Height = 0;								// use window height = 0
	d3d11SwapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;// BGRA Format, BGRA ist StandardFormat für die Desktopdubli.
	d3d11SwapChainDesc.SampleDesc.Count = 1;					// Kantenglättung. Wieviel Pixel für ein Pixel für Glättung: 1Px für ein Px = aus
	d3d11SwapChainDesc.SampleDesc.Quality = 0;					// AntiAliasing bzw. Kantenglättung ist aus, folgt Quality 0
	d3d11SwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; // Wie wird Buffer genutzt
	d3d11SwapChainDesc.BufferCount = 1;							// 1 Backbuffer und 1 Frontbuffer
	d3d11SwapChainDesc.Scaling = DXGI_SCALING_STRETCH;			// DXGI passt BackbufferGröße an Ziel an 
	d3d11SwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;	// kopiert den Buffer mit jedem Call von device->Draw in den WindowDesktopManager
																// DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL shared mit WDM, jederzeit Zugriff ohne CopyOperation
	d3d11SwapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED; // Alphakanal/Transparenz nicht definiert
	d3d11SwapChainDesc.Flags = 0;								// z.B.: DXGI_SWAP_CHAIN_FLAG_GDI_COMPATIBLE erlaubt das GDI in DXI rendert		

	hResult = pD3dDevice->QueryInterface(IID_PPV_ARGS(&dxgiDevice));	// castet pD3dDevice-Obj in dxgiDevice
	hResult = dxgiDevice->GetAdapter(&pdxgiAdapter);						// Pointer zum Adapter-COM-IF-Obj
	hResult = pdxgiAdapter->GetParent(IID_PPV_ARGS(&dxgiFactory));		// Addresse  des Pointers zum ElternObj von Adapter  
	/*
	* &ComPtr Adresse des ComPtr Obj selbst
	* ComPtr.get() Adresse des Ziels vom IF-Pointer
	* ComPtr.getadressof() Adresse des IF-Pointers selbst
	* &ComPtr genauso wie .getadressof() mit dem Unterschied, dass das Ziel des Pointers zurückgesetzt wird. (Neues Obj wird init.)
	*/
	hResult = dxgiFactory->CreateSwapChainForHwnd(
		pD3dDevice.Get(),										// Ziel des Pointers, dass Wiederum ein Pointer auf ein COM-IF-Obj ist
		g_hWndApp,												// globaler Handle zu meinem Window
		&d3d11SwapChainDesc,									// Swapchain Description per Addresse
		nullptr,												// Description für Fullscreen
		NULL,													// Falls FullscreenDescription, hier das COMIF-Obj für den Monitor
																// per dxgiAdapter->EnumOutput(..)
		&pDxgiSwapChain											// Speichert hier die Adresse des Ziels, was ein Pointer zum COM-IF-Obj ist
	);
}//END-FUNC CreateSwapChain

// Structure für ein Array das pro Element aus diesem Struct besteht
/*
struct Vertex
{
	struct
	{
		float x;
		float y;
	} fPos;
	struct
	{
		float h;
		float v;
	} fTex;
	struct
	{
		unsigned char r;
		unsigned char g;
		unsigned char b;
		unsigned char a;
	} fcol;
};
*/
// Structure die für die init. von VertexBuffer benötigt wird
/*
struct VertexBufferContent
{
	UINT uStride = 0;
	UINT uOffset = 0;
	D3D11_BUFFER_DESC VertexBufferDesc = {};
	D3D11_SUBRESOURCE_DATA VertexSubResData = {};
	Vertex aVertices[4] = {
	-0.5f,0.5f,		0.f, 0.f,	255, 255, 0,0,
	0.5f,  0.5f,	1.f, 0.f,	0,0,0,0,
	0.5f,-0.5f,		1.f, 1.f,	0,255,0,0,
	-0.5f,-0.5f,	0.f, 1.f,	255,255,255,0
	};
	ComPtr<ID3D11Buffer> pVertexBuffer;
};
*/
/// <summary>
/// Erstellt einen VertexBuffer-Speicher. Mit CreateBuffer wird Speicher auf GPU allocated und mit SetVertexBuffers wird es auf GPU kopiert
/// </summary>
/// <param name="VBufContent">Referenz zu meinem VertexStruct, dass folgende Daten beinhaltet:
///								uStride: Größe eines Elementes im VertexArray
///								uOffset: Offset im Array
///								VertexBufferDesc: Description die benötigt wird um ein VertexBuffer auf GPU zu erstellen
///								aVertices:	Punkte für das Zeichnen der 2 Dreiecke (Quad)
///								VertexSubResourceData: beinhaltet die Daten (aVertices) 
///								</param>
void CreateVertexBuffer()
{
	VertexBufferContent VBufContent;						// Punkte
	HRESULT hResult = NULL;									// Rückgabewert von ApiFkt. für Errorhandling
	VBufContent.VertexBufferDesc.BindFlags
		= D3D11_BIND_VERTEX_BUFFER;							// Buffer soll für Vertexe verwendet werden, Vertex Buffer in IA-Stage
	VBufContent.VertexBufferDesc.Usage
		= D3D11_USAGE_DEFAULT;								// Default: GPU read and write
	VBufContent.VertexBufferDesc.ByteWidth
		= sizeof(VBufContent.aVertices);					// Gesamte Größe von aVertices oder: sizeof(Vertex*4) 80
	VBufContent.VertexSubResData.pSysMem
		= VBufContent.aVertices;							// Zuweisung der Daten in SubResourceTyp
	VBufContent.uStride = sizeof(Vertex);					// Größe pro Element: 20
	VBufContent.uOffset = 0;								// Offset, wenn er von Element zu Element springt (um z.B. erste 4 Bits zu ignorieren)

	// auf GPU allocaten
	hResult = pD3dDevice->CreateBuffer(&VBufContent.VertexBufferDesc, &VBufContent.VertexSubResData, &VBufContent.pVertexBuffer);
	// auf GPU kopieren
	pD3dContext->IASetVertexBuffers(0, 1, VBufContent.pVertexBuffer.GetAddressOf(), &VBufContent.uStride, &VBufContent.uOffset);
}//END-FUNC

// Structure die für die init. von IndexBuffer benötigt wird
/*
struct IndexBufferContent
{
	ComPtr<ID3D11Buffer> pIndexBuffer;
	D3D11_BUFFER_DESC IndexBufferDesc = {};
	D3D11_SUBRESOURCE_DATA IndexSubResData = {};

	// 2 dreiecke jeweils im uhrzeigersinn = viereck
	const unsigned short iIndices[6] =
	{
		0,1,2,
		0,2,3
	};
};
*/
/// <summary>
/// Erstellt einen IndexBuffer. Dieser Index bezieht sich auf den VertexBuffer. 
/// Um ein Viereck mit 2 Dreiecken darzustellen, benötigt man an wenigen Stellen 2 Punkte die eine identische Position haben.
/// um Speicher zu sparen wird dort nicht jedesmal ein neuer Vertex definiert, sondern bezieht sich auf ein Index.
/// Damit wird jeder Vertex nur einmal angelegt, kann aber mehrmals per Index genutzt werden
/// </summary>
/// <param name="IBufContent">Referenz zu meinem IndexStruct, dass folgende Daten beinhaltet:
///								IndexBufferDesc: Description die benötigt wird um ein IndexBuffer auf GPU zu erstellen
///								iIndices:	Indices für die Vertices 1= Vertex 1 in aVertices
///								IndexSubResData: beinhaltet die Daten (iIndices) </param>
void CreateIndexBuffer(IndexBufferContent& IBufContent)
{
	HRESULT hResult = NULL;									// Rückgabewert von ApiFkt. für Errorhandling
	IBufContent.IndexBufferDesc.BindFlags					// Buffer soll als IndexBuffer genutzt werde-> IndexBufferFlag setzen
		= D3D11_BIND_INDEX_BUFFER;							// Buffer soll für VertexIndices verwendet werden, IndexBuffer in IA-Stage
	IBufContent.IndexBufferDesc.Usage
		= D3D11_USAGE_DEFAULT;								// Default: GPU read and write
	IBufContent.IndexBufferDesc.ByteWidth
		= sizeof(IBufContent.iIndices);						// Gesamte Größe von iIndices oder: sizeof(iIndices*6)= sizeof(short)*6 = 16*6
	IBufContent.IndexBufferDesc.CPUAccessFlags = 0;
	IBufContent.IndexBufferDesc.MiscFlags = 0;
	IBufContent.IndexSubResData.pSysMem
		= IBufContent.iIndices;								// Zuweisung der Daten in SubResourceTyp
	// Reservierung des Speichers auf GPU
	hResult = pD3dDevice->CreateBuffer(						// Erstellung des IndexBuffer auf GPU
		&IBufContent.IndexBufferDesc,						// Bufferbeschreibung (als IndexBuffer)
		&IBufContent.IndexSubResData,						// Datencontainer	
		&IBufContent.pIndexBuffer);							// IF-Pointer zum IndexBufferObj

	pD3dContext->IASetIndexBuffer(							// COPY GPU zu GPU
		IBufContent.pIndexBuffer.Get(),						// setzt alle notwendigen Daten, die dann beim StageDurchgang abgefragt werden
		DXGI_FORMAT_R16_UINT, 0);							// Format der Elemente (short integer)
}//END-FUNC
/*
struct ConstantBuffer
{
	struct
	{
		float element[4][4];
	} transform;							// registered cbuffer in VertexShader (not about name, about structure and index)
};

struct ConstantBufferContent
{
	float fAngle = 0.0f;
	ConstantBuffer MyConstantBuffer =
	{
		{
			std::cos(fAngle), std::sin(fAngle), 0.0f, 0.0f,
			-std::sin(fAngle), std::cos(fAngle), 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
		}
	};
	ComPtr<ID3D11Buffer> pConstantBuffer;
};
*/
/// <summary>
/// Erstellung des ConstantBuffers: 
/// - Erstellen einer Matrix
/// - Matrix als Subresource speichern
/// - als Buffer in GPU speichern 
/// </summary>
/// <param name="CBufContent">Referenz zu meinem ConstantStruct, dass folgende Daten beinhaltet:
///								fAngle: Winkel der pro Frame geändert wird (in Matrix)
///								ConstantBuffer:	Array aus structs was Matrizen enthält (hier kein Array, da nur eine Matrix)
///								ConstantSubResData: beinhaltet die Daten (Matrizen) </param>
void CreateConstantBuffer(ConstantBufferContent& CBufConstant)
{
	HRESULT hResult = NULL;

	D3D11_BUFFER_DESC ConstantBufferDesc = {};				// Description für einen Buffer
	D3D11_SUBRESOURCE_DATA ConstantSubResData = {};			// Data Container	

	ConstantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;				// Buffer als ConstantBuffer def.
	ConstantBufferDesc.ByteWidth = sizeof(CBufConstant.MyConstantBuffer);	// Array von Matritzen, haben nur ein Element: 4x4xfloat = 4*4*4 = 64 Bytes
	ConstantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;				// wird pro Frame geändert, muss für CPU gelockt werden
	ConstantBufferDesc.MiscFlags = 0;
	ConstantBufferDesc.StructureByteStride = 0;								// Größe pro Element, wird nicht benötigt
	ConstantBufferDesc.Usage = D3D11_USAGE_DYNAMIC;							// Dynamic: CPU: write; GPU: read
	ConstantSubResData.pSysMem = &CBufConstant.MyConstantBuffer;			// Daten in Datencontainer speichern
	// Constant Buffer allocaten
	hResult = pD3dDevice->CreateBuffer(&ConstantBufferDesc, &ConstantSubResData, &CBufConstant.pConstantBuffer);
	// Hier werden nicht die Daten auf GPU kopiert, weil dies ja pro Frame passiert
	// und vorher von CPU noch manipuliert wird - Schreibrechte
}//END-FUNC
/// <summary>
/// Anstatt pro Frame alle Vertices zu ändern, wird eine Matrix pro Frame geändert, die dann auf GPU-Seite mit Vertices operiert
/// 1. weniger Daten von CPU zu GPU
/// 2. Operation zur Manipulation wird auf GPU ausgeführt. GPU ist wesentlich schneller für solche Operationen.
/// -> benötigen CPU AccessRights, d.h. Daten werden gemappt(gelockt) wenn CPU die Daten manipuliert. 
///	   GPU liest sie danach aus und kopiert sie auf VRAM
/// </summary>
/// <param name="CBufContent">Referenz zu meinem ConstantStruct, dass folgende Daten beinhaltet:
///								fAngle: Winkel der pro Frame geändert wird (in Matrix)
///								ConstantBuffer:	Array aus structs was Matrizen enthält (hier kein Array, da nur eine Matrix)
///								ConstantSubResData: beinhaltet die Daten (Matrizen) </param>
void SetConstantBuffer(ConstantBufferContent& CBufConstant)
{
	HRESULT hResult = NULL;
	CBufConstant.fAngle = timer.Peek();						// Winkeländerung pro Frame
	D3D11_MAPPED_SUBRESOURCE mappedRes;						// Datencontainer

	// 4x4 Matrix
	// 1. x-Koord
	// 2. y-Koord
	// 3. z-Koord: keine Änderung, d.h. Einheitswerte
	// 4. für Mondifikation: keine Änderung, d.h. Einheitswerte
	// 4x2 Matrix geht nicht für Berechnung -> Zur Berechnung muss Anzahl Zeile = Anzahl Spalte -> 4x4Matrix
	CBufConstant.MyConstantBuffer =
	{
		{
			std::cos(CBufConstant.fAngle), std::sin(CBufConstant.fAngle), 0.0f, 0.0f,
			-std::sin(CBufConstant.fAngle), std::cos(CBufConstant.fAngle), 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
		}
	};
	// Locken des Buffers, sodass CPU drauf schreiben kann (gpu muss mit lesen warten)
	hResult = pD3dContext->Map(
		CBufConstant.pConstantBuffer.Get(),					// Welcher Buffer soll gelockt werden
		0,													// Index für Subresource (können mehrere im Array sein)
		D3D11_MAP_WRITE_DISCARD,							// gelockt fürs schreiben, alter Inhalt wird nicht mehr definiert sein (nicht gemerkt)
		0,													// Flag setzen falls nicht gelockt werden kann und was CPU derweil tun soll
		&mappedRes);										// Datencontainer des Buffers

	ConstantBuffer* dataPtr = (ConstantBuffer*)mappedRes.pData; // Zeiger auf Daten im Datencontainer 
	memcpy(dataPtr, &CBufConstant.MyConstantBuffer, sizeof(ConstantBuffer)); // neue Daten reinschreiben
	pD3dContext->Unmap(CBufConstant.pConstantBuffer.Get(), 0); // Buffer wieder unlocken (CPU ist fertig)
	pD3dContext->VSSetConstantBuffers(0, 1, CBufConstant.pConstantBuffer.GetAddressOf()); // Manipulierten Buffer direkt in VS-Stage setzen
}//END-FUNC
/// <summary>
/// Erstellt einen Sampler, der die Texture auf das Quad (2 Dreiecke) mappen kann.
/// Benötigt für jedes Pixel Koordinaten auf Texture um die richtige Farbe darzustellen.
/// Die UV-TextureCoord werden von InputAssamblerStage (Input von Vertexdaten: Coord, UV-Maps,Farbe) bis in den PixelshaderStage übertragen
/// Siehe Input PixelShader
/// Bei der Erstellung eines Samplers hat dieser eine feste Position in der GPU.
/// Sampler kann somit im Pixelshader wie folgt angesprochen werden: register(s0) s=Sampler, 0= Index im PixelShader.hlsl
/// </summary>
void CreateAndSetSampler()
{
	HRESULT hResult = NULL;
	// Create Sampler State
	D3D11_SAMPLER_DESC samplerDesc = {};
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;	// punktuelles Mapping (pro Punkt eine Koordinate),wird nicht interpoliert/berechnet
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;	// Freiräume bekommen eine Farbe, es wird die Texture nicht gespiegelt etc.
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	samplerDesc.BorderColor[0] = 1.0f;						// Farbe bei Freiräumen
	samplerDesc.BorderColor[1] = 1.0f;
	samplerDesc.BorderColor[2] = 1.0f;
	samplerDesc.BorderColor[3] = 1.0f;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;	// Verhalten von alten zu neuen Daten, z.B. bei Desktopdupl nur mappen wo Änderung

	ComPtr<ID3D11SamplerState> samplerState;								// Sampler IF-Pointer
	hResult = pD3dDevice->CreateSamplerState(&samplerDesc, &samplerState);	// Sampler auf GPU allocaten
	pD3dContext->PSSetSamplers(0, 1, samplerState.GetAddressOf());			// Sampler setzen (COPY von GPU zu GPU)
}//END-FUNC
/// <summary>
/// Generiert ein Bild und speichert es als SubResourceData.
/// Erstellung eines Texture-Obj per D3D11 CreateFunktion
/// Erstellung eines IF-Pointers zu einem TexturObj. Obj enthält SubResourceData
/// </summary>
/// <param name="ImgData">Struct mit ImgDaten: Breite, Höhe, Bytes pro Pixel, Pointer zum IF-Pointer</param>
void GenerateD3D11Texture(ImgWndData& ImgData)
{
	int iWidth = ImgData.iWidth;											// Übergabe an lokale Variablen, wegen der Übersicht
	int iHeight = ImgData.iHeight;
	int iBpp = ImgData.iBpp;
	//ImgData.pRGBQUAD = new RGBQUAD[ImgData.GetPx()];

	ImgData.pData = new unsigned char[ImgData.GetBits()];					// DatenBuffer für PixelDaten; GetBits = Width*Height*Bpp
	ImgData.pRGBQUAD = (RGBQUAD*)ImgData.pData;
	unsigned char* pData = ImgData.pData;
	D3D11_TEXTURE2D_DESC TextureDesc = {};									// TextureDescription für TextureObj

	for (int y = 0; y < ImgData.iHeight; ++y)
	{
		for (int x = 0; x < ImgData.iWidth; ++x)
		{
			// y = 0;x = 0;
			// 0*100 + 0*4 = 0 an stelle 0 wird ein 0xff wert eingfügt
			// 0*100 + 0*4 +1 = 1 an stelle 1 (9. Bit bzw 2. Byte) wird ein 0x00 eingefügt
			// 0*100 + 0*4 +2 = 2 an Stelle 2 (17. Bit bzw. 3 Byte) wird ein 0x00 eingefügt
			// Hexwerte mit max ff = 8 bit = 1 Byte. Heißt  1 Stelle sind  8 Bit
			// red + green + blue + alpha = 4 Byte, deswegen auch 100* 100*4
			// 4 byte/Pixel
			// ein Charakter ist 1 Byte groß. 1 Byte = 8 bit = 0xFF = 1111 1111
			if (x <= 1000)
			{
				ImgData.pRGBQUAD->rgbRed = pData[y * iWidth * iBpp + x * iBpp] = 0xff; // red, 8 Bit
				ImgData.pRGBQUAD->rgbGreen = pData[y * iWidth * iBpp + x * iBpp + 1] = 0xff; // green, 8 Bit
				ImgData.pRGBQUAD->rgbBlue = pData[y * iWidth * iBpp + x * iBpp + 2] = 0x00; // blue, 8 Bit
				ImgData.pRGBQUAD->rgbReserved = pData[y * iWidth * iBpp + x * iBpp + 3] = 0xff; // alpha, 8 Bit
			}
			else
			{
				ImgData.pRGBQUAD->rgbRed = pData[y * iWidth * iBpp + x * iBpp] = 0xff; // red, 8 Bit
				ImgData.pRGBQUAD->rgbGreen = pData[y * iWidth * iBpp + x * iBpp + 1] = 0x00; // green, 8 Bit
				ImgData.pRGBQUAD->rgbBlue = pData[y * iWidth * iBpp + x * iBpp + 2] = 0x00; // blue, 8 Bit
				ImgData.pRGBQUAD->rgbReserved = pData[y * iWidth * iBpp + x * iBpp + 3] = 0xff; // alpha, 8 Bit
			}//END-IF
		}//END-FOR x

	}//END-FOR y

	//BitMapData BitMapData(ImgData.pData, iDeltaX, iDeltaY);	// Pointer of SubResource.Data, TargetResolution for new Bmp
	//BitBltToFile(BitMapData);
	//BitBltToD3D11SubResource(BitMapData);					// Übergebe Target
	//ProgressingSinkWriter(BitMapData.pDataOFSubResource);
	TextureDesc.Width = iWidth;							// Width
	TextureDesc.Height = iHeight;							// Height
	TextureDesc.MipLevels = 1;								// MipLvl 0/1 = aus
	TextureDesc.ArraySize = 1;								// ArraySize der Subresource, da nur 1. Adresse des 1. Elements übergeben wird = 1
	TextureDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;		// Bildformat: Desktopdupl. ist immer BGRA
	TextureDesc.SampleDesc.Count = 1;
	TextureDesc.Usage = D3D11_USAGE_DYNAMIC;// D3D11_USAGE_DEFAULT;				// Default: Read/Write für GPU
	TextureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;		// wird an VertexShader-Stage gebunden
	TextureDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;							// keine CPU-Rights notwendig

	//D3D11_SUBRESOURCE_DATA SubResData = {};					// Container für Daten

	ImgData.SubResData.pSysMem = (const void*)pData;				// Daten in Container speichern
	ImgData.SubResData.SysMemPitch = iWidth * iBpp;				// Eine Pixelreihe als Width*4: 1 Pixel = 4 Bytes
	ImgData.SubResData.SysMemSlicePitch = 0;						// nur wichtig bei 3D-Texturen

	// TextureObj wird erstellt (geerbt von IF). pTexture ist nun Pointer zum IF-Pointer.
	pD3dDevice->CreateTexture2D(&TextureDesc, &ImgData.SubResData, &ImgData.pTexture);
}//END-FUNC
/// <summary>
/// Compiliert ein ShaderProgramm für die GPU
/// Blob-IF-Pointer hat Zugriff auf Funktionen des Obj, was die compilierten ShaderProgrammDaten als Buffer enthält.
/// Erstellen des Programmes auf GPU mit CreateVertexShader: 
/// IF-Pointer greift auf Fkt. GetBufferPointer zu, die Pointer zu den VertexShaderProgr liefert
/// Es wird ein IF-Pointer zum erstellten VertexShader-Obj erstellt: pVertexShader
/// MitVSSetShader wird COPY von GPU zu GPU ausgeführt 
/// (Erstellter Shader wird intern auf GPU auf Speicher kopiert wo er ausgeführt werden kann)
/// </summary>
/// <param name="vsBlob">Referenz zum leeren ComPtr für ShaderProgrammDaten, wo sie gespeichert werden sollen</param>
/// <returns></returns>
int CreateVetexShader(ComPtr<ID3DBlob>& vsBlob)
{
	ComPtr<ID3D11VertexShader> pVertexShader;				// IF-Pointer zum VertexShaderProgramm auf GPU
	ComPtr<ID3DBlob> pShaderCompileErrorsBlob;				// IF-Pointer zum Error
	HRESULT hResult = D3DCompileFromFile(
		L"VertexShader1.hlsl",								// ShaderProgrammCode
		nullptr,
		nullptr,
		"main",												// EntryPoint zur MainFunktion im ShaderProgramm
		"vs_5_0",											// vordefinierte Strings die angeben was wir compilieren
		0,													// Flags
		0,													// Flags
		&vsBlob,											// IF-Pointer, hat Zugriff auf compilierte Programmdaten
		&pShaderCompileErrorsBlob);							// IF-Pointer, hat Zugriff auf CompilerErrors
	if (FAILED(hResult))									// falls was schiefgeht, CompilerErrors ausgeben
	{
		const char* errorString = NULL;
		if (hResult == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
			errorString = "Could not compile shader; file not found";
		else if (pShaderCompileErrorsBlob.Get()) {
			errorString = (const char*)pShaderCompileErrorsBlob->GetBufferPointer();
			pShaderCompileErrorsBlob->Release();
		}//END-ELSE hResult
		MessageBoxA(0, errorString, "Shader Compiler Error", MB_ICONERROR | MB_OK);
		return 0;
	}//END-IF FAIL

	hResult = pD3dDevice->CreateVertexShader(
		vsBlob->GetBufferPointer(),							// IF-Pointer zu den compilierten Programmdaten 
		vsBlob->GetBufferSize(),							// Größe der Daten
		nullptr,
		&pVertexShader);									// IF-Pointer, indirekter Zugriff auf ContainerObj für VertexShaderProgramm 
	assert(SUCCEEDED(hResult));
	pD3dContext->VSSetShader(pVertexShader.Get(), nullptr, 0); // COPY intern von GPU(creationplace) zu GPU(runplace)
	return 1;
}//END-FUNC
/// <summary>
/// Compiliert ein ShaderProgramm für die GPU
/// Blob-IF-Pointer hat Zugriff auf Funktionen des Obj, was die compilierten ShaderProgrammDaten als Buffer enthält.
/// Erstellen des Programmes auf GPU mit CreatePixelShader: 
/// IF-Pointer greift auf Fkt. GetBufferPointer zu, die Pointer zu den PixelShaderProgr liefert
/// Es wird ein IF-Pointer zum erstellten PuixelShader-Obj erstellt: pPixelShader
/// MitPSSetShader wird COPY von GPU zu GPU ausgeführt 
/// (Erstellter Shader wird intern auf GPU auf Speicher kopiert wo er ausgeführt werden kann)
/// </summary>
/// <param name="vsBlob">Referenz zum leeren ComPtr für ShaderProgrammDaten, wo sie gespeichert werden sollen</param>
/// <returns></returns>
int CreatePixelShader(ComPtr<ID3DBlob>& psBlob)
{
	ComPtr<ID3D11PixelShader> pPixelShader;					// IF-Pointer zum VertexShaderProgramm auf GPU
	ComPtr<ID3DBlob> pShaderCompileErrorsBlob;				// IF-Pointer zum Error
	HRESULT hResult = D3DCompileFromFile(
		L"PixelShader.hlsl",
		nullptr,
		nullptr,
		"main",												// EntryPoint zur MainFunktion im ShaderProgramm
		"ps_5_0",											// vordefinierte Strings die angeben was wir compilieren
		0,													// Flags
		0,													// Flags
		&psBlob,											// IF-Pointer, hat Zugriff auf compilierte Programmdaten
		&pShaderCompileErrorsBlob);							// IF-Pointer, hat Zugriff auf CompilerErrors
	if (FAILED(hResult))									// falls was schiefgeht, CompilerErrors ausgeben
	{
		const char* errorString = NULL;
		if (hResult == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
			errorString = "Could not compile shader; file not found";
		else if (pShaderCompileErrorsBlob) {
			errorString = (const char*)pShaderCompileErrorsBlob->GetBufferPointer();
			pShaderCompileErrorsBlob->Release();
		}
		MessageBoxA(0, errorString, "Shader Compiler Error", MB_ICONERROR | MB_OK);
		return 0;
	}

	hResult = pD3dDevice->CreatePixelShader(
		psBlob->GetBufferPointer(),							// IF-Pointer zu den compilierten Programmdaten 
		psBlob->GetBufferSize(),							// Größe der Daten
		nullptr,
		&pPixelShader);										// IF-Pointer, indirekter Zugriff auf ContainerObj für VertexShaderProgramm 
	assert(SUCCEEDED(hResult));
	pD3dContext->PSSetShader(pPixelShader.Get(), nullptr, 0); // COPY intern von GPU(creationplace) zu GPU(runplace)
	return 1;
}//END-FUNC
/// <summary>
/// Erstellt Metadaten für die Inputparameter des VertexShaders
/// - Legt Typ und Größe der Inputparamter fest und übergibt sie an erste Stage (InputAssambler)
/// - Übergibt nicht selbst die InputParamter:
///		passiert über die ganzen Createfunktionen, womit Daten auf richtigen Speicherort auf GPU gespeichert werden
///		die dann vom ShaderProgramm abgerufen werden können
/// - Übergabe des compilierten ShaderProgramms über die erste Stage: InputAssambler
/// </summary>
/// <param name="vsBlob">compiliertes ShaderProgramm, beinhaltet Pointer und Größe</param>
void CreateInputLayout(ComPtr<ID3DBlob>& vsBlob)
{
	HRESULT hResult = NULL;
	ComPtr<ID3D11InputLayout> pInputLayout;					// IF-Pointer zum Layout

	const D3D11_INPUT_ELEMENT_DESC InputElementDesc[] =
	{
		{"POS",0,DXGI_FORMAT_R32G32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0},
		{"TEX",0,DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{"COL",0,DXGI_FORMAT_R8G8B8A8_UNORM,0,D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0}
		// Nachdem die Werte den Vertexshader verlassen, müssen sie normalized sein
		// mit UNORM sind sie sogar schon normalized wenn sie in den Vertexshader reinkommen
		// ABER: die Bitgröße muss  beachtet werden, wenn die Color aus 4 floats besteht (je 32 bit), dann darf ich kein DXGI_FORMAT_R16G16B16A16_UNORM verweden
		// unsigned char ist 8 bit groß. Pro Farbe RGBA/BGRA. Ergibt 4*8Bit = 32Bit: DXGI_FORMAT_R8G8B8A8_UNORM

	};
	// Damit InputAssambler weiss was er machen soll mit den Daten wenn er fertig ist.
	// Er sendet sie an die Adresse des VertexShaders die hier mitgegeben wird
	hResult = pD3dDevice->CreateInputLayout(
		InputElementDesc,									// Beschreibung der Inputparameter für ShaderProgramm
		ARRAYSIZE(InputElementDesc),						// Anzahl der Elemente
		vsBlob->GetBufferPointer(),							// Pointer zum kompilierten Shaderprogramm. Befindet sich bereits auf VRAM
		vsBlob->GetBufferSize(),							// Größe des ShaderProgramms
		&pInputLayout);										// IF-Pointer zum LayoutObj
	assert(SUCCEEDED(hResult));
	pD3dContext->IASetInputLayout(pInputLayout.Get());		// COPY von GPU zu GPU (creationplace to executeplace)
}//END-FUNC

int CreateDesktopDuplication(ComPtr<IDXGIOutputDuplication>& pDeskDupl, DXGI_OUTDUPL_DESC& OutputDuplDesc, ComPtr<IDXGIAdapter>& pDxgiAdapter)
{
	HRESULT hResult = NULL;
	ComPtr<IDXGIOutput> pDxgiOutput;
	ComPtr<IDXGIOutput1> pDxgiOutput1;

	hResult = pDxgiAdapter->EnumOutputs(0, &pDxgiOutput);
	if (FAILED(hResult))
		return 0;

	pDxgiOutput->QueryInterface(IID_PPV_ARGS(&pDxgiOutput1));// Es gibt 2 IF die ein DxgiOutput erbt/implementiert. 
															// Bei der Erstellung eines Objs wird es über eine CreateFunktion erstellt, per Polymorphism:
															// Je nachdem ob Obj per Polym über Output1 oder Output 2 erstellt wird überschreibt es unterschiedliche Funktionen
															// QuaryInterface ist eine art CrossCast: IFOutput1 und IFOutput2 erben von der selben Klasse,
															// def. aber unterschiedliche leere virtuelle Funktionen
															// Interface
															// |     |
															// IF1   IF2
															// |      |
															// HiddenObjClass(MyIFPtr)
															// Create: Interface*ptr; MyIFPtr = new IF2; 
	if (!pDxgiOutput1)
		return 0;


	hResult = pDxgiOutput1->DuplicateOutput(				// Creates a complete Duplication of a Output/Monitor 
		pD3dDevice.Get(),
		&pDeskDupl);										// pDeskDupl beinhalted Zugriff auf Frames etc.
	pDeskDupl->GetDesc(&OutputDuplDesc);					// Description des Monitors wie Auflösung etc.
	return 1;
}


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
	//ComPtr<ID3D11Device> pD3dDevice;
	//ComPtr<ID3D11DeviceContext> pD3dContext;
	ComPtr<IDXGISwapChain1> pDxgiSwapChain;
	ComPtr<IDXGIAdapter> pDxgiAdapter;

	UINT uiHeight = 0;								// Auflösungsparameter
	UINT uiWidth = 0;
	HRESULT hResult;										// Rückgabewert für Win32Api Funktionen
	UNREFERENCED_PARAMETER(hPrevInstance);					// Nicht referenzierte Parameter Warning unterdrücken
	UNREFERENCED_PARAMETER(lpCmdLine);
	HMONITOR hMon;
	char strFileName[256] = "";
	char strWndTitle[256] = "";


	/**********WindowCreation************/
	ClsWnd oMyWnd(											// führt CreateWindowEx aus, d.h. erzeugt Fenster die reg. sind. 
															// WNDCLASS und Register wird bereits beim Start des Programms ausgeführt
		L"MyProgramm");									    // Titel

	oMyWnd.SetVisibility(true);								// Fenster sichtbar machen
	g_hWndApp = oMyWnd.GetHWND();

	uiWidth = oMyWnd.GetResolution().GetWidth();
	uiHeight = oMyWnd.GetResolution().GetHeight();
	BitBltData MyBitBltData(uiWidth, uiHeight, BITDEPTH, g_hWndApp);

	strcpy_s(strFileName, "Screenshot.bmp");
	strcpy_s(strWndTitle, "Editor");
	MyBitBltData.SetBmpFileName(strFileName);
	MyBitBltData.SetSrcWndTitle(strWndTitle);

	ClsD3D11 oD3D11(&MyBitBltData);
	ClsBitBlt oWin32GDI(&MyBitBltData);
	oD3D11.CreateDevice();
	oD3D11.CreateSwapChain();
	oD3D11.CreateSwapChainBuffer();
	oD3D11.CreateVertexBuffer();
	oD3D11.CreateIndexBuffer();
	oD3D11.CreateConstantBuffer();
	oD3D11.CreateAndSetSampler();

	oWin32GDI.GenerateBitBltData();
	oD3D11.BitBltToD3D11Texture();

	oWin32GDI.BitBltToFile();

	oD3D11.CreateShaderView();

	oD3D11.CreateVetexShader();
	oD3D11.CreatePixelShader();
	oD3D11.CreateInputLayout();
	oD3D11.InitDesktopDuplication();

	while (oMyWnd.RunMsgLoop())								// Pro Durchgang wird ein FrontBuffer oder VertexShaderView gezeigt
															// Haben Buffer für HWND -> selbe HZ-Zahl wie Windows selbst.
															// WindowsRefreshrate = 165Hz: 165 Durchgänge pro Sek
	{
		oWin32GDI.GetBitBltDataFromWindow();
		oD3D11.BitBltDataToD3D11SubResource();
		//oD3D11.OverrideTextureWithDeskDupl();
		oD3D11.SetConstantBuffer();

		RECT MyAppRect = oWin32GDI.GetTargetWndRect();
		oD3D11.PresentTexture(MyAppRect);

	}


	oD3D11.ReleaseAll();
	oWin32GDI.ReleaseAll();

	return 0;




















	/**********Device and Swapchain******/
	CreateDevice();
	CreateSwapChain(pDxgiAdapter, pDxgiSwapChain);
	/**********RenderTargetView**********/
	ComPtr<ID3D11Texture2D> pBackBuffer;					// Ressourcen-Buffer Smartpointer
	ComPtr<ID3D11RenderTargetView> pTarRenderView;			// RenderTargetView
	hResult = pDxgiSwapChain
		->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));			// 1:param: 0 für BackBuffer, 2.: Adresse des BackBuffers in pBackBuffer speichern
	hResult = pD3dDevice->CreateRenderTargetView(			// Erstellt eine View auf Backbuffer, können nur Views an Pipeline binden
															// View wird später per OMSetRenderTarget an OutputMerger-Stage gebunden
		pBackBuffer.Get(),									// Wert des Pointers, also Adresse des IF-Pointers
		0,													// Description für View, NULL wenn 1. Param kein Mipmap hat
		&pTarRenderView);									// Übergabe der Adresse des Ptrs der auf IF-Pointer zeigen soll.
															// In CreateFkt wird nur Ziel des Ptrs geändert, nicht Adresse selbst (geht ja nicht)
	/**********Buffers*******************/
	CreateVertexBuffer();									// Erstellung von 4 Punkten auf GPU
	IndexBufferContent IBufContent;
	CreateIndexBuffer(IBufContent);							// IndexBuffer, bezieht sich auf VertexBuffer, Zugriff per Index auf Punkte 
															// Bei 2 Dreiecken, die ein Quad darstellen soll gibt es 2 Punkte mit identischer Position
															// Per IndexZugriff spart man sich den Speicher für 2 Punkte		
	ConstantBufferContent CBufConstant;
	CreateConstantBuffer(CBufConstant);						// Funktion/Matrix die pro Frame an GPU übergeben wird, Manipulation des Quads (Winkel)
	/**********Sampler*******************/
	CreateAndSetSampler();									// Greift per Koordinaten (UV-Mapping) auf Texture zu 
															// und setzt die entsprechende Farbe des Pixels innerhalb des Quads
															// Bei der Erstellung eines Samplers hat dieser eine feste Position in der GPU.
															// Sampler kann somit im Pixelshader wie folgt angesprochen werden: 
															// register(s0) s=Sampler, 0= Index im PixelShader.hlsl
	/**********ImageProcessing***********/
	// Texture für Sampler-Stage (in PixelShader-Stage) erstellen
	ImgWndData MyImgWndData(uiWidth, uiHeight, 4);

	/*BitMapData BitMapData(ImgData.pData, uiWidth, uiHeight);	// Pointer of SubResource.Data, TargetResolution for new Bmp
	BitBltToFile(BitMapData);
	BitBltToD3D11SubResource(BitMapData);					// Übergebe Target
	ProgressingSinkWriter(BitMapData.pDataOFSubResource);*/
	GenerateD3D11Texture(MyImgWndData);					    // generiert ein zweifarbiges Img mit entsprechender Auflösung und Bittiefe (4Byte = 32Bit)
															// und erstellt aus Img daten eine Subresource, diese Subresource wird genutzt 
															// um eine D3D11 Texture zu erstellen
	BitBltToFile(MyImgWndData);
	//BitBltToD3D11SubResource(MyImgWndData);					// Übergebe Target
	/**********ShaderView****************/
	ComPtr<ID3D11ShaderResourceView> ShaderView;
	// Erstellung einer ShaderView, diese View kann direkt an den PixelShader-Stage gebunden (per Adresse) und angezeigt werden
	// pTexture enthält bis jetzt nur die generierte Texture von GenerateD3D11Texture(), noch kein WindowsAbbild
	hResult = pD3dDevice->CreateShaderResourceView(MyImgWndData.pTexture.Get(), nullptr, &ShaderView);
	pD3dContext->PSSetShaderResources(0, 1, ShaderView.GetAddressOf()); // beinhaltet unsere Texture, welche unser ImgTexture oder DesktopDuplTexture beinhaltet
	/**********ShaderCompile*************/
	ComPtr<ID3DBlob> vsBlob;
	if (!CreateVetexShader(vsBlob))							// Erstellung eines compilierten ShaderProgramms was auf GPU gespeichert wird
															// Die Metadaten für die Input-Parameter und Pointer zum VertexShader 
															// werden bereits in der ersten Stage mit übergeben (InputLayout an InputAssambler-Stage übergeben)
		return 1;
	ComPtr<ID3DBlob> psBlob;
	if (!CreatePixelShader(psBlob))
		return 1;
	/**********InputLayout***************/
	CreateInputLayout(vsBlob);								// Legt Typ und Größe der InputParameter für Shader fest, übergibt Shader an IA-Stage
	/**********DESKTOP DUPLICATION*******/
	ComPtr<IDXGIOutputDuplication> pDeskDupl;
	DXGI_OUTDUPL_DESC OutputDuplDesc = {};

	CreateDesktopDuplication(								// Erstellt IF-Output-Pointer (Monitor) über dxgiAdapter. Castet es auf OutputType1.
		pDeskDupl,											// OutputType1 bietet Funktion, welche eine DesktopDupl liefert
		OutputDuplDesc,										// Description wie Auflösung etc.
		pDxgiAdapter);										// Adapter muss mit übergeben werden, um Outputs zu bestimmen
	/************GET FRAME OF DESKTOP*********/
	ComPtr<IDXGIResource> pDxgiDesktopResource;
	DXGI_OUTDUPL_FRAME_INFO lFrameInfo;
	ComPtr<ID3D11Texture2D> pD3dAcquiredDesktopImage;

	PrepareSinkWriter();

	while (oMyWnd.RunMsgLoop())								// Pro Durchgang wird ein FrontBuffer oder VertexShaderView gezeigt
															// Haben Buffer für HWND -> selbe HZ-Zahl wie Windows selbst.
															// WindowsRefreshrate = 165Hz: 165 Durchgänge pro Sek
	{
		// ShaderResourceView wird pro Loop angezeigt
		// in MyImgWndData ist eine D3D11Texture die an diese View gebunden ist
		// diese D3D11Texture bekommt pro Durchgang neue BildDaten aus entsprechenden Fenster
		BitBltToD3D11SubResource(MyImgWndData);				// Übergebe Target
		// todo bis hier her funktionen in klassen eingebunden:
		// alles mit dx in ClsD3d11, alles mit bitblt in ClsBitblt:
		// klassen sind von einander abhängig: gemeinsamer struct: MyBitBltData
		// wird inclsBitBlt erstellt: 1. generation eines 2farbbildes GenerateBitBltData()
		//							  2. PixelDaten eines Fensters auslesen und als BitBlt speichern: GetBitBltDataFromWindow()
		// wird in clsD3D11 in d3d11texture bzw. subresource konvertiert
		// -> reihenfolge der jeweilgen methodenAufrufe der 2 klassen wichtig
		ProgressingSinkWriter(MyImgWndData.pData);
		hResult = pDeskDupl->AcquireNextFrame(				// Fragt Frame von DesktopDupl ab, d.h. von Hauptmonitor (siehe CreateDesktopDuplication)
			500,											// wielange ist dieses Frame aktuell. Muss noch beim nächsten Durchlauf verfügbar sein
			&lFrameInfo,									// MouseCurserInformationen, letzte aktualisierung des DesktopFrames seitens DWM
			&pDxgiDesktopResource);							// Container für ImgDaten

		hResult = pDxgiDesktopResource->QueryInterface(IID_PPV_ARGS(&pD3dAcquiredDesktopImage)); // low (hardwarenah) zu higher(apinah) cast
		/*********Override Texture**********/
		// Überschreibung meiner ImgProcessingD3D11Texture mit DesktopD3D11Texture
		// Texture ist D3D11Texture und in ShaderView gebunden ShaderView ist per Adresse an den PixelShader gebunden
		//pD3dContext->CopyResource(ImgData.pTexture.Get(), pD3dAcquiredDesktopImage.Get());
		/**********ConstantBuffer***********/
		SetConstantBuffer(CBufConstant);					// wurde mit CreateConstantBuffer erstellt und hier wird er (Matrix) modifiziert
		/**********Clear Frame**************/
		float Color[4] = { 0.3f, 1.0f, 0.3f, 1.0f };		// Farbe mit der das Frame gecleared wird
		pD3dContext->ClearRenderTargetView(					// Backbuffer wird nun gecleared. Dieser ist per CreateRenderTargetView per Adresse an pTarRenderView gebunden
			pTarRenderView.Get(),
			Color);											// Farbe die gesetzt werden soll
		/**********Input Assembler**********/
		pD3dContext->IASetPrimitiveTopology(
			D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);			// Legt interpretation der eingehenden Daten/Vertices fest: 
															// Punkte, Linie, Linien, Dreicke, zusammenhängende Dreiecke etc.
		//pD3dContext->IASetIndexBuffer(IBufContent.pIndexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);
		/**********Set ViewportArea*********/
		RECT winRect;										// Container für WindowClientArea. Größe des Fensters ohne Menu und Borders. (left,top,right,bottom)
		GetClientRect(oMyWnd.GetHWND(), &winRect);			// WindowClientArea abfragen
		// Bindet die View. Damit später der Rasterizer weiss wieviel max. dargestellt werden soll
		D3D11_VIEWPORT viewport = { 0.0f, 0.0f, (FLOAT)(winRect.right - winRect.left), (FLOAT)(winRect.bottom - winRect.top), 0.0f, 1.0f };
		pD3dContext->RSSetViewports(1, &viewport);			// Die Area wo Rasterizer zeichnet, genau definieren
		//RECT sicRect = { 0, 0, uiWidth, uiHeight };
		//pD3dContext->RSSetScissorRects(1, &sicRect);		// der zu refreshende Bereich innerhalb der gebundenen WindowClientArea
		/********** Output Merger **********/
		/* Erstellt das sichtbare Bild aus:
		* Vertexshader: Dargestellte Form (Übergabe an PixelShader)
		* PixelShader: Sampler mit UV-Mapping. Mapped Texture auf dargestellte Form. (Übergabe an OutputMerger)
		* Backbuffer/TargetRenderView: Hintergrund/ClearColor (direktes setzen im OutputMerger)
		* DephstencilView: Bei 3D, welche Ebene ist weiter vorn und sichtbar und welche nicht (direktes setzen im OutputMerger)*/
		pD3dContext->OMSetRenderTargets(1, pTarRenderView.GetAddressOf(), 0);
		/********** Draw *******************/
		/*Durchläuft alle Stages und die vorher hinterlegten Daten in den jeweiligen Stages werden brücksichtigt*/
		//pD3dContext->Draw(6, 0u);
		pD3dContext->DrawIndexed(							// verwenden Indices fürs zeichnen
			(UINT)std::size(IBufContent.iIndices),			// sizeof(iIndices/iIndices[0]); 12/2 = 6; 12Byte/2Byte (2Byte/16Bit = short int)
			0,												// StartIndex
			0												// IndexAddition: bevor mit jedem Index im VertexArray gesucht wird, wird zum Index 1 addiert
		);

		pDxgiSwapChain->Present(
			1,												/* Wie das fertige Bild in den FrontBuffer geladen wird:
															Für die BitBlockÜbertragung DXGI_SWAP_EFFECT_DISCARD oder DXGI_SWAP_EFFECT_SEQUENTIAL
															0:	keine Sync. Bild wird in den FrontBuffer geladen sobald es fertig ist
															1-4: Anzahl der VBlanks die abgewartet werden bis neues Bild in den FrontBuffer geladen wird
															*/
			0												// Flags: z.B. Tearing erlauben, neuladen eines Bildes, ohne VBlanks zu berücksichtigen
		);
		hResult = pDeskDupl->ReleaseFrame();				// Frame wieder freigeben, Abfrage beendet
	}//END-WHILE Window/Frame-Loop
	FinalizeSinkWriter();
	delete MyImgWndData.pData;
	MyImgWndData.pData = nullptr;
	MyImgWndData.pRGBQUAD = nullptr;
	return 0;
}//END-FUNC Main

/***************GPU ACCESS TEXTURE ONLY**********/
	/*
	ComPtr<ID3D11Texture2D> lGDIImage;
	// Create GUI drawing texture
	lDeskDupl->GetDesc(&lOutputDuplDesc);
	D3D11_TEXTURE2D_DESC GPUTextureDesc = {};
	GPUTextureDesc.Width = lOutputDuplDesc.ModeDesc.Width;
	GPUTextureDesc.Height = lOutputDuplDesc.ModeDesc.Height;
	GPUTextureDesc.Format = lOutputDuplDesc.ModeDesc.Format;
	GPUTextureDesc.ArraySize = 1;
	GPUTextureDesc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_RENDER_TARGET;
	GPUTextureDesc.MiscFlags = D3D11_RESOURCE_MISC_GDI_COMPATIBLE;
	GPUTextureDesc.SampleDesc.Count = 1;
	GPUTextureDesc.SampleDesc.Quality = 0;
	GPUTextureDesc.MipLevels = 1;
	GPUTextureDesc.CPUAccessFlags = 0;
	GPUTextureDesc.Usage = D3D11_USAGE_DEFAULT;

	hResult = pD3dDevice->CreateTexture2D(&GPUTextureDesc, NULL, &lGDIImage);

	// Copy image into GDI drawing texture (gpu to gpu)


		//d3dContext->CopyResource(lGDIImage.Get(), lAcquiredDesktopImage.Get());
	*/

	/// <summary>
	/// Bekommt als Input einen Handle zum Fenster.
	/// Liest das DisplayDC des Fensters aus
	/// Erstellt aus DisplayDC ein MemDC.
	/// Erstellt ein HBITMAP im Speicher mit passenden Metadaten, und bindet es an das MemDC
	/// Liest Pixeldaten in das HBITMAP ein mit StretchBlt bzw. BitBlt
	/// Metadaten vom HBitmap-Handle auslesen
	/// Mit GetBitmapBits Pixeldaten ins Bitmap-Obj kopieren
	/// Mit Memcpy BitmapDaten in D3D11-Texture-Typ kopieren
	/// </summary>
	/// <param name="MyBitMapData">Struktur: - Titel des Fenster enthält um das Handle zu bestimmen
	///										 - Neue gestreckte Maße (Höhe und Breite)
	///										 - Subressource der D3D11Texture
	/// </param>
void BitBltToD3D11SubResource(ImgWndData& MyImgWndData)
{
	DWORD ulWidth, ulHeight = 0;
	UINT uiPixelDataSize = 0;

	RECT WindowClientRect = {};										// RECT for ResolutionInfo
	BITMAP myBitmap = {};
	MyHandle SrcWnd = {};

	HANDLE hDIB = NULL;
	HBITMAP hBitmap = NULL;
	HDC hDisplayDC = NULL;
	HDC hMemDC = NULL;
	BYTE* pImgData = NULL;

	strcpy_s(SrcWnd.title, MyImgWndData.strTitle);					// Titel des Fensters
	EnumWindows(EnumWindowsProc, reinterpret_cast<LPARAM>(&SrcWnd));// WindowHandle mit SrcWnd.title finden
	hDisplayDC = GetDC(SrcWnd.hWndSrc);								// DC des Fensterhandles

	GetClientRect(SrcWnd.hWndSrc, &WindowClientRect);				// Auflösung des Fensters
	ulWidth = WindowClientRect.right - WindowClientRect.left;		// Breite
	ulHeight = WindowClientRect.bottom - WindowClientRect.top;		// Höhe

	hMemDC = CreateCompatibleDC(hDisplayDC);						// MemoryDC aus DisplayDC erstellen und Handle darauf zurückbekommen
	hBitmap = CreateCompatibleBitmap(								// Hier werden nur Metadaten gesetzt, keine Pixeldaten kopiert
		hDisplayDC,													// Memory Bitmap erstellen, aus Bitmap  des DisplayDC. 
		MyImgWndData.iWidth,										// aber mit größerer Auflösung
		MyImgWndData.iHeight
	);
	SelectObject(hMemDC, hBitmap);									// Ersetzen des 1x1Px Platzhalters im MemDC durch mein Bitmap

	StretchBlt(														// Kopieren der BitmapDaten die als Bitmap-Unterobj in den DCs existieren
		hMemDC,														// Ziel: MemoryDC
		0,															// X (0,0 ist oben links)
		0,															// Y (0,0 ist oben links) Wenn 500, ersten 500 pixel von oben aus wären dann schwarz
		MyImgWndData.iWidth,										// Breite ab X
		MyImgWndData.iHeight,										// Höhe ab Y
		hDisplayDC,													// Source hDisplayDC, also der DisplayDeviceContext
		0,															// X (0,0 ist oben links)
		0,															// Y (0,0 ist oben links)
		ulWidth,													// Breite ab X
		ulHeight,													// Höhe ab Y
		SRCCOPY);													// Flags: SRCOPY: kopiert Viereck der Quelle ins Viereck des Ziels
																	// -> habe nun fertiges MemoryDC mit fertigem, gefülltem Bitmap 

	GetObject(hBitmap, sizeof(myBitmap), (LPVOID)&myBitmap);		// Metadaten des Objektes auslesen auf das sich der Handle bezieht
	uiPixelDataSize = myBitmap.bmWidth * myBitmap.bmHeight * (myBitmap.bmBitsPixel / 8); // PixelDatengröße

	hDIB = GlobalAlloc(GPTR, uiPixelDataSize);						// Speicher für Pixeldaten reservieren
	pImgData = (BYTE*)GlobalLock(hDIB);

	GetBitmapBits(hBitmap, uiPixelDataSize, pImgData);				// Pixeldaten aus hBitmap auslesen


	D3D11_MAPPED_SUBRESOURCE subResource = {};
	HRESULT hr = pD3dContext->Map(MyImgWndData.pTexture.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &subResource);
	memcpy(subResource.pData, pImgData, uiPixelDataSize);			// Pixeldaten in SubResource von D3d11Texture kopieren
	pD3dContext->Unmap(MyImgWndData.pTexture.Get(), 0);

	memcpy(MyImgWndData.pData, pImgData, uiPixelDataSize);			// Pixeldaten in buffer kopieren

	GlobalUnlock(hDIB);
	GlobalFree(hDIB);												// Speicher freigeben

	DeleteDC(hMemDC);												// DC löschen
	DeleteBitmap(hBitmap);											// Handle löschen
	ReleaseDC(SrcWnd.hWndSrc, hDisplayDC);
}
BOOL CALLBACK EnumWindowsProc(HWND hWnd, LPARAM lParam) {

	TCHAR buf[1024]{};
	TCHAR strName[1024]{};
	USES_CONVERSION;

	MyHandle* pSrcWnd = reinterpret_cast<MyHandle*>(lParam);
	GetClassName(hWnd, buf, 100);
	GetWindowText(hWnd, strName, 1024);
	if (_tcsstr(strName, A2T(pSrcWnd->title)))
	{
		pSrcWnd->hWndSrc = hWnd;
		return FALSE;
	}
	return TRUE;
}

/// <summary>
/// Als Input bekommt er das Fensterhandle wovon ein Bild gemacht werden soll.
/// DisplayDC wird aus WindowHandle bestimmt
/// Es wird ein MemDC aus dem DisplayDC erstellt
/// Es wird ein HBITMAP aus dem DisplayDC erstellt (liefert Metadaten)
/// HBitmap wird dem MemDC zugeordnet (1x1-Px-Platzhalter Platzhalter wird ersetzt mit SelectObject)
/// Pixeldaten werden aus dem DisplayDC in MemDC gelesen, wobei indirekt auf das Unterobjekt: Bitmap zugegriffen wird
/// -> Daten sind nun komplett im MemDC (was als Unterobjekt ein Bitmap hat)
/// Kopieren der PixelDaten in einen PixelBuffer (der per Offset über den BmpFileData angegeben wurde: pPixelData)
/// Erstellen des kompletten BmpFiles mit FileHEader, Bmp-Header und Pixeldaten
/// 
/// </summary>
/// <param name="MyBitMapData">Struktur: - Titel des Fenster enthält um das Handle zu bestimmen
///										 - Neue gestreckte Maße (Höhe und Breite)
///										 - Subressource der D3D11Texture
///										 - DateiName der BMP-Datei die wir erstellen
/// </param>
void BitBltToFile(ImgWndData& MyImgWndData)
{
	DWORD ulWidth, ulHeight, ulFileSize = 0, ulTmp = 0;
	RECT WindowClientRect = {};										// RECT for ResolutionInfo
	MyHandle SrcWnd = {};
	char* pBmpFileData = NULL;
	RGBQUAD* pPixelData = NULL;
	PBITMAPFILEHEADER pFileHeader = NULL;
	PBITMAPINFOHEADER pBmpHeader = NULL;
	HDC hDisplayDC, hMemDC = NULL;
	HBITMAP hBitmap = NULL;
	HANDLE hFile, hDIB = NULL;

	strcpy_s(SrcWnd.title, MyImgWndData.strTitle);					// gesuchten Titel kopieren
	EnumWindows(EnumWindowsProc, reinterpret_cast<LPARAM>(&SrcWnd));// WindowHandle mit SrcWnd.title finden

	hDisplayDC = GetDC(SrcWnd.hWndSrc);								// DC des Fensters erstellen
	GetClientRect(SrcWnd.hWndSrc, &WindowClientRect);				// Auflösung des Fensters
	//MyPoint.x = WindowClientRect.left;
	//MyPoint.y = WindowClientRect.top;
	//ClientToScreen(SrcWnd.hWndSrc, &MyPoint);
	//int x = MyPoint.x - WindowRect.left;
	//int y = WindowRect.top - MyPoint.y;
	//RECT extendedFrameBounds{ 0,0,0,0 };
	//HRESULT hr = DwmGetWindowAttribute(SrcWnd.hWndSrc,
	//	DWMWA_EXTENDED_FRAME_BOUNDS,
	//	&extendedFrameBounds,
	//	sizeof(extendedFrameBounds));

	ulWidth = WindowClientRect.right - WindowClientRect.left;		// Breite
	ulHeight = WindowClientRect.bottom - WindowClientRect.top;		// Höhe 
	ulFileSize =
		sizeof(BITMAPFILEHEADER)									// Größe FileHeader
		+ sizeof(BITMAPINFOHEADER)									// Größe BmpHeader
		+ (sizeof(RGBQUAD)											// ExtraBytes für Dateiende (Größe eines Pixels)
			+ (MyImgWndData.iWidth * MyImgWndData.iHeight * 4) // Größe der Pixeldaten
			);

	hDIB = GlobalAlloc(GMEM_ZEROINIT, ulFileSize);					// Speicher entsprechend der Pixeldaten reservieren
	pBmpFileData = (char*)GlobalLock(hDIB);							// Speicher für diese Anwendung locken und Adresse in Pointer übergeben

	pFileHeader = (PBITMAPFILEHEADER)pBmpFileData;					// Adresse des FileHeaders (Beginnt natürlich beim Anfang des Buffers)
	pBmpHeader = (PBITMAPINFOHEADER)&pBmpFileData[sizeof(BITMAPFILEHEADER)]; // Adresse des BmpHeaders durch Offset von FileHeader (Beginnt nach FileHeader)

	pFileHeader->bfType = 0x4D42;									// Format: in ASCII "BM" für Bitmap
	pFileHeader->bfSize = sizeof(BITMAPFILEHEADER);					// FileHeaderSize
	pFileHeader->bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER); // Offset ab wann Pixeldaten beginnen dürfen

	pBmpHeader->biSize = sizeof(BITMAPINFOHEADER);					// BmpHeaderSize
	pBmpHeader->biPlanes = 1;										// Ebenen
	pBmpHeader->biBitCount = 32;									// Bits per Pixel
	pBmpHeader->biCompression = BI_RGB;								// Kompression BI_RGB = 0 = keine
	pBmpHeader->biHeight = MyImgWndData.iHeight;					// Höhe
	pBmpHeader->biWidth = MyImgWndData.iWidth;						// Breite

	pPixelData = (RGBQUAD*)&pBmpFileData[pFileHeader->bfOffBits];	// Pointer zu Pixeldaten mit Hilfe des Offsets von den beiden Headern

	hMemDC = CreateCompatibleDC(hDisplayDC);						// MemDC aus DisplayDC erstellen
	hBitmap = CreateCompatibleBitmap(								// Neues leeres Bitmap mit entsprechender Größe erstellen
		hDisplayDC,
		MyImgWndData.iWidth,
		MyImgWndData.iHeight
	);
	SelectObject(hMemDC, hBitmap);									// Platzhalter 1x1Px-Bitmap in DC wird durch neues Bitmap ersetzt
	/*BitBlt(															// GDI Funktion die je nach DC entsprechenden Treiber anspricht und Bild in MemDC zeichnet
		hMemDC,														// Ziel: MemoryDC
		0,															// X (0,0 ist oben links)
		0,															// Y (0,0 ist oben links) Wenn 500, ersten 500 pixel von oben aus wären dann schwarz
		ulWidth,													// Breite ab X
		ulHeight,													// Höhe ab Y
		hDisplayDC,													// Source hDisplayDC, also der DisplayDeviceContext
		0,															// X (0,0 ist oben links)
		0,															// Y (0,0 ist oben links)
		SRCCOPY | CAPTUREBLT);										// Flags: SRCOPY: kopiert Viereck der Quelle ins Viereck des Ziels*/

	StretchBlt(														// Kopieren der BitmapDaten die als Bitmap-Unterobj in den DCs existieren
		hMemDC,														// Ziel: MemoryDC bzw. Unterobjekt Bitmap
		0,															// X (0,0 ist oben links)
		0,															// Y (0,0 ist oben links) Wenn 500, ersten 500 pixel von oben aus wären dann schwarz
		MyImgWndData.iWidth,										// Breite ab X
		MyImgWndData.iHeight,										// Höhe ab Y
		hDisplayDC,													// Source hDisplayDC, also der DisplayDeviceContext
		0,															// X (0,0 ist oben links)
		0,															// Y (0,0 ist oben links)
		ulWidth,													// Breite ab X
		ulHeight,													// Höhe ab Y
		SRCCOPY);													// Flags: SRCOPY: kopiert Viereck der Quelle ins Viereck des Ziels
																	// -> habe nun fertiges MemoryDC mit fertigem, gefülltem Bitmap 

	GetDIBits(														// Bitweises kopieren
		hMemDC,														// von MemDC
		hBitmap,													// Unterobjekt-Bitmap von MemDC
		0,															// StartPosition ab welches Bit kopiert wird
		MyImgWndData.iHeight,										// Zeilen		
		pPixelData,													// Buffer, wo Pixeldaten reinkopiert werden. Ist PixelBuffer meines BmpFiles
		(LPBITMAPINFO)pBmpHeader,									// BmpHeader
		DIB_RGB_COLORS);
	hFile = CreateFileA(MyImgWndData.strBmpFileName, GENERIC_WRITE, FILE_SHARE_WRITE, 0, CREATE_ALWAYS, 0, 0); // File Blub.bmp erstellen
	WriteFile(hFile, pBmpFileData, ulFileSize, &ulTmp, 0);			// Bufferdaten in File schreiben
	CloseHandle(hFile);												// File unlocken
	GlobalUnlock(hDIB);												// allocateten Speicher für das BMP-File wieder freigeben
	GlobalFree(hDIB);												// Speicher freigeben
	DeleteDC(hMemDC);
	DeleteBitmap(hBitmap);
	ReleaseDC(SrcWnd.hWndSrc, hDisplayDC);
}
