
// MFCClientDlg.h : ��� ����
//

#pragma once
#include "../Packet/TCP_Client.h"
#include <map>

// CMFCClientDlg ��ȭ ����
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
	afx_msg void OnClose();
};
