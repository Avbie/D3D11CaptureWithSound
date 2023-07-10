#pragma once
#include "framework.h"
#include "FrameData.h"
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
	private:
		void CreateBmpFileHeader();
		void GenerateBitBltData();
		void SetFrameData(FrameData** pFrameData);
		HRESULT BitBltToFile(const HDC hMemDC, const HBITMAP hBitmap, const UINT uiWindowFlag);
		
	};
}