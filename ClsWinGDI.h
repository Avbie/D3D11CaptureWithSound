#pragma once
#include "framework.h"
//#include "ClsDataContainer.h"
#include "ClsWndCallback.h"
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
		ClsWndHandle m_MyClsWndHandle;
		CopyMethod m_myCpyMethod;
		const char* m_strTitle;
		char* m_strBmpFileName;
		FrameData* m_pFrameData;
		
	public:
		ClsWinGDI();
		~ClsWinGDI();
	public:
		ClsScreenshot ScreenShot;
	public:
		void SetFrameData(FrameData** pFrameData);
		void SetCpyMethod(CopyMethod& myCpyMethod);
		void SetSrcWndTitle(const char* strTitle);
		HRESULT GetBitBltDataFromWindow();
		HRESULT FindSetWindow();
		HRESULT TakeScreenshot();
	private:
		BOOL IsScaled();
		BOOL IsWndTitle();
		HRESULT AllocateMemDC();
		HRESULT GetResolutionOfWndHandle();
		HRESULT CopyBitmapDataToMemDC();
	};
}