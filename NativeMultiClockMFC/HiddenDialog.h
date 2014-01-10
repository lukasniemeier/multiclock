#pragma once


// HiddenDialog dialog

class HiddenDialog : public CDialog
{
	DECLARE_DYNAMIC(HiddenDialog)

public:
	HiddenDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~HiddenDialog();

	enum { IDD = IDD_HIDDENDIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg LRESULT OnTaskbarCreated(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnCustomTrayIcon(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnClose(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnDestroy(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnUnhookInjection(WPARAM wParam, LPARAM lParam);
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

protected:
	bool SetupNotification();
	void ShowContextMenu();
	static int GetClockCount();

private:
	NOTIFYICONDATA notificationData;
	bool isCreated;
	
};
