#pragma once
#include "framework.h"

class ClsSinkWMetaData
{
private:
	UINT m_uiBitRate;
	UINT m_uiDuration;
	UINT m_uiFPS;
	WCHAR m_wstrFilename[MAXSIZE] = L"";
	GUID m_MyOutputFormat = MFVideoFormat_RGB32; // uncompressed as default
	GUID m_MyInputFormat = MFVideoFormat_RGB32;
public:
	/// <summary>
	/// Set the FPS of the VideoFile for the Sinkwriter
	/// </summary>
	/// <param name="uiFPS"></param>
	void SetFPS(UINT uiFPS)
	{
		m_uiFPS = uiFPS;
		m_uiDuration = 1000 * 1000 * 10 / m_uiFPS;
		// Angabe in  Dauer in 100NanosekundenEinheiten
		// Wieviele 100-NanosekundenEinheiten ein Bild angezeigt werden soll:
		// 30FPS: 1/30 = 0,03Sekunden pro Bild
		// 1000*1000*10/30  = 333.333  100-Nanosekundeneinheiten pro Bild
		// 33.333.333Nanosekunden pro Bild = 0,03 Sekunden pro Bild 
	}
	UINT GetFPS()
	{
		return m_uiFPS;
	}
	UINT GetDuaration()
	{
		return m_uiDuration;
	}
	/// <summary>
	/// Set the BitRate of the VideoFile for the Sinkwriter
	/// </summary>
	/// <param name="uiBitRate"></param>
	void SetBitRate(UINT32 uiBitRate)
	{
		m_uiBitRate = uiBitRate;
	}//END-FUNC
	UINT GetBitRate()
	{
		return m_uiBitRate;
	}
	/// <summary>
	/// Sets the Input and/or Output Format.
	/// When Parameter is NULL, nothing will change
	/// </summary>
	/// <param name="MyInputFormat">SinkWriter InputFormat</param>
	/// <param name="MyOutputFormat">SinkWriter OutputFormat</param>
	void SetFormats(GUID MyInputFormat, GUID MyOutputFormat)
	{
		m_MyInputFormat = MyInputFormat;
		m_MyOutputFormat = MyOutputFormat;
	}
	GUID& GetInputFormat()
	{
		return m_MyInputFormat;
	}
	GUID& GetOutputFormat()
	{
		return m_MyOutputFormat;
	}
	/// <summary>
	/// Set the Name of the VideoFile for the Sinkwriter (char* to WideChar[256])
	/// </summary>
	/// <param name="strFileName"></param>
	void SetFileName(const char* strFileName)
	{
		// const char* to wchar
		size_t uiWritten;
		mbstowcs_s(&uiWritten, m_wstrFilename, strlen(strFileName) + 1, strFileName, strlen(strFileName));
	}//END-FUNC
	WCHAR* GetFileName()
	{
		return m_wstrFilename;
	}
};