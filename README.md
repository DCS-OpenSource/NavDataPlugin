# Caffeine Simulations Nav Info System
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

## User Guide

1. Navigate to the releases tab, download the latest version.
2. Place the `Nav/` folder inside the zip into `Cockpit/Scripts/Systems/`, the path to `Nav.lua` should look like: `Cockpit/Scripts/Systems/Nav/Nav.lua`
3. Add `dofile(LockOn_Options.script_path.."Systems/Nav/Nav.lua")` at the top of any file you want to access airport data
4. Get the data `local airportData = getAirports()`
5. Use the data for whatever you need, right now I have a moving map and Divert page working, both using the data from this

## Additional Features
### `Nav.lua`
* `sortAirportsByDistance(ownPos)` a function which gets parsed your own position and sorts them by distance.
    * I put this function in the `update()` of my lua device to update the list as I move around
    * I also run this once a second, I haven't tested it running at standard lua device update speeds
```lua
local ownPos = {own_latitude:get(), own_longitude:get()}
local airports = sortAirportsByDistance(ownPos) -- returns the same list of airports, but sorted by distance to the player
```
### `Nav_Utils.lua` 
(You will need to have `dofile(LockOn_Options.script_path.."Systems/Nav/Nav_Utils.lua")` to use these)
* `printTableContents(table)` A function to recursively print data from any table
    * This is a handy function to quickly see the contents of some unknown data you arent aware of.
    * This was extremely helpful for debugging the output of the `Terrain` module, and hopefully you find it useful too.
    * Try not to run it in `update()` or your PC will melt, DCS doesnt like 1000s of `print_message_to_user()` messages

* `getBearing(lat1, lon1, lat2, lon2)` a function to calculate the bearing from the player to an airport (or any point)
    * lat1, lon1 being ownPos

* `haversine(lat1, lon1, lat2, lon2)` Haversine formula to calculate the distance between two points on the Earth

## Wishlist
* Edit DCS data for specific maps, so your data will overwrite (but only where specified)
    * this is half done, see /additionalData, but right now that has no effect
* Beacons, TACAN, VOR etc...
* Map Support testing and tuning, different maps handle data differently, but I think I got it all
* More a T-38 thing, but a way to list Beacons at an airport (this is relevant for my personal avionics)

## Can I use this?
* Yes you can :-)
* Please Leave the comments at the top of the file untouched
* Feel free to create pull requests to add features, but try not to make breaking changes to the existing tables
* I will continue to update this as time goes on
