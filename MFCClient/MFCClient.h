
// MFCClient.h : PROJECT_NAME ���� ���α׷��� ���� �� ��� �����Դϴ�.
//

#pragma once

#ifndef __AFXWIN_H__
	#error "PCH�� ���� �� ������ �����ϱ� ���� 'stdafx.h'�� �����մϴ�."
#endif

#include "resource.h"		// �� ��ȣ�Դϴ�.


// CMFCClientApp:
// �� Ŭ������ ������ ���ؼ��� MFCClient.cpp�� �����Ͻʽÿ�.
//

class CMFCClientApp : public CWinApp
{
public:
	CMFCClientApp();

// �������Դϴ�.
public:
	virtual BOOL InitInstance();

// �����Դϴ�.

	DECLARE_MESSAGE_MAP()
};

extern CMFCClientApp theApp;