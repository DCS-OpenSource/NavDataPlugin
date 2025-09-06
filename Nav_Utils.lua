-- MIT License

-- Copyright (c) 2025 DCS OpenSource

-- Permission is hereby granted, free of charge, to any person obtaining a copy
-- of this software and associated documentation files (the "Software"), to deal
-- in the Software without restriction, including without limitation the rights
-- to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
-- copies of the Software, and to permit persons to whom the Software is
-- furnished to do so, subject to the following conditions:

-- The above copyright notice and this permission notice shall be included in all
-- copies or substantial portions of the Software.

-- THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
-- IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
-- FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
-- AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
-- LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
-- OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
-- SOFTWARE.

-- This is commented out for Brian so his dummy library doesn't break... one day I can uncomment this
-- and remove the massive ugly list of beacons below
-- dofile(LockOn_Options.common_script_path.."../../../World/Radio/BeaconTypes.lua")

local log  = require("log") -- DCS Log Module

-- Beacon constants, there are loaded as globals in this file, or any file that loads this file
BEACON_TYPE_NULL = 0
BEACON_TYPE_VOR = 1
BEACON_TYPE_DME = 2
BEACON_TYPE_VOR_DME = 3
BEACON_TYPE_TACAN = 4
BEACON_TYPE_VORTAC = 5
BEACON_TYPE_RSBN = 128
BEACON_TYPE_BROADCAST_STATION = 1024

BEACON_TYPE_HOMER = 8
BEACON_TYPE_AIRPORT_HOMER = 4104
BEACON_TYPE_AIRPORT_HOMER_WITH_MARKER = 4136
BEACON_TYPE_ILS_FAR_HOMER = 16408
BEACON_TYPE_ILS_NEAR_HOMER = 16424

BEACON_TYPE_ILS_LOCALIZER = 16640
BEACON_TYPE_ILS_GLIDESLOPE = 16896

BEACON_TYPE_PRMG_LOCALIZER = 33024
BEACON_TYPE_PRMG_GLIDESLOPE = 33280

BEACON_TYPE_ICLS_LOCALIZER = 131328
BEACON_TYPE_ICLS_GLIDESLOPE = 131584

BEACON_TYPE_NAUTICAL_HOMER = 65536

BEACON_TYPE_TACAN_RANGE = 262144

-- ================ CONSTANTS ================
M_TO_FT = 3.28084       -- conversion factor from meters to feet
M_TO_NM = 0.000539957   -- conversion factor from meters to nautical miles
NM_TO_M = 1852          -- conversion factor from nautical miles to meters

-- ===========================================


---Function to calculate the distance between two points in meters
---@param x1 number X coordinate of the first point
---@param y1 number Y coordinate of the first point
---@param x2 number X coordinate of the second point
---@param y2 number Y coordinate of the second point
---@return number Distance in meters between the two points
local function getDistanceBetweenPoints(x1, y1, x2, y2)
    return math.sqrt((x2 - x1)^2 + (y2 - y1)^2)
end

local function getDistanceBetweenPointsWithAltitude(x1, y1, z1, x2, y2, z2)
    return math.sqrt(((x2 - x1)^2) + ((y2 - y1)^2) + ((z2 - z1)^2))
end


---Function to calculate the bearing from one point to another
---@param x1 number X coordinate of the first point
---@param y1 number Y coordinate of the first point
---@param x2 number X coordinate of the second point
---@param y2 number Y coordinate of the second point
---@return number Bearing in degrees from the first point to the second point
local function getBearing(x1, y1, x2, y2)
    local deltaX = x2 - x1
    local deltaY = y2 - y1
    local bearing = math.deg(math.atan2(deltaY, deltaX))

    -- Normalize the bearing to be within 0-360 degrees
    if bearing < 0 then bearing = bearing + 360 end

    return bearing
end


---Function to get the radio frequencies for a specific airport
---@param radio table A table containing the radio frequencies for the airport
---@param Radios table A table containing all available radio frequencies indexed by airport ID
---@return table|nil A table containing the radio frequencies, or nil if no frequencies are set
local function getAirportRadios(radio, Radios)
    if not radio or not radio[1] then return nil end
    if Radios[radio[1]] then return Radios[radio[1]]
    else return nil end
end


---Function to transform a boolean civilian status into a string for the T-38C only
---@param civilian boolean True if the aircraft is civilian, false if military
---@return string "CIV" for civilian, "MIL" for military
local function getCivilianStatus(civilian)
    if civilian then return "CIV" else return "MIL" end
end


---Function to convert a position in meters to latitude and longitude coordinates
---@param reference_point table A table containing x and y coordinates in meters
---@return table A table containing latitude and longitude coordinates
local function convertPosToLatLon(reference_point)
    local location = lo_to_geo_coords(reference_point.x, reference_point.y)
    location.x = reference_point.x
    location.y = reference_point.y
    return location
end


---Function to deeply merge two tables, updating only the values that are changed
---@param target table The target table to be updated
---@param source table The source table containing new values
---@return table The updated target table with merged values
local function deepMerge(target, source) 
    for key, value in pairs(source) do
        if type(value) == "table" and type(target[key]) == "table" then
            deepMerge(target[key], value)
        else
            target[key] = value
        end
    end
    return target
end

---Wrapper function for writing to DCS.log using the built in log module.
---Labelled with tag WARNING. 
---@param string string Message to appear in the log
local function log_warning(string)
    log.write("NAVDATAPLUGIN", log.WARNING, string)
end


return {
    getDistanceBetweenPoints = getDistanceBetweenPoints,
    getDistanceBetweenPointsWithAltitude = getDistanceBetweenPointsWithAltitude,
    getBearing = getBearing,
    getAirportRadios = getAirportRadios,
    getCivilianStatus = getCivilianStatus,
    convertPosToLatLon = convertPosToLatLon,
    deepMerge = deepMerge,

    log_warning = log_warning
}
