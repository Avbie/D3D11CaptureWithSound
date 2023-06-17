// MyWindowPrj.cpp : Definiert den Einstiegspunkt f�r die Anwendung.
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
	D3D11_SUBRESOURCE_DATA SubResData = {};					// Container f�r Texture Daten
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
/// Erstellt eine g�ltiges Device und DeviceContext.
/// Liefert somit g�ltige Adresse zum Pointer der wiederum auf ein erstelltes ComObj zeigt.
/// Dies wird f�r Device und DeviceContext erledigt
/// </summary>
void CreateDevice()
{
	HRESULT hResult = NULL;									// R�ckgabewert von ApiFkt. f�r Errorhandling
	ComPtr<IDXGIDevice2> dxgiDevice;						// Smartpointer zum COM-DXGIDevice-Interface-Obj
	D3D_FEATURE_LEVEL pSelectedFeatureLvl;					// will recieve the FeatureLvl set by GPU
	D3D_FEATURE_LEVEL aFeatureLevels[] =					// FeatureLvlArray, bricht ab nachdem er eins erfolgreich best�tigt hat
	{
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_0
	};

	UINT iSizeFeaturLvl = ARRAYSIZE(aFeatureLevels);		// Size of FeatureLvl Array
	/*�bergibt alle gesetzten Parameter und speichert im pD3dDevice eine Adresse zum Pointer des DeviceObj was �ber das COM-IF erstellt wurde
	* Device: Obj f�r die Erstellung s�mtliche Obj bzgl DX
	* Context: Zur Manipulation s�mtlicher erstellten Obj von Device*/
	hResult = D3D11CreateDevice(
		NULL,												// Adapter, auszulesen �ber D3D11Adapter->EnumAdapters oder NULL: DefaultAdapter
		D3D_DRIVER_TYPE_HARDWARE,							// Hardwarebeschleunigung der GPU nutzen
		NULL,												// Handle falls Software genutzt wird (CPU statt GPU)
		D3D11_CREATE_DEVICE_BGRA_SUPPORT |					// Mit BGRA support (Blue,Green,red,alpha) 
		D3D11_CREATE_DEVICE_DEBUG,							// zus�tzliche DebugMsgs im Compilerfenster
		aFeatureLevels,										// Array mit FeatureLvl,von 0 bis n, bricht bei Erfolg ab
		ARRAYSIZE(aFeatureLevels),							// Gr��e des FeatureLvlArrays
		D3D11_SDK_VERSION,									// Installierte DX SDK
		&pD3dDevice,										// wird Adresse zum Pointer des DeviceIFObjs speichern
		&pSelectedFeatureLvl,								// recieved FeatureLvl
		&pD3dContext										// wird Adresse zum Pointer des DeviceContextIFObjs speichern
	);
}//END-FUNC CreateDevice

/// <summary>
/// - Erstellt eine Swapchain1 anstatt Swapchain
/// - F�r Swapchain1 ist eine Factory notwendig: 
///		- pD3dDevice auf dxgiDevice. Polymorphie des Ziels unseres Pointers von pD3dDevice
///		- Erstellt �ber dxgiDevice ein COM-IF-OBj f�r Adpater
///		- Parent von dxgiAdapter ist dxgiFactory
///  mit �bergebenen Device
/// </summary>
/// <param name="dxgiAdapter">Referenz zum leeren ComPtr f�r den Adapter</param>
/// <param name="pDxgiSwapChain">Referenz zum leeren ComPtr f�r Swapchain</param>
void CreateSwapChain(ComPtr<IDXGIAdapter>& pdxgiAdapter, ComPtr<IDXGISwapChain1>& pDxgiSwapChain)
{
	HRESULT hResult = NULL;										// R�ckgabewert von ApiFkt. f�r Errorhandling
	ComPtr<IDXGIDevice2> dxgiDevice;							// Smartpointer bzw Adresse zum COM-Factory-Interface-Obj
	ComPtr<IDXGIFactory2> dxgiFactory;							// Smartpointer zum COM-Factory-Interface-Obj

	DXGI_SWAP_CHAIN_DESC1 d3d11SwapChainDesc = {};				// typed Struct f�r die SwapchainDescription

	d3d11SwapChainDesc.Width = 0;								// use window width = 0
	d3d11SwapChainDesc.Height = 0;								// use window height = 0
	d3d11SwapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;// BGRA Format, BGRA ist StandardFormat f�r die Desktopdubli.
	d3d11SwapChainDesc.SampleDesc.Count = 1;					// Kantengl�ttung. Wieviel Pixel f�r ein Pixel f�r Gl�ttung: 1Px f�r ein Px = aus
	d3d11SwapChainDesc.SampleDesc.Quality = 0;					// AntiAliasing bzw. Kantengl�ttung ist aus, folgt Quality 0
	d3d11SwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; // Wie wird Buffer genutzt
	d3d11SwapChainDesc.BufferCount = 1;							// 1 Backbuffer und 1 Frontbuffer
	d3d11SwapChainDesc.Scaling = DXGI_SCALING_STRETCH;			// DXGI passt BackbufferGr��e an Ziel an 
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
	* &ComPtr genauso wie .getadressof() mit dem Unterschied, dass das Ziel des Pointers zur�ckgesetzt wird. (Neues Obj wird init.)
	*/
	hResult = dxgiFactory->CreateSwapChainForHwnd(
		pD3dDevice.Get(),										// Ziel des Pointers, dass Wiederum ein Pointer auf ein COM-IF-Obj ist
		g_hWndApp,												// globaler Handle zu meinem Window
		&d3d11SwapChainDesc,									// Swapchain Description per Addresse
		nullptr,												// Description f�r Fullscreen
		NULL,													// Falls FullscreenDescription, hier das COMIF-Obj f�r den Monitor
																// per dxgiAdapter->EnumOutput(..)
		&pDxgiSwapChain											// Speichert hier die Adresse des Ziels, was ein Pointer zum COM-IF-Obj ist
	);
}//END-FUNC CreateSwapChain

