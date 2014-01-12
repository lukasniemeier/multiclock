#include "ClockWindow.h"
#include <vector>

using namespace Gdiplus;

#ifdef _DEBUG
static void SaveBitmap(Bitmap* bitmap)
{
	CLSID guid;
	::CLSIDFromString(L"{557CF406-1A04-11D3-9A73-0000F81EF32E}", &guid);
	Status saveStatus = bitmap->Save(L"C:\\Users\\Hugo\\bitmap.png", &guid);
	if (saveStatus != Status::Ok)
	{
		Beep(200, 100);
		Beep(200, 100);
	}
}
#endif

static std::wstring TextTime(SYSTEMTIME& time, DWORD flag, LPCWSTR format = nullptr)
{
	int textLength = ::GetTimeFormatEx(LOCALE_NAME_USER_DEFAULT, flag, &time, nullptr, nullptr, 0);
	wchar_t* textBuffer = new wchar_t[textLength];
	::GetTimeFormatEx(LOCALE_NAME_USER_DEFAULT, flag, &time, nullptr, textBuffer, textLength);
	std::wstring text(textBuffer);
	delete textBuffer;
	return text;
}

static std::wstring TextDate(SYSTEMTIME& time, DWORD flag, LPCWSTR format = nullptr)
{
	int textLength = ::GetDateFormatEx(LOCALE_NAME_USER_DEFAULT, flag | DATE_AUTOLAYOUT, &time, format, nullptr, 0, nullptr);
	wchar_t* textBuffer = new wchar_t[textLength];
	::GetDateFormatEx(LOCALE_NAME_USER_DEFAULT, flag | DATE_AUTOLAYOUT, &time, format, textBuffer, textLength, nullptr);
	std::wstring text(textBuffer);
	delete textBuffer;
	return text;
}

void ClockWindow::RenderClickedState(Gdiplus::Graphics* graphics, int width, int height) const
{
	Rect stateRect(1, 0, width - 1, height);
	SolidBrush brush(Color(5, 255, 255, 255));
	graphics->FillRectangle(&brush, stateRect);
}

void ClockWindow::RenderHighlight(Gdiplus::Graphics* graphics, int width, int height) const
{
	int otherColorsCount = 1;

	Color centerBarColor = Color(192, 200, 200, 200);
	Color otherBarColors[] = { Color(0, 200, 200, 200) };
	// Draw left highlight bar...
	const Rect leftHightlightRect(1, 0, 2, height);
	GraphicsPath leftHighlightPath;
	leftHighlightPath.AddRectangle(leftHightlightRect);

	PathGradientBrush leftHighlightBrush(&leftHighlightPath);
	leftHighlightBrush.SetCenterColor(centerBarColor);
	leftHighlightBrush.SetSurroundColors(otherBarColors, &otherColorsCount);

	graphics->FillPath(&leftHighlightBrush, &leftHighlightPath);

	// ... and draw the right one...
	const Rect rightHightlightRect(width - 3, 0, 2, height);
	GraphicsPath rightHighlightPath;
	rightHighlightPath.AddRectangle(rightHightlightRect);

	PathGradientBrush rightHighlightBrush(&rightHighlightPath);
	rightHighlightBrush.SetCenterColor(centerBarColor);
	rightHighlightBrush.SetSurroundColors(otherBarColors, &otherColorsCount);

	graphics->FillPath(&rightHighlightBrush, &rightHighlightPath);

	// ... and the blue highlight dot
	Color centerDotColor = Color(200, 177, 211, 255);
	Color otherDotColors[] = { Color(0, 255, 255, 255) };
	GraphicsPath dotPath;
	dotPath.AddEllipse(width / 3.f, height * 0.66f, width / 3.f, (REAL)height);
	PathGradientBrush dotBursh(&dotPath);
	dotBursh.SetCenterColor(centerDotColor);
	dotBursh.SetSurroundColors(otherDotColors, &otherColorsCount);
	graphics->FillPath(&dotBursh, &dotPath);

	const Rect dotLineRect(0, height - 1, width, 1);
	Color colors[] =
	{
		otherDotColors[0],
		centerDotColor,
		otherDotColors[0]
	};
	REAL positions[] = {
		0.0f,
		0.5f,
		1.0f };
	LinearGradientBrush dotLineBrush(
		Point(dotLineRect.X, dotLineRect.Y),
		Point(dotLineRect.GetRight(), dotLineRect.GetBottom()),
		Color(255, 0, 0, 0),
		Color(255, 255, 255, 255));
	dotLineBrush.SetInterpolationColors(colors, positions, 3);
	graphics->FillRectangle(&dotLineBrush, dotLineRect);
}

void ClockWindow::RenderTime(HDC context, int maxWidth, int maxHeight) const
{
	// Get the current system font...
	NONCLIENTMETRICS metrics;
	metrics.cbSize = sizeof(NONCLIENTMETRICS);
	::SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICS), &metrics, 0);

	HFONT font = ::CreateFontIndirect(&metrics.lfStatusFont);
	HFONT oldFont = (HFONT)::SelectObject(context, font);
	int oldBkMode = ::SetBkMode(context, TRANSPARENT);
	int oldTextColor = ::SetTextColor(context, GetSysColor(COLOR_3DFACE));

	std::wstring timeText;
	{
		RECT textRect;
		int textRectWidth;

		SYSTEMTIME time;
		GetLocalTime(&time);

		std::wstring dateText;
		std::wstring dayText;

		timeText = TextTime(time, TIME_NOSECONDS);
		if (maxHeight >= 36)
		{
			if (maxHeight >= 53)
			{
				dayText = TextDate(time, 0, L"dddd");
				textRect = { 0, 0, maxWidth, maxHeight };

				std::vector<wchar_t> textBuffer(dayText.begin(), dayText.end());
				textBuffer.push_back('\0');
				::DrawTextEx(context, textBuffer.data(), -1, &textRect, DT_CALCRECT | DT_CENTER | DT_NOCLIP, nullptr);
				textRectWidth = textRect.right - textRect.left;
				if (textRectWidth <= maxWidth)
				{
					timeText += L"\n";
					timeText += dayText;
				}
			}
			dateText = TextDate(time, DATE_SHORTDATE);
			textRect = { 0, 0, maxWidth, maxHeight };

			std::vector<wchar_t> textBuffer(dayText.begin(), dayText.end());
			textBuffer.push_back('\0');
			::DrawTextEx(context, textBuffer.data(), -1, &textRect, DT_CALCRECT | DT_CENTER | DT_NOCLIP, nullptr);
			textRectWidth = textRect.right - textRect.left;
			if (textRectWidth <= maxWidth)
			{
				timeText += L"\n";
				timeText += dateText;
			}
		}
	}

	RECT textRect = { 0, 0, maxWidth, maxHeight };

	std::vector<wchar_t> textBuffer(timeText.begin(), timeText.end());
	textBuffer.push_back('\0');
	::DrawTextEx(context, textBuffer.data(), -1, &textRect, DT_CALCRECT | DT_CENTER | DT_NOCLIP, nullptr);

	// move text rect to (0,0)
	::OffsetRect(&textRect, -textRect.left, -textRect.top);
	// center text rect
	::OffsetRect(&textRect, (int)((maxWidth - textRect.right) / 2.0), (int)((maxHeight - textRect.bottom) / 2.0));
	// draw
	::DrawTextEx(context, textBuffer.data(), -1, &textRect, DT_CENTER | DT_NOCLIP, nullptr);

	::SetTextColor(context, oldTextColor);
	::SetBkMode(context, oldBkMode);
	::SelectObject(context, oldFont);
	::DeleteObject(font);
}

void ClockWindow::RenderHighlighting(Graphics* graphics, int width, int height) const
{
	SolidBrush burshClear(Color::Transparent);
	graphics->FillRectangle(&burshClear, 0, 0, width, height);
	if (isClicked)
	{
		RenderClickedState(graphics, width, height);
	}
	if (isHighlighted)
	{
		RenderHighlight(graphics, width, height);
	}
}

bool ClockWindow::IsVisible(const RECT& clientRect)
{
	HDC parent = ::GetDC(this->GetParent());
	RECT clip;
	int result = ::GetClipBox(parent, &clip);
	::ReleaseDC(this->GetParent(), parent);

	bool isVisible;
	switch (result)
	{
	case SIMPLEREGION:
	case COMPLEXREGION:
		{
			OUTPUT_DEBUG_STRING(L"Clock is partially or fully visible");
			RECT inParentRect(clientRect);
			MapWindowPoints(this->GetParent(), &inParentRect);

			RECT intersectRect;
			::IntersectRect(&intersectRect, &clip, &inParentRect);

			isVisible = !::IsRectEmpty(&intersectRect);
		}
		break;
	case NULLREGION:
	default:
		OUTPUT_DEBUG_STRING(L"Clock is completly covered");
		isVisible = false;
		break;
	}

	if (!isVisible)
	{
		OUTPUT_DEBUG_STRING(L"Clock not visible, ignoring refresh request.");
	}
	return isVisible;
}

void ClockWindow::Refresh(bool force)
{
	OUTPUT_DEBUG_STRING(L"Clock refresh requested. Force =", force);

	RECT clientRect;
	GetClientRect(&clientRect);

	if (!force && !IsVisible(clientRect))
	{
		return;
	}

	int width = clientRect.right - clientRect.left;
	int height = clientRect.bottom - clientRect.top;

	Bitmap* bitmap = new Bitmap(width, height, PixelFormat32bppPARGB);
	Status status = bitmap->GetLastStatus();
	if (status != Status::Ok)
	{
		delete bitmap;
		return;
	}

	Graphics* g = Graphics::FromImage(bitmap);
	RenderHighlighting(g, width, height);
	delete g;

	HBITMAP hbitmap;
	bitmap->GetHBITMAP(NULL, &hbitmap);

	HDC hdcScreen = ::GetDC(nullptr);
	HDC hDC = ::CreateCompatibleDC(hdcScreen);
	HBITMAP hbitmapOld = (HBITMAP) ::SelectObject(hDC, hbitmap);

	RenderTime(hDC, width, height);

	SIZE sizeWnd = { width, height };
	POINT ptSrc = { 0, 0 };
	BLENDFUNCTION blend = { 0 };
	blend.BlendOp = AC_SRC_OVER;
	blend.SourceConstantAlpha = 255;
	blend.AlphaFormat = AC_SRC_ALPHA;
	::UpdateLayeredWindow(this->m_hWnd, hdcScreen, nullptr, &sizeWnd, hDC, &ptSrc, 0, &blend, ULW_ALPHA);

	::SelectObject(hDC, hbitmapOld);
	::DeleteObject(hbitmap);
	::DeleteDC(hDC);
	::ReleaseDC(NULL, hdcScreen);

	delete bitmap;

	OUTPUT_DEBUG_STRING(L"Clock has been refreshed.");
}
