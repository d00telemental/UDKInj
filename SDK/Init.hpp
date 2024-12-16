/**
 * @file        SDK/Init.hpp
 * @brief       This file provides logic for initializing global variables.
 */

#pragma once
#include <vector>

namespace UGHSDK {

    /**
     * @brief
     *   Flag set if it makes sense to enable @c bTestAccess for @ref Initializer methods.
     */
    static constexpr bool kb_testAccess = false;

    /**
     * @brief
     *   Variant for a variable / function address specification that
     *   the @ref Initializer resolves into a typed pointer it may need.
     */
    class Address final {
        enum Mode : int {
            k_modeAbsolute = 1,
            k_modeOffset = 2,
            k_modePattern = 3,
            k_modeInstrRelative = 4,
        };

        union {
            void*       m_address;
            size_t      m_offset;
            char*       m_pattern;
        };
        Mode            m_valueMode{ 0 };
        long long       m_resolveShift{ 0 };

        friend class    Initializer;

    public:

        /** Constructs an @ref Address as an absolute address. */
        static Address FromAbsolute(void* Addr, long long Shift = 0);
        /** Constructs an @ref Address as an RVA offset (to the main game module). */
        static Address FromOffset(size_t Offset, long long Shift = 0);
        /** Constructs an @ref Address as a pattern that initializer must find. */
        static Address FromPattern(char const* Pattern, long long Shift = 0);

        /**
         * @brief
         * Constructs an @ref Address as a pattern to a MOV / LEA instruction
         * which uses RIP-relative addressing in source operand that initializer must find.
         *
         * @remarks
         * Across the previous-generation SDK this was known as "findAddressLeaMov()" (ME3Tweaks)
         * or "post-rip" (PHM-LE1), though the latter would search for an instruction *after* the one
         * actually using RIP-relative addressing.
         */
        static Address FromInstrRelative(char const* Pattern);

        /**
         * @brief
         * Constructs an @ref as a partial pattern that initializer must find
         * in a way that is compatible with the "post-hook" technique.
         *
         * @param[in] Pattern - The partial pattern searched, must have the first five bytes omitted.
         */
        static Address FromPostHook(char const* Pattern);
    };


    /**
     * @brief
     *   Utility type for initializing SDK-required global variables.
     */
    class Initializer final {
    public:

        void*                   ModuleBase{ nullptr };
        std::vector<void*>      HookTargets{};

        Initializer();
        ~Initializer() noexcept;

        void* Resolve(Address InAddr) const;
        void* InstallHook(void* Target, void* Detour);
        bool  UninstallAllHooks();

        template<typename T>
        T* ResolveTyped(Address const InAddr) const {
            void* const Resolved = Resolve(InAddr);
            return reinterpret_cast<T*>(Resolved);
        }

    private:

        static bool ParseCombinedPattern(char* InPattern, BYTE* OutPatternBuffer, BYTE* OutMaskBuffer, size_t* OutLength);
        static void* FindPattern(char* InPattern);
    };


    /**
     * @brief       Finds base address of the main game module.
     * @return      Base address of the module starting with "UDK" as a pointer.
     */
    void* GetMainModuleBase();

    HMODULE GetGameModuleRange(BYTE** pBegin, BYTE** pEnd);
    BYTE* ScanProcess(BYTE* pattern, BYTE* mask);

}


// ! Patterns for built-in globals.
// ========================================

#if defined(SDK_TARGET_UDK)

    #define BUILTIN_CALLFUNCTION_PAT                ::UGHSDK::Address::FromPattern("40 55 53 56 57 41 54 41 55 41 56 41 57 48 81 EC C8 04 00 00")
    #define BUILTIN_GENGINE_RIP                     ::UGHSDK::Address::FromInstrRelative("48 8B 05 ?? ?? ?? ?? 66 0F 6E 90 ?? ?? ?? ?? 0F 5A D2")
    #define BUILTIN_GMALLOC_RIP                     ::UGHSDK::Address::FromInstrRelative("48 8B 0D ?? ?? ?? ?? 48 8B 01 44 8B C3 8B D7 48 8B 5C 24 ?? 48 83 C4 20 5F 48 FF 60 ??")
    #define BUILTIN_GNATIVES_RIP                    ::UGHSDK::Address::FromInstrRelative("4C 8D 25 ?? ?? ?? ?? 80 38 04 74 36")
    #define BUILTIN_GOBOBJECTS_RIP                  ::UGHSDK::Address::FromInstrRelative("48 89 05 ?? ?? ?? ?? 3B 1D ?? ?? ?? ?? 7C 0E")
    #define BUILTIN_GWORLD_RIP                      ::UGHSDK::Address::FromInstrRelative("48 8B 0D ?? ?? ?? ?? E8 ?? ?? ?? ?? 48 89 87 ?? ?? ?? ?? 48 89 74 24")
    #define BUILTIN_PROCESSEVENT_PAT                ::UGHSDK::Address::FromPattern("40 55 41 55 41 56 48 81 EC C0 00 00 00")
    #define BUILTIN_PROCESSINTERNAL_PAT             ::UGHSDK::Address::FromPattern("40 53 55 56 57 41 54 48 81 EC 90 00 00 00 48 C7 44 24 ?? FE FF FF FF")
    #define BUILTIN_FNAMEINIT_PAT                   ::UGHSDK::Address::FromPattern("40 55 56 57 41 54 41 55 41 56 41 57 48 81 EC D0 0C 00 00")
    #define BUILTIN_FNAMEARRAY_RIP                  ::UGHSDK::Address::FromInstrRelative("48 89 05 ?? ?? ?? ?? 89 3E 48 8D 8C 24 C0")
    #define BUILTIN_STATICALLOCATEOBJECT_PAT        ::UGHSDK::Address::FromPattern("4C 89 44 24 ?? 55 56 57 41 54 41 55 41 56 41 57 48 8D AC 24 ?? ?? ?? ?? 48 81 EC 80 0B 00 00")

#endif
