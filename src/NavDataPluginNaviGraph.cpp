#include "NavDataPluginNaviGraph.h"

extern "C"
{
    #include "lua.h"
    #include "lauxlib.h"
}

#include <format>
#include <string>


#include "CockpitAPI_Declare.h"
#include "sqlite3.h"

#include <filesystem>
namespace fs = std::filesystem;


CockpitAPI cockpitAPI = CockpitAPI();

#define REGISTER_FUNCTION(function) { #function, l_ ## function }

namespace NavDataPluginNaviGraph
{
    
    static std::string navDataFilePath;

    int l_setNavDataFilePath(lua_State* L)
{
    const char* path = luaL_checkstring(L, 1);
    navDataFilePath = path;

    try {
        fs::path dir{ navDataFilePath };
        // if they passed a full filename, take its parent directory
        if (!dir.has_extension())
        {
            // treat `path` itself as the directory
        }
        else
        {
            dir = dir.parent_path();
        }
        if (!dir.empty() && !fs::exists(dir)) {
            fs::create_directories(dir);
        }
    }
    catch (const fs::filesystem_error& e) {
        // report back to Lua
        return luaL_error(L,
            "NavDataPlugin: failed to create folder '%s': %s",
            e.path1().string().c_str(),
            e.what());
    }

    return 0;
}


    int l_getNavDataFilePath(lua_State* L)
    {
        lua_pushstring(L, navDataFilePath.c_str());
        return 1; // return the path as a string
    }



    // make sure to add any new functions to the bottom (minus the 'l_' part)
    int l_ExampleTable(lua_State* L)
    {
        lua_newtable(L);
        
        // this does newtable["1"] = 123
        lua_pushstring(L, "1");
        lua_pushinteger(L, 123);
        lua_settable(L, -3);

        return 1; // only 1 return value: table
    }

    int l_ParamTest(lua_State* L)
    {
        void* luaParam = cockpitAPI.getParamHandle("SOME_PARAM_WE_DECLARED_IN_LUA");
        cockpitAPI.setParamNumber(luaParam, 1);
        int luaValue = cockpitAPI.getParamNumber(luaParam);
        void* aStringLuaParam = cockpitAPI.getParamHandle("SOME_STRING_FROM_LUA");
        cockpitAPI.setParamString(aStringLuaParam, "hello");
        char helloBuffer[64];
        cockpitAPI.getParamString(aStringLuaParam, helloBuffer);

        return 0; // don't return anything
    }

    int l_Add(lua_State* L)
    {
        int a = luaL_checkinteger(L, 1);
        int b = luaL_checkinteger(L, 2);
        lua_pushinteger(L, a + b);
        return 1;
    }

    extern int l_CreateNavDataPluginNaviGraph(lua_State* L)
    {
        // Create Table
        lua_newtable( L );

        static const luaL_Reg functions[] = {
            REGISTER_FUNCTION(setNavDataFilePath),
            REGISTER_FUNCTION(getNavDataFilePath),
            REGISTER_FUNCTION(Add),
            REGISTER_FUNCTION(ExampleTable),
            REGISTER_FUNCTION(ParamTest),
            { nullptr, nullptr } // Sentinel to mark the end of the array
        };

        luaL_register( L, nullptr, functions );

        lua_pushvalue( L, -1 );
        lua_setfield( L, -2, "__index" );

        lua_pushstring( L, "NavDataPluginNaviGraph");
        lua_setfield( L, -2, "__name" );

        lua_pushinteger( L, 0 );
        lua_setfield( L, -2, "depth");
        return 1;
    }
} // namespace NavDataPluginNaviGraph
