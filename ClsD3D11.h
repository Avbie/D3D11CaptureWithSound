#pragma once
#include "framework.h"
#include "FrameData.h"
#include "D3DBuffer.h"
#include "ClsD2D1.h"
#include "ClsCalcViewPort.h"
//#include "ClsDataContainer.h"

using namespace Microsoft::WRL;
using namespace ABI::Windows::Foundation;
using Microsoft::WRL::ComPtr;

#define PFORMAT DXGI_FORMAT_B8G8R8A8_UNORM;


namespace D3D
{
	/// <summary>
	/// Klasse für die initialisierung eines D3DFensters
	/// </summary>
	class ClsD3D11
	{
	private:
		HWND m_hWnd;										// Target Window for displaying Data
		float m_fClearColor[4] = { 0.3f, 1.0f, 0.3f, 1.0f };// ClearColor
		FrameData* m_pFrameData;							// Frame Format Information
		
		// Device, Factory, SwapChain
		ComPtr<ID3D11Device> m_pD3dDevice;					// needed for LOW LVL Device Interface and for CreatingStuff in the Pipeline (write in VRAM)
		ComPtr<ID3D11DeviceContext> m_pD3dContext;			// needed for SettingStuff in the Pipeline (allocate VRAM)
		ComPtr<IDXGIDevice2> m_pDxgiDevice2;				// LOW DXGI LVL Interface needed for DXGI Factory
		ComPtr<IDXGIAdapter> m_pdxgiAdapter;				// needed for Creating a Factory
		ComPtr<IDXGIFactory2> m_pdxgiFactory2;				// needed for Creating a SwapChain
		
		// Swapchain
		ComPtr<IDXGISwapChain1> m_pDxgiSwapChain;			// needed for Backbuffer access
		ComPtr<ID3D11Texture2D> m_pBackBuffer;				// Backbuffer of the SwapChain
		ComPtr<ID3D11RenderTargetView> m_pTarRenderView;	// RenderTargetView, will be binded to the Backbuffer

		// Size SwapChain
		ClsCalcViewPort m_myClsCalcViewPort;

		// Displaying Texture, will be feeded with PixelData
		ComPtr<ID3D11Texture2D> m_pD3D11Texture;			// Texture that will be binded to the ShaderView
		ComPtr<ID3D11ShaderResourceView> m_pShaderView;		// Will be used in PixelShaderStage as ShaderRessource (finally the Output for the Display)

		// Shader Programs
		ComPtr<ID3DBlob> m_pVertexShaderBlob;				// Will be have the ProgramCode for the VertexShader
		ComPtr<ID3DBlob> m_pPixelShaderBlob;				// Will be have the ProgramCode for the PixelShader

		// Includes the Buffer, Description and Indices 
		// for the 2 Displaying triangles. Its the RECT for the Window
		IndexBufferContent m_myIndexBuffer;
		
		// DesktopDupl
		DXGI_OUTDUPL_FRAME_INFO m_myFrameInfo;				// additional Information of the Output
		D3D11_MAPPED_SUBRESOURCE m_mySubResD3D11Texture;	// SubRessource of the m_pD3D11TextureCPUAccess
		D3D11_TEXTURE2D_DESC m_myTextureDescCPUAccess;		// TextureDescription
		ComPtr<IDXGIOutputDuplication> m_pDeskDupl;			// Information of a MonitorOutput (DXGIOutput)
		ComPtr<ID3D11Texture2D> m_pD3D11TextureCPUAccess;	// Texture with CPU Access, same Data as m_pD3dAcquiredDesktopImage
		
		// D2D1 Interface
		ClsD2D1* m_pClsD2D1;
		
		// Constant buffer transformation, 
		// can be updated every frame per MatrixFunction without slow CPU operations
		ConstantBufferContent m_myConstantBuffer;				
		ClsTimer m_oTimer;									// Timer Obj for ConstantBuffer, Value for Matrixcalculation for every frame (Constantbuffer are not used yet)
	public:
		ClsD3D11();
		~ClsD3D11();
	public:
		void SetFrameData(FrameData** pFrameData);
		void SetWnd(HWND hWnd);
		HRESULT SetZoomPercentage(float fZoomPercentage);
		HRESULT AdjustD3D11Ratio();
		//HRESULT AdjustWindowSize(UINT uiPercentage);
		HRESULT BitBltDataToRT();
		HRESULT CreateAndSetSampler();
		HRESULT CreateAndSetShader();
		HRESULT CreateBuffers();
		HRESULT CreateD3D11Texture();
		HRESULT CreateDevice();
		HRESULT CreateInputLayout();
		HRESULT CreateShaderView();
		HRESULT CreateSwapChain();
		HRESULT CreateSwapChainBuffer();
		HRESULT PresentTexture();
		HRESULT SetConstantBuffer();
		ClsCalcViewPort& GetClsCalcViewPort();
	private:
		HRESULT CreateConstantBuffer();
		HRESULT CreateIndexBuffer();
		HRESULT CreatePixelShader();
		HRESULT CreateVertexBuffer();
		HRESULT CreateVertexShader();
		HRESULT BitBltDataToD2D1Resource();
		HRESULT BitBltDataToD3D11SubResource();
		HRESULT DesktopDuplToRAM(ComPtr<ID3D11Texture2D> pD3dAcquiredDesktopImage); // only full Desktop of a Monitor
		HRESULT GetDesktopDuplByGPU(); // only full Desktop of a Monitor
		HRESULT InitD2D1();
		HRESULT InitDesktopDuplication();
		HRESULT PrepareDesktopDuplToRAM(); // Prepare Variables for RAMDupl
		HRESULT UpdateMySubResource();
	};
}