#pragma once

struct lua_State;

namespace NavDataPluginNaviGraph
{
    int l_getLatestSimbriefOFP(lua_State* L);
    int l_setSimbriefUserName(lua_State* L);
}
