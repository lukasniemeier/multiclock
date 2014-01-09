#include "ClockWindow.h"
#include <WindowsX.h>
#include "Dwmapi.h"

#define FLYOUT_MARGIN 16
#define WM_USER_SHOW_FLYOUT (WM_USER + 102)

ClockWindow::ClockWindow()
{
	toolTipWindow = nullptr;
	isHighlighted = false;
	isClicked = false;

	trackMouseEventInfo.cbSize = sizeof(TRACKMOUSEEVENT);

	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	Gdiplus::Status status = GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
	if (status != Gdiplus::Status::Ok)
	{
		return;
	}
	
	INITCOMMONCONTROLSEX commonControls;
	commonControls.dwSize = sizeof(INITCOMMONCONTROLSEX);
	commonControls.dwICC = ICC_STANDARD_CLASSES | ICC_WIN95_CLASSES;
	::InitCommonControlsEx(&commonControls);
}

void ClockWindow::OnFinalMessage(HWND hwnd)
{
	Gdiplus::GdiplusShutdown(gdiplusToken);
}

void ClockWindow::RepositionIn(HWND taskbar)
{
	RECT targetRect;
	RECT taskbarRect;
	::GetClientRect(taskbar, &taskbarRect);

	UINT MARGIN = 2;
	Orientation orientation = GetTaskbarOrientation(taskbar);
	if (orientation == LEFT || orientation == RIGHT)
	{
		targetRect.left = taskbarRect.left + (orientation == LEFT ? 0 : MARGIN); 
		targetRect.right = taskbarRect.right - (orientation == LEFT ? 2 : MARGIN);

		int width = targetRect.right - targetRect.left;
		int height = width > 70 ? 53 : 36;

		targetRect.top = taskbarRect.bottom - height;
		targetRect.bottom = taskbarRect.bottom;
	}
	else
	{
		targetRect.left = taskbarRect.right - 75;
		targetRect.right = taskbarRect.right;
		targetRect.top = taskbarRect.top + MARGIN;
		targetRect.bottom = taskbarRect.bottom;
	}
	SetWindowPos(HWND_TOP, &targetRect, SWP_SHOWWINDOW | SWP_NOOWNERZORDER | SWP_NOACTIVATE);
}


LRESULT ClockWindow::OnLeftButtonUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	isClicked = false;
	Refresh();

	bHandled = true;
	return 0;
}

void ClockWindow::MoveClockFlyout() const
{
	HWND flyout = ::FindWindowEx(nullptr, nullptr, ORIGINAL_CLOCK_FLYOUT_CLASS, nullptr);
	if (flyout == nullptr)
	{
		return;
	}

	RECT flyoutRect;
	::GetWindowRect(flyout, &flyoutRect);
	::ShowWindow(flyout, SW_HIDE);

	RECT taskbarRect;
	::GetWindowRect(this->GetParent(), &taskbarRect);

	long flyoutHeight = flyoutRect.bottom - flyoutRect.top + FLYOUT_MARGIN;
	long flyoutWidth = flyoutRect.right - flyoutRect.left + FLYOUT_MARGIN;

	int x, y;
	Orientation orientation = GetTaskbarOrientation(this->GetParent());
	switch (orientation)
	{
	case LEFT:
		x = taskbarRect.right + FLYOUT_MARGIN;
		y = taskbarRect.bottom - flyoutHeight;
		break;
	case RIGHT:
		x = taskbarRect.left - flyoutWidth;
		y = taskbarRect.bottom - flyoutHeight;
		break;
	case TOP:
		x = taskbarRect.right - flyoutWidth;
		y = taskbarRect.bottom + FLYOUT_MARGIN;
		break;
	case BOTTOM:
		x = taskbarRect.right - flyoutWidth;
		y = taskbarRect.top - flyoutHeight;
		break;
	default:
		x = 0;
		y = 0;
	}

	::SetWindowPos(flyout, nullptr, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
	::ShowWindow(flyout, SW_SHOW);
}

LRESULT ClockWindow::OnLeftButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (toolTipWindow != nullptr)
	{
		::SendMessage(toolTipWindow, TTM_TRACKACTIVATE, (WPARAM)FALSE, (LPARAM)&toolTipInfo);
	}

	HWND clockWidget = ClockWindow::GetOriginalClock();
	if (clockWidget != nullptr)
	{
		::SendMessage(clockWidget, WM_USER_SHOW_FLYOUT, 0x1, 0x0);
		MoveClockFlyout();
	}

	isClicked = true;
	Refresh();

	bHandled = true;
	return 0;
}

