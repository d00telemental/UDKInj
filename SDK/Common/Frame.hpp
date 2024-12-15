/**
 * @file        SDK/Common/Frame.hpp
 * @brief       This file implements some core virtual machine types.
 */

#pragma once

#include "Common/Core.hpp"
#include "Common/TArray.hpp"


#pragma pack(push, 4)

struct FFrame {
    struct FOutParmRec final {
        UProperty*          Property;
        BYTE*               PropAddr;
        FOutParmRec*        NextOutParm;
    };

    void*                   Vfptr;
    DWORD                   Unknown1;
    DWORD                   Unknown2;
    DWORD                   Unknown3;

    UStruct*                Node;
    UObject*                Object;
    BYTE*                   Code;
    BYTE*                   Locals;
    FFrame*                 PreviousFrame;
    FOutParmRec*            OutParms;

    void Step(UObject* Context, void* Result);
};

struct FStateFrame : public FFrame {
    struct FPushedState final {
        UState*             State;
        UStruct*            Node;
        BYTE*               Code;
    };

    UState*                 StateNode;
    DWORD                   ProbeMask;
    WORD                    LatentAction;
    BYTE                    ContinuedState;
    TArray<FPushedState>    StateStack;
    UClass*                 LocalsOwner;
};

#pragma pack(pop)


using tProcessInternal = void (UObject* Context, FFrame* Stack, void* Result);
using tProcessEvent = void (UObject* Context, UFunction* Function, void* Parms, void* Result);
using tCallFunction = void (UObject* Context, FFrame* Stack, void* Result, UFunction* Function);

using tNative = void (UObject* Context, FFrame* Stack, void* Result);

extern tNative** GNatives;
