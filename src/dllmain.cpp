#include <Windows.h>

extern "C"
{
#include "lua.h"
}


#include "NavDataPluginExtension.h"
#include <optional>

BOOL APIENTRY DLLMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
        case DLL_PROCESS_ATTACH:
            break;
        case DLL_THREAD_ATTACH:
            break;
        case DLL_THREAD_DETACH:
            break;
        case DLL_PROCESS_DETACH:
            break;
    }
    return TRUE;
}



extern "C" int __declspec( dllexport ) luaopen_NavDataPluginExtension( lua_State* L )
{
    return NavDataPluginExtension::l_CreateNavDataPluginExtension(L);
}