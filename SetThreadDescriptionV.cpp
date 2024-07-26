#include <Windows.h>
#include <winternl.h>
#include <string_view>

#pragma warning(disable: 6262) // warning about 64kB on stack

HRESULT SetThreadDescriptionV (_In_ HANDLE hThread, _In_ std::wstring_view wsvThreadDescription) {
    
    bool trunc = false;
    auto size = wsvThreadDescription.size () * sizeof (WCHAR);
    if (size > 0xFFFE) {
        size = 0xFFFE;
        trunc = true;
    }

    UNICODE_STRING string = {
        (USHORT) size,
        (USHORT) size,
        (PWSTR) wsvThreadDescription.data ()
    };

    HRESULT status = NtSetInformationThread (hThread, (THREADINFOCLASS) 0x26, &string, sizeof string);

    if (SUCCEEDED (status)) {
        if (trunc) {
            status = HRESULT_FROM_WIN32 (RPC_S_STRING_TOO_LONG) & ~0x80000000; // report as info only
        }
    }
    return HRESULT_FROM_NT (status);
}

HRESULT GetThreadDescriptionV (_In_ HANDLE hThread, wchar_t * buffer, std::size_t & length) {
    ULONG retlen;
    HRESULT status = NtQueryInformationThread (hThread, (THREADINFOCLASS) 0x26, buffer, (ULONG) (length * sizeof (WCHAR)), &retlen);

    if (SUCCEEDED (status)) {
        auto header = reinterpret_cast <UNICODE_STRING *> (buffer);
        length = header->Length / sizeof (WCHAR);
        
        std::memmove (buffer, header + 1,
                      std::min ((std::size_t) header->Length,
                                (std::size_t) retlen - sizeof (UNICODE_STRING)));
    }
    return status;
}

HRESULT GetThreadDescriptionV (_In_ HANDLE hThread, _In_ std::wstring & wsThreadDescription) {
    struct {
        UNICODE_STRING header;
        wchar_t text [0x8000];
    } buffer;

    ULONG retlen;
    HRESULT status = NtQueryInformationThread (hThread, (THREADINFOCLASS) 0x26, &buffer, sizeof buffer, &retlen);

    if (SUCCEEDED (status)) {
        try {
            wsThreadDescription.assign (buffer.text, buffer.header.Length / sizeof (WCHAR));
        } catch (std::bad_alloc) {
            status = STATUS_NO_MEMORY;
        } catch (...) {
            status = STATUS_IN_PAGE_ERROR;
        }
    }
    return status;
}
