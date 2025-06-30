#include "NavDataPluginSimbrief.h"

extern "C" {
    #include "lua.h"
    #include "lauxlib.h"
}

#include <string>
#include <windows.h>
#include <wininet.h>
#pragma comment(lib, "wininet.lib")

#include <nlohmann/json.hpp>

namespace NavDataPluginNaviGraph
{
    std::string simbrief_username;

    // Helper to convert UTF-8 std::string → UTF-16 std::wstring
    static std::wstring utf8_to_wstring(const std::string& s) {
        if (s.empty()) return {};
        int size_needed = MultiByteToWideChar(
            CP_UTF8, 0, s.data(), (int)s.size(), nullptr, 0);
        std::wstring wstr(size_needed, 0);
        MultiByteToWideChar(
            CP_UTF8, 0, s.data(), (int)s.size(), wstr.data(), size_needed);
        return wstr;
    }

    // Recursively push a nlohmann::json value onto the Lua stack
    void push_json(lua_State* L, const nlohmann::json& j) {
        switch (j.type()) {
            case nlohmann::json::value_t::null:
                lua_pushnil(L);
                break;
            case nlohmann::json::value_t::boolean:
                lua_pushboolean(L, j.get<bool>());
                break;
            case nlohmann::json::value_t::number_integer:
                lua_pushinteger(L, j.get<long long>());
                break;
            case nlohmann::json::value_t::number_unsigned:
                lua_pushinteger(L, j.get<unsigned long long>());
                break;
            case nlohmann::json::value_t::number_float:
                lua_pushnumber(L, j.get<double>());
                break;
            case nlohmann::json::value_t::string: {
                const auto& s = j.get_ref<const std::string&>();
                lua_pushlstring(L, s.c_str(), s.size());
                break;
            }
            case nlohmann::json::value_t::array: {
                lua_newtable(L);
                int idx = 1;
                for (const auto& el : j) {
                    push_json(L, el);
                    lua_rawseti(L, -2, idx++);
                }
                break;
            }
            case nlohmann::json::value_t::object: {
                lua_newtable(L);
                for (auto it = j.begin(); it != j.end(); ++it) {
                    push_json(L, it.value());
                    lua_setfield(L, -2, it.key().c_str());
                }
                break;
            }
            default:
                lua_pushnil(L);
                break;
        }
    }

    // Lua binding: setUsername(username)
    int l_setSimbriefUserName(lua_State* L) {
        const char* user = luaL_checkstring(L, 1);
        simbrief_username = user;
        return 0;
    }

    // Helper: fetch the full body of an HTTPS GET into a std::string
    static std::string fetchUrlWinInet(const std::string& url) {
        std::string result;
        std::wstring wurl = utf8_to_wstring(url);

        HINTERNET hSession = InternetOpenW(
            L"NavDataPluginSimbrief",
            INTERNET_OPEN_TYPE_PRECONFIG,
            nullptr, nullptr, 0);
        if (!hSession) return {};

        HINTERNET hRequest = InternetOpenUrlW(
            hSession,
            wurl.c_str(),
            nullptr, 0,
            INTERNET_FLAG_SECURE | INTERNET_FLAG_NO_CACHE_WRITE,
            0);
        if (!hRequest) {
            InternetCloseHandle(hSession);
            return {};
        }

        char buffer[4096];
        DWORD bytesRead = 0;
        while (InternetReadFile(hRequest, buffer, sizeof(buffer), &bytesRead) && bytesRead) {
            result.append(buffer, bytesRead);
        }

        InternetCloseHandle(hRequest);
        InternetCloseHandle(hSession);
        return result;
    }

    // Lua binding: ofp_table = getLatestSimbriefOFP()
    int l_getLatestSimbriefOFP(lua_State* L) {
        if (simbrief_username.empty()) {
            return luaL_error(L, "SimBrief username not set. Call setSimbriefUserName(user) first.");
        }

        // build URL
        std::string apiUrl =
            "https://www.simbrief.com/api/xml.fetcher.php?username=" +
            simbrief_username + "&json=1";

        // perform request via WinInet
        std::string response = fetchUrlWinInet(apiUrl);
        if (response.empty()) {
            return luaL_error(L, "HTTP GET failed or returned empty response");
        }

        // parse JSON
        nlohmann::json j;
        try {
            j = nlohmann::json::parse(response);
        } catch (const std::exception& e) {
            return luaL_error(L, "JSON parse error: %s", e.what());
        }

        // convert JSON → Lua table
        push_json(L, j);
        return 1;  // one return value (the table)
    }
}
