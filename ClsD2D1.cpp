#include "ClsD2D1.h"

namespace D3D
{
	/// <summary>
	/// Constructor
	/// </summary>
	/// <param name="pD3D11Texture">ID3D11Texture2D, must be already init. by the ClsD3D11-class</param>
	/// <param name="hWnd">Target Window Handle</param>
	ClsD2D1::ClsD2D1(ComPtr<ID3D11Texture2D> pD3D11Texture, HWND hWnd)
	{
		m_uiDPI = GetDpiForWindow(hWnd);
		m_uiPitch = 0;
		m_MyBitmapFormat = {};
		m_MyRTFormat = {};

		m_pD3D11Texture = pD3D11Texture;
		m_pD2dBitmap.ReleaseAndGetAddressOf();
		m_pD2dFactory.ReleaseAndGetAddressOf();
		m_pD2dRenderTarget.ReleaseAndGetAddressOf();
		m_pFrameData = NULL;
	}
	void ClsD2D1::SetFrameData(FrameData** ppFrameData)
	{
		m_pFrameData = *ppFrameData;
	}
	/// <summary>
	/// Cast the ID3D11Texture2D in a DXGISurface (D2D1 Texture)
	/// Creates a ID2D1Factory2
	/// With the help of the Factory it creates a ID2D1RenderTarget
	/// Init. Rendertarget of the DXGISurface
	/// </summary>
	/// <returns>HRESULT</returns>
	HRESULT ClsD2D1::InitD2D1RT()
	{
		HRESULT hr = NULL;
		IDXGISurface* pDxgiSurface = NULL;
		D2D1_RENDER_TARGET_PROPERTIES MyD2D1RTProperties;

		// Description of the RenderTarget. The RenderTarget will get the dxgiSurface as Source later
		MyD2D1RTProperties = D2D1::RenderTargetProperties(
			D2D1_RENDER_TARGET_TYPE_DEFAULT,
			D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE),
			m_uiDPI,
			m_uiDPI);
		// Create a d2d1 Factory
		HR_RETURN_ON_ERR(hr, D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, IID_PPV_ARGS(&m_pD2dFactory)));
		// Creates the dxgiSurface from the D3D11Texture, 
		//	- D3D11Texture will be the Source of ShaderView
		//	- ShaderView will be set as Source for the PixelShader
		//	- PixelShader will send it to the OutputMerger during the PipelineProcess (to the m_pBackBuffer of the D3D11 Swapchain)
		HR_RETURN_ON_ERR(hr, m_pD3D11Texture->QueryInterface(&pDxgiSurface));
		// D2D1RenderTarget is binded to the Surface, Surface is binded to the D3D11Texture, ShaderView in PixelShader
		HR_RETURN_ON_ERR(hr, m_pD2dFactory->CreateDxgiSurfaceRenderTarget(
			pDxgiSurface,
			&MyD2D1RTProperties,
			&m_pD2dRenderTarget));

		return hr;
	}
	/// <summary>
	/// Creates a ID2D1Bitmap for the ID2D1RenderTarget
	/// Bitmap can be updated with the GDI-Pixeldata
	/// </summary>
	/// <returns>HRESULT</returns>
	HRESULT ClsD2D1::CreateBitmapForD2D1RT()
	{
		HRESULT hr = NULL;

		D2D1_SIZE_U MyBitmapSize = {};
		D2D1_PIXEL_FORMAT MyPixelFormat = {};
		D2D1_BITMAP_PROPERTIES MyBitmapProperty = {};

		UINT& uiHeightDest = m_pFrameData->uiHeightDest;
		UINT& uiWidthDest = m_pFrameData->uiWidthDest;
		UINT& uiBpp = m_pFrameData->uiBpp;
		unsigned char* pData = m_pFrameData->pData;

		m_MyBitmapFormat = D2D1::RectU(0, 0, uiWidthDest, uiHeightDest); // the Rectangle that will be copied from pData to m_pD2dBitmap
		m_MyRTFormat = D2D1::RectF(0, 0, uiWidthDest, uiHeightDest); // the Rectangle that will be shown in the m_pD2dRenderTarget
		//unsigned char* pData = m_pData;
		MyBitmapSize = D2D1::SizeU(uiWidthDest, uiHeightDest); // Size of the Bitmap
		m_uiPitch = uiWidthDest * uiBpp;	// Row in Bytes
		MyPixelFormat = D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE);
		MyBitmapProperty = D2D1::BitmapProperties(MyPixelFormat, m_uiDPI, m_uiDPI);

		if (!m_pD2dBitmap)
			HR_RETURN_ON_ERR(hr, m_pD2dRenderTarget->CreateBitmap(MyBitmapSize, pData, m_uiPitch, &MyBitmapProperty, &m_pD2dBitmap));

		return hr;
	}
	/// <summary>
	/// Updates the ID2D1Bitmap with the current Frame/GDI-Pixeldata
	/// </summary>
	/// <returns>HRESULT</returns>
	HRESULT ClsD2D1::UpdateD2D1RenderTarget()
	{
		HRESULT hr = NULL;
		unsigned char* pData = m_pFrameData->pData;

		m_pD2dRenderTarget->BeginDraw();
		HR_RETURN_ON_ERR(hr, m_pD2dBitmap->CopyFromMemory(&m_MyBitmapFormat, pData, m_uiPitch));
		m_pD2dRenderTarget->DrawBitmap(m_pD2dBitmap.Get(), &m_MyRTFormat, 1, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR);
		HR_RETURN_ON_ERR(hr, m_pD2dRenderTarget->EndDraw());

		return hr;
	}
}