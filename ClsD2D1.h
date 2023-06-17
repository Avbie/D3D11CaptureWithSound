#pragma once
//#include "ClsDataContainer.h"
#include "framework.h"
namespace D3D
{
	class ClsD2D1
	{
	public:
		friend class ClsD3D11;
	public:
		ClsD2D1(ComPtr<ID3D11Texture2D> pD3D11Texture, HWND hWnd);
	private:
		void SetFrameData(FrameData** pFrameData);
		HRESULT InitD2D1RT();
		HRESULT CreateBitmapForD2D1RT();
		HRESULT UpdateD2D1RenderTarget();
	private:
		UINT m_uiDPI;
		UINT m_uiPitch = 0;
		D2D1_RECT_U m_MyBitmapFormat;
		D2D1_RECT_F m_MyRTFormat;
		ComPtr<ID3D11Texture2D> m_pD3D11Texture;
		ComPtr<ID2D1Bitmap> m_pD2dBitmap;
		ComPtr<ID2D1Factory2> m_pD2dFactory;
		ComPtr<ID2D1RenderTarget> m_pD2dRenderTarget;
		FrameData* m_pFrameData;
	};
}