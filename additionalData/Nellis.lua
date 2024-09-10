-- This Lua file contains an array of data to supplement the existng data pulled from DCS

local NellisAirports = {
    ["Nellis"] = {
        updatedName = "Nellis AFB",
        updatedType = "MIL", -- CIV, MIL, BOTH
    }, 
    ["North Las Vegas"] = {
        updatedName = "North Las Vegas Airport",
        updatedType = "CIV", -- CIV, MIL, BOTH
    },
    ["McCarran International"] = { -- its harry reid, however comma, dcs labels it mccarran
        updatedName = "McCarran International Airport",
        updatedType = "CIV", -- CIV, MIL, BOTH
    },
    ["Henderson Executive"] = {
        updatedName = "Henderson Executive Airport",
        updatedType = "CIV", -- CIV, MIL, BOTH
    },
    ["Boulder City"] = {
        updatedName = "Boulder City Municipal Airport",
        updatedType = "CIV", -- CIV, MIL, BOTH
    },
    ["Jean"] = {
        updatedName = "Jean Airport",
        updatedType = "CIV", -- CIV, MIL, BOTH
    },
    ["Creech"] = {
        updatedName = "Creech AFB",
        updatedType = "MIL", -- CIV, MIL, BOTH
    }
}


-- Function to retrieve the array
local function getDataArray()
    return Nellis
end

-- Return the function for accessing the data
return {
    getDataArray = getDataArray
}


-- example usage

-- local NellisData = require("Nellis")

-- local data = NellisData.getDataArray()

-- for i, item in pairs(data) do
--     print(item)
-- end