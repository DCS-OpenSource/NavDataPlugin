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

package.path = package.path..";"..LockOn_Options.script_path.."NavDataPlugin/?.lua"

local Nav_Utils = require('Nav_Utils')
local Terrain = require('terrain') -- DCS terrain module

local ILS_beacons           = {}
local TCN_beacons           = {}
local VOR_beacons           = {}
local FilteredAirportData   = {} -- Data filtered for relevant info and has extra info added from /additionalData
local Radios                = {}


-- ===================== Data Initialize ====================
local aircraftType = get_aircraft_type() -- this enables me to only use some features for the T-38C

local rawAirportData = get_terrain_related_data("Airdromes")

do_mission_file("mission") -- Load the mission file
local theatre = mission.theatre -- map name string

-- Load Beacon Data
local fileName =  get_terrain_related_data("beacons") or get_terrain_related_data("beaconsFile")
if fileName then 
    local f = loadfile(fileName)
    if f then
        f() -- makes array "beacons" available
    end
end

-- Categorize beacons by type (can be in more than one category (VORTAC))
for key, beacon in ipairs(beacons) do
    if (beacon.type == BEACON_TYPE_VOR) or (beacon.type == BEACON_TYPE_VORTAC) or (beacon.type == BEACON_TYPE_VOR_DME) then
        VOR_beacons[beacon.frequency] = beacon
    end
    if (beacon.type == BEACON_TYPE_TACAN) or (beacon.type == BEACON_TYPE_VORTAC) then
        TCN_beacons[beacon.channel] = beacon
    end
    if (beacon.type == BEACON_TYPE_ILS_LOCALIZER) or (beacon.type == BEACON_TYPE_ILS_GLIDESLOPE) or (beacon.type == BEACON_TYPE_ILS_FAR_HOMER) or (beacon.type == BEACON_TYPE_ILS_NEAR_HOMER) then
        ILS_beacons[beacon.frequency] = beacon
    end
end

-- ===================== End of Data Initialize ====================


-- ===================== Helper Functions ====================

---Function to get runway data for a specific airport
---@param roadnet string Path to the roadnet file of the airport
---@return table runways table containing runway data including length, name, and end coordinates
local function GetRunwayData(roadnet)
    -- unlike radios, this loads only the runway data for the specific roadnet
    local runwayList = Terrain.getRunwayList(roadnet)
    local runways = {}

    for i, v in pairs(runwayList) do
        runways[i] = {
            runwayLength = Nav_Utils.getDistanceBetweenPoints(v.edge1x, v.edge1y, v.edge2x, v.edge2y)  * M_TO_FT,
            name = v.edge1name .."-"..v.edge2name,
            runwayEnd1 = {x=v.edge1x, y=v.edge1y},
            runwayEnd2 = {x=v.edge2x, y=v.edge2y}
        }
    end

    -- Sort runways by length, largest first
    table.sort(runways, function(a, b)
        return a.runwayLength > b.runwayLength
    end)

    return runways
end


---Function to load and initialize airport data
---@return nil
local function loadAirports()
    for i, v in pairs(rawAirportData) do
        local airport = {
            name = v.display_name,
            ICAO = v.code,
            runways = GetRunwayData(v.roadnet),
            position = v.reference_point,
            -- positionLatLon = Nav_Utils.convertPosToLatLon(v.reference_point),
            positionLatLon = lo_to_geo_coords(v.reference_point.x, v.reference_point.y),
            radioid = v.radio,
            radios = Nav_Utils.getAirportRadios(v.radio, Radios),
            isCivilian = v.civilian,
            beacons = v.beacons,
        }
        FilteredAirportData[v.display_name] = airport
        FilteredAirportData[i] = airport
        FilteredAirportData[v.code] = airport

        if aircraftType == "T-38C" then
            -- this is boolean, however for my use I am converting to "CIV" or "MIL" or "BOTH"
            FilteredAirportData[v.display_name].isCivilian = Nav_Utils.getCivilianStatus(v.civilian)
        end
    end
end

---Helper function to load additional data for airports if available
---@return table additionalData A table containing additional data for airports, or an empty table if not found
local function loadAdditionalData()
    local additionalDataPath = LockOn_Options.script_path .. "NavDataPluginExtra/"..theatre.."/"..theatre..".lua"
    local additionalData = {}
    local f = loadfile(additionalDataPath)
    if f then
        local AirportData = f()
        additionalData = AirportData
    else
        print_message_to_user("NavDataPlugin: No additional data file found for theatre: " .. theatre)
    end
    return additionalData
end

---Function to supplement airport data with additional information from external files.
---This function merges additional data into the FilteredAirportData table
---@return nil
local function supplementAirportData()
    local additionalData = loadAdditionalData()
    for airportName, data in pairs(additionalData) do
        if FilteredAirportData[airportName] then
            Nav_Utils.deepMerge(FilteredAirportData[airportName], data)
        else
            print_message_to_user("NavDataPlugin: Airport " .. airportName .. " not found in FilteredAirportData table")
        end
    end
end

