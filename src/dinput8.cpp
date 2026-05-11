#include <cstdint>
#include <cstring>

#include <windows.h>

#include "utils.h"

namespace
{

[[maybe_unused]] auto patch_1() -> void
{
    static constexpr auto patch_addr = std::uintptr_t{0x004717de};
    static constexpr auto patch_byte = std::uint8_t{0xeb};

    const auto auto_prot = wardite::AutoProtect{reinterpret_cast<void *>(patch_addr), 1, PAGE_EXECUTE_READWRITE};

    std::memcpy(reinterpret_cast<void *>(patch_addr), &patch_byte, 1);

    wardite::log("patched byte at {:X} to {:X}", patch_addr, patch_byte);
}

[[maybe_unused]] auto patch_2() -> void
{
    static constexpr auto patch_addr = std::uintptr_t{0x00470B85};
    static constexpr auto patch_byte = std::uint8_t{0x83};

    const auto auto_prot = wardite::AutoProtect{reinterpret_cast<void *>(patch_addr), 1, PAGE_EXECUTE_READWRITE};

    std::memcpy(reinterpret_cast<void *>(patch_addr), &patch_byte, 1);

    wardite::log("patched byte at {:X} to {:X}", patch_addr, patch_byte);
}

auto patch_3() -> void
{
    auto payload = std::array<std::uint8_t, 19>{
        0x8B,
        0x57,
        0x04,
        0x83,
        0x3F,
        0x00,
        0x75,
        0x02,
        0x33,
        0xD2,
        0x3B,
        0xC2,
        0x68,
        0x73,
        0x0B,
        0x47,
        0x00,
        0xC3,
    };

    auto *payload_addr = ::VirtualAlloc(nullptr, payload.size(), MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    wardite::ensure(payload_addr, "failed to allocate memory for payload: {}", ::GetLastError());
    std::memcpy(payload_addr, payload.data(), payload.size());

    static constexpr auto patch_addr = std::uintptr_t{0x00470B6E};

    const auto rel32 = static_cast<std::int32_t>(
        reinterpret_cast<std::intptr_t>(payload_addr) - static_cast<std::intptr_t>(patch_addr + 5));
    const auto rel32_bytes = std::as_bytes(std::span{&rel32, 1});

    const auto patch_bytes = std::array<std::uint8_t, 5>{
        0xE9,
        static_cast<std::uint8_t>(rel32_bytes[0]),
        static_cast<std::uint8_t>(rel32_bytes[1]),
        static_cast<std::uint8_t>(rel32_bytes[2]),
        static_cast<std::uint8_t>(rel32_bytes[3]),
    };

    const auto auto_prot = wardite::AutoProtect{reinterpret_cast<void *>(patch_addr), 5, PAGE_EXECUTE_READWRITE};
    std::memcpy(reinterpret_cast<void *>(patch_addr), patch_bytes.data(), patch_bytes.size());

    wardite::log("patched bytes at {:X}, cave at {:X}", patch_addr, reinterpret_cast<std::uintptr_t>(payload_addr));
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
            patch_1();
            patch_2();
            patch_3();
        }
        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
        case DLL_PROCESS_DETACH: break;
    }

    return 1;
}
}