// Structure f�r ein Array das pro Element aus diesem Struct besteht
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
// Structure die f�r die init. von VertexBuffer ben�tigt wird
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
///								uStride: Gr��e eines Elementes im VertexArray
///								uOffset: Offset im Array
///								VertexBufferDesc: Description die ben�tigt wird um ein VertexBuffer auf GPU zu erstellen
///								aVertices:	Punkte f�r das Zeichnen der 2 Dreiecke (Quad)
///								VertexSubResourceData: beinhaltet die Daten (aVertices) 
///								</param>
void CreateVertexBuffer()
{
	VertexBufferContent VBufContent;						// Punkte
	HRESULT hResult = NULL;									// R�ckgabewert von ApiFkt. f�r Errorhandling
	VBufContent.VertexBufferDesc.BindFlags
		= D3D11_BIND_VERTEX_BUFFER;							// Buffer soll f�r Vertexe verwendet werden, Vertex Buffer in IA-Stage
	VBufContent.VertexBufferDesc.Usage
		= D3D11_USAGE_DEFAULT;								// Default: GPU read and write
	VBufContent.VertexBufferDesc.ByteWidth
		= sizeof(VBufContent.aVertices);					// Gesamte Gr��e von aVertices oder: sizeof(Vertex*4) 80
	VBufContent.VertexSubResData.pSysMem
		= VBufContent.aVertices;							// Zuweisung der Daten in SubResourceTyp
	VBufContent.uStride = sizeof(Vertex);					// Gr��e pro Element: 20
	VBufContent.uOffset = 0;								// Offset, wenn er von Element zu Element springt (um z.B. erste 4 Bits zu ignorieren)

	// auf GPU allocaten
	hResult = pD3dDevice->CreateBuffer(&VBufContent.VertexBufferDesc, &VBufContent.VertexSubResData, &VBufContent.pVertexBuffer);
	// auf GPU kopieren
	pD3dContext->IASetVertexBuffers(0, 1, VBufContent.pVertexBuffer.GetAddressOf(), &VBufContent.uStride, &VBufContent.uOffset);
}//END-FUNC

