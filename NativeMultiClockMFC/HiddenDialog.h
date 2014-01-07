#pragma once


// HiddenDialog dialog

class HiddenDialog : public CDialog
{
	DECLARE_DYNAMIC(HiddenDialog)

public:
	HiddenDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~HiddenDialog();

// Dialog Data
	enum { IDD = IDD_HIDDENDIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

protected:
	void ShowContextMenu(bool expert);
	afx_msg LRESULT OnCustomTrayIcon(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnClose(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnUnhookInjection(WPARAM wParam, LPARAM lParam);
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

	static int GetClockCount();
};
