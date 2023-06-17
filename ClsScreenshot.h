#pragma once
#include "framework.h"
//#include "ClsDataContainer.h"
//#include "ClsBitBlt.h"

namespace GDI
{
	class ClsScreenshot
	{
	public:
		friend class ClsWinGDI;
	private:
		DWORD m_dwFileSize;
		HANDLE m_hHeaderHandle;
		const char* m_strBmpFileName;
		unsigned char* m_pFileData;
		unsigned char* m_pPixelData;
		PBITMAPFILEHEADER m_pFileHeader;
		PBITMAPINFOHEADER m_pInfoHeader;
		FrameData* m_pFrameData;

	public:
		ClsScreenshot();
		void SetFrameData(FrameData** pFrameData);
	private:
		HRESULT BitBltToFile(HDC hMemDC, HBITMAP hBitmap, UINT uiWindowFlag);
		void GenerateBitBltData();
		void  CreateBmpFileHeader();
	};
}