
// MicroAutoFocus.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CMicroAutoFocusApp: 
// �йش����ʵ�֣������ MicroAutoFocus.cpp
//

class CMicroAutoFocusApp : public CWinApp
{
public:
	CMicroAutoFocusApp();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CMicroAutoFocusApp theApp;