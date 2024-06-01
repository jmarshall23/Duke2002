#pragma once

class CSplashScreen : public CWnd
{
public:
	CSplashScreen();
	~CSplashScreen();

	static void ShowSplashScreen(CWnd* pParentWnd = NULL);
	static void HideSplashScreen();

protected:
	void Hide();
	void PostNcDestroy();
	BOOL Create(CWnd* pParentWnd = NULL);
	void DrawSplash(CDC* pDC);

	static CSplashScreen* c_pSplashWnd;
	CBitmap m_bitmap;

	DECLARE_MESSAGE_MAP()
	afx_msg void OnPaint();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
};