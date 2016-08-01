
// MFCClientDlg.h : 헤더 파일
//

#pragma once
#include "../Packet/TCP_Client.h"
#include <map>

// CMFCClientDlg 대화 상자
class CMFCClientDlg : public CDialogEx
{
public:
	boost::asio::io_service m_service;
	TCP_Client* m_pClient;
	std::vector< boost::thread* > m_pServiceThread;

	void PacketProc(PacketBase* pBase)
	{
		switch (pBase->id)
		{
		case MOVE:
			MovePacket* pMove = pBase->Cast<MovePacket>();

			CStatic* pTargetStatic = nullptr;
			std::map<std::size_t, CStatic*>::iterator find = unitMap.find(pMove->sessionID);
			if (find == unitMap.end())
			{
				pTargetStatic = new CStatic();

				TCHAR buffer[20] = { 0, };
				_itot_s((int)pMove->sessionID, buffer, 10);
				pTargetStatic->Create(buffer, WS_CHILD | WS_VISIBLE | SS_LEFT, CRect(pMove->position[0], pMove->position[1], pMove->position[0]+30, pMove->position[1]+30), this);

				unitMap.insert(std::make_pair(pMove->sessionID, pTargetStatic));
			}
			else
			{
				pTargetStatic = find->second;
				pTargetStatic->MoveWindow(CRect(pMove->position[0], pMove->position[1], pMove->position[0] + 30, pMove->position[1] + 30));
			}
			pTargetStatic->UpdateData(FALSE);
			pTargetStatic->Invalidate();

			break;
		}
	}

	std::map<std::size_t, CStatic*> unitMap;

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
	afx_msg void OnClose();
};