---Function to load radio frequencies for all airports (due to how DCS works, this loads all frequencies for all airports).
---To get a specific airport's frequencies, use the `radios` field in the FilteredAirportData table, in the NavUtils.getAirportRadios function.
---@return nil
local function loadRadios()
    -- this loads every radio frequency for every airport even for a specific roadnet
    local _, firstAirport = next(rawAirportData)
    if not firstAirport or not firstAirport.roadnet then
        print_message_to_user("NavDataPlugin: No valid airport data or roadnet found for radios")
        return
    end
    local radioList = Terrain.getRadio(firstAirport.roadnet)
    if not radioList then
        print_message_to_user("NavDataPlugin: No radio list found for airport roadnet")
        return
    end
    for i, v in pairs(radioList) do
        -- Initialize the radio entry in the Radios table
        Radios[v.radioId] = {
            radioId = v.radioId,
            uniform = nil,
            victor = nil,
        }

        -- Check if the frequency data exists
        if v.frequency then
            for _, freqTable in pairs(v.frequency) do
                -- Convert the frequency to the desired format
                if freqTable[2] then
                    local freq = freqTable[2] / 1000000
                    -- Assign to the correct category based on the frequency value
                    if freq >= 225.0 then
                        Radios[v.radioId].uniform = freq
                    elseif freq >= 118.0 and freq < 225.0 then
                        Radios[v.radioId].victor = freq -- TODO test and fix for other maps
                    end
                end
            end
        else
            print_message_to_user("NavDataPlugin: No frequency data available for radioId: ".. v.radioId)
        end
    end
end


-- ============ Available functions ============

---Function to sort airports by distance from a given position
---@param ownPos table A table containing the player's position in the format {x, y} in meters
---@return table sortedAirportList A sorted list of airports with additional distance and bearing information
function sortAirportsByDistance(ownPos)
    local sortedAirportList = {}

    for i, v in pairs(FilteredAirportData) do
        v.distanceToPlayerNM = Nav_Utils.getDistanceBetweenPoints(ownPos[1], ownPos[2], v.position.x, v.position.y) * M_TO_NM
        v.bearingToPlayer = Nav_Utils.getBearing(ownPos[1], ownPos[2], v.position.x, v.position.y)

        table.insert(sortedAirportList, v)
    end

    -- Sort the list by distance
    table.sort(sortedAirportList, function(a, b)
        return a.distanceToPlayerNM < b.distanceToPlayerNM
    end)
    return sortedAirportList
end


---Function to get the list of ILS beacons
---@return table ILS_beacons A table containing all ILS beacons with their relevant data
function Get_ILS_Beacons()
    return ILS_beacons
end


---Function to get the list of TCN beacons
---@return table TCN_beacons A table containing all TCN beacons with their relevant data
function Get_TCN_Beacons()
    return TCN_beacons
end


---Function to get the list of VOR beacons
---@return table VOR_beacons A table containing all VOR beacons with their relevant data
function Get_VOR_Beacons()
    return VOR_beacons
end


---Function to get the list of all available airports
---@return table FilteredAirportData A table containing all available airports with their relevant data
function GetAirports()
    return FilteredAirportData
end

---A function that gets you common data regarding your ownship to a beacon
---@param ownship table A table of your ownship values. Units are in DCS x,y,z. position [1] [2] [3] = x, y, z. .heading in degrees
---@param beacon table A table provided by the beacon. should not be nor needed to be modified from the beacon's return.
---@return number distance Return the distance in DCS units to the beacon
---@return number radial Return the planes current radial in relation to the beacon
---@return string toFrom returns either "TO" or "FROM" depending on the planes relative bearing to the beacon
function GetDistanceRadialAndToFromVOR(ownship, beacon)
    local dx = ownship.x - beacon.position[1]
    local dz = ownship.z - beacon.position[3]
    local raw = math.deg(math.atan2(dx, dz))
    local distance = math.sqrt(math.pow(dx, 2) + math.pow(dz, 2))

    local radial = raw % 360

    local courseToBeacon = (radial + 100) % 360

    local function angleDiff(a, b)
        local diff = (a - b + 180) % 360 - 180
        return diff
    end

    local dTo = math.abs(angleDiff(ownship.heading, courseToBeacon))
    local dFrom = math.abs(angleDiff(ownship.heading, radial))

    local toFrom
    if (dTo < dFrom) then
        toFrom = "TO"
    else
        toFrom = "FROM"
    end

    return distance, radial, toFrom
end

---A function to get you Distance and Radial from a specified beacon, this includes the line of sight check, and is corrected for Magnetic Variation
---@param ownship table A table of ownship values: x, y, z, magHeading, heading. (x, y, z in DCS units/meters, headings in degrees)
---@param beacon table Beacon value from the beacon table, do not modify the table, simply take it out of the beacons table and parse it
---@return number|nil range range in meters to beacon from player
---@return number|nil bearing bearing in degrees from player to beacon
function GetDistanceRadialToBeacon(ownship, beacon)
    local range
    local bearing
    local magVariation = ownship.magHeading - (360 - ownship.heading)
    
    if Terrain.isVisible(ownship.x, ownship.y, ownship.z, beacon.position[1], beacon.position[2] + 15, beacon.position[3]) then -- L.O.S Check
        bearing = (math.deg(math.atan2((beacon.position[3]-ownship.z),(beacon.position[1]- ownship.x)))+ magVariation) % 360
        if beacon.type ~= BEACON_TYPE_VOR then -- VOR has no range information
            range = math.sqrt( (beacon.position[1] - ownship.x)^2 + (beacon.position[2] - ownship.y)^2 + (beacon.position[3] - ownship.z)^2)
        end
    end
    return range, bearing
end

-- ============ End of Available functions ============


-- ============ Load data at mission start ============
loadRadios()            -- needs to be called before loadAirports() to ensure radios are available
loadAirports()
supplementAirportData() -- these will only load if NavDataPluginExtra exists

-- for some reason theres changes here or something