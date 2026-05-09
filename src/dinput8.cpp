#include <cstdint>
#include <cstring>

#include <windows.h>

#include "utils.h"

namespace
{

auto patch_jmp() -> void
{
    static constexpr auto patch_addr = std::uintptr_t{0x004717de};
    static constexpr auto patch_byte = std::uint8_t{0xeb};

    const auto auto_prot = wardite::AutoProtect{reinterpret_cast<void *>(patch_addr), 1, PAGE_EXECUTE_READWRITE};

    std::memcpy(reinterpret_cast<void *>(patch_addr), &patch_byte, 1);

    wardite::log("patched byte at {:X} to {:X}", patch_addr, patch_byte);
}

}

extern "C"
{

__declspec(dllexport) ::HRESULT WINAPI
DirectInput8Create(HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID *ppvOut, LPUNKNOWN punkOuter);

__declspec(dllexport) ::HRESULT WINAPI
DirectInput8Create(HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID *ppvOut, LPUNKNOWN punkOuter)
{
    wardite::log("DirectInput8Create called for dinput8.dll");

    const auto dinput8_lib = ::LoadLibraryA("C:\\Windows\\System32\\dinput8.dll");
    wardite::ensure(dinput8_lib, "failed to load original dinput8.dll: {}", ::GetLastError());

    const auto original_func =
        reinterpret_cast<decltype(&DirectInput8Create)>(::GetProcAddress(dinput8_lib, "DirectInput8Create"));
    wardite::ensure(original_func, "failed to get address of DirectInput8Create: {}", ::GetLastError());

    auto result = original_func(hinst, dwVersion, riidltf, ppvOut, punkOuter);
    wardite::log("original DirectInput8Create returned: {:X}", result);

    return result;
}

::DWORD WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved);
::DWORD WINAPI DllMain(HINSTANCE, DWORD fdwReason, LPVOID)
{
    switch (fdwReason)
    {
        case DLL_PROCESS_ATTACH:
        {
            wardite::log("DLLMain called for dinput8.dll");
            patch_jmp();
        }
        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
        case DLL_PROCESS_DETACH: break;
    }

    return 1;
}
}
