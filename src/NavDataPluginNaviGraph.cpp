#include "NavDataPluginNaviGraph.h"

extern "C"
{
    #include "lua.h"
    #include "lauxlib.h"
}

#include <format>

#include "CockpitAPI_Declare.h"
#include "sqlite3.h"

CockpitAPI cockpitAPI = CockpitAPI();

#define REGISTER_FUNCTION(function) { #function, l_ ## function }

namespace NavDataPluginNaviGraph
{
    
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
        const char* helloBuffer = cockpitAPI.getParamString(aStringLuaParam, 50);

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
