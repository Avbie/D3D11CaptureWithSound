#pragma once
#include "framework.h"

class ClsCalcD3DWnd
{
private:
	HWND m_hWnd;
	UINT m_uiD3DWndWidth;
	UINT m_uiD3DWndHeight;
	UINT m_uiD3DWndTopX;
	UINT m_uiD3DWndTopY;
	FrameData* m_pFrameData;
	D3D11_VIEWPORT m_myViewPort;
public:
	ClsCalcD3DWnd()
	{
		m_hWnd = NULL;
		m_uiD3DWndHeight = 0;
		m_uiD3DWndWidth = 0;
		m_uiD3DWndTopX = 0;
		m_uiD3DWndTopY = 0;
		m_myViewPort = {};
		m_myViewPort.MaxDepth = 1.0f;
		m_myViewPort.MinDepth = 0.0f;
		m_pFrameData = NULL;
	}
	void SetFrameData(FrameData** ppFrameData)
	{
		m_pFrameData = *ppFrameData;
	}//END-FUNC
	void SetWnd(HWND hWnd)
	{
		m_hWnd = hWnd;
	}
	void CalcNewWndSize()
	{
		UINT uiSrcHeight = m_pFrameData->uiHeightSrc;
		UINT uiSrcWidth = m_pFrameData->uiWidthSrc;
		UINT uiWndWidth = 0;
		UINT uiWndHeight = 0;
		double dRatio = 0;
		RECT myClientRect = {};
		
		GetClientRect(m_hWnd, &myClientRect);
		uiWndWidth = myClientRect.right - myClientRect.left;
		uiWndHeight = myClientRect.bottom - myClientRect.top;

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
	void CalcNewWndPos()
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
			m_uiD3DWndTopX = (uiWndWidth - m_uiD3DWndWidth) / 2;
		}
		// falls gleich groﬂ, dann gibt es nicht szu zentrieren
		else if (uiWndWidth == m_uiD3DWndWidth)
			m_uiD3DWndTopX = 0;
		// zentriere Breite falls WndBreite grˆﬂer D3DWinBreite
		if (uiWndHeight > m_uiD3DWndHeight)
		{
			m_uiD3DWndTopY = (uiWndHeight - m_uiD3DWndHeight) / 2;
		}
		// falls gleich groﬂ, dann gibt es nicht szu zentrieren
		else if (uiWndHeight == m_uiD3DWndHeight)
			m_uiD3DWndTopY = 0;

		m_myViewPort.TopLeftX = (FLOAT)m_uiD3DWndTopX;
		m_myViewPort.TopLeftY = (FLOAT)m_uiD3DWndTopY;
	}
	void CalcPosAndSize()
	{
		CalcNewWndSize();
		CalcNewWndPos();
	}
	D3D11_VIEWPORT GetD3DViewPort()
	{
		return m_myViewPort;
	}
	UINT GetD3DWndHeight()
	{
		return m_uiD3DWndHeight + m_uiD3DWndTopY;
	}
	UINT GetD3DWndWidth()
	{
		return m_uiD3DWndWidth + m_uiD3DWndTopX;
	}
};