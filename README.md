# Caffeine Simulations Nav Info System
A small drop in "API" to dynamically pull data from DCS for airport information

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