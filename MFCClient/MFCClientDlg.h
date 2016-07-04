
// MFCClientDlg.h : ��� ����
//

#pragma once
#include "../Packet/TCP_Client.h"

// CMFCClientDlg ��ȭ ����
class CMFCClientDlg : public CDialogEx
{
private:
	boost::asio::io_service m_service;
	TCP_Client* m_pClient;
	boost::thread* m_pServiceThread;

	void PacketProc(PacketBase* pBase)
	{
		int a = 0;
	}

// �����Դϴ�.
public:
	CMFCClientDlg(CWnd* pParent = NULL);	// ǥ�� �������Դϴ�.
	~CMFCClientDlg();

// ��ȭ ���� �������Դϴ�.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MFCCLIENT_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV �����Դϴ�.


// �����Դϴ�.
protected:
	HICON m_hIcon;

	// ������ �޽��� �� �Լ�
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedLoginButton();
};
