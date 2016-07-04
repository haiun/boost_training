
// MFCClientDlg.h : 헤더 파일
//

#pragma once
#include "../Packet/TCP_Client.h"

// CMFCClientDlg 대화 상자
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

// 생성입니다.
public:
	CMFCClientDlg(CWnd* pParent = NULL);	// 표준 생성자입니다.
	~CMFCClientDlg();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MFCCLIENT_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 지원입니다.


// 구현입니다.
protected:
	HICON m_hIcon;

	// 생성된 메시지 맵 함수
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedLoginButton();
};
