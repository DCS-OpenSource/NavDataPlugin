-- ========================== Caffeine Simulations Nav System ==========================
-- Developed by Hayds_93, 2024 for Caffeine Simulations and the T-38C
-- Free to use and modify, keeping this comment here, see the README for more info
-- =====================================================================================

dofile(LockOn_Options.script_path.."Systems/Nav/Nav_Utils.lua")

local Terrain = require('terrain') -- DCS terrain module
local rawAirportData = get_terrain_related_data("Airdromes")
local beaconsFile = get_terrain_related_data("beaconsFile")

if beaconsFile then
    local f = loadfile(beaconsFile)
    if f then
        f()
    end
end

local ILS_beacons = {}
local TCN_beacons = {}
local VOR_beacons = {}

local Runways     = {}
local FilteredAirportData = {} -- Data filtered for relevant info and has extra info added from /additionalData
local Radios      = {}

-- ILS_idx = 0
-- TCN_idx = 0
-- VOR_idx = 0

-- local function Load_Beacons()
--     for i, v in pairs(beacons) do -- this is erroring
--         if v.type == BEACON_TYPE_VOR then
--             VOR_beacons[#VOR_beacons+1] = {
--                 type         = v.type,
--                 beaconId     = v.beaconId,
--                 posGeo       = v.positionGeo,
--                 display_name = v.display_name,
--                 channel      = v.channel,
--                 pos          = v.position,
--                 direction    = v.direction,
--                 callsign     = v.callsign,
--                 freq         = v.freq
--             }
--         elseif v.type == BEACON_TYPE_TACAN then
--             TCN_beacons[#TCN_beacons+1] = {
--                 type         = v.type,
--                 beaconId     = v.beaconId,
--                 posGeo       = v.positionGeo,
--                 display_name = v.display_name,
--                 channel      = v.channel,
--                 pos          = v.position,
--                 direction    = v.direction,
--                 callsign     = v.callsign,
--                 freq         = v.freq
--             }
--         elseif v.type == BEACON_TYPE_ILS_LOCALIZER then
--             ILS_beacons[#ILS_beacons+1] = {
--                 type         = v.type,
--                 beaconId     = v.beaconId,
--                 posGeo       = v.positionGeo,
--                 display_name = v.display_name,
--                 channel      = v.channel,
--                 pos          = v.position,
--                 direction    = v.direction,
--                 callsign     = v.callsign,
--                 freq         = v.freq
--             }
--         end
--     end
-- end

local function GetRunwayData(airport)
    -- unlike radios, this loads only the runway data for the specific roadnet
    local runwayList = Terrain.getRunwayList(airport)
    local runways = {}

    for i, v in pairs(runwayList) do
        runways[i] = {
            runwayLength = calculateRunwayLength(v.edge1x, v.edge1y, v.edge2x, v.edge2y),
            name = v.edge1name .."-"..v.edge2name
        }
    end

    -- Sort runways by length, largest first
    table.sort(runways, function(a, b)
        return a.runwayLength > b.runwayLength
    end)

    return runways
end

local function loadAirports()
-- Load all airport data at mission start
    for i, v in pairs(rawAirportData) do
        FilteredAirportData[v.display_name] = {
            name = v.display_name,
            ICAO = v.code,
            runways = GetRunwayData(v.roadnet),
            position = getAirportLocation(v.reference_point),
            radioid = v.radio,
            radios = getAirportRadios(v.radio),
            -- isCivilian = v.civilian, -- this is boolean, however for my use I am converting to "CIV" or "MIL" or "BOTH"
            isCivilian = getCivilianStatus(v.civilian), -- Comment this out and use above line if you want boolean
            beacons = v.beacons,
        }

    end
end

local function loadRadios()
    -- this loads every radio frequency for every airport even for a specific roadnet
    local radioList = Terrain.getRadio(rawAirportData[1].roadnet)

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
                    else
                        Radios[v.radioId].victor = freq -- TODO test and fix for other maps
                    end
                end
            end
        else
            print_message_to_user("Warning: No frequency data available for radioId: ".. v.radioId)
        end
    end
end

function sortAirportsByDistance(ownPos)
    local sortedAirportList = {}

    for i, v in pairs(FilteredAirportData) do
        -- local distanceToPlayerFeet = getDistanceInFeet(ownPos[1], ownPos[2], v.position.lat, v.position.lon)
        local distanceToPlayerNM = haversine(ownPos[1], ownPos[2], v.position.lat, v.position.lon)
        local bearingToPlayer = getBearing(ownPos[1], ownPos[2], v.position.lat, v.position.lon)

        -- v.distanceToPlayerFeet = distanceToPlayerFeet
        v.distanceToPlayerNM = distanceToPlayerNM
        v.bearingToPlayer = bearingToPlayer

        table.insert(sortedAirportList, v)
    end

    table.sort(sortedAirportList, function(a, b)
        return a.distanceToPlayerNM < b.distanceToPlayerNM
    end)
    return sortedAirportList
end


function Get_ILS_beacons()
    return ILS_beacons
end

function Get_TCN_beacons()
    return TCN_beacons
end

function Get_VOR_beacons()
    return VOR_beacons
end

function getAirports()
    return FilteredAirportData
end

function debug_TCN_beacons()
    for i, v in pairs(TCN_beacons) do
        print_message_to_user("TCN: " .. v.display_name .. v.callsign)
    end
end

function debug_ILS_beacons()
    for i, v in pairs(ILS_beacons) do
        print_message_to_user("ILS: " .. v.display_name)
    end
end

function debug_VOR_beacons()
    for i, v in pairs(VOR_beacons) do
        print_message_to_user("VOR: " .. v.display_name)
    end
end

function debug_Radios()
    printTableContents(Radios)
end

function debugFilteredAirports()
    printTableContents(FilteredAirportData)
end

loadRadios()
loadAirports()
