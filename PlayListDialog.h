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
	void GetSelectedIndex();

	int OnInitDialog() override;

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOGPLAYLIST };
#endif

protected:
	void DoDataExchange(CDataExchange* pDX) override;    // DDX/DDV 支持

	CListCtrl m_listCtrlPlayList;
	PlaylistController* m_pPlaylistController;
	DECLARE_MESSAGE_MAP()

	int m_nDragIndex = -1;
	BOOL m_bDragging = FALSE;
	CImageList* m_pDragImage = nullptr;

	void RefreshPlaylist();

	afx_msg void OnLvnBegindragListplaylist(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);

};
