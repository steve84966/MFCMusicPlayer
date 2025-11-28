// MusicPlayerSettingDlg.cpp: 实现文件
//

#include "pch.h"
#include "MFCMusicPlayer.h"
#include "afxdialogex.h"
#include "MusicPlayerSettingDlg.h"


// MusicPlayerSettingDlg 对话框

IMPLEMENT_DYNAMIC(MusicPlayerSettingDlg, CDialogEx)

MusicPlayerSettingDlg::MusicPlayerSettingDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_MUSICPLAYERSETTINGDLG, pParent)
{

}

MusicPlayerSettingDlg::~MusicPlayerSettingDlg()
{
}

void MusicPlayerSettingDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TABSETTINGS, m_sysTabSettings);
}


BEGIN_MESSAGE_MAP(MusicPlayerSettingDlg, CDialogEx)
END_MESSAGE_MAP()


// MusicPlayerSettingDlg 消息处理程序
