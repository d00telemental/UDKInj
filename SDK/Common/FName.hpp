/**
 * @file        SDK/Common/Core.hpp
 * @brief       This file partially implements FName.
 */

#pragma once

#include <compare>

#include "Common/Core.hpp"
#include "Common/FString.hpp"
#include "Common/TArray.hpp"


#pragma pack(push, 1)

struct FNameEntry {
    UINT64          Flags;
    UINT32          HashIndex : 31;
    UINT32          bUnicode : 1;
    FNameEntry*     HashNext;

    inline bool IsUnicode() const { return bUnicode == 1; }
    inline std::size_t GetLength() const {
        return !IsUnicode()
            ? std::strlen(&AnsiName[0])
            : std::wcslen(&WideName[0]);
    }

    union
    {
        char        AnsiName[1];
        wchar_t     WideName[1];
    };
};

struct FName {
    // Member variables.

    UINT32          Index;
    INT32           Number;

    // Global variables.

    using tInitMethod = void(FName* Self, WCHAR const* InName, INT InNumber, UBOOL bCreateIfNotFound, UBOOL bSplitName);

    static tInitMethod*             GInitMethod;
    static TArray<FNameEntry*>*     GNameArray;

    // Formatting flags.

    enum FormatMode : int {
        // Format only string contents.
        k_formatBasic = 1,
        // Format both string contents and number.
        k_formatExtended = 2,
        // If number is zero, format only string contents, otherwise also format (number - 1).
        k_formatInstanced = 3,
    };

    // Public interface.

    FName() = default;
    FName(char const* Lookup, int Instance, bool bSplit = false);
    FName(wchar_t const* Lookup, int Instance, bool bSplit = false);

    bool operator==(FName const& Other) const noexcept {
        auto const Left = *(SIZE_T*)this;
        auto const Right = *(SIZE_T*)&Other;
        return Left == Right;
    }

    auto operator<=>(FName const& Other) const noexcept {
        auto const Left = *(SIZE_T*)this;
        auto const Right = *(SIZE_T*)&Other;
        return Left <=> Right;
    }

    FNameEntry const* GetEntry() const noexcept;
    char const* GetName() const noexcept;
    SIZE_T GetLength() const noexcept;

    template<bool WithRAII>
    void AppendToString(FStringBase<WithRAII>& OutString, FormatMode Mode) const;
    inline FString ToString(FormatMode Mode = k_formatInstanced) const;

    /** Hash function for associative containers. */
    friend inline DWORD GetTypeHash(FName const Value) noexcept {
        return static_cast<DWORD>(*reinterpret_cast<QWORD const*>(&Value));
    }
};

static_assert(sizeof(FName) == 8);


template<bool WithRAII>
void FName::AppendToString(FStringBase<WithRAII>& OutString, FormatMode const Mode) const {
    FNameEntry const* const Entry = GetEntry();

    OutString.Reserve(static_cast<UINT>(Entry->GetLength() + (Mode > k_formatBasic ? 5 : 0)));

    if (!Entry->IsUnicode()) {
        OutString.AppendAnsi(Entry->AnsiName);
    } else {
        OutString.Append(Entry->WideName);
    }

    if (Mode == k_formatExtended) {
        OutString.AppendFormat(L"_%d", Number);
    } else if (Mode == k_formatInstanced && Number > 0) {
        OutString.AppendFormat(L"_%d", Number - 1);
    }
}

inline FString FName::ToString(FormatMode const Mode) const {
    FString OutString{};
    AppendToString(OutString, Mode);
    return OutString;
}


#pragma pack(pop)
