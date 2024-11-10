-- This Lua file contains an array of data to supplement the existing data pulled from DCS

local Airports = {
    ["Al Minhad AFB"] = {   
        radios = {
            uniform = 250.100,
        },
    },
}

-- Function to retrieve the array
local function getAirportData()
    return Airports
end

-- Return the function for accessing the data
return {
    getAirportData = getAirportData
}
