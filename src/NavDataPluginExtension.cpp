#include "NavDataPluginExtension.h"

extern "C"
{
    #include "lua.h"
    #include "lauxlib.h"
}

#include <format>

#define REGISTER_FUNCTION(function) { #function, l_ ## function }

namespace NavDataPluginExtension
{

    int l_Add(lua_State* L)
    {
        int a = luaL_checkinteger(L, 1);
        int b = luaL_checkinteger(L, 2);
        lua_pushinteger(L, a + b);
        return 1;
    }

    extern int l_CreateNavDataPluginExtension(lua_State* L)
    {
        // Create Table
        lua_newtable( L );

        static const luaL_Reg functions[] = {
            REGISTER_FUNCTION(Add),
            { nullptr, nullptr } // Sentinel to mark the end of the array
        };

        luaL_register( L, nullptr, functions );

        lua_pushvalue( L, -1 );
        lua_setfield( L, -2, "__index" );

        lua_pushstring( L, "NavDataPluginExtension");
        lua_setfield( L, -2, "__name" );

        lua_pushinteger( L, 0 );
        lua_setfield( L, -2, "depth");
        return 1;
    }
} // namespace NavDataPluginExtension
