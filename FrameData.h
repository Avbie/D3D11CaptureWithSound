#pragma once
#include <Windows.h>
//#include "ClsWndHandle.h"

struct FrameData
{
public:
	FrameData()
	{
		bIsRecording = FALSE;
		bWndAsSrc = FALSE;
		pIsAudio = FALSE;
		uiBpp = 0;
		uiFPS = DEFFPS;
		uiHeightDest = 0; // Target Res for Sinkwriter e.g. 2560x1440
		uiWidthDest = 0;
		uiHeightSrc = 0;	// SourceResolution of the Source e.g. Monitor 1920x1080
		uiWidthSrc = 0;
		uiPixelDataSize = 0;
		uiTop = 0;			// Window using virtual Monitor System 2. Monitor e.g. Mon1Width + Mon2Width
		uiLeft = 0;
		pCpyMethod = NULL;
		pPickedMonitor = NULL;
		//pClsWndHandle = NULL;
		pData = NULL;

	}
public:
	BOOL bIsRecording;
	BOOL bWndAsSrc;
	BOOL* pIsAudio;
	UINT uiBpp;
	UINT uiFPS;
	UINT uiHeightDest; // Target Res for Sinkwriter e.g. 2560x1440
	UINT uiWidthDest;
	UINT uiHeightSrc;	// SourceResolution of the Source e.g. Monitor 1920x1080
	UINT uiWidthSrc;
	UINT uiPixelDataSize;
	UINT uiTop;			// Window using virtual Monitor System 2. Monitor e.g. Mon1Width + Mon2Width
	UINT uiLeft;
	UINT* pPickedMonitor;
	CopyMethod* pCpyMethod;
	//GDI::ClsWndHandle* pClsWndHandle;
	unsigned char* pData;
};