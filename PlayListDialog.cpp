// PlayListDialog.cpp: 实现文件
//

#include "pch.h"
#include "MFCMusicPlayer.h"
#include "afxdialogex.h"
#include "PlayListDialog.h"


// PlayListDialog 对话框

IMPLEMENT_DYNAMIC(PlayListDialog, CDialogEx)

PlayListDialog::PlayListDialog(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOGPLAYLIST, pParent)
{

}

PlayListDialog::~PlayListDialog()
{
}

void PlayListDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LISTPLAYLIST, m_listCtrlPlayList);
}


BEGIN_MESSAGE_MAP(PlayListDialog, CDialogEx)
END_MESSAGE_MAP()


// PlayListDialog 消息处理程序
