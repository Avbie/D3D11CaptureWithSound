#pragma once
#include <cstdlib>
#include "framework.h"
#include "FrameData.h"
//#include "ClsCalcD3DWnd.h"

namespace D3D
{
	class ClsCalcViewPort
	{
	private:
		HWND m_hWnd;
		UINT m_uiD3DWndWidth;
		UINT m_uiD3DWndHeight;
		int m_iD3DWndTopX;
		int m_iD3DWndTopY;
		FrameData* m_pFrameData;
		D3D11_VIEWPORT m_myViewPort;
	public:
		ClsCalcViewPort();
		void SetFrameData(FrameData** ppFrameData);
		void SetWnd(HWND hWnd);
		void CalcViewPortSize();
		void CalcViewPortPos();
		void CalcViewPortSizeAndPos();
		D3D11_VIEWPORT GetViewPort();
	};
}