#pragma once
#include "framework.h"
#include "ClsWndHandle.h"
#include "ClsScreenshot.h"

namespace GDI
{
	class ClsWinGDI
	{
	private:
		HANDLE m_hFileData;
		HDC m_hMemDC;
		HDC m_hDisplayDC;
		HBITMAP m_hBitmap;
		UINT m_uiWindowFlag;
		CopyMethod m_myCpyMethod;
		const char* m_strTitle;
		char* m_strBmpFileName;
		FrameData* m_pFrameData;
		
		
		ClsScreenshot m_myClsScreenShot;
		ClsWndHandle m_MyClsWndHandle;
	public:
		ClsWinGDI();
		~ClsWinGDI();
	public:
		/******Get-Set-Methods********/
		ClsScreenshot& ScreenShot();
		void SetCpyMethod(CopyMethod& myCpyMethod);
		void SetFrameData(FrameData** pFrameData);
		void SetSrcWndTitle(const char* strTitle);
		/*****************************/
		HRESULT FindSetWindow();
		HRESULT GetBitBltDataFromWindow();
		HRESULT TakeScreenshot();
	private:
		static BOOL CALLBACK EnumWindowsProc(HWND hWnd, LPARAM lParam);
		BOOL IsScaled();
		BOOL IsWndTitle();
		HRESULT AllocateMemDC();
		HRESULT CopyBitmapDataToMemDC();
		HRESULT GetResolutionOfWndHandle();
	};
}