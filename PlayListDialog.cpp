// PlayListDialog.cpp: 实现文件
//

#include "pch.h"
#include "MFCMusicPlayer.h"
#include "afxdialogex.h"
#include "PlayListDialog.h"

#include "MFCMusicPlayerDlg.h"
#include "PlaylistController.h"


// PlayListDialog 对话框

IMPLEMENT_DYNAMIC(PlayListDialog, CDialogEx)

PlayListDialog::PlayListDialog(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOGPLAYLIST, pParent), m_pPlaylistController(nullptr) {
}

PlayListDialog::~PlayListDialog() = default;

void PlayListDialog::SetPlaylistController(PlaylistController *pPlaylistController) {
	m_pPlaylistController = pPlaylistController;
}

int PlayListDialog::OnInitDialog() {

	CDialogEx::OnInitDialog();
	if (m_pPlaylistController) {
		m_listCtrlPlayList.SetExtendedStyle(
			m_listCtrlPlayList.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
		m_listCtrlPlayList.ModifyStyle(0, LVS_SHOWSELALWAYS);

		CRect rect;
		m_listCtrlPlayList.GetClientRect(&rect);
		int totalWidth = rect.Width();

		m_listCtrlPlayList.InsertColumn(0, _T("序号"), LVCFMT_CENTER, 60);
		m_listCtrlPlayList.InsertColumn(1, _T("文件名"), LVCFMT_LEFT, totalWidth - 60 - 20);

		RefreshPlaylist();

		int selectedIndex = m_pPlaylistController->GetCurrentIndex();
		ATLTRACE("info: cur selected-index = %d", selectedIndex);
		if (selectedIndex >= 0 && selectedIndex < m_listCtrlPlayList.GetItemCount())
		{
			m_listCtrlPlayList.SetItemState(selectedIndex, LVIS_SELECTED | LVIS_FOCUSED,
				LVIS_SELECTED | LVIS_FOCUSED);
			m_listCtrlPlayList.EnsureVisible(selectedIndex, FALSE);
		}
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

void PlayListDialog::OnMenuPlayListCtrlPlaySelected()
{
	POSITION pos = m_listCtrlPlayList.GetFirstSelectedItemPosition();
	if (pos == nullptr) {
		ATLTRACE("warn: no item selected to play!\n");
		return;
	}
	int selection = m_listCtrlPlayList.GetNextSelectedItem(pos);
	ATLTRACE("info: play selected index=%d\n", selection);
	m_pPlaylistController->SetIndex(selection);
	AfxGetMainWnd()->PostMessage(WM_PLAYLIST_CHANGED);
}

void PlayListDialog::OnMenuPlayListCtrlPlayNextSelected()
{
	POSITION pos = m_listCtrlPlayList.GetFirstSelectedItemPosition();
	if (pos == nullptr) {
		ATLTRACE("warn: no item selected to play!\n");
		return;
	}
	int selection = m_listCtrlPlayList.GetNextSelectedItem(pos);
	m_pPlaylistController->SetNextIndex(selection);
}

void PlayListDialog::OnMenuPlayListCtrlDeleteSelected()
{
	POSITION pos = m_listCtrlPlayList.GetFirstSelectedItemPosition();
	if (pos == nullptr) {
		ATLTRACE("warn: no item selected to play!\n");
		return;
	}
	int selection = m_listCtrlPlayList.GetNextSelectedItem(pos);
	CString prev_filename = m_pPlaylistController->GetMusicFileAt(m_pPlaylistController->GetCurrentIndex());
	if (selection >= 0) {
		m_pPlaylistController->MoveItem(selection, -1); // move to invalid index to delete
		RefreshPlaylist();
		int i = 0;
		for (; i < static_cast<int>(m_pPlaylistController->GetPlaylistSize()); i++) {
			CString cur_filename = m_pPlaylistController->GetMusicFileAt(i);
			if (cur_filename.Compare(prev_filename) == 0) {
				m_pPlaylistController->SetIndex(i);
				break;
			}
		}
		if (i == static_cast<int>(m_pPlaylistController->GetPlaylistSize())) {
			// deleted current playing item, notify to update
			AfxGetMainWnd()->PostMessage(WM_PLAYLIST_CHANGED);
		}
	}
}

void PlayListDialog::OnMenuPlayListCtrlClearPlaylist()
{
	if (m_pPlaylistController) {
		m_pPlaylistController->ClearPlaylist();
		RefreshPlaylist();
	}
	AfxGetMainWnd()->PostMessage(WM_PLAYLIST_CHANGED);
}


BEGIN_MESSAGE_MAP(PlayListDialog, CDialogEx)
	ON_NOTIFY(LVN_BEGINDRAG, IDC_LISTPLAYLIST, &PlayListDialog::OnLvnBegindragListplaylist)
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_WM_CONTEXTMENU()
	ON_WM_DESTROY()
	ON_COMMAND(ID_MENU_PLAYLISTCTRL_PLAYSELECTED, &PlayListDialog::OnMenuPlayListCtrlPlaySelected)
	ON_COMMAND(ID_MENU_PLAYLISTCTRL_PLAYNEXTSELECTED, &PlayListDialog::OnMenuPlayListCtrlPlayNextSelected)
	ON_COMMAND(ID_MENU_PLAYLISTCTRL_DELETESELECTED, &PlayListDialog::OnMenuPlayListCtrlDeleteSelected)
	ON_COMMAND(ID_MENU_PLAYLISTCTRL_CLEARPLAYLIST, &PlayListDialog::OnMenuPlayListCtrlClearPlaylist)
END_MESSAGE_MAP()


// PlayListDialog 消息处理程序


void PlayListDialog::OnLvnBegindragListplaylist(NMHDR* pNMHDR, LRESULT* pResult)
{
	auto pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);

	m_nDragIndex = pNMLV->iItem;
	ATLTRACE(_T("info: begin drag item index=%d\n"), m_nDragIndex);
	ATLTRACE(_T("info: action point x=%d, y=%d\n"),
			pNMLV->ptAction.x, pNMLV->ptAction.y);
	m_listCtrlPlayList.EnsureVisible(m_nDragIndex, FALSE);
	m_listCtrlPlayList.UpdateWindow();

	POINT pt = { 0, 0 };
	m_pDragImage = m_listCtrlPlayList.CreateDragImage(m_nDragIndex, &pt);
	if (m_pDragImage) {
		CPoint ptScreen(::GetMessagePos());
		CPoint ptDlg = ptScreen;
		ScreenToClient(&ptDlg);
		CPoint ptList = ptScreen;
		m_listCtrlPlayList.ScreenToClient(&ptList);

		CRect rcItem;
		m_listCtrlPlayList.GetItemRect(m_nDragIndex, &rcItem, LVIR_BOUNDS);
		CPoint ptHotSpot(ptList.x - rcItem.left, ptList.y - rcItem.top);

		m_pDragImage->BeginDrag(0, ptHotSpot);
		CImageList::DragEnter(this, ptDlg);
		m_bDragging = TRUE;
		SetCapture();
	} else {
		ATLTRACE(_T("warn: CreateDragImage failed!\n"));
	}

	*pResult = 0;
}

void PlayListDialog::OnMouseMove(UINT nFlags, CPoint point)
{
	if (m_bDragging && m_pDragImage) {
		CImageList::DragMove(point);
	}

	CDialogEx::OnMouseMove(nFlags, point);
}

void PlayListDialog::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (m_bDragging) {
		m_bDragging = FALSE;

		if (m_pDragImage) {
			CImageList::DragLeave(this);
			CImageList::EndDrag();
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

void PlayListDialog::OnContextMenu(CWnd* pWnd, CPoint point)
{
	CRect rect;
	m_listCtrlPlayList.GetClientRect(&rect);
	m_listCtrlPlayList.ScreenToClient(&point);
	if (rect.PtInRect(point))
	{
		CMenu menu;
		if (menu.LoadMenu(IDR_MENUPLAYLISTCTRL))
		{
			CMenu* pPopup = menu.GetSubMenu(0);
			ASSERT(pPopup != nullptr);

			m_listCtrlPlayList.ClientToScreen(&point);
			pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON,
				point.x, point.y, this);
		}
	}
	CDialogEx::OnContextMenu(pWnd, point);
}

void PlayListDialog::PostNcDestroy()
{
	CDialogEx::PostNcDestroy();
	delete this;
}

void PlayListDialog::OnDestroy()
{
	if (GetOwner()) {
		auto* pMainDlg = reinterpret_cast<CMFCMusicPlayerDlg*>(GetOwner());
		if (pMainDlg) pMainDlg->m_pPlaylistDlg = nullptr;
	}
}
