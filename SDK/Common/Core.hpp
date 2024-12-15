/**
 * @file        SDK/Common/Core.hpp
 * @brief       This file includes core scalar typedefs, forward declarations, and utility macros.
 */

#pragma once

#include <Windows.h>
#undef lst1
#undef lst2
#undef lst3
#undef lst4
#undef lst5
#undef lst6
#undef lst7
#undef lst8
#undef lst9
#undef lst10
#undef lst11
#undef lst12
#undef lst13
#undef lst14
#undef lst15
#undef lst16

#include <cassert>


 // ! Basic assertion macros (can be overriden by consumers).
 // ========================================

#if !defined(UGHSDK_CHECK_OVERRIDE) || UGHSDK_CHECK_OVERRIDE == 0

#ifndef NDEBUG
    // #include <cassert>
    #define UGHSDK_CHECK(cond, msg) assert((cond) && msg)
#else
    #define UGHSDK_CHECK(cond, msg) (void)(cond)
#endif

#endif

#if !defined(UGHSDK_WARN_OVERRIDE) || UGHSDK_WARN_OVERRIDE == 0

namespace Detail { void DefaultWarn(char const* Message); }
#define UGHSDK_WARN(msg) ::Detail::DefaultWarn(msg)

#endif


// ! Basic type definitions.
// ========================================

typedef signed char             SBYTE;
typedef signed short            SWORD;
typedef signed int              SDWORD;
typedef unsigned long long      QWORD;
typedef signed long long        SQWORD;

typedef size_t                  SIZE_T;


static_assert(sizeof(BYTE) == 1 && sizeof(SBYTE) == 1);
static_assert(sizeof(WORD) == 2 && sizeof(SWORD) == 2);
static_assert(sizeof(DWORD) == 4 && sizeof(SDWORD) == 4);
static_assert(sizeof(QWORD) == 8 && sizeof(SQWORD) == 8);


typedef unsigned int UBOOL;


// ! Implementation stuff for containers.
// ========================================

#define CONTAINER_TYPEDEFS(Elem, Size, Diff)                        \
    public:                                                         \
        using value_type = Elem;                                    \
        using size_type = Size;                                     \
        using difference_type = Diff;                               \
        using reference = value_type&;                              \
        using const_reference = value_type const&;                  \
        using pointer = value_type*;                                \
        using const_pointer = value_type const*;                    \
        using iterator = value_type*;                               \
        using const_iterator = value_type const*;


// ! Forward declarations.
// ========================================

struct FFrame;
struct FStateFrame;

class UObject;
class UField;
class UEnum;
class UConst;
class UStruct;
class UFunction;
class UState;
class UClass;
class UProperty;
class UEngine;
class UWorld;
class USystem;


// ! Unreal Engine's global allocator.
// ========================================

#if defined(SDK_TARGET_UDK)

class FMallocLike {
public:

    virtual UBOOL Exec(WCHAR const* Cmd, void* Ar) = 0;                         // vtable offset = 0x00
    virtual DWORD GetSize(DWORD Count, DWORD Alignment) = 0;                    // vtable offset = 0x08
    virtual void* Malloc(DWORD Count, DWORD Alignment) = 0;                     // vtable offset = 0x10
    virtual void* Realloc(void* Orig, DWORD Count, DWORD Alignment) = 0;        // vtable offset = 0x18
    virtual void Free(void* Orig) = 0;                                          // vtable offset = 0x20
    // ...
};

#endif

// No idea what those "Unknown" methods are...
// Maybe some kind of a security feature that blows up vtable for giggles?


static constexpr unsigned int UN_DEFAULT_ALIGNMENT = sizeof(void*) * 2;
static_assert(UN_DEFAULT_ALIGNMENT == 16);


void* sdkMalloc(DWORD Count, DWORD Alignment = UN_DEFAULT_ALIGNMENT);
void* sdkRealloc(void* Orig, DWORD Count, DWORD Alignment = UN_DEFAULT_ALIGNMENT);
void  sdkFree(void* Orig);

template<typename T>
T* sdkMallocTyped(DWORD const Num, DWORD const Alignment = UN_DEFAULT_ALIGNMENT) {
    return reinterpret_cast<T*>(sdkMalloc(static_cast<DWORD>(Num * sizeof(T)), Alignment));
}

template<typename T>
T* sdkReallocTyped(T* const Orig, DWORD const Num, DWORD const Alignment = UN_DEFAULT_ALIGNMENT) {
    return reinterpret_cast<T*>(sdkRealloc(Orig, static_cast<DWORD>(Num * sizeof(T)), Alignment));
}

template<typename T>
void sdkFreeTyped(T* const Orig) {
    sdkFree(Orig);
}


// ! Non-member global variables.
// ========================================

extern FMallocLike**    GMalloc;

extern UEngine**        GEngine;
extern USystem**        GSys;
extern UWorld**         GWorld;
