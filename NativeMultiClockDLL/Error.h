#pragma once
#include <Windows.h>


void MessageBoxError(DWORD error)
{
	LPTSTR errorText = nullptr;
	DWORD result = ::FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, error, 0, (LPTSTR)&errorText, 0, NULL);
	if (error != 0)
	{
		::MessageBox(nullptr, errorText, L"Error", MB_OK | MB_ICONERROR);

		if (errorText != nullptr)
		{
			::LocalFree(errorText);
			errorText = nullptr;
		}
	}
}
