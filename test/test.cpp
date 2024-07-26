#include <Windows.h>
#include "..\Windows_StringViewAPIs.h"

// extern "C" WINBASEAPI NTSTATUS WINAPI RtlInitUnicodeStringEx (UNICODE_STRING * DestinationString, PCWSTR SourceString);
HRESULT (WINAPI * ptrGetThreadDescription) (_In_ HANDLE, _Outptr_result_z_ PWSTR *) = NULL;

void init ();
void test_SetThreadDescriptionV ();

int main () {
    init ();
    test_SetThreadDescriptionV ();


    return 0;
}


template <typename P>
bool LoadSymbol (HMODULE h, P & pointer, const char * name) {
    if (P p = reinterpret_cast <P> (GetProcAddress (h, name))) {
        pointer = p;
        return true;
    } else
        return false;
}

void init () {
    if (HMODULE h = GetModuleHandleW (L"KERNELBASE")) {
        LoadSymbol (h, ptrGetThreadDescription, "GetThreadDescription");
    }
}

void test_SetThreadDescriptionV () {
    using namespace std::literals;

    auto string = L"a\0b\0c\0d\0e"sv;

    SetThreadDescriptionV (GetCurrentThread (), string);

    // GetThreadDescriptionV -> std::wstring

    std::wstring retrieved;
    HRESULT hr = GetThreadDescriptionV (GetCurrentThread (), retrieved);

    if (SUCCEEDED (hr)) {
        if (string == retrieved) {
            std::wprintf (L"SUCCESS: GetThreadDescriptionV to std::wstring\n");
        } else {
            std::wprintf (L"FAILED: GetThreadDescriptionV returned wrong string: \"%s\" retrieved\n", retrieved.c_str ());
        }
    } else {
        std::wprintf (L"FAILED: GetThreadDescriptionV return value: %08X\n", hr);
    }

    // GetThreadDescriptionV -> direct buffer

    wchar_t retrieved_direct [32];
    std::size_t retrieved_direct_length = 32;

    hr = GetThreadDescriptionV (GetCurrentThread (), retrieved_direct, retrieved_direct_length);
    if (SUCCEEDED (hr)) {
        if (string == std::wstring_view (retrieved_direct, retrieved_direct_length)) {
            std::wprintf (L"SUCCESS: GetThreadDescriptionV to buffer\n");
        } else {
            std::wprintf (L"FAILED: GetThreadDescriptionV returned wrong string: \"%s\" (%u) retrieved\n",
                          std::wstring (retrieved_direct, retrieved_direct_length).c_str (), (unsigned) retrieved_direct_length);
        }
    } else {
        std::wprintf (L"FAILED: GetThreadDescriptionV return value: %08X\n", hr);
    }
}
