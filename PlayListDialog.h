#pragma once
#include "framework.h"


// PlayListDialog 对话框

class PlaylistController;

class PlayListDialog : public CDialogEx
{
	DECLARE_DYNAMIC(PlayListDialog)

public:
	PlayListDialog(CWnd* pParent = nullptr);   // 标准构造函数
	~PlayListDialog() override;
	void SetPlaylistController(PlaylistController* pPlaylistController);

	int OnInitDialog() override;

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOGPLAYLIST };
#endif

protected:
	void DoDataExchange(CDataExchange* pDX) override;    // DDX/DDV 支持

	CListCtrl m_listCtrlPlayList;
	PlaylistController* m_pPlaylistController;
	afx_msg LRESULT OnMainDialogNotify(WPARAM wParam, LPARAM lParam);
	afx_msg void OnMenuPlayListCtrlPlaySelected();
	afx_msg void OnMenuPlayListCtrlPlayNextSelected();
	afx_msg void OnMenuPlayListCtrlDeleteSelected();
	afx_msg void OnMenuPlayListCtrlClearPlaylist();
	DECLARE_MESSAGE_MAP()

	int m_nDragIndex = -1;
	BOOL m_bDragging = FALSE, m_bAdjustParentPosition = FALSE;
	CImageList* m_pDragImage = nullptr;
	CDialogEx* m_pParent = nullptr;

	void RefreshPlaylist();

	afx_msg void OnNMDblclkListplaylist(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnLvnBegindragListplaylist(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	void PostNcDestroy() override;
	afx_msg void OnDestroy();
	afx_msg void OnCancel();
	afx_msg void OnOK();
	afx_msg void OnMove(int cx, int cy);
public:
	void SetAdjustParentPosition(BOOL bAdjustParentPosition) { m_bAdjustParentPosition = bAdjustParentPosition; }
};
