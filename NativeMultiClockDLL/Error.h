#pragma once
#include <Windows.h>


void MessageBoxError(DWORD error)
{
	LPVOID lpMsgBuf;
	

	DWORD result = ::FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpMsgBuf, 0, NULL);
	if (result != 0)
	{
		LPTSTR lpszFunction = L"Operation";
		LPVOID lpDisplayBuf = (LPVOID)::LocalAlloc(
			LMEM_ZEROINIT, 
			(lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)lpszFunction) + 40) * sizeof(TCHAR));
		::StringCchPrintf((LPTSTR)lpDisplayBuf,
			LocalSize(lpDisplayBuf) / sizeof(TCHAR),
			TEXT("%s failed with error %d: %s"),
			lpszFunction, 
			error, 
			lpMsgBuf);
		::MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK | MB_ICONERROR);

		::LocalFree(lpMsgBuf);
		::LocalFree(lpDisplayBuf);
	}
	else
	{
		LPVOID lpDisplayBuf = (LPVOID)::LocalAlloc(LMEM_ZEROINIT, 40 * sizeof(TCHAR));
		::StringCchPrintf((LPTSTR)lpDisplayBuf, LocalSize(lpDisplayBuf) / sizeof(TCHAR), L"%0xd", error);
		::MessageBox(nullptr, (LPCTSTR)lpDisplayBuf, L"Error", MB_OK | MB_ICONERROR);
		::LocalFree(lpDisplayBuf);
	}
}
