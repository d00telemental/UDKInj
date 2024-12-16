#include "dllmain.hpp"

#include <SDK/Init.hpp>
#include <SDK/Headers.hpp>


std::unique_ptr<UGHSDK::Initializer> gp_sdkInitializer;

DWORD WINAPI OnAttachThread
(
    [[maybe_unused]] LPVOID                     const lpParameter
)
{
    LOG_INFO(L"Hello there, %s!", L"UDKAgent");

    UGHSDK::InitializeConsole();
    gp_sdkInitializer = std::make_unique<UGHSDK::Initializer>();

    GMalloc = gp_sdkInitializer->ResolveTyped<FMallocLike*>(BUILTIN_GMALLOC_RIP);
    UObject::GObjObjects = gp_sdkInitializer->ResolveTyped<TArray<UObject*>>(BUILTIN_GOBOBJECTS_RIP);
    FName::GNameArray = gp_sdkInitializer->ResolveTyped<TArray<FNameEntry*>>(BUILTIN_FNAMEARRAY_RIP);
    FName::GInitMethod = gp_sdkInitializer->ResolveTyped<FName::tInitMethod>(BUILTIN_FNAMEINIT_PAT);

    UGHSDK_CHECK(GMalloc != nullptr, "failed to resolve: GMalloc");
    UGHSDK_CHECK(UObject::GObjObjects != nullptr, "failed to resolve: UObject::GObjObjects");
    UGHSDK_CHECK(FName::GNameArray != nullptr, "failed to resolve: FName::GNameArray");
    UGHSDK_CHECK(FName::GInitMethod != nullptr, "failed to resolve: FName::GInitMethod");

    LOG_INFO(L"Core pointers initialized.");

    {
        FString TestString{};

        TestString.Append(L"Hello there, ");
        TestString.AppendAnsi("UDKAgent!");

        LOG_INFO(L"Test string = %s", *TestString);
    }

    return TRUE;
}

VOID WINAPI OnDetach()
{
    if (!gp_sdkInitializer->UninstallAllHooks())
    {
        LOG_ERROR(L"Failed to uninstall one or more hooks!");
        Sleep(1000);
    }
    gp_sdkInitializer.reset();

    LOG_INFO(L"Goodbye, %s...", L"UDKAgent");
    UGHSDK::TerminateConsole();
}

ASI_EXPORT ExportListObjects()
{
    LOG_INFO(L"Listing objects...");

    std::size_t Counter = 0;
    for (UObject* const Object : *UObject::GObjObjects)
    {
        if (Object == nullptr) continue;
        LOG_INFO(L"[%llu] %s at 0x%p", Counter++, *Object->GetFullName(), (void*)Object);
    }

    LOG_INFO(L"Done");
}

BOOL APIENTRY DllMain
(
    [[maybe_unused]] HMODULE                    const hModule,
    [[maybe_unused]] DWORD                      const dwReasonForCall,
    [[maybe_unused]] LPVOID                     const lpReserved
)
{
    switch (dwReasonForCall)
    {
    case DLL_PROCESS_ATTACH:
        OnAttachThread(nullptr);
        break;
    case DLL_PROCESS_DETACH:
        OnDetach();
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        break;
    }

    return TRUE;
}

