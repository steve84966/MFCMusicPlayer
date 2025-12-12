// PlayListDialog.cpp: 实现文件
//

#include "pch.h"
#include "MFCMusicPlayer.h"
#include "afxdialogex.h"
#include "PlayListDialog.h"
#include "PlaylistController.h"


// PlayListDialog 对话框

IMPLEMENT_DYNAMIC(PlayListDialog, CDialogEx)

PlayListDialog::PlayListDialog(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOGPLAYLIST, pParent), m_pPlaylistController(nullptr) {
}

PlayListDialog::~PlayListDialog()
{
}

void PlayListDialog::SetPlaylistController(PlaylistController *pPlaylistController) {
	m_pPlaylistController = pPlaylistController;
}

int PlayListDialog::OnInitDialog() {

	if (m_pPlaylistController) {

	}

	CDialogEx::OnInitDialog();
	if (m_pPlaylistController) {
		m_listCtrlPlayList.SetExtendedStyle(
			LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

		CRect rect;
		m_listCtrlPlayList.GetClientRect(&rect);
		int totalWidth = rect.Width();

		m_listCtrlPlayList.InsertColumn(0, _T("Index"), LVCFMT_CENTER, 60);
		m_listCtrlPlayList.InsertColumn(1, _T("文件名"), LVCFMT_LEFT, totalWidth - 60 - 20);

		RefreshPlaylist();
	}
	return TRUE;
}

void PlayListDialog::RefreshPlaylist()
{
	if (!m_pPlaylistController)
		return;

	m_listCtrlPlayList.DeleteAllItems();

	for (int i = 0; i < static_cast<int>(m_pPlaylistController->GetPlaylistSize()); i++) {
		CString filePath = m_pPlaylistController->GetMusicFileAt(i);

		CString fileName = filePath;
		int pos = filePath.ReverseFind(_T('\\'));
		if (pos != -1) {
			fileName = filePath.Mid(pos + 1);
		}

		CString indexStr;
		indexStr.Format(_T("%d"), i + 1);
		m_listCtrlPlayList.InsertItem(i, indexStr);

		m_listCtrlPlayList.SetItemText(i, 1, fileName);

		m_listCtrlPlayList.SetItemData(i, i);
	}
}

void PlayListDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LISTPLAYLIST, m_listCtrlPlayList);
}


BEGIN_MESSAGE_MAP(PlayListDialog, CDialogEx)
	ON_NOTIFY(LVN_BEGINDRAG, IDC_LISTPLAYLIST, &PlayListDialog::OnLvnBegindragListplaylist)
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
END_MESSAGE_MAP()


// PlayListDialog 消息处理程序


void PlayListDialog::OnLvnBegindragListplaylist(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);

	m_nDragIndex = pNMLV->iItem;

	POINT pt = { 0, 0 };
	m_pDragImage = m_listCtrlPlayList.CreateDragImage(m_nDragIndex, &pt);
	if (m_pDragImage) {
		m_pDragImage->BeginDrag(0, CPoint(0, 0));
		m_pDragImage->DragEnter(this, pNMLV->ptAction);
		m_bDragging = TRUE;
		SetCapture();
	}

	*pResult = 0;
}

void PlayListDialog::OnMouseMove(UINT nFlags, CPoint point)
{
	if (m_bDragging && m_pDragImage) {
		m_pDragImage->DragMove(point);
	}

	CDialogEx::OnMouseMove(nFlags, point);
}

void PlayListDialog::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (m_bDragging) {
		m_bDragging = FALSE;

		if (m_pDragImage) {
			m_pDragImage->DragLeave(this);
			m_pDragImage->EndDrag();
			delete m_pDragImage;
			m_pDragImage = nullptr;
		}

		ReleaseCapture();

		CPoint ptList = point;
		m_listCtrlPlayList.ScreenToClient(&ptList);
		ClientToScreen(&point);
		m_listCtrlPlayList.ScreenToClient(&point);

		int nDropIndex = m_listCtrlPlayList.HitTest(point);
		if (nDropIndex == -1) {
			nDropIndex = m_listCtrlPlayList.GetItemCount() - 1;
		}

		if (m_nDragIndex != nDropIndex && m_nDragIndex >= 0 && m_pPlaylistController) {
			m_pPlaylistController->MoveItem(m_nDragIndex, nDropIndex);
			RefreshPlaylist();

			m_listCtrlPlayList.SetItemState(nDropIndex, LVIS_SELECTED | LVIS_FOCUSED,
				LVIS_SELECTED | LVIS_FOCUSED);
		}

		m_nDragIndex = -1;
	}

	CDialogEx::OnLButtonUp(nFlags, point);
}
