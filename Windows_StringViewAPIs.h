#ifndef WINDOWS_STRINGVIEWAPIS_H
#define WINDOWS_STRINGVIEWAPIS_H

#include <Windows.h>
#include <string_view>

HRESULT SetThreadDescriptionV (_In_ HANDLE hThread, _In_ std::wstring_view wsvThreadDescription);
HRESULT GetThreadDescriptionV (_In_ HANDLE hThread, _In_ std::wstring & wsThreadDescription);
HRESULT GetThreadDescriptionV (_In_ HANDLE hThread, wchar_t * buffer, std::size_t & length);

HANDLE CreateFileV (std::wstring_view lpFileName,
                    DWORD dwDesiredAccess, DWORD dwShareMode,
                    LPSECURITY_ATTRIBUTES lpSecurityAttributes,
                    DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes);

#endif