// Structure die f�r die init. von IndexBuffer ben�tigt wird
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
/// Um ein Viereck mit 2 Dreiecken darzustellen, ben�tigt man an wenigen Stellen 2 Punkte die eine identische Position haben.
/// um Speicher zu sparen wird dort nicht jedesmal ein neuer Vertex definiert, sondern bezieht sich auf ein Index.
/// Damit wird jeder Vertex nur einmal angelegt, kann aber mehrmals per Index genutzt werden
/// </summary>
/// <param name="IBufContent">Referenz zu meinem IndexStruct, dass folgende Daten beinhaltet:
///								IndexBufferDesc: Description die ben�tigt wird um ein IndexBuffer auf GPU zu erstellen
///								iIndices:	Indices f�r die Vertices 1= Vertex 1 in aVertices
///								IndexSubResData: beinhaltet die Daten (iIndices) </param>
void CreateIndexBuffer(IndexBufferContent& IBufContent)
{
	HRESULT hResult = NULL;									// R�ckgabewert von ApiFkt. f�r Errorhandling
	IBufContent.IndexBufferDesc.BindFlags					// Buffer soll als IndexBuffer genutzt werde-> IndexBufferFlag setzen
		= D3D11_BIND_INDEX_BUFFER;							// Buffer soll f�r VertexIndices verwendet werden, IndexBuffer in IA-Stage
	IBufContent.IndexBufferDesc.Usage
		= D3D11_USAGE_DEFAULT;								// Default: GPU read and write
	IBufContent.IndexBufferDesc.ByteWidth
		= sizeof(IBufContent.iIndices);						// Gesamte Gr��e von iIndices oder: sizeof(iIndices*6)= sizeof(short)*6 = 16*6
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
///								fAngle: Winkel der pro Frame ge�ndert wird (in Matrix)
///								ConstantBuffer:	Array aus structs was Matrizen enth�lt (hier kein Array, da nur eine Matrix)
///								ConstantSubResData: beinhaltet die Daten (Matrizen) </param>
void CreateConstantBuffer(ConstantBufferContent& CBufConstant)
{
	HRESULT hResult = NULL;

	D3D11_BUFFER_DESC ConstantBufferDesc = {};				// Description f�r einen Buffer
	D3D11_SUBRESOURCE_DATA ConstantSubResData = {};			// Data Container	

	ConstantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;				// Buffer als ConstantBuffer def.
	ConstantBufferDesc.ByteWidth = sizeof(CBufConstant.MyConstantBuffer);	// Array von Matritzen, haben nur ein Element: 4x4xfloat = 4*4*4 = 64 Bytes
	ConstantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;				// wird pro Frame ge�ndert, muss f�r CPU gelockt werden
	ConstantBufferDesc.MiscFlags = 0;
	ConstantBufferDesc.StructureByteStride = 0;								// Gr��e pro Element, wird nicht ben�tigt
	ConstantBufferDesc.Usage = D3D11_USAGE_DYNAMIC;							// Dynamic: CPU: write; GPU: read
	ConstantSubResData.pSysMem = &CBufConstant.MyConstantBuffer;			// Daten in Datencontainer speichern
	// Constant Buffer allocaten
	hResult = pD3dDevice->CreateBuffer(&ConstantBufferDesc, &ConstantSubResData, &CBufConstant.pConstantBuffer);
	// Hier werden nicht die Daten auf GPU kopiert, weil dies ja pro Frame passiert
	// und vorher von CPU noch manipuliert wird - Schreibrechte
}//END-FUNC
/// <summary>
/// Anstatt pro Frame alle Vertices zu �ndern, wird eine Matrix pro Frame ge�ndert, die dann auf GPU-Seite mit Vertices operiert
/// 1. weniger Daten von CPU zu GPU
/// 2. Operation zur Manipulation wird auf GPU ausgef�hrt. GPU ist wesentlich schneller f�r solche Operationen.
/// -> ben�tigen CPU AccessRights, d.h. Daten werden gemappt(gelockt) wenn CPU die Daten manipuliert. 
///	   GPU liest sie danach aus und kopiert sie auf VRAM
/// </summary>
/// <param name="CBufContent">Referenz zu meinem ConstantStruct, dass folgende Daten beinhaltet:
///								fAngle: Winkel der pro Frame ge�ndert wird (in Matrix)
///								ConstantBuffer:	Array aus structs was Matrizen enth�lt (hier kein Array, da nur eine Matrix)
///								ConstantSubResData: beinhaltet die Daten (Matrizen) </param>
void SetConstantBuffer(ConstantBufferContent& CBufConstant)
{
	HRESULT hResult = NULL;
	CBufConstant.fAngle = timer.Peek();						// Winkel�nderung pro Frame
	D3D11_MAPPED_SUBRESOURCE mappedRes;						// Datencontainer

	// 4x4 Matrix
	// 1. x-Koord
	// 2. y-Koord
	// 3. z-Koord: keine �nderung, d.h. Einheitswerte
	// 4. f�r Mondifikation: keine �nderung, d.h. Einheitswerte
	// 4x2 Matrix geht nicht f�r Berechnung -> Zur Berechnung muss Anzahl Zeile = Anzahl Spalte -> 4x4Matrix
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
		0,													// Index f�r Subresource (k�nnen mehrere im Array sein)
		D3D11_MAP_WRITE_DISCARD,							// gelockt f�rs schreiben, alter Inhalt wird nicht mehr definiert sein (nicht gemerkt)
		0,													// Flag setzen falls nicht gelockt werden kann und was CPU derweil tun soll
		&mappedRes);										// Datencontainer des Buffers

	ConstantBuffer* dataPtr = (ConstantBuffer*)mappedRes.pData; // Zeiger auf Daten im Datencontainer 
	memcpy(dataPtr, &CBufConstant.MyConstantBuffer, sizeof(ConstantBuffer)); // neue Daten reinschreiben
	pD3dContext->Unmap(CBufConstant.pConstantBuffer.Get(), 0); // Buffer wieder unlocken (CPU ist fertig)
	pD3dContext->VSSetConstantBuffers(0, 1, CBufConstant.pConstantBuffer.GetAddressOf()); // Manipulierten Buffer direkt in VS-Stage setzen
}//END-FUNC
/// <summary>
/// Erstellt einen Sampler, der die Texture auf das Quad (2 Dreiecke) mappen kann.
/// Ben�tigt f�r jedes Pixel Koordinaten auf Texture um die richtige Farbe darzustellen.
/// Die UV-TextureCoord werden von InputAssamblerStage (Input von Vertexdaten: Coord, UV-Maps,Farbe) bis in den PixelshaderStage �bertragen
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
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;	// Freir�ume bekommen eine Farbe, es wird die Texture nicht gespiegelt etc.
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	samplerDesc.BorderColor[0] = 1.0f;						// Farbe bei Freir�umen
	samplerDesc.BorderColor[1] = 1.0f;
	samplerDesc.BorderColor[2] = 1.0f;
	samplerDesc.BorderColor[3] = 1.0f;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;	// Verhalten von alten zu neuen Daten, z.B. bei Desktopdupl nur mappen wo �nderung

	ComPtr<ID3D11SamplerState> samplerState;								// Sampler IF-Pointer
	hResult = pD3dDevice->CreateSamplerState(&samplerDesc, &samplerState);	// Sampler auf GPU allocaten
	pD3dContext->PSSetSamplers(0, 1, samplerState.GetAddressOf());			// Sampler setzen (COPY von GPU zu GPU)
}//END-FUNC
/// <summary>
/// Generiert ein Bild und speichert es als SubResourceData.
/// Erstellung eines Texture-Obj per D3D11 CreateFunktion
/// Erstellung eines IF-Pointers zu einem TexturObj. Obj enth�lt SubResourceData
/// </summary>
/// <param name="ImgData">Struct mit ImgDaten: Breite, H�he, Bytes pro Pixel, Pointer zum IF-Pointer</param>
void GenerateD3D11Texture(ImgWndData& ImgData)
{
	int iWidth = ImgData.iWidth;											// �bergabe an lokale Variablen, wegen der �bersicht
	int iHeight = ImgData.iHeight;
	int iBpp = ImgData.iBpp;
	//ImgData.pRGBQUAD = new RGBQUAD[ImgData.GetPx()];

	ImgData.pData = new unsigned char[ImgData.GetBits()];					// DatenBuffer f�r PixelDaten; GetBits = Width*Height*Bpp
	ImgData.pRGBQUAD = (RGBQUAD*)ImgData.pData;
	unsigned char* pData = ImgData.pData;
	D3D11_TEXTURE2D_DESC TextureDesc = {};									// TextureDescription f�r TextureObj

	for (int y = 0; y < ImgData.iHeight; ++y)
	{
		for (int x = 0; x < ImgData.iWidth; ++x)
		{
			// y = 0;x = 0;
			// 0*100 + 0*4 = 0 an stelle 0 wird ein 0xff wert eingf�gt
			// 0*100 + 0*4 +1 = 1 an stelle 1 (9. Bit bzw 2. Byte) wird ein 0x00 eingef�gt
			// 0*100 + 0*4 +2 = 2 an Stelle 2 (17. Bit bzw. 3 Byte) wird ein 0x00 eingef�gt
			// Hexwerte mit max ff = 8 bit = 1 Byte. Hei�t  1 Stelle sind  8 Bit
			// red + green + blue + alpha = 4 Byte, deswegen auch 100* 100*4
			// 4 byte/Pixel
			// ein Charakter ist 1 Byte gro�. 1 Byte = 8 bit = 0xFF = 1111 1111
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
	//BitBltToD3D11SubResource(BitMapData);					// �bergebe Target
	//ProgressingSinkWriter(BitMapData.pDataOFSubResource);
	TextureDesc.Width = iWidth;							// Width
	TextureDesc.Height = iHeight;							// Height
	TextureDesc.MipLevels = 1;								// MipLvl 0/1 = aus
	TextureDesc.ArraySize = 1;								// ArraySize der Subresource, da nur 1. Adresse des 1. Elements �bergeben wird = 1
	TextureDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;		// Bildformat: Desktopdupl. ist immer BGRA
	TextureDesc.SampleDesc.Count = 1;
	TextureDesc.Usage = D3D11_USAGE_DYNAMIC;// D3D11_USAGE_DEFAULT;				// Default: Read/Write f�r GPU
	TextureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;		// wird an VertexShader-Stage gebunden
	TextureDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;							// keine CPU-Rights notwendig

	//D3D11_SUBRESOURCE_DATA SubResData = {};					// Container f�r Daten

	ImgData.SubResData.pSysMem = (const void*)pData;				// Daten in Container speichern
	ImgData.SubResData.SysMemPitch = iWidth * iBpp;				// Eine Pixelreihe als Width*4: 1 Pixel = 4 Bytes
	ImgData.SubResData.SysMemSlicePitch = 0;						// nur wichtig bei 3D-Texturen

	// TextureObj wird erstellt (geerbt von IF). pTexture ist nun Pointer zum IF-Pointer.
	pD3dDevice->CreateTexture2D(&TextureDesc, &ImgData.SubResData, &ImgData.pTexture);
}//END-FUNC
/// <summary>
/// Compiliert ein ShaderProgramm f�r die GPU
/// Blob-IF-Pointer hat Zugriff auf Funktionen des Obj, was die compilierten ShaderProgrammDaten als Buffer enth�lt.
/// Erstellen des Programmes auf GPU mit CreateVertexShader: 
/// IF-Pointer greift auf Fkt. GetBufferPointer zu, die Pointer zu den VertexShaderProgr liefert
/// Es wird ein IF-Pointer zum erstellten VertexShader-Obj erstellt: pVertexShader
/// MitVSSetShader wird COPY von GPU zu GPU ausgef�hrt 
/// (Erstellter Shader wird intern auf GPU auf Speicher kopiert wo er ausgef�hrt werden kann)
/// </summary>
/// <param name="vsBlob">Referenz zum leeren ComPtr f�r ShaderProgrammDaten, wo sie gespeichert werden sollen</param>
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
		vsBlob->GetBufferSize(),							// Gr��e der Daten
		nullptr,
		&pVertexShader);									// IF-Pointer, indirekter Zugriff auf ContainerObj f�r VertexShaderProgramm 
	assert(SUCCEEDED(hResult));
	pD3dContext->VSSetShader(pVertexShader.Get(), nullptr, 0); // COPY intern von GPU(creationplace) zu GPU(runplace)
	return 1;
}//END-FUNC
/// <summary>
/// Compiliert ein ShaderProgramm f�r die GPU
/// Blob-IF-Pointer hat Zugriff auf Funktionen des Obj, was die compilierten ShaderProgrammDaten als Buffer enth�lt.
/// Erstellen des Programmes auf GPU mit CreatePixelShader: 
/// IF-Pointer greift auf Fkt. GetBufferPointer zu, die Pointer zu den PixelShaderProgr liefert
/// Es wird ein IF-Pointer zum erstellten PuixelShader-Obj erstellt: pPixelShader
/// MitPSSetShader wird COPY von GPU zu GPU ausgef�hrt 
/// (Erstellter Shader wird intern auf GPU auf Speicher kopiert wo er ausgef�hrt werden kann)
/// </summary>
/// <param name="vsBlob">Referenz zum leeren ComPtr f�r ShaderProgrammDaten, wo sie gespeichert werden sollen</param>
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
		psBlob->GetBufferSize(),							// Gr��e der Daten
		nullptr,
		&pPixelShader);										// IF-Pointer, indirekter Zugriff auf ContainerObj f�r VertexShaderProgramm 
	assert(SUCCEEDED(hResult));
	pD3dContext->PSSetShader(pPixelShader.Get(), nullptr, 0); // COPY intern von GPU(creationplace) zu GPU(runplace)
	return 1;
}//END-FUNC
/// <summary>
/// Erstellt Metadaten f�r die Inputparameter des VertexShaders
/// - Legt Typ und Gr��e der Inputparamter fest und �bergibt sie an erste Stage (InputAssambler)
/// - �bergibt nicht selbst die InputParamter:
///		passiert �ber die ganzen Createfunktionen, womit Daten auf richtigen Speicherort auf GPU gespeichert werden
///		die dann vom ShaderProgramm abgerufen werden k�nnen
/// - �bergabe des compilierten ShaderProgramms �ber die erste Stage: InputAssambler
/// </summary>
/// <param name="vsBlob">compiliertes ShaderProgramm, beinhaltet Pointer und Gr��e</param>
void CreateInputLayout(ComPtr<ID3DBlob>& vsBlob)
{
	HRESULT hResult = NULL;
	ComPtr<ID3D11InputLayout> pInputLayout;					// IF-Pointer zum Layout

	const D3D11_INPUT_ELEMENT_DESC InputElementDesc[] =
	{
		{"POS",0,DXGI_FORMAT_R32G32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0},
		{"TEX",0,DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{"COL",0,DXGI_FORMAT_R8G8B8A8_UNORM,0,D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0}
		// Nachdem die Werte den Vertexshader verlassen, m�ssen sie normalized sein
		// mit UNORM sind sie sogar schon normalized wenn sie in den Vertexshader reinkommen
		// ABER: die Bitgr��e muss  beachtet werden, wenn die Color aus 4 floats besteht (je 32 bit), dann darf ich kein DXGI_FORMAT_R16G16B16A16_UNORM verweden
		// unsigned char ist 8 bit gro�. Pro Farbe RGBA/BGRA. Ergibt 4*8Bit = 32Bit: DXGI_FORMAT_R8G8B8A8_UNORM

	};
	// Damit InputAssambler weiss was er machen soll mit den Daten wenn er fertig ist.
	// Er sendet sie an die Adresse des VertexShaders die hier mitgegeben wird
	hResult = pD3dDevice->CreateInputLayout(
		InputElementDesc,									// Beschreibung der Inputparameter f�r ShaderProgramm
		ARRAYSIZE(InputElementDesc),						// Anzahl der Elemente
		vsBlob->GetBufferPointer(),							// Pointer zum kompilierten Shaderprogramm. Befindet sich bereits auf VRAM
		vsBlob->GetBufferSize(),							// Gr��e des ShaderProgramms
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
															// Bei der Erstellung eines Objs wird es �ber eine CreateFunktion erstellt, per Polymorphism:
															// Je nachdem ob Obj per Polym �ber Output1 oder Output 2 erstellt wird �berschreibt es unterschiedliche Funktionen
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
	pDeskDupl->GetDesc(&OutputDuplDesc);					// Description des Monitors wie Aufl�sung etc.
	return 1;
}


/// <summary>
/// WinMain Funktion
/// </summary>
/// <param name="hInstance">Handle zur Anwendung, idendifier f�r das OS </param>
/// <param name="hPrevInstance">Pr�ft ob Instanz bereits vorhanden, bis Win 3.1</param>
/// <param name="lpCmdLine">Pointer auf Cmdline Cmds</param>
/// <param name="nCmdShow">Flag f�r minimized, maximized</param>
/// <returns>NULL</returns>
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
	//ComPtr<ID3D11Device> pD3dDevice;
	//ComPtr<ID3D11DeviceContext> pD3dContext;
	ComPtr<IDXGISwapChain1> pDxgiSwapChain;
	ComPtr<IDXGIAdapter> pDxgiAdapter;

	UINT uiHeight = 0;								// Aufl�sungsparameter
	UINT uiWidth = 0;
	HRESULT hResult;										// R�ckgabewert f�r Win32Api Funktionen
	UNREFERENCED_PARAMETER(hPrevInstance);					// Nicht referenzierte Parameter Warning unterdr�cken
	UNREFERENCED_PARAMETER(lpCmdLine);
	HMONITOR hMon;
	char strFileName[256] = "";
	char strWndTitle[256] = "";


	/**********WindowCreation************/
	ClsWnd oMyWnd(											// f�hrt CreateWindowEx aus, d.h. erzeugt Fenster die reg. sind. 
															// WNDCLASS und Register wird bereits beim Start des Programms ausgef�hrt
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
															// Haben Buffer f�r HWND -> selbe HZ-Zahl wie Windows selbst.
															// WindowsRefreshrate = 165Hz: 165 Durchg�nge pro Sek
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
		->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));			// 1:param: 0 f�r BackBuffer, 2.: Adresse des BackBuffers in pBackBuffer speichern
	hResult = pD3dDevice->CreateRenderTargetView(			// Erstellt eine View auf Backbuffer, k�nnen nur Views an Pipeline binden
															// View wird sp�ter per OMSetRenderTarget an OutputMerger-Stage gebunden
		pBackBuffer.Get(),									// Wert des Pointers, also Adresse des IF-Pointers
		0,													// Description f�r View, NULL wenn 1. Param kein Mipmap hat
		&pTarRenderView);									// �bergabe der Adresse des Ptrs der auf IF-Pointer zeigen soll.
															// In CreateFkt wird nur Ziel des Ptrs ge�ndert, nicht Adresse selbst (geht ja nicht)
	/**********Buffers*******************/
	CreateVertexBuffer();									// Erstellung von 4 Punkten auf GPU
	IndexBufferContent IBufContent;
	CreateIndexBuffer(IBufContent);							// IndexBuffer, bezieht sich auf VertexBuffer, Zugriff per Index auf Punkte 
															// Bei 2 Dreiecken, die ein Quad darstellen soll gibt es 2 Punkte mit identischer Position
															// Per IndexZugriff spart man sich den Speicher f�r 2 Punkte		
	ConstantBufferContent CBufConstant;
	CreateConstantBuffer(CBufConstant);						// Funktion/Matrix die pro Frame an GPU �bergeben wird, Manipulation des Quads (Winkel)
	/**********Sampler*******************/
	CreateAndSetSampler();									// Greift per Koordinaten (UV-Mapping) auf Texture zu 
															// und setzt die entsprechende Farbe des Pixels innerhalb des Quads
															// Bei der Erstellung eines Samplers hat dieser eine feste Position in der GPU.
															// Sampler kann somit im Pixelshader wie folgt angesprochen werden: 
															// register(s0) s=Sampler, 0= Index im PixelShader.hlsl
	/**********ImageProcessing***********/
	// Texture f�r Sampler-Stage (in PixelShader-Stage) erstellen
	ImgWndData MyImgWndData(uiWidth, uiHeight, 4);

	/*BitMapData BitMapData(ImgData.pData, uiWidth, uiHeight);	// Pointer of SubResource.Data, TargetResolution for new Bmp
	BitBltToFile(BitMapData);
	BitBltToD3D11SubResource(BitMapData);					// �bergebe Target
	ProgressingSinkWriter(BitMapData.pDataOFSubResource);*/
	GenerateD3D11Texture(MyImgWndData);					    // generiert ein zweifarbiges Img mit entsprechender Aufl�sung und Bittiefe (4Byte = 32Bit)
															// und erstellt aus Img daten eine Subresource, diese Subresource wird genutzt 
															// um eine D3D11 Texture zu erstellen
	BitBltToFile(MyImgWndData);
	//BitBltToD3D11SubResource(MyImgWndData);					// �bergebe Target
	/**********ShaderView****************/
	ComPtr<ID3D11ShaderResourceView> ShaderView;
	// Erstellung einer ShaderView, diese View kann direkt an den PixelShader-Stage gebunden (per Adresse) und angezeigt werden
	// pTexture enth�lt bis jetzt nur die generierte Texture von GenerateD3D11Texture(), noch kein WindowsAbbild
	hResult = pD3dDevice->CreateShaderResourceView(MyImgWndData.pTexture.Get(), nullptr, &ShaderView);
	pD3dContext->PSSetShaderResources(0, 1, ShaderView.GetAddressOf()); // beinhaltet unsere Texture, welche unser ImgTexture oder DesktopDuplTexture beinhaltet
	/**********ShaderCompile*************/
	ComPtr<ID3DBlob> vsBlob;
	if (!CreateVetexShader(vsBlob))							// Erstellung eines compilierten ShaderProgramms was auf GPU gespeichert wird
															// Die Metadaten f�r die Input-Parameter und Pointer zum VertexShader 
															// werden bereits in der ersten Stage mit �bergeben (InputLayout an InputAssambler-Stage �bergeben)
		return 1;
	ComPtr<ID3DBlob> psBlob;
	if (!CreatePixelShader(psBlob))
		return 1;
	/**********InputLayout***************/
	CreateInputLayout(vsBlob);								// Legt Typ und Gr��e der InputParameter f�r Shader fest, �bergibt Shader an IA-Stage
	/**********DESKTOP DUPLICATION*******/
	ComPtr<IDXGIOutputDuplication> pDeskDupl;
	DXGI_OUTDUPL_DESC OutputDuplDesc = {};

	CreateDesktopDuplication(								// Erstellt IF-Output-Pointer (Monitor) �ber dxgiAdapter. Castet es auf OutputType1.
		pDeskDupl,											// OutputType1 bietet Funktion, welche eine DesktopDupl liefert
		OutputDuplDesc,										// Description wie Aufl�sung etc.
		pDxgiAdapter);										// Adapter muss mit �bergeben werden, um Outputs zu bestimmen
	/************GET FRAME OF DESKTOP*********/
	ComPtr<IDXGIResource> pDxgiDesktopResource;
	DXGI_OUTDUPL_FRAME_INFO lFrameInfo;
	ComPtr<ID3D11Texture2D> pD3dAcquiredDesktopImage;

	PrepareSinkWriter();

	while (oMyWnd.RunMsgLoop())								// Pro Durchgang wird ein FrontBuffer oder VertexShaderView gezeigt
															// Haben Buffer f�r HWND -> selbe HZ-Zahl wie Windows selbst.
															// WindowsRefreshrate = 165Hz: 165 Durchg�nge pro Sek
	{
		// ShaderResourceView wird pro Loop angezeigt
		// in MyImgWndData ist eine D3D11Texture die an diese View gebunden ist
		// diese D3D11Texture bekommt pro Durchgang neue BildDaten aus entsprechenden Fenster
		BitBltToD3D11SubResource(MyImgWndData);				// �bergebe Target
		// todo bis hier her funktionen in klassen eingebunden:
		// alles mit dx in ClsD3d11, alles mit bitblt in ClsBitblt:
		// klassen sind von einander abh�ngig: gemeinsamer struct: MyBitBltData
		// wird inclsBitBlt erstellt: 1. generation eines 2farbbildes GenerateBitBltData()
		//							  2. PixelDaten eines Fensters auslesen und als BitBlt speichern: GetBitBltDataFromWindow()
		// wird in clsD3D11 in d3d11texture bzw. subresource konvertiert
		// -> reihenfolge der jeweilgen methodenAufrufe der 2 klassen wichtig
		ProgressingSinkWriter(MyImgWndData.pData);
		hResult = pDeskDupl->AcquireNextFrame(				// Fragt Frame von DesktopDupl ab, d.h. von Hauptmonitor (siehe CreateDesktopDuplication)
			500,											// wielange ist dieses Frame aktuell. Muss noch beim n�chsten Durchlauf verf�gbar sein
			&lFrameInfo,									// MouseCurserInformationen, letzte aktualisierung des DesktopFrames seitens DWM
			&pDxgiDesktopResource);							// Container f�r ImgDaten

		hResult = pDxgiDesktopResource->QueryInterface(IID_PPV_ARGS(&pD3dAcquiredDesktopImage)); // low (hardwarenah) zu higher(apinah) cast
		/*********Override Texture**********/
		// �berschreibung meiner ImgProcessingD3D11Texture mit DesktopD3D11Texture
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
															// Punkte, Linie, Linien, Dreicke, zusammenh�ngende Dreiecke etc.
		//pD3dContext->IASetIndexBuffer(IBufContent.pIndexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);
		/**********Set ViewportArea*********/
		RECT winRect;										// Container f�r WindowClientArea. Gr��e des Fensters ohne Menu und Borders. (left,top,right,bottom)
		GetClientRect(oMyWnd.GetHWND(), &winRect);			// WindowClientArea abfragen
		// Bindet die View. Damit sp�ter der Rasterizer weiss wieviel max. dargestellt werden soll
		D3D11_VIEWPORT viewport = { 0.0f, 0.0f, (FLOAT)(winRect.right - winRect.left), (FLOAT)(winRect.bottom - winRect.top), 0.0f, 1.0f };
		pD3dContext->RSSetViewports(1, &viewport);			// Die Area wo Rasterizer zeichnet, genau definieren
		//RECT sicRect = { 0, 0, uiWidth, uiHeight };
		//pD3dContext->RSSetScissorRects(1, &sicRect);		// der zu refreshende Bereich innerhalb der gebundenen WindowClientArea
		/********** Output Merger **********/
		/* Erstellt das sichtbare Bild aus:
		* Vertexshader: Dargestellte Form (�bergabe an PixelShader)
		* PixelShader: Sampler mit UV-Mapping. Mapped Texture auf dargestellte Form. (�bergabe an OutputMerger)
		* Backbuffer/TargetRenderView: Hintergrund/ClearColor (direktes setzen im OutputMerger)
		* DephstencilView: Bei 3D, welche Ebene ist weiter vorn und sichtbar und welche nicht (direktes setzen im OutputMerger)*/
		pD3dContext->OMSetRenderTargets(1, pTarRenderView.GetAddressOf(), 0);
		/********** Draw *******************/
		/*Durchl�uft alle Stages und die vorher hinterlegten Daten in den jeweiligen Stages werden br�cksichtigt*/
		//pD3dContext->Draw(6, 0u);
		pD3dContext->DrawIndexed(							// verwenden Indices f�rs zeichnen
			(UINT)std::size(IBufContent.iIndices),			// sizeof(iIndices/iIndices[0]); 12/2 = 6; 12Byte/2Byte (2Byte/16Bit = short int)
			0,												// StartIndex
			0												// IndexAddition: bevor mit jedem Index im VertexArray gesucht wird, wird zum Index 1 addiert
		);

		pDxgiSwapChain->Present(
			1,												/* Wie das fertige Bild in den FrontBuffer geladen wird:
															F�r die BitBlock�bertragung DXGI_SWAP_EFFECT_DISCARD oder DXGI_SWAP_EFFECT_SEQUENTIAL
															0:	keine Sync. Bild wird in den FrontBuffer geladen sobald es fertig ist
															1-4: Anzahl der VBlanks die abgewartet werden bis neues Bild in den FrontBuffer geladen wird
															*/
			0												// Flags: z.B. Tearing erlauben, neuladen eines Bildes, ohne VBlanks zu ber�cksichtigen
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
	/// <param name="MyBitMapData">Struktur: - Titel des Fenster enth�lt um das Handle zu bestimmen
	///										 - Neue gestreckte Ma�e (H�he und Breite)
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

	GetClientRect(SrcWnd.hWndSrc, &WindowClientRect);				// Aufl�sung des Fensters
	ulWidth = WindowClientRect.right - WindowClientRect.left;		// Breite
	ulHeight = WindowClientRect.bottom - WindowClientRect.top;		// H�he

	hMemDC = CreateCompatibleDC(hDisplayDC);						// MemoryDC aus DisplayDC erstellen und Handle darauf zur�ckbekommen
	hBitmap = CreateCompatibleBitmap(								// Hier werden nur Metadaten gesetzt, keine Pixeldaten kopiert
		hDisplayDC,													// Memory Bitmap erstellen, aus Bitmap  des DisplayDC. 
		MyImgWndData.iWidth,										// aber mit gr��erer Aufl�sung
		MyImgWndData.iHeight
	);
	SelectObject(hMemDC, hBitmap);									// Ersetzen des 1x1Px Platzhalters im MemDC durch mein Bitmap

	StretchBlt(														// Kopieren der BitmapDaten die als Bitmap-Unterobj in den DCs existieren
		hMemDC,														// Ziel: MemoryDC
		0,															// X (0,0 ist oben links)
		0,															// Y (0,0 ist oben links) Wenn 500, ersten 500 pixel von oben aus w�ren dann schwarz
		MyImgWndData.iWidth,										// Breite ab X
		MyImgWndData.iHeight,										// H�he ab Y
		hDisplayDC,													// Source hDisplayDC, also der DisplayDeviceContext
		0,															// X (0,0 ist oben links)
		0,															// Y (0,0 ist oben links)
		ulWidth,													// Breite ab X
		ulHeight,													// H�he ab Y
		SRCCOPY);													// Flags: SRCOPY: kopiert Viereck der Quelle ins Viereck des Ziels
																	// -> habe nun fertiges MemoryDC mit fertigem, gef�lltem Bitmap 

	GetObject(hBitmap, sizeof(myBitmap), (LPVOID)&myBitmap);		// Metadaten des Objektes auslesen auf das sich der Handle bezieht
	uiPixelDataSize = myBitmap.bmWidth * myBitmap.bmHeight * (myBitmap.bmBitsPixel / 8); // PixelDatengr��e

	hDIB = GlobalAlloc(GPTR, uiPixelDataSize);						// Speicher f�r Pixeldaten reservieren
	pImgData = (BYTE*)GlobalLock(hDIB);

	GetBitmapBits(hBitmap, uiPixelDataSize, pImgData);				// Pixeldaten aus hBitmap auslesen


	D3D11_MAPPED_SUBRESOURCE subResource = {};
	HRESULT hr = pD3dContext->Map(MyImgWndData.pTexture.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &subResource);
	memcpy(subResource.pData, pImgData, uiPixelDataSize);			// Pixeldaten in SubResource von D3d11Texture kopieren
	pD3dContext->Unmap(MyImgWndData.pTexture.Get(), 0);

	memcpy(MyImgWndData.pData, pImgData, uiPixelDataSize);			// Pixeldaten in buffer kopieren

	GlobalUnlock(hDIB);
	GlobalFree(hDIB);												// Speicher freigeben

	DeleteDC(hMemDC);												// DC l�schen
	DeleteBitmap(hBitmap);											// Handle l�schen
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
/// Kopieren der PixelDaten in einen PixelBuffer (der per Offset �ber den BmpFileData angegeben wurde: pPixelData)
/// Erstellen des kompletten BmpFiles mit FileHEader, Bmp-Header und Pixeldaten
/// 
/// </summary>
/// <param name="MyBitMapData">Struktur: - Titel des Fenster enth�lt um das Handle zu bestimmen
///										 - Neue gestreckte Ma�e (H�he und Breite)
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
	GetClientRect(SrcWnd.hWndSrc, &WindowClientRect);				// Aufl�sung des Fensters
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
	ulHeight = WindowClientRect.bottom - WindowClientRect.top;		// H�he 
	ulFileSize =
		sizeof(BITMAPFILEHEADER)									// Gr��e FileHeader
		+ sizeof(BITMAPINFOHEADER)									// Gr��e BmpHeader
		+ (sizeof(RGBQUAD)											// ExtraBytes f�r Dateiende (Gr��e eines Pixels)
			+ (MyImgWndData.iWidth * MyImgWndData.iHeight * 4) // Gr��e der Pixeldaten
			);

	hDIB = GlobalAlloc(GMEM_ZEROINIT, ulFileSize);					// Speicher entsprechend der Pixeldaten reservieren
	pBmpFileData = (char*)GlobalLock(hDIB);							// Speicher f�r diese Anwendung locken und Adresse in Pointer �bergeben

	pFileHeader = (PBITMAPFILEHEADER)pBmpFileData;					// Adresse des FileHeaders (Beginnt nat�rlich beim Anfang des Buffers)
	pBmpHeader = (PBITMAPINFOHEADER)&pBmpFileData[sizeof(BITMAPFILEHEADER)]; // Adresse des BmpHeaders durch Offset von FileHeader (Beginnt nach FileHeader)

	pFileHeader->bfType = 0x4D42;									// Format: in ASCII "BM" f�r Bitmap
	pFileHeader->bfSize = sizeof(BITMAPFILEHEADER);					// FileHeaderSize
	pFileHeader->bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER); // Offset ab wann Pixeldaten beginnen d�rfen

	pBmpHeader->biSize = sizeof(BITMAPINFOHEADER);					// BmpHeaderSize
	pBmpHeader->biPlanes = 1;										// Ebenen
	pBmpHeader->biBitCount = 32;									// Bits per Pixel
	pBmpHeader->biCompression = BI_RGB;								// Kompression BI_RGB = 0 = keine
	pBmpHeader->biHeight = MyImgWndData.iHeight;					// H�he
	pBmpHeader->biWidth = MyImgWndData.iWidth;						// Breite

	pPixelData = (RGBQUAD*)&pBmpFileData[pFileHeader->bfOffBits];	// Pointer zu Pixeldaten mit Hilfe des Offsets von den beiden Headern

	hMemDC = CreateCompatibleDC(hDisplayDC);						// MemDC aus DisplayDC erstellen
	hBitmap = CreateCompatibleBitmap(								// Neues leeres Bitmap mit entsprechender Gr��e erstellen
		hDisplayDC,
		MyImgWndData.iWidth,
		MyImgWndData.iHeight
	);
	SelectObject(hMemDC, hBitmap);									// Platzhalter 1x1Px-Bitmap in DC wird durch neues Bitmap ersetzt
	/*BitBlt(															// GDI Funktion die je nach DC entsprechenden Treiber anspricht und Bild in MemDC zeichnet
		hMemDC,														// Ziel: MemoryDC
		0,															// X (0,0 ist oben links)
		0,															// Y (0,0 ist oben links) Wenn 500, ersten 500 pixel von oben aus w�ren dann schwarz
		ulWidth,													// Breite ab X
		ulHeight,													// H�he ab Y
		hDisplayDC,													// Source hDisplayDC, also der DisplayDeviceContext
		0,															// X (0,0 ist oben links)
		0,															// Y (0,0 ist oben links)
		SRCCOPY | CAPTUREBLT);										// Flags: SRCOPY: kopiert Viereck der Quelle ins Viereck des Ziels*/

	StretchBlt(														// Kopieren der BitmapDaten die als Bitmap-Unterobj in den DCs existieren
		hMemDC,														// Ziel: MemoryDC bzw. Unterobjekt Bitmap
		0,															// X (0,0 ist oben links)
		0,															// Y (0,0 ist oben links) Wenn 500, ersten 500 pixel von oben aus w�ren dann schwarz
		MyImgWndData.iWidth,										// Breite ab X
		MyImgWndData.iHeight,										// H�he ab Y
		hDisplayDC,													// Source hDisplayDC, also der DisplayDeviceContext
		0,															// X (0,0 ist oben links)
		0,															// Y (0,0 ist oben links)
		ulWidth,													// Breite ab X
		ulHeight,													// H�he ab Y
		SRCCOPY);													// Flags: SRCOPY: kopiert Viereck der Quelle ins Viereck des Ziels
																	// -> habe nun fertiges MemoryDC mit fertigem, gef�lltem Bitmap 

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
	GlobalUnlock(hDIB);												// allocateten Speicher f�r das BMP-File wieder freigeben
	GlobalFree(hDIB);												// Speicher freigeben
	DeleteDC(hMemDC);
	DeleteBitmap(hBitmap);
	ReleaseDC(SrcWnd.hWndSrc, hDisplayDC);
}
