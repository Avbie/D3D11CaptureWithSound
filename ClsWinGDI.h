#pragma once
#include "framework.h"
//#include "FrameData.h"
#include "ClsWndHandle.h"
#include "ClsScreenshot.h"

namespace GDI
{
	class ClsWinGDI
	{
	private:
		//BOOL m_bIsWnd;
		HANDLE m_hFileData;
		HDC m_hMemDC;
		HDC m_hDisplayDC;
		HBITMAP m_hBitmap;
		UINT m_uiWindowFlag;
		//CopyMethod m_myCpyMethod;
		const char* m_strTitle;
		char* m_strBmpFileName;
		FrameData* m_pFrameData;
		vector<ActiveWnd> m_vWndCollection;
		
		ClsScreenshot m_myClsScreenShot;
		ClsWndHandle m_myClsWndHandle;
	public:
		ClsWinGDI();
		~ClsWinGDI();
	public:
		/******Get-Set-Methods********/
		
		void SetFrameData(FrameData** pFrameData);
		BOOL SetActiveWindow(const UINT uiWndNr);
		const vector<ActiveWnd>& GetWindowList();
		ClsScreenshot& ScreenShot();
		ClsWndHandle& WndHandle();
		/*****************************/
		HRESULT CreateWindowList();
		HRESULT FindSetWindow();
		HRESULT GetBitBltDataFromWindow();
		HRESULT TakeScreenshot();
	private:
		void ClearObjects();
		static BOOL CALLBACK CreateWindowListProc(HWND hWnd, LPARAM lParam);
		static BOOL CALLBACK EnumWindowsProc(HWND hWnd, LPARAM lParam);
		BOOL IsScaled();
		HRESULT AllocateMemDC();
		HRESULT CopyBitmapDataToMemDC();
	};
}