void ClockWindow::StartTrackingOn(HWND taskbar)
{
	if (!isHighlighted)
	{
		trackMouseEventInfo.dwFlags = TME_LEAVE | TME_HOVER | TME_NONCLIENT;
		trackMouseEventInfo.hwndTrack = taskbar;
		if (::TrackMouseEvent(&trackMouseEventInfo))
		{
			isHighlighted = true;
			Refresh();
		}
	}
}

LRESULT ClockWindow::OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	return 0;
}

LRESULT ClockWindow::OnMouseHover(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (toolTipWindow == nullptr)
	{
		toolTipWindow = ::CreateWindowExW(WS_EX_TOPMOST,
			TOOLTIPS_CLASSW, 0, WS_POPUP | TTS_ALWAYSTIP | TTS_NOANIMATE | TTS_NOFADE,
			CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
			0, 0, GetModuleHandle(nullptr), 0);
		if (toolTipWindow != nullptr)
		{
			toolTipInfo = {};
			toolTipInfo.cbSize = sizeof(TOOLINFOW);
			toolTipInfo.uFlags = TTF_ABSOLUTE | TTF_TRACK;
			toolTipInfo.hwnd = *this;
			toolTipInfo.uId = (UINT)toolTipWindow;

			::SendMessage(toolTipWindow, TTM_ADDTOOLW, 0, (LPARAM)&toolTipInfo);
		}
	}

	if (toolTipWindow == nullptr)
	{
		return 0;
	}

	// TODO: correct tool tip
	//std::wstring time = GetTimeString(200);
	//toolTipInfo.lpszText = &time[0];
	toolTipInfo.lpszText = L"Freitag, 3. Januar 2014";

	HDC context = GetDC();
	Gdiplus::RectF rect;
	Gdiplus::Graphics g(context);
	Gdiplus::Font font(context, GetWindowFont(toolTipWindow));
	ReleaseDC(context);
	Gdiplus::Status status = g.MeasureString(toolTipInfo.lpszText, -1, &font, { 0.0f, 0.0f, }, &rect);
	
	RECT rc = { 0, 0, (LONG) ::ceil(rect.Width), (LONG) ::ceil(rect.Height) };
	::SendMessage(toolTipWindow, TTM_ADJUSTRECT, TRUE, (LPARAM)&rc);

	LONG toolTipWidth = rc.right - rc.left;
	LONG toolTipHeight = rc.bottom - rc.top;

	RECT taskbarRect;
	::GetWindowRect(this->GetParent(), &taskbarRect);

	LONG toolTipX, toolTipY;
	Orientation orientation = GetTaskbarOrientation(this->GetParent());
	switch (orientation)
	{
	case LEFT:
		toolTipX = taskbarRect.right;
		toolTipY = taskbarRect.bottom - toolTipHeight;
		break;
	case RIGHT:
		toolTipX = taskbarRect.left - toolTipWidth;
		toolTipY = taskbarRect.bottom - toolTipHeight;
		break;
	case BOTTOM:
		toolTipX = taskbarRect.right - toolTipWidth;
		toolTipY = taskbarRect.top - toolTipHeight;
		break;
	case TOP:
		toolTipX = taskbarRect.right - toolTipWidth;
		toolTipY = taskbarRect.bottom;
		break;
	default:
		toolTipX = 0;
		toolTipY = 0;
		break;
	}

	::SendMessage(toolTipWindow, TTM_UPDATETIPTEXT, 0, (LPARAM)&toolTipInfo);
	::SendMessage(toolTipWindow, TTM_TRACKPOSITION, 0, (LPARAM)MAKELONG(toolTipX, toolTipY));
	::SendMessage(toolTipWindow, TTM_TRACKACTIVATE, (WPARAM)TRUE, (LPARAM)&toolTipInfo);
	return 0;
}

LRESULT ClockWindow::OnMouseLeave(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	::SendMessage(toolTipWindow, TTM_TRACKACTIVATE, (WPARAM)FALSE, (LPARAM)&toolTipInfo);

	bool needsRefresh = false;
	if (isClicked)
	{
		isClicked = false;
		needsRefresh = true;
	}
	if (isHighlighted)
	{
		trackMouseEventInfo.dwFlags = TME_CANCEL | TME_LEAVE | TME_HOVER | TME_NONCLIENT;
		::TrackMouseEvent(&trackMouseEventInfo);
		isHighlighted = false;
		needsRefresh = true;
	}
	if (needsRefresh)
	{
		Refresh();
	}
	return 0;
}

