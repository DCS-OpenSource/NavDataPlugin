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
    static bool navDataAvailable = false;

    // This function sets the path for the nav data file. and creates the directory if it does not exist.
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

    // This function retrieves the current nav data file path.
    int l_getNavDataFilePath(lua_State* L)
    {
        lua_pushstring(L, navDataFilePath.c_str());
        return 1; // return the path as a string
    }

    // This function loads the nav data header from the database file and returns it as a Lua table.
    int l_loadNavDataHeader(lua_State* L)
    {
        navDataAvailable = false;

        if (navDataFilePath.empty()) {
            lua_pushnil(L);
            return 1;
        }

        fs::path dir{ navDataFilePath };
        if (!fs::exists(dir) || !fs::is_directory(dir)) {
            lua_pushnil(L);
            return 1;
        }

        // Scan for all files matching ng_jeppesen_fwdfd_XXXX.s3db
        const std::string prefix = "ng_jeppesen_fwdfd_";
        const std::string ext    = ".s3db";

        fs::path bestFile;
        int      bestCycle = -1;

        for (auto &ent : fs::directory_iterator(dir)) {
            if (!ent.is_regular_file()) continue;
            auto p        = ent.path();
            auto name     = p.filename().string();

            // must start with prefix and end with ext
            if (p.extension() == ext && name.rfind(prefix, 0) == 0) {
                // extract the cycle substring:
                //   name = prefix + cycle(4 chars) + ext
                auto cycleStr = name.substr(
                    prefix.size(),
                    name.size() - prefix.size() - ext.size()
                );
                // try to parse
                int cycleNum = -1;
                try {
                    cycleNum = std::stoi(cycleStr);
                } catch (...) {
                    continue;
                }
                if (cycleNum > bestCycle) {
                    bestCycle = cycleNum;
                    bestFile  = p;
                }
            }
        }

        if (bestFile.empty()) {
            // nothing found
            lua_pushnil(L);
            return 1;
        }

        // Open and query the header table
        sqlite3 *db = nullptr;
        if (sqlite3_open(bestFile.string().c_str(), &db) != SQLITE_OK) {
            sqlite3_close(db);
            lua_pushnil(L);
            return 1;
        }

        const char *sql =
          "SELECT creator, cycle, data_provider, dataset_version,"
          "       dataset, effective_fromto, parsed_at, revision"
          "  FROM tbl_hdr_header LIMIT 1;";
        sqlite3_stmt *stmt = nullptr;
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
            sqlite3_close(db);
            lua_pushnil(L);
            return 1;
        }

        if (sqlite3_step(stmt) != SQLITE_ROW) {
            sqlite3_finalize(stmt);
            sqlite3_close(db);
            lua_pushnil(L);
            return 1;
        }

        // If we reach here, NavData is available
        navDataAvailable = true;

        // Build and return Lua table
        lua_newtable(L);
        auto pushF = [&](const char *k, int i) {
            lua_pushstring(L, k);
            lua_pushstring(L,
              reinterpret_cast<const char*>(sqlite3_column_text(stmt,i)));
            lua_settable(L, -3);
        };
        pushF("creator",          0);
        pushF("cycle",            1);
        pushF("data_provider",    2);
        pushF("dataset_version",  3);
        pushF("dataset",          4);
        pushF("effective_fromto", 5);
        pushF("parsed_at",        6);
        pushF("revision",         7);

        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return 1;
    }

    // This function checks if the nav data is available and returns a boolean.
    int l_isNavDataAvailable(lua_State* L)
    {
        lua_pushboolean(L, navDataAvailable);
        return 1;
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
            REGISTER_FUNCTION(loadNavDataHeader),
            REGISTER_FUNCTION(isNavDataAvailable),
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
