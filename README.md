# DCSOpenSource Navigation Information Plugin
By Hayds_93 and the Caffeine Simulations Team
A small drop in "API" to dynamically pull data from DCS for airport information.

This should work on every map. An exception is Normandy, as since its a map from the 1940s it doesnt have ICAO codes, however every other parameter should work.
It is untested on:
* Afghanistan
* South Atlantic

## Data included
Below is an example of one entry in the resulting table.

```lua
{
    "Nellis" = {
        name = "Nellis",
        position = {
            lon = -115.03300055101,
            lat = 36.235224110884
        },
        runways = {
            1 = {
                runwayLength = 9455.97,
                name = 03L-21R
            },
            2 = {
                runwayLength = 9439.81,
                name = 03R-21L
            } 
        },
        radioid = { -- this is used to link radio data to airfield
            1 = airfield4_0
        },
        beacons = {
            -- Beacon data exists, but is WIP
            -- this will only have airfield beacons
            -- map beacons will be handled seperatly
        },
        radios = {
            radioId = "airfield4_0",
            uniform = 327,
            victor = 132.55
        },
        isCivilian = false,
        ICAO = KLSV
    }
}
```

# User Guide

## Install Guide

### .git submodule
1. Navigate to `Cockpit/Scripts/Systems` in your terminal
2. Run `git submodule add https://github.com/DCS-OpenSource/NavDataPlugin.git`
3. commit the submodule file to your repo

### Manual Install
1. Download the latest release (the .zip, not source code)
2. unzip, and place the folder in `Cockpit/Scripts/Systems`
3. verify the relative path to `Nav.lua` is `Cockpit/Scripts/Systems/NavDataPlugin/Nav.lua`

## config
* edit the config at the top of `Nav.lua`, I would recommend setting both to false, at least initially.
```
local doICAO = true                 -- set to false to disable ICAO data
local doDataSupplementation = true  -- set to false to disable additional data supplementation
```


## Usage
1. Add `dofile(LockOn_Options.script_path.."Systems/NavDataPlugin/Nav.lua")` at the top of any file you want to access airport data
2. Get the data `local airportData = getAirports()` which returns the lua table above.

# Additional Features
## `Nav.lua`
* `sortAirportsByDistance(ownPos)` a function which gets parsed your own position and sorts them by distance.
    * I put this function in the `update()` of my lua device to update the list as I move around
    * I also run this once a second, I haven't tested it running at standard lua device update speeds
```lua
local ownPos = {own_latitude:get(), own_longitude:get()}
local airports = sortAirportsByDistance(ownPos) -- returns the same list of airports, but sorted by distance to the player
```
## `Nav_Utils.lua` 
(You will need to have `dofile(LockOn_Options.script_path.."Systems/NavDataPlugin/Nav_Utils.lua")` to use these)
* `printTableContents(table)` A function to recursively print data from any table
    * This is a handy function to quickly see the contents of some unknown data you arent aware of.
    * This was extremely helpful for debugging the output of the `Terrain` module, and hopefully you find it useful too.
    * Try not to run it in `update()` or your PC will melt, DCS doesnt like 1000s of `print_message_to_user()` messages
    * *note: while this works, using JNelsons ImGui is a much better way to do this, found [here](https://github.com/08jne01/dcs-lua-imgui/tree/main)*
* `getBearing(lat1, lon1, lat2, lon2)` a function to calculate the bearing from the player to an airport (or any point)
    * lat1, lon1 being ownPos

* `haversine(lat1, lon1, lat2, lon2)` Haversine formula to calculate the distance between two points on the Earth

## Wishlist
* Beacons, TACAN, VOR etc...
* Map Support testing and tuning, different maps handle data differently, but I think I got it all
* More a T-38 thing, but a way to list Beacons at an airport (this is relevant for my personal avionics)

## Can I use this?
* Yes you can :-)
* Please Leave the comments at the top of the file untouched
* Feel free to create pull requests to add features, but try not to make breaking changes to the existing tables
* I will continue to update this as time goes on
