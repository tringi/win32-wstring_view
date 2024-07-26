#include <Windows.h>
#include <winternl.h>
#include <string_view>

extern "C" WINBASEAPI NTSTATUS WINAPI NtSetInformationFile (HANDLE, PIO_STATUS_BLOCK, PVOID, ULONG, FILE_INFORMATION_CLASS);

namespace {
    static const UCHAR mapWin32toNT [] = {
        // CREATE_NEW,  CREATE_ALWAYS,  OPEN_EXISTING, OPEN_ALWAYS, TRUNCATE_EXISTING
        FILE_CREATE, FILE_OVERWRITE_IF, FILE_OPEN, FILE_OPEN_IF, FILE_OPEN
    };
}

HANDLE CreateFileV (std::wstring_view wsvFileName, DWORD dwDesiredAccess, DWORD dwShareMode,
                    LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes) {

    ULONG disposition = 0;
    if (dwCreationDisposition >= 1 && dwCreationDisposition <= 5) {
        disposition = mapWin32toNT [dwCreationDisposition - 1];
    } else {
        SetLastError (ERROR_INVALID_PARAMETER);
        return INVALID_HANDLE_VALUE;
    }

    ULONG flags = FILE_NON_DIRECTORY_FILE;

    if (dwFlagsAndAttributes & FILE_FLAG_OVERLAPPED) flags |= FILE_SYNCHRONOUS_IO_NONALERT;
    if (dwFlagsAndAttributes & FILE_FLAG_NO_BUFFERING) flags |= FILE_NO_INTERMEDIATE_BUFFERING;
    if (dwFlagsAndAttributes & FILE_FLAG_WRITE_THROUGH) flags |= FILE_WRITE_THROUGH;
    if (dwFlagsAndAttributes & FILE_FLAG_RANDOM_ACCESS) flags |= FILE_RANDOM_ACCESS;
    if (dwFlagsAndAttributes & FILE_FLAG_DELETE_ON_CLOSE) flags |= FILE_DELETE_ON_CLOSE;
    if (dwFlagsAndAttributes & FILE_FLAG_SEQUENTIAL_SCAN) flags |= FILE_SEQUENTIAL_ONLY;
    if (dwFlagsAndAttributes & FILE_FLAG_BACKUP_SEMANTICS) flags |= FILE_OPEN_FOR_BACKUP_INTENT;
    if (dwFlagsAndAttributes & FILE_FLAG_OPEN_NO_RECALL) flags |= FILE_OPEN_NO_RECALL;
    if (dwFlagsAndAttributes & FILE_FLAG_OPEN_REPARSE_POINT) flags |= FILE_OPEN_REPARSE_POINT;

    if (dwFlagsAndAttributes & FILE_FLAG_DELETE_ON_CLOSE) dwDesiredAccess |= DELETE;

    UNICODE_STRING filename = {
        (USHORT) wsvFileName.size () * sizeof (WCHAR),
        (USHORT) wsvFileName.size () * sizeof (WCHAR),
        (PWSTR) wsvFileName.data ()
    };

    HANDLE directory = NULL;

    // TODO: open directory and construct NT path

    OBJECT_ATTRIBUTES attributes;
    InitializeObjectAttributes (&attributes, &filename, OBJ_CASE_INSENSITIVE, directory, NULL);

    if (lpSecurityAttributes) {
        if (lpSecurityAttributes->lpSecurityDescriptor) {
            attributes.SecurityDescriptor = lpSecurityAttributes->lpSecurityDescriptor;
        }
        if (lpSecurityAttributes->bInheritHandle) {
            attributes.Attributes |= OBJ_INHERIT;
        }
    }

    HANDLE h;
    IO_STATUS_BLOCK io {};

    auto hr = NtCreateFile (&h, dwDesiredAccess | SYNCHRONIZE | FILE_READ_ATTRIBUTES,
                            &attributes, &io, NULL, dwFlagsAndAttributes, dwShareMode,
                            disposition, flags, NULL, 0);
    if (SUCCEEDED (hr)) {

        if ((dwCreationDisposition == CREATE_ALWAYS && io.Information == FILE_OVERWRITTEN) ||
            (dwCreationDisposition == OPEN_ALWAYS && io.Information == FILE_OPENED)) {

            SetLastError (ERROR_ALREADY_EXISTS);
        } else {
            SetLastError (ERROR_SUCCESS);
        }

        if (dwCreationDisposition == TRUNCATE_EXISTING) {
            SetFilePointer (h, 0, NULL, FILE_BEGIN);
            if (!SetEndOfFile (h)) {
                CloseHandle (h);
                h = INVALID_HANDLE_VALUE;
            }
        }

        return h;
    } else {
        switch (hr) {
            //case STATUS_OBJECT_NAME_COLLISION:  SetLastError (ERROR_FILE_EXISTS); break;
            // case STATUS_FILE_IS_A_DIRECTORY:  SetLastError (ERROR_ACCESS_DENIED); break;
            // default: 
        }
        return INVALID_HANDLE_VALUE;
    }
}
