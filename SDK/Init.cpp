#include <cstring>
#include <string>

#include <Windows.h>
#include <psapi.h>
#include <tlhelp32.h>

#include <MinHook.h>

#include "Headers.hpp"
#include "Init.hpp"


namespace UGHSDK {

    // ! Address implementation.
    // ========================================

    Address Address::FromAbsolute(void* const Addr, long long const Shift) {
        Address OutAddr{};
        OutAddr.m_address = Addr;
        OutAddr.m_valueMode = k_modeAbsolute;
        OutAddr.m_resolveShift = Shift;
        return OutAddr;
    }

    Address Address::FromOffset(size_t const Offset, long long const Shift) {
        Address OutAddr{};
        OutAddr.m_offset = Offset;
        OutAddr.m_valueMode = k_modeOffset;
        OutAddr.m_resolveShift = Shift;
        return OutAddr;
    }

    Address Address::FromPattern(char const* const Pattern, long long const Shift) {
        UGHSDK_CHECK(Pattern != nullptr, "");
        UGHSDK_CHECK(std::strlen(Pattern) < 300, "");

        Address OutAddr{};
        OutAddr.m_pattern = const_cast<char*>(Pattern);
        OutAddr.m_valueMode = k_modePattern;
        OutAddr.m_resolveShift = Shift;
        return OutAddr;
    }

    Address Address::FromInstrRelative(char const* const Pattern) {
        UGHSDK_CHECK(Pattern != nullptr, "");
        UGHSDK_CHECK(std::strlen(Pattern) < 300, "");

        Address OutAddr{};
        OutAddr.m_pattern = const_cast<char*>(Pattern);
        OutAddr.m_valueMode = k_modeInstrRelative;
        OutAddr.m_resolveShift = 0;
        return OutAddr;
    }

    Address Address::FromPostHook(char const* const Pattern) {
        return FromPattern(Pattern, -5);
    }


    // ! Initializer implementation.
    // ========================================

    Initializer::Initializer() {
        ModuleBase = GetMainModuleBase();
        MH_STATUS const MinHookStatus = MH_Initialize();
        UGHSDK_CHECK(MinHookStatus == MH_OK, "");
    }

    Initializer::~Initializer() noexcept
    {
        MH_STATUS const MinHookStatus = MH_Uninitialize();
        UGHSDK_CHECK(MinHookStatus == MH_OK, "");
    }

    void* Initializer::Resolve(Address const InAddr) const {
        void* OutAddr = nullptr;

        switch (InAddr.m_valueMode) {
        case Address::k_modeAbsolute:
            OutAddr = reinterpret_cast<BYTE*>(InAddr.m_address);
            break;
        case Address::k_modeOffset:
            UGHSDK_CHECK(ModuleBase != nullptr, "failed to resolve rva offset");
            OutAddr = reinterpret_cast<BYTE*>(ModuleBase) + InAddr.m_offset;
            break;
        case Address::k_modePattern:
            OutAddr = FindPattern(InAddr.m_pattern);
            break;
        case Address::k_modeInstrRelative:
            OutAddr = FindPattern(InAddr.m_pattern);
            OutAddr = (reinterpret_cast<BYTE*>(OutAddr) + 7) +
                *reinterpret_cast<INT*>(reinterpret_cast<BYTE*>(OutAddr) + 3);
            break;
        }

        UGHSDK_CHECK(OutAddr != nullptr, "failed to find pattern or it's invalid");
        if (OutAddr != nullptr) OutAddr = reinterpret_cast<BYTE*>(OutAddr) + InAddr.m_resolveShift;
        return OutAddr;
    }

    void* Initializer::InstallHook(void* const Target, void* const Detour) {
        UGHSDK_CHECK(Target != nullptr, "");
        UGHSDK_CHECK(Detour != nullptr, "");

        void* pOutOriginal = nullptr;

        if (MH_CreateHook(Target, Detour, &pOutOriginal) != MH_OK)
            return nullptr;

        HookTargets.push_back(Target);

        if (MH_EnableHook(Target) != MH_OK)
            return nullptr;

        return pOutOriginal;
    }

    bool Initializer::UninstallAllHooks() {
        if (HookTargets.empty())
            return true;

        if (MH_DisableHook(MH_ALL_HOOKS) != MH_OK)
            return false;

        bool bUninstalledAllHooks = true;

        for (void* Target : HookTargets) {
            if (MH_RemoveHook(Target) != MH_OK)
                bUninstalledAllHooks = false;
        }

        return bUninstalledAllHooks;
    }

    bool Initializer::ParseCombinedPattern(char* const InPattern,
        BYTE* const OutPatternBuffer, BYTE* const OutMaskBuffer, size_t* const OutLength)
    {
        auto len = InPattern ? std::strlen(InPattern) : 0;
        if (len < 6) return false;

        char* token = nullptr;
        char* endPtr = nullptr;
        long byteValue = 0;
        size_t parsedLength = 0;
        size_t patternLength = 0;

        do
        {
            token = std::strtok(parsedLength == 0 ? InPattern : nullptr, " ");
            if (token)
            {
                parsedLength = token + 2 - InPattern;

                if (strlen(token) != 2)
                {
                    // GLogger.writeln(L"ParseCombinedPattern ERROR: parsed token was not 2 chars long: %S", token);
                    return false;
                }

                if (0 == strcmp(token, "??"))
                {
                    OutPatternBuffer[patternLength] = 0u;
                    OutMaskBuffer[patternLength] = '?';
                    ++patternLength;
                }
                else
                {
                    errno = 0;
                    if (0 == (byteValue = std::strtol(token, &endPtr, 0x10)) && errno != 0)
                    {
                        // GLogger.writeln(L"ParseCombinedPattern ERROR: strtol returned an error (errno = %d)", errno);
                        return false;
                    }
                    else
                    {
                        OutPatternBuffer[patternLength] = static_cast<BYTE>(byteValue);
                        OutMaskBuffer[patternLength] = 'x';
                        ++patternLength;
                    }
                }
            }
            else
            {
                if (parsedLength != len)
                {
                    // GLogger.writeln(L"ParseCombinedPattern ERROR: token was null but inPattern end wasn't reached (%d != %d)", parsedLength, len);
                    return false;
                }
            }
        } while (token);

        *OutLength = patternLength;
        return true;
    }

    void* Initializer::FindPattern(char* const InPattern) {
        UGHSDK_CHECK(InPattern != nullptr, "");
        UGHSDK_CHECK(strlen(InPattern) < 300, "");

        // Unpack the combined pattern into pattern and a mask.

        auto inPatternCopy = _strdup(InPattern);

        BYTE patternBytes[100];
        BYTE maskBytes[100];
        size_t patternLength = 0;

        ZeroMemory(patternBytes, 100);
        ZeroMemory(maskBytes, 100);

        if (!ParseCombinedPattern(inPatternCopy, (BYTE*)patternBytes, (BYTE*)maskBytes, &patternLength))
        {
            UGHSDK_CHECK(false, "failed to parse pattern");
            return nullptr;
        }

        free(inPatternCopy);

        // Use the built-in memory scanner.

        auto offset = ScanProcess(patternBytes, maskBytes);
        return offset;
    }


    // ! Global functions.
    // ========================================

    void* GetMainModuleBase() {
        DWORD const CurrentProcess = GetCurrentProcessId();
        HANDLE const ProcessSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, CurrentProcess);

        BYTE* BaseAddress = nullptr;

        if (ProcessSnapshot != INVALID_HANDLE_VALUE) {
            MODULEENTRY32 ModuleEntry{};
            ModuleEntry.dwSize = sizeof ModuleEntry;

            if (Module32First(ProcessSnapshot, &ModuleEntry)) {
                do {
                    wchar_t const* const ModuleName = ModuleEntry.szModule;
                    if (ModuleName == std::wcsstr(ModuleName, L"UDK")) {
                        BaseAddress = ModuleEntry.modBaseAddr;
                        break;
                    }
                } while (Module32Next(ProcessSnapshot, &ModuleEntry));
            }

            CloseHandle(ProcessSnapshot);
        }

        UGHSDK_CHECK(BaseAddress != nullptr, "failed to locate main module");
        return BaseAddress;
    }

    HMODULE GetGameModuleRange(BYTE** const pBegin, BYTE** const pEnd) {
        HMODULE exeModule = GetModuleHandle(nullptr);
        MODULEINFO exeModuleInfo;
        if (!GetModuleInformation(GetCurrentProcess(), exeModule, &exeModuleInfo, sizeof(MODULEINFO)))
        {
            return nullptr;
        }

        *pBegin = (BYTE*)exeModuleInfo.lpBaseOfDll;
        *pEnd = *pBegin + exeModuleInfo.SizeOfImage;
        return exeModule;
    }

    BYTE* ScanProcess(BYTE* const pattern, BYTE* const mask) {
        size_t patternLength = strlen((char*)mask);

        BYTE* start, * end, * pointer;
        if (!GetGameModuleRange(&start, &end)) {
            // GLogger.writeln(L"ScanProcess ERROR: GetGameModuleRange failed.");
            return nullptr;
        }

        pointer = start;
        while (pointer < end) {
            for (size_t matchLength = 0; matchLength < patternLength; matchLength++) {
                if (matchLength + 1 == patternLength)
                {
                    return pointer;
                }
                if (pointer + matchLength >= end)
                {
                    return nullptr;
                }
                if ((pattern[matchLength] != pointer[matchLength]) && (mask[matchLength] != '?'))
                {
                    break;
                }
            }

            pointer++;
        }

        return nullptr;
    }

}
