#pragma once
#include "framework.h"


// PlayListDialog 对话框

class PlayListDialog : public CDialogEx
{
	DECLARE_DYNAMIC(PlayListDialog)

public:
	PlayListDialog(CWnd* pParent = nullptr);   // 标准构造函数
	~PlayListDialog() override;

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOGPLAYLIST };
#endif

protected:
	void DoDataExchange(CDataExchange* pDX) override;    // DDX/DDV 支持

	CListCtrl m_listCtrlPlayList;
	DECLARE_MESSAGE_MAP()
};
