# Win32 API support for std::wstring_view
*Experimental reimplementations of some Windows API functions taking std::wstring_view as argument*

## The problem

Modern C++ and other languages today provide extensive facilities to efficiently work with strings, especially with substrings, that **aren't** NUL-terminated.
But a vast majority of Windows API function calls, e.g. [CreateFile](https://learn.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-createfilew),
consume their string argument(s) via NUL-terminated strings.
This forces the application to allocate an unnecessary NUL-terminated copy of the string.

Those Win32 API functions then convert these parameters into
[UNICODE_STRING](https://learn.microsoft.com/en-us/windows/win32/api/subauth/ns-subauth-unicode_string)s
before passing it to appropiate NT API(s). UNICODE_STRING is basically a potentially-owning limited-size wstring_view.

This effectively means Windows is vasting millions of cycles a second by doing allocations and copying that's unnecessary.

## This repository

If Windows were to implement parallel set of Win32 APIs to support `std::wstring_view`, what could they look like?

**Note:** Microsoft would probably use two parameters, `LPWCSTR` and `SIZE_T`, but we can afford actual `std::wstring_view` for our experiments.

## Better solutions

* [RtlInitUnicodeString](https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/wdm/nf-wdm-rtlinitunicodestring) level:
  https://github.com/tringi/papers/blob/main/win32-wstring_view_api.md
   * alternatively the application could [detour](https://github.com/microsoft/Detours) the `RtlInitUnicodeString` for itself

## Implemented functions

* [(S/G)etThreadDescriptionV](SetThreadDescriptionV.cpp) alternative to
  [(S/G)etThreadDescription](https://learn.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-getthreaddescription)

