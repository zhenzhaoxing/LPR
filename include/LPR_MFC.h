
// LPR_MFC.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CLPR_MFCApp: 
// �йش����ʵ�֣������ LPR_MFC.cpp
//

class CLPR_MFCApp : public CWinApp
{
public:
	CLPR_MFCApp();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CLPR_MFCApp theApp;