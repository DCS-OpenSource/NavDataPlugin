-- This Lua file contains an array of data to supplement the existing data pulled from DCS

local Airports = {
    ["Nellis"] = {
        name = "Nellis Air Force Base",
        isCivilian = "MIL",
        ICAO = "KLSV",
    }, 
    ["North Las Vegas"] = {
        name = "North Las Vegas Airport",
        isCivilian = "CIV",
    },
    ["McCarran International"] = { -- It's Harry Reid, however, DCS labels it McCarran
        name = "Harry Reid International Airport",
        isCivilian = "CIV",
    },
    ["Henderson Executive"] = {
        name = "Henderson Executive Airport",
        isCivilian = "CIV",
    },
    ["Boulder City"] = {
        name = "Boulder City Municipal Airport",
        isCivilian = "CIV",
    },
    ["Jean"] = {
        name = "Jean Airport",
        isCivilian = "CIV",
        ICAO = "0L7",
    },
    ["Creech"] = {
        name = "Creech Air Force Base",
        isCivilian = "MIL",
    },
    ["Tonopah Test Range"] = {
        name = "Tonopah Test Range Airport",
        isCivilian = "MIL",
    },
    ["Groom Lake"] = { -- Area 51
        name = "Homey Airport",
        isCivilian = "MIL",
    }
}

-- Function to retrieve the array
local function getAirportData()
    return Airports
end

-- Return the function for accessing the data
return {
    getAirportData = getAirportData
}