LRESULT ClockWindow::OnInitMenu(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (isHighlighted)
	{
		HMENU hmenu = (HMENU)wParam;

		MENUITEMINFO seperator;
		seperator.cbSize = sizeof(MENUITEMINFO);
		seperator.fMask = MIIM_TYPE;
		seperator.fType = MFT_SEPARATOR;
		::InsertMenuItem(hmenu, 0, TRUE, &seperator);

		MENUITEMINFO info;
		info.cbSize = sizeof(MENUITEMINFO);
		info.fMask = MIIM_ID | MIIM_TYPE | MIIM_STATE;
		info.wID = 408;
		info.fType = MFT_STRING;
		info.fState = MFS_ENABLED;
		info.dwTypeData = L"Adjust date/time";
		info.cch = 16;

		::InsertMenuItem(hmenu, 0, TRUE, &info);
	}
	return 0;
}

LRESULT ClockWindow::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	HWND taskbar = this->GetParent();
	HWND workerW = ::FindWindowEx(taskbar, nullptr, TASKBAR_WORKER_CLASS, nullptr);
	HWND clock = ClockWindow::GetOriginalClock();
	::SetWindowSubclass(taskbar, ClockWindow::TaskbarSubclassProc, 0, (DWORD_PTR) this);
	::SetWindowSubclass(workerW, ClockWindow::WorkerWSubclassProc, 2, (DWORD_PTR) this);
	::SetWindowSubclass(clock, ClockWindow::OriginalClockSubclassProc, 1, (DWORD_PTR) this);

	int val = 1;
	DwmSetWindowAttribute(m_hWnd, DWMWA_EXCLUDED_FROM_PEEK, &val, sizeof(val));
	return 0;
}

LRESULT ClockWindow::OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	HWND taskbar = this->GetParent();
	HWND workerW = ::FindWindowEx(taskbar, nullptr, TASKBAR_WORKER_CLASS, nullptr);
	HWND clock = ClockWindow::GetOriginalClock();
	::RemoveWindowSubclass(taskbar, ClockWindow::TaskbarSubclassProc, 0);
	::RemoveWindowSubclass(workerW, ClockWindow::WorkerWSubclassProc, 2);
	::RemoveWindowSubclass(clock, ClockWindow::OriginalClockSubclassProc, 1);
	DestroyWindow();
	bHandled = FALSE;
	return 0;
}

LRESULT ClockWindow::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	bHandled = FALSE;
	return 0;
}

LRESULT CALLBACK ClockWindow::OriginalClockSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
	switch (uMsg)
	{
		case WM_TIMER:
		if (wParam == 0x0)
		{
			((ClockWindow*)dwRefData)->Refresh();
		}
		break;
		case WM_NCDESTROY:
		::RemoveWindowSubclass(hWnd, ClockWindow::OriginalClockSubclassProc, uIdSubclass);
		break;
	}

	return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK ClockWindow::TaskbarSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
	ClockWindow* clock = (ClockWindow*)dwRefData;

	BOOL what = FALSE;
	BOOL& handled = what;
	switch (uMsg)
	{
		case WM_LBUTTONDOWN:
		case WM_NCLBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_NCLBUTTONUP:
		case WM_MOUSEMOVE:
		case WM_NCMOUSEMOVE:
		case WM_MOUSEHOVER:
		case WM_NCMOUSEHOVER:
		{
			int xPos = GET_X_LPARAM(lParam);
			int yPos = GET_Y_LPARAM(lParam);
			POINT hit = { xPos, yPos };
			RECT rcButton;
			::GetWindowRect(*clock, &rcButton);

			LRESULT result = 0;
			if (PtInRect(&rcButton, hit) == TRUE)
			{
				if (uMsg == WM_NCLBUTTONDOWN || uMsg == WM_LBUTTONDOWN)
				{
					result = clock->OnLeftButtonDown(uMsg, wParam, lParam, handled);
				}
				if (uMsg == WM_NCLBUTTONUP || uMsg == WM_LBUTTONUP)
				{
					result = clock->OnLeftButtonUp(uMsg, wParam, lParam, handled);
				}
				else if (uMsg == WM_NCMOUSEMOVE || uMsg == WM_MOUSEMOVE)
				{
					clock->StartTrackingOn(hWnd);
					result = clock->OnMouseMove(uMsg, wParam, lParam, handled);
				}
				else if (uMsg == WM_NCMOUSEHOVER || uMsg == WM_MOUSEHOVER)
				{
					result = clock->OnMouseHover(uMsg, wParam, lParam, handled);
				}
			}
			else
			{
				result = clock->OnMouseLeave(uMsg, wParam, lParam, handled);
			}

			if (handled)
			{
				return result;
			}
			break;
		}
		case WM_MOUSELEAVE:
		case WM_NCMOUSELEAVE:
		{
			clock->OnMouseLeave(uMsg, wParam, lParam, handled);
			break;
		}
		case WM_WINDOWPOSCHANGED:
		{
			clock->RepositionIn(hWnd);
			clock->Refresh();
			break;
		}
		case WM_TIMECHANGE:
		clock->Refresh();
		break;
		case WM_INITMENU:
		{
			clock->OnInitMenu(uMsg, wParam, lParam, handled);
			break;
		}
		case WM_DESTROY:
		{
			clock->OnClose(WM_CLOSE, 0x0, 0x0, handled);
			break;
		}
		case WM_NCDESTROY:
		{
			::RemoveWindowSubclass(hWnd, ClockWindow::TaskbarSubclassProc, uIdSubclass);
			break;
		}
		default:
		break;
	}
	return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK ClockWindow::WorkerWSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
	ClockWindow* clock = (ClockWindow*)dwRefData;
	HWND taskbar = clock->GetParent();

	switch (uMsg)
	{
		case WM_WINDOWPOSCHANGING:
		{
			WINDOWPOS* pos = (WINDOWPOS*)lParam;
			if (!(pos->flags&SWP_NOMOVE) || !(pos->flags&SWP_NOSIZE))
			{
				RECT taskbarRect;
				taskbarRect.left = pos->x;
				taskbarRect.right = pos->x + pos->cx;
				taskbarRect.top = pos->y;
				taskbarRect.bottom = pos->y + pos->cy;

				RECT clockRect;
				::GetClientRect(*clock, &clockRect);
				::MapWindowPoints(*clock, taskbar, (LPPOINT)&clockRect, 2);
			
				Orientation orientation = GetTaskbarOrientation(taskbar);
				if (orientation == LEFT || orientation == RIGHT)
				{
					pos->cy -= (clockRect.bottom - clockRect.top);
				}
				else
				{
					pos->cx -= (clockRect.right - clockRect.left);
				}
				pos->flags &= ~SWP_NOSIZE;
			}
			break;
		}
		case WM_NCDESTROY:
		{
			::RemoveWindowSubclass(hWnd, ClockWindow::WorkerWSubclassProc, uIdSubclass);
			break;
		}
		default:
		break;
	}
	return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}

ClockWindow::Orientation ClockWindow::GetTaskbarOrientation(HWND hwnd)
{
	RECT rc;
	::GetWindowRect(hwnd, &rc);
	MONITORINFO info = { sizeof(info) };
	HMONITOR monitor = ::MonitorFromRect(&rc, MONITOR_DEFAULTTONEAREST);
	::GetMonitorInfo(monitor, &info);
	int dx = rc.left + rc.right - info.rcWork.left - info.rcWork.right;
	int dy = rc.top + rc.bottom - info.rcWork.top - info.rcWork.bottom;

	if (dx<-abs(dy)) return Orientation::LEFT;
	if (dx>abs(dy)) return Orientation::RIGHT;
	if (dy < -abs(dx)) return Orientation::TOP;
	return Orientation::BOTTOM;
}

HWND GlobalClockWidget;

BOOL CALLBACK SearchClockWidget(HWND hwnd, LPARAM lParam)
{
	bool result = true;
	wchar_t* className = new wchar_t[128];
	GetClassName(hwnd, className, 128);
	if (wcscmp(className, ORIGINAL_CLOCK_CLASS) == 0)
	{
		*((HWND*)lParam) = hwnd;
		result = false;
	}
	delete[] className;
	return result;
}

HWND ClockWindow::GetOriginalClock()
{
	if (!::IsWindow(GlobalClockWidget))
	{
		HWND taskbar = ::FindWindow(TASKBAR_MAIN_CLASS, nullptr);
		HWND taskbarNotify = ::FindWindowEx(taskbar, nullptr, TASKBAR_NOTIFICATION_AREA_CLASS, nullptr);
		HWND clockWidget;
		::EnumChildWindows(taskbarNotify, SearchClockWidget, (LPARAM)&clockWidget);
		GlobalClockWidget = clockWidget;
	}
	return GlobalClockWidget;
}