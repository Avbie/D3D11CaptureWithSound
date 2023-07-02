#include "ClsCalcViewPort.h"

namespace D3D
{
	ClsCalcViewPort::ClsCalcViewPort()
	{
		m_hWnd = NULL;
		m_uiD3DWndHeight = 0;
		m_uiD3DWndWidth = 0;
		m_iD3DWndTopX = 0;
		m_iD3DWndTopY = 0;
		m_myViewPort = {};
		m_myViewPort.MaxDepth = 1.0f;
		m_myViewPort.MinDepth = 0.0f;
		m_pFrameData = NULL;
	}
	void ClsCalcViewPort::SetFrameData(FrameData** ppFrameData)
	{
		m_pFrameData = *ppFrameData;
	}//END-FUNC
	void ClsCalcViewPort::SetWnd(HWND hWnd)
	{
		m_hWnd = hWnd;
	}
	void ClsCalcViewPort::CalcViewPortSize()
	{
		UINT uiSrcHeight = m_pFrameData->uiHeightSrc;
		UINT uiSrcWidth = m_pFrameData->uiWidthSrc;
		UINT uiWndWidth = 0;
		UINT uiWndHeight = 0;
		double dRatio = 0;
		RECT myClientRectAppWnd = {};
		//RECT myClientRectSrcWnd = {};
		
		// Src Window could change the size during stream

		/*if (m_pFrameData->bWndAsSrc)
		{
			GetClientRect(m_pFrameData->pClsWndHandle->GetHandle(), &myClientRectSrcWnd);
			m_pFrameData->uiWidthSrc = myClientRectSrcWnd.right - myClientRectSrcWnd.left;
			m_pFrameData->uiHeightSrc = myClientRectSrcWnd.bottom - myClientRectSrcWnd.top;
			m_pFrameData->uiTop = 0;
			m_pFrameData->uiLeft = 0;
		}*/

		GetClientRect(m_hWnd, &myClientRectAppWnd);
		uiWndWidth = myClientRectAppWnd.right - myClientRectAppWnd.left;
		uiWndHeight = myClientRectAppWnd.bottom - myClientRectAppWnd.top;

		if (uiSrcHeight > uiSrcWidth)
		{
			dRatio = (double)uiSrcHeight / (double)uiSrcWidth;
		}
		else
		{
			dRatio = (double)uiSrcWidth / (double)uiSrcHeight;
		}
		// Height = Width*(9zu16) = width*(1/16zu9)
		m_uiD3DWndHeight = (UINT)(uiWndWidth * (1 / dRatio));
		m_uiD3DWndWidth = uiWndWidth;

		if (m_uiD3DWndHeight > uiWndHeight)
		{
			// Width = Height*16zu9
			m_uiD3DWndWidth = (UINT)(uiWndHeight * dRatio);
			m_uiD3DWndHeight = uiWndHeight;
		}
		m_myViewPort.Height = (FLOAT)m_uiD3DWndHeight;
		m_myViewPort.Width = (FLOAT)m_uiD3DWndWidth;
	}
	void ClsCalcViewPort::CalcViewPortPos()
	{
		UINT uiWndWidth = 0;
		UINT uiWndHeight = 0;
		RECT myClientRect = {};

		GetClientRect(m_hWnd, &myClientRect);
		uiWndWidth = myClientRect.right - myClientRect.left;
		uiWndHeight = myClientRect.bottom - myClientRect.top;
		// Das D3DWnd kann nicht grˆﬂer sein als das WinAPiWnd dank CalcNewWndSize()
		// zentriere Hˆhe falls WndHˆhe grˆﬂer D3DWinHˆhe
		if (uiWndWidth > m_uiD3DWndWidth)
		{
			m_iD3DWndTopX = (uiWndWidth - m_uiD3DWndWidth) / 2;
		}
		// falls gleich groﬂ, dann gibt es nicht szu zentrieren
		else if (uiWndWidth == m_uiD3DWndWidth)
			m_iD3DWndTopX = 0;
		// zentriere Breite falls WndBreite grˆﬂer D3DWinBreite
		if (uiWndHeight > m_uiD3DWndHeight)
		{
			m_iD3DWndTopY = (uiWndHeight - m_uiD3DWndHeight) / 2;
		}
		// falls gleich groﬂ, dann gibt es nicht szu zentrieren
		else if (uiWndHeight == m_uiD3DWndHeight)
			m_iD3DWndTopY = 0;

		m_myViewPort.TopLeftX = (FLOAT)m_iD3DWndTopX;
		m_myViewPort.TopLeftY = (FLOAT)m_iD3DWndTopY;
	}
	//void ClsCalcD3DWnd::
	void ClsCalcViewPort::CalcViewPortSizeAndPos()
	{
		CalcViewPortSize();
		CalcViewPortPos();
	}
	D3D11_VIEWPORT ClsCalcViewPort::GetViewPort()
	{
		return m_myViewPort;
	}
}