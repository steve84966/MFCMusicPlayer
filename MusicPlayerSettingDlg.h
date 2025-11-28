#pragma once
#include "afxdialogex.h"


// MusicPlayerSettingDlg 对话框

class MusicPlayerSettingDlg : public CDialogEx
{
	DECLARE_DYNAMIC(MusicPlayerSettingDlg)

public:
	MusicPlayerSettingDlg(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~MusicPlayerSettingDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MUSICPLAYERSETTINGDLG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CTabCtrl m_sysTabSettings;
};
