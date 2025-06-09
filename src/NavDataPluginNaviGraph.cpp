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
    struct AirportItem {
        std::string airport_identifier;        // 4
        std::string airport_name;              // 30
        double      airport_ref_latitude;      // numeric
        double      airport_ref_longitude;     // numeric
        std::string airport_type;              // 1
        std::string area_code;                 // 3
        std::string ata_iata_code;             // 3
        std::string city;                      // 24
        std::string continent;                 // 40
        std::string country_3letter;           // 3
        std::string country;                   // 40
        double      elevation;                 // numeric
        std::string fuel;                      // 14
        std::string icao_code;                 // 2
        std::string ifr_capability;            // 1
        std::string longest_runway_surface_code; // 1
        double      magnetic_variation;        // numeric
        std::string speed_limit_altitude;      // 5
        double      speed_limit;               // numeric
        std::string state_2letter;             // 2
        std::string state;                     // 50
        std::string time_zone;                 // 3
        double      transition_altitude;       // numeric
        double      transition_level;          // numeric
    };

    std::vector<AirportItem> airportItems;

    fs::path findBestCycleFile();

    std::string navDataFilePath;
    bool navDataAvailable = false;

    // scans navDataFilePath for ng_jeppesen_fwdfd_XXXX.s3db,
    // returns the path to the highest-numbered cycle file (or empty path)
    fs::path findBestCycleFile()
    {
        fs::path dir{ navDataFilePath };
        if (!fs::exists(dir) || !fs::is_directory(dir))
            return {};

        const std::string prefix = "ng_jeppesen_fwdfd_";
        const std::string ext    = ".s3db";

        fs::path bestFile;
        int      bestCycle = -1;

        for (auto &ent : fs::directory_iterator(dir)) {
            if (!ent.is_regular_file()) continue;
            auto p    = ent.path();
            auto name = p.filename().string();
            if (p.extension() == ext && name.rfind(prefix, 0) == 0) {
                auto cycleStr = name.substr(
                    prefix.size(),
                    name.size() - prefix.size() - ext.size()
                );
                int cycleNum = -1;
                try { cycleNum = std::stoi(cycleStr); }
                catch (...) { continue; }
                if (cycleNum > bestCycle) {
                    bestCycle = cycleNum;
                    bestFile  = p;
                }
            }
        }

        return bestFile;  // empty if none found
    }

    // This function safely retrieves text from a SQLite statement.
    inline std::string safeText(sqlite3_stmt* stmt, int idx) {
        const unsigned char* t = sqlite3_column_text(stmt, idx);
        return t ? reinterpret_cast<const char*>(t) : std::string();
    }


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

    // this function grabs all Navigation data, and stores it in a struct
    int l_initAirportData(lua_State* L)
    {
        airportItems.clear();
    
        fs::path bestFile = findBestCycleFile();
        if (bestFile.empty()) {
            lua_pushnil(L);
            return 1;
        }
    
        sqlite3* db = nullptr;
        if (sqlite3_open(bestFile.string().c_str(), &db) != SQLITE_OK) {
            sqlite3_close(db);
            lua_pushnil(L);
            return 1;
        }
    
        const char* sql =
          "SELECT airport_identifier,"
          "       airport_name,"
          "       airport_ref_latitude,"
          "       airport_ref_longitude,"
          "       airport_type,"
          "       area_code,"
          "       ata_iata_code,"
          "       city,"
          "       continent,"
          "       country_3letter,"
          "       country,"
          "       elevation,"
          "       fuel,"
          "       icao_code,"
          "       ifr_capability,"
          "       longest_runway_surface_code,"
          "       magnetic_variation,"
          "       speed_limit_altitude,"
          "       speed_limit,"
          "       state_2letter,"
          "       state,"
          "       time_zone,"
          "       transition_altitude,"
          "       transition_level"
          "  FROM tbl_pa_airports;";
        sqlite3_stmt* stmt = nullptr;
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
            sqlite3_close(db);
            lua_pushnil(L);
            return 1;
        }
    
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            AirportItem itm;
            int col = 0;
            itm.airport_identifier        = safeText(stmt, col++);
            itm.airport_name              = safeText(stmt, col++);
            itm.airport_ref_latitude      = sqlite3_column_double(stmt, col++);
            itm.airport_ref_longitude     = sqlite3_column_double(stmt, col++);
            itm.airport_type              = safeText(stmt, col++);
            itm.area_code                 = safeText(stmt, col++);
            itm.ata_iata_code             = safeText(stmt, col++);
            itm.city                      = safeText(stmt, col++);
            itm.continent                 = safeText(stmt, col++);
            itm.country_3letter           = safeText(stmt, col++);
            itm.country                   = safeText(stmt, col++);
            itm.elevation                 = sqlite3_column_double(stmt, col++);
            itm.fuel                      = safeText(stmt, col++);
            itm.icao_code                 = safeText(stmt, col++);
            itm.ifr_capability            = safeText(stmt, col++);
            itm.longest_runway_surface_code = safeText(stmt, col++);
            itm.magnetic_variation        = sqlite3_column_double(stmt, col++);
            itm.speed_limit_altitude      = safeText(stmt, col++);
            itm.speed_limit               = sqlite3_column_double(stmt, col++);
            itm.state_2letter             = safeText(stmt, col++);
            itm.state                     = safeText(stmt, col++);
            itm.time_zone                 = safeText(stmt, col++);
            itm.transition_altitude       = sqlite3_column_double(stmt, col++);
            itm.transition_level          = sqlite3_column_double(stmt, col++);
            airportItems.push_back(std::move(itm));
        }

    
        sqlite3_finalize(stmt);
        sqlite3_close(db);
    
        // sort by airport_identifier
        std::sort(airportItems.begin(), airportItems.end(),
            [](auto &a, auto &b){
                return a.airport_identifier < b.airport_identifier;
            });
        
        // push Lua array
        lua_newtable(L);
        int idx = 1;
        for (auto &a : airportItems) {
            lua_newtable(L);
            auto S = [&](const char* k, const std::string &v){
                lua_pushstring(L, k);
                lua_pushstring(L, v.c_str());
                lua_settable(L, -3);
            };
            auto N = [&](const char* k, double d){
                lua_pushstring(L, k);
                lua_pushnumber(L, d);
                lua_settable(L, -3);
            };
            S("airport_identifier",        a.airport_identifier);
            S("airport_name",              a.airport_name);
            N("airport_ref_latitude",      a.airport_ref_latitude);
            N("airport_ref_longitude",     a.airport_ref_longitude);
            S("airport_type",              a.airport_type);
            S("area_code",                 a.area_code);
            S("ata_iata_code",             a.ata_iata_code);
            S("city",                      a.city);
            S("continent",                 a.continent);
            S("country_3letter",           a.country_3letter);
            S("country",                   a.country);
            N("elevation",                 a.elevation);
            S("fuel",                      a.fuel);
            S("icao_code",                 a.icao_code);
            S("ifr_capability",            a.ifr_capability);
            S("longest_runway_surface_code", a.longest_runway_surface_code);
            N("magnetic_variation",        a.magnetic_variation);
            S("speed_limit_altitude",      a.speed_limit_altitude);
            N("speed_limit",               a.speed_limit);
            S("state_2letter",             a.state_2letter);
            S("state",                     a.state);
            S("time_zone",                 a.time_zone);
            N("transition_altitude",       a.transition_altitude);
            N("transition_level",          a.transition_level);
        
            lua_rawseti(L, -2, idx++);
        }
    
        return 1;
    }

    // This function gets an airport item by its identifier and returns it as a Lua table.
    int l_getAirport(lua_State* L)
    {
        const char* ident = luaL_checkstring(L,1);
        for (auto &a : airportItems) {
            if (a.airport_identifier == ident) {
                // push one table (same as in init)
                lua_newtable(L);
                auto S = [&](const char* k, const std::string &v){
                    lua_pushstring(L, k);
                    lua_pushstring(L, v.c_str());
                    lua_settable(L, -3);
                };
                auto N = [&](const char* k, double d){
                    lua_pushstring(L, k);
                    lua_pushnumber(L, d);
                    lua_settable(L, -3);
                };
                S("airport_identifier",        a.airport_identifier);
                S("airport_name",              a.airport_name);
                N("airport_ref_latitude",      a.airport_ref_latitude);
                N("airport_ref_longitude",     a.airport_ref_longitude);
                S("airport_type",              a.airport_type);
                S("area_code",                 a.area_code);
                S("ata_iata_code",             a.ata_iata_code);
                S("city",                      a.city);
                S("continent",                 a.continent);
                S("country_3letter",           a.country_3letter);
                S("country",                   a.country);
                N("elevation",                 a.elevation);
                S("fuel",                      a.fuel);
                S("icao_code",                 a.icao_code);
                S("ifr_capability",            a.ifr_capability);
                S("longest_runway_surface_code", a.longest_runway_surface_code);
                N("magnetic_variation",        a.magnetic_variation);
                S("speed_limit_altitude",      a.speed_limit_altitude);
                N("speed_limit",               a.speed_limit);
                S("state_2letter",             a.state_2letter);
                S("state",                     a.state);
                S("time_zone",                 a.time_zone);
                N("transition_altitude",       a.transition_altitude);
                N("transition_level",          a.transition_level);
                return 1;
            }
        }
        lua_pushnil(L);
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
            REGISTER_FUNCTION(initAirportData),
            REGISTER_FUNCTION(getAirport),
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
