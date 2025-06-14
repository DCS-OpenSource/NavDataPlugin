#include <Windows.h>
#include <stdio.h>


extern "C" {
#include "sqlite3.h"
#include "lua.h"
}


#include "NavDataPluginNaviGraph.h"
#include <optional>

BOOL APIENTRY DLLMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    sqlite3 *db;
    char *zErrMsg = 0;
    int rc;

    // if( argc!=3 ){
    //   fprintf(stderr, "Usage: %s DATABASE SQL-STATEMENT\n", argv[0]);
    //   return(1);
    // }
    sqlite3_open("database.db", nullptr);

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



extern "C" int __declspec( dllexport ) luaopen_NavDataPluginNaviGraph( lua_State* L )
{
    return NavDataPluginNaviGraph::l_CreateNavDataPluginNaviGraph(L);
}