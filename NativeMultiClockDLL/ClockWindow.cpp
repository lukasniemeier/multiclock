#include "ClockWindow.h"
#include <WindowsX.h>

BOOL CALLBACK SearchClockWidget(HWND hwnd, LPARAM lParam)
{
	bool result = true;
	wchar_t* className = new wchar_t[128];
	GetClassName(hwnd, className, 128);
	if (wcscmp(className, L"TrayClockWClass") == 0)
	{
		*((HWND*)lParam) = hwnd;
		result = false;
	}
	delete className;
	return result;
}

ClockWindow::ClockWindow()
{
	toolTipWindow = nullptr;
	isHighlighted = false;
	isClicked = false;

	trackMouseEventInfo.cbSize = sizeof(TRACKMOUSEEVENT);
}

std::wstring ClockWindow::GetTimeString(int maxHeight)
{
	SYSTEMTIME time;
	GetLocalTime(&time);

	wchar_t* timeTextBuffer;
	int timeLength = ::GetTimeFormatEx(LOCALE_NAME_USER_DEFAULT, TIME_NOSECONDS, &time, nullptr, nullptr, 0);
	timeTextBuffer = new wchar_t[timeLength];
	::GetTimeFormatEx(LOCALE_NAME_USER_DEFAULT, TIME_NOSECONDS, &time, nullptr, timeTextBuffer, timeLength);
	std::wstring timeText(timeTextBuffer);
	delete timeTextBuffer;

	if (maxHeight >= 36)
	{
		int dateLength = ::GetDateFormatEx(LOCALE_NAME_USER_DEFAULT, DATE_SHORTDATE | DATE_AUTOLAYOUT, &time, nullptr, nullptr, 0, nullptr);
		wchar_t* dateTextBuffer = new wchar_t[dateLength];
		::GetDateFormatEx(LOCALE_NAME_USER_DEFAULT, DATE_SHORTDATE | DATE_AUTOLAYOUT, &time, nullptr, dateTextBuffer, dateLength, nullptr);
		std::wstring dateText(dateTextBuffer);
		delete dateTextBuffer;

		if (maxHeight >= 60)
		{
			int dayLength = ::GetDateFormatEx(LOCALE_NAME_USER_DEFAULT, DATE_AUTOLAYOUT, &time, L"dddd", nullptr, 0, nullptr);
			wchar_t* dayTextBuffer = new wchar_t[dayLength];
			::GetDateFormatEx(LOCALE_NAME_USER_DEFAULT, DATE_AUTOLAYOUT, &time, L"dddd", dayTextBuffer, dayLength, nullptr);
			std::wstring dayText(dayTextBuffer);
			delete dayTextBuffer;

			timeText += '\n';
			timeText += dayText;
		}

		timeText += '\n';
		timeText += dateText;
	}
	return timeText;
}

LRESULT ClockWindow::OnLeftButtonUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	isClicked = false;
	Refresh();

	bHandled = true;
	return 0;
}

LRESULT ClockWindow::OnLeftButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	HWND clockWidget = ClockWindow::GetOriginalClock();

	SendMessage(clockWidget, WM_USER + 104, 0x3, 0x0);
	SendMessage(clockWidget, WM_USER + 102, 0x1, 0x0);
	SendMessage(clockWidget, WM_USER + 104, 0x2, 0x0);

	//SendMessage(clockWidget, WM_USER + 104, 0x2, 0x0); // DISPLAYS HIGHLIGHT
	isClicked = true;
	Refresh();

	bHandled = true;
	return 0;
}

void ClockWindow::StartTrackingOn(HWND tray)
{
	if (!isHighlighted)
	{
		trackMouseEventInfo.dwFlags = TME_LEAVE | TME_HOVER | TME_NONCLIENT;
		trackMouseEventInfo.hwndTrack = tray;
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

/*#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")*/

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

	/*
	// TODO: fit in screen, position next to taskbar!
	RECT rc;
	GetWindowRect(&rc);
	rc.left = (LONG) -rect.GetRight();
	rc.top -= (LONG) rect.GetBottom();
	::SendMessage(toolTipWindow, TTM_ADJUSTRECT, TRUE, (LPARAM)&rc);
	rc.left -= rc.right;*/

	::SendMessage(toolTipWindow, TTM_UPDATETIPTEXT, 0, (LPARAM)&toolTipInfo);
	//::SendMessage(toolTipWindow, TTM_TRACKPOSITION, 0, (LPARAM)MAKELONG(rc.left, rc.top));
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

UINT GetTaskbarOrientation(HWND hwnd)
{
	RECT rc;
	GetWindowRect(hwnd, &rc);
	MONITORINFO info = { sizeof(info) };
	HMONITOR monitor = MonitorFromRect(&rc, MONITOR_DEFAULTTONEAREST);
	GetMonitorInfo(monitor, &info);
	int dx = rc.left + rc.right - info.rcWork.left - info.rcWork.right;
	int dy = rc.top + rc.bottom - info.rcWork.top - info.rcWork.bottom;
	if (dx<-abs(dy)) return ABE_LEFT;
	if (dx>abs(dy)) return ABE_RIGHT;
	if (dy < -abs(dx)) return ABE_TOP;
	return ABE_BOTTOM;
}

void ClockWindow::RepositionInTray(HWND tray)
{
	RECT targetRect;
	RECT trayRect;
	::GetClientRect(tray, &trayRect);

	UINT MARGIN = 2;
	UINT uEdge = GetTaskbarOrientation(tray);
	if (uEdge == ABE_LEFT || uEdge == ABE_RIGHT)
	{
		targetRect.left = trayRect.left + (uEdge == ABE_LEFT ? 0 : MARGIN);
		targetRect.right = trayRect.right - (uEdge == ABE_LEFT ? 2 : MARGIN);
		targetRect.top = trayRect.bottom - 36;
		targetRect.bottom = trayRect.bottom;
	}
	else
	{
		targetRect.left = trayRect.right - 75;
		targetRect.right = trayRect.right;
		targetRect.top = trayRect.top + MARGIN;
		targetRect.bottom = trayRect.bottom;
	}
	SetWindowPos(HWND_TOP, &targetRect, SWP_SHOWWINDOW | SWP_NOOWNERZORDER | SWP_NOACTIVATE);
}

LRESULT ClockWindow::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	HWND tray = this->GetParent();
	HWND workerW = ::FindWindowEx(tray, nullptr, L"WorkerW", nullptr);
	HWND clock = ClockWindow::GetOriginalClock();
	::SetWindowSubclass(tray, ClockWindow::TaskbarSubclassProc, 0, (DWORD_PTR) this);
	::SetWindowSubclass(workerW, ClockWindow::WorkerWSubclassProc, 2, (DWORD_PTR) this);
	::SetWindowSubclass(clock, ClockWindow::OriginalClockSubclassProc, 1, (DWORD_PTR) this);
	return 0;
}

LRESULT ClockWindow::OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	HWND tray = this->GetParent();
	HWND workerW = ::FindWindowEx(tray, nullptr, L"WorkerW", nullptr);
	HWND clock = ClockWindow::GetOriginalClock();
	::RemoveWindowSubclass(tray, ClockWindow::TaskbarSubclassProc, 0);
	::RemoveWindowSubclass(workerW, ClockWindow::WorkerWSubclassProc, 2);
	::RemoveWindowSubclass(clock, ClockWindow::OriginalClockSubclassProc, 1);
	DestroyWindow();
	return 0;
}

HWND GlobalClockWidget;

HWND ClockWindow::GetOriginalClock()
{
	if (!::IsWindow(GlobalClockWidget))
	{
		HWND tray = ::FindWindow(L"Shell_TrayWnd", L"");
		HWND trayNotify = ::FindWindowEx(tray, nullptr, L"TrayNotifyWnd", L"");
		HWND clockWidget;
		::EnumChildWindows(trayNotify, SearchClockWidget, (LPARAM)&clockWidget);
		GlobalClockWidget = clockWidget;
	}
	return GlobalClockWidget;
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
			clock->RepositionInTray(hWnd);
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
	HWND tray = clock->GetParent();

	switch (uMsg)
	{
		case WM_WINDOWPOSCHANGING:
		{
			WINDOWPOS* pos = (WINDOWPOS*)lParam;
			if (!(pos->flags&SWP_NOMOVE) || !(pos->flags&SWP_NOSIZE))
			{
				RECT trayRect;
				trayRect.left = pos->x;
				trayRect.right = pos->x + pos->cx;
				trayRect.top = pos->y;
				trayRect.bottom = pos->y + pos->cy;

				RECT clockRect;
				::GetClientRect(*clock, &clockRect);
				::MapWindowPoints(*clock, tray, (LPPOINT)&clockRect, 2);
			
				UINT uEdge = GetTaskbarOrientation(tray);
				if (uEdge == ABE_LEFT || uEdge == ABE_RIGHT)
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