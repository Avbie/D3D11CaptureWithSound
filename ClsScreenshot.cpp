#include "ClsScreenshot.h"
namespace GDI
{
	/// <summary>
	/// Constructor
	/// </summary>
	ClsScreenshot::ClsScreenshot()
	{
		m_dwFileSize = 0;
		m_hHeaderHandle = NULL;
		m_pFileData = NULL;
		m_pPixelData = NULL;
		m_pFileHeader = NULL;
		m_pInfoHeader = NULL;
		m_strBmpFileName = BMPFILENAME;
		m_pFrameData = NULL;
	}//END-CONS
	/// <summary>
	/// Create the two Headers for a BitmapFile.
	/// m_pFileData includes the whole Data of the BitmapFile.
	/// FileHeader: m_pFileHeader
	/// BitmapInfoHeader: m_pInfoHeader
	/// The PixelData can be adressed within m_pFileData (Position 54 = Offset from the 2 Headers)
	/// </summary>
	void  ClsScreenshot::CreateBmpFileHeader()
	{
		UINT& uiHeightDest = m_pFrameData->uiHeightDest;
		UINT& uiWidthDest = m_pFrameData->uiWidthDest;
		UINT& uiBpp = m_pFrameData->uiBpp;

		m_dwFileSize =
			sizeof(BITMAPFILEHEADER)								// Größe FileHeader
			+ sizeof(BITMAPINFOHEADER)								// Größe BmpHeader
			+ (sizeof(RGBQUAD)										// ExtraBytes für Dateiende (Größe eines Pixels)
				+ (static_cast<UINT64>(uiWidthDest) * 
					static_cast<UINT64>(uiHeightDest) * 
					static_cast<UINT64>(uiBpp))						// Größe der Pixeldaten
				);

		m_hHeaderHandle = GlobalAlloc(GMEM_ZEROINIT, m_dwFileSize);	// Speicher entsprechend der Pixeldaten reservieren
		
		if (m_hHeaderHandle == NULL)
		{
			printf("Cant allocate memory for BMP-File Header %u\n", GetLastError());
			return;
		}//END-IF
		
		m_pFileData = (unsigned char*)GlobalLock(m_hHeaderHandle);	// Speicher für diese Anwendung locken und Adresse in Pointer übergeben
		if (m_pFileData == NULL)
		{
			printf("Cant lock memory for BMP-File Header %u\n", GetLastError());
			return;
		}//END-IF
		m_pFileHeader = (PBITMAPFILEHEADER)m_pFileData;				// Adresse des FileHeaders (Beginnt natürlich beim Anfang des Buffers)
		m_pInfoHeader = (PBITMAPINFOHEADER)&m_pFileData[sizeof(BITMAPFILEHEADER)]; // Adresse des BmpHeaders durch Offset von FileHeader (Beginnt nach FileHeader)

		m_pFileHeader->bfType = 0x4D42;								// Format: in ASCII "BM" für Bitmap
		m_pFileHeader->bfSize = sizeof(BITMAPFILEHEADER);			    // FileHeaderSize
		m_pFileHeader->bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER); // Offset ab wann Pixeldaten beginnen dürfen

		m_pInfoHeader->biSize = sizeof(BITMAPINFOHEADER);			// BmpHeaderSize
		m_pInfoHeader->biPlanes = 1;								// Ebenen
		m_pInfoHeader->biBitCount = uiBpp * 8;						// Bits per Pixel
		m_pInfoHeader->biCompression = BI_RGB;						// Kompression BI_RGB = 0 = keine
		m_pInfoHeader->biHeight = uiHeightDest;						// Höhe
		m_pInfoHeader->biWidth = uiWidthDest;						// Breite

		m_pPixelData = &m_pFileData[m_pFileHeader->bfOffBits];		// Pointer zu Pixeldaten mit Hilfe des Offsets von den beiden Headern
	}//END-FUNC
	/// <summary>
	/// Generate a two-color-Picture
	/// It was used for testing
	/// </summary>
	void ClsScreenshot::GenerateBitBltData()
	{
		//unsigned char* pData = NULL;
		D3D11_TEXTURE2D_DESC TextureDesc = {};									// TextureDescription für TextureObj
		UINT& uiWidth = m_pFrameData->uiWidthDest;
		UINT& uiHeight = m_pFrameData->uiHeightDest;
		UINT& uiBpp = m_pFrameData->uiBpp;
		unsigned char* pData = m_pFrameData->pData;

		for (UINT y = 0; y < uiHeight; ++y)
		{
			for (UINT x = 0; x < uiWidth; ++x)
			{
				if (x <= 1000)
				{
					pData[y * uiWidth * uiBpp + x * uiBpp] = 0xff; // red, 8 Bit
					pData[y * uiWidth * uiBpp + x * uiBpp + 1] = 0xff; // green, 8 Bit
					pData[y * uiWidth * uiBpp + x * uiBpp + 2] = 0x00; // blue, 8 Bit
					pData[y * uiWidth * uiBpp + x * uiBpp + 3] = 0xff; // alpha, 8 Bit
				}
				else
				{
					pData[y * uiWidth * uiBpp + x * uiBpp] = 0xff; // red, 8 Bit
					pData[y * uiWidth * uiBpp + x * uiBpp + 1] = 0x00; // green, 8 Bit
					pData[y * uiWidth * uiBpp + x * uiBpp + 2] = 0x00; // blue, 8 Bit
					pData[y * uiWidth * uiBpp + x * uiBpp + 3] = 0xff; // alpha, 8 Bit
					// mit (RGBQUAD*)pData;
					//pRGBQuad->rgbReserved = pData[y * uiWidthDest * uiBpp + x * uiBpp + 3] = 0xff; // alpha, 8 Bit
				}//END-IF
			}//END-FOR x

		}//END-FOR y
	}//END-FUNC
	/// <summary>
	/// Set the FrameDataPointer. Includes Information about the FrameFormat.
	/// CalledBy: SuperClass ClsD3D11Recording
	/// </summary>
	/// <param name="ppFrameData">Adress of the Pointer of the FrameDataStructure</param>
	void ClsScreenshot::SetFrameData(FrameData** ppFrameData)
	{
		m_pFrameData = *ppFrameData;
	}//END-FUNC
	/// <summary>
	/// Fall1:
	/// Beim Kopieren, wenn Fenster die Quelle ist bzw keine Desktopduplication
	/// Kopieren der Pixeldaten von hBitmap das sich in hMemDC befindet.
	/// GetDIBits braucht HDC (hMemDC) und hBitmap um die Pixeldaten zu kopieren.
	/// Fall2:
	/// Kopieren der Pixeldaten vom Container der sie von der DesktopDuplication bekommen hat
	/// Hier erfolgt eine Umkehrung von unten links nach oben rechts ZU oben links nach unten rechts
	/// (Notwendig weil es beim Bitmap nunmal so ist. GetDIBits macht das ebenfalls)
	/// CalledBy: ClsWinGDI::TakeScreenshot()
	/// </summary>
	/// <param name="hMemDC">Compatibles Memory HDC des Device Contextes vom Fensterhandle</param>
	/// <param name="hBitmap">HBitmap Bitmap im Memory</param>
	/// <param name="uiWindowFlag">Flag ob Quelle Desktopduplication oder Fensterhandle ist</param>
	/// <returns>HRESULT</returns>
	HRESULT ClsScreenshot::BitBltToFile(HDC hMemDC, HBITMAP hBitmap, UINT uiWindowFlag)
	{
		HRESULT hr = NULL;
		HANDLE hFile = NULL;
		DWORD dwWritten = 0;
		int iErr = 0;
		UINT iNewYOrder = 0;
		UINT& uiWidth = m_pFrameData->uiWidthDest;
		UINT& uiHeight = m_pFrameData->uiHeightDest;
		UINT& uiBpp = m_pFrameData->uiBpp;
		unsigned char* pData = m_pFrameData->pData;

		CreateBmpFileHeader();								// BMP-FileHEader

		if (uiWindowFlag == WNDFOUND)
		{
			iErr = GetDIBits(								// Bitweises kopieren
				hMemDC,									    // von MemDC
				hBitmap,									// Unterobjekt-Bitmap von MemDC
				0,											// StartPosition ab welches Bit kopiert wird
				uiHeight,								    // Zeilen		
				m_pPixelData,								// Buffer, wo Pixeldaten reinkopiert werden. Ist PixelBuffer meines BmpFiles
				(LPBITMAPINFO)m_pInfoHeader,				// BmpHeader
				DIB_RGB_COLORS);
			HR_RETURN_ON_INT_ERR(iErr);
		}
		else
		{
			for (int y = uiHeight - 1; y >= 0; y--)
			{
				for (int x = 0; x < uiWidth; x++)
				{
					m_pPixelData[iNewYOrder * uiWidth * uiBpp + x * uiBpp] = pData[y * uiWidth * uiBpp + x * uiBpp];
					m_pPixelData[iNewYOrder * uiWidth * uiBpp + x * uiBpp + 1] = pData[y * uiWidth * uiBpp + x * uiBpp + 1];
					m_pPixelData[iNewYOrder * uiWidth * uiBpp + x * uiBpp + 2] = pData[y * uiWidth * uiBpp + x * uiBpp + 2];
					m_pPixelData[iNewYOrder * uiWidth * uiBpp + x * uiBpp + 3] = pData[y * uiWidth * uiBpp + x * uiBpp + 3];
				}
				iNewYOrder++;
			}
		}
		// File erstellen
		hFile = CreateFileA(
			m_strBmpFileName,
			GENERIC_WRITE,
			FILE_SHARE_WRITE,
			0,
			CREATE_ALWAYS, 0, 0);
		HR_RETURN_ON_HANDLE_ERR(hFile);
		// Bufferdaten in File schreiben
		iErr = WriteFile(
			hFile,
			m_pFileData,
			m_dwFileSize,
			&dwWritten, 0);
		HR_RETURN_ON_INT_ERR(iErr);

		CloseHandle(hFile);									// File unlocken
		GlobalUnlock(m_hHeaderHandle);						// allocateten Speicher für das BMP-File wieder freigeben
		GlobalFree(m_hHeaderHandle);						// Speicher freigeben
		return hr;
	}//END-FUNC
}//END-NAMESPACE GDI