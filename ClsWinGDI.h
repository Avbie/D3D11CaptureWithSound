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
		
		void SetActiveWindow(UINT uiWndNr);
		//void SetCpyMethod(CopyMethod& myCpyMethod);
		void SetFrameData(FrameData** pFrameData);
		//void SetSrcWndTitle(const char* strTitle);
		void SetWndAsSrc(BOOL bWnd);
		ClsScreenshot& ScreenShot();
		vector<ActiveWnd>* GetWindowList();
		/*****************************/
		void CreateWindowList();
		HRESULT FindSetWindow();
		HRESULT GetBitBltDataFromWindow();
		HRESULT TakeScreenshot();
	private:
		void ClearObjects();
		static BOOL CALLBACK CreateWindowListProc(HWND hWnd, LPARAM lParam);
		static BOOL CALLBACK EnumWindowsProc(HWND hWnd, LPARAM lParam);
		BOOL IsScaled();
		BOOL IsWnd();
		//BOOL IsWndSet();
		
		HRESULT AllocateMemDC();
		HRESULT CopyBitmapDataToMemDC();
		//HRESULT GetResolutionOfWndHandle();
	};
}