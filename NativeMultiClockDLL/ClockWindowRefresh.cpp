#include "ClockWindow.h"

using namespace Gdiplus;


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

SizeF ClockWindow::MeasureText(const Graphics* graphics, const std::wstring& text, const Gdiplus::Font& font, const Gdiplus::StringFormat& format) const
{
	PointF origin(0.0f, 0.0f);
	RectF boundingRect;
	Gdiplus::Status status = graphics->MeasureString(text.c_str(), -1, &font, origin, &format, &boundingRect);

	SizeF size;
	size.Width = boundingRect.Width;
	size.Height = boundingRect.Height;

	return size;
}

std::wstring ClockWindow::GetFittingText(const Gdiplus::Graphics* graphics, int maxWidth, int maxHeight, const Gdiplus::Font& font, const Gdiplus::StringFormat& format) const
{
	SYSTEMTIME time;
	GetLocalTime(&time);

	std::wstring timeText;
	std::wstring dateText;
	std::wstring dayText;
	
	timeText = TextTime(time, TIME_NOSECONDS);
	if (maxHeight >= 36)
	{
		if (maxHeight >= 53)
		{
			dayText = TextDate(time, 0, L"dddd");
			if (MeasureText(graphics, dayText, font, format).Width <= maxWidth)
			{
				timeText += L"\n";
				timeText += dayText;
			}
		}
		dateText = TextDate(time, DATE_SHORTDATE);
		if (MeasureText(graphics, dateText, font, format).Width <= maxWidth)
		{
			timeText += L"\n";
			timeText += dateText;
		}
	}

	return timeText;
}

void ClockWindow::RenderTime(Graphics* graphics, int width, int height) const
{
	// Get the current system font...
	NONCLIENTMETRICS metrics;
	metrics.cbSize = sizeof(NONCLIENTMETRICS);
	::SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICS), &metrics, 0);
	HDC context = graphics->GetHDC();
	Font font(context, &metrics.lfStatusFont);
	graphics->ReleaseHDC(context);

	// ... define some font smoothing...
	//graphics->SetTextRenderingHint(TextRenderingHint::TextRenderingHintSingleBitPerPixel);
	StringFormat format;
	format.SetAlignment(StringAlignment::StringAlignmentCenter);
	format.SetLineAlignment(StringAlignment::StringAlignmentCenter);

	std::wstring text = GetFittingText(graphics, width, height, font, format);

	// ... use all available space...
	RectF layoutRect(0.0, 0.0, (REAL)width, (REAL)height);

	// ... select default font color...
	Gdiplus::Color textColor;
	textColor.SetFromCOLORREF(GetSysColor(COLOR_3DFACE));
	SolidBrush brush(textColor);

	// ... and draw!
	graphics->DrawString(text.c_str(), -1, &font, layoutRect, &format, &brush);
}

void ClockWindow::DrawClockControl(Graphics* graphics, int width, int height) const
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
	RenderTime(graphics, width, height);
}

void ClockWindow::Refresh() const
{
	RECT clientRect;
	GetClientRect(&clientRect);
	int width = clientRect.right - clientRect.left;
	int height = clientRect.bottom - clientRect.top;

	// Draw the control...
	Bitmap* bitmap = new Bitmap(width, height, PixelFormat32bppPARGB);
	Status status = bitmap->GetLastStatus();
	if (status != Status::Ok)
	{
		delete bitmap;
		return;
	}
	DWORD error = GetLastError();
	Graphics* graphics = Graphics::FromImage(bitmap);
	DrawClockControl(graphics, width, height);
	delete graphics;

	// ... extract legacy bitmap...
	HBITMAP hbitmap;
	bitmap->GetHBITMAP(NULL, &hbitmap);
	delete bitmap;

	// ...and pass it to UpdateLayeredWindow
	HDC hdcScreen = ::GetDC(nullptr);
	HDC hDC = ::CreateCompatibleDC(hdcScreen);
	HBITMAP hbitmapOld = (HBITMAP) ::SelectObject(hDC, hbitmap);
	
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
}