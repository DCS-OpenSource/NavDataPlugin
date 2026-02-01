# MorseCodePlayer

MorseCodePlayer is a lightweight Lua module for playing Morse code audio in DCS World style device scripts. It converts text strings into properly timed Morse code using pre generated dit and dash sounds, supports looping beacon style playback, and is designed to run inside an update loop.

This module is suitable for navigation beacons, ADF or NDB simulation, cockpit audio cues, and general Morse signaling.

## Features

- Converts text strings into Morse code timing sequences
- Supports letters A to Z and digits 0 to 9
- Optional looping with configurable spacing in seconds
- Volume adjustment
- Debug helpers for testing


## Usage

### Creating the player

```lua
local MorsePlayer = require("NavDataPlugin.MorseCodePlayer.MorsePlayer")

local morse = MorsePlayer:new(update_rate, 18)
```
update_rate must match the value passed to make_default_activity.
WPM defaults to 18 if not provided.

### Loading sounds

Sound creation must happen in post_initialize.

```lua
function post_initialize()
    morse:loadSounds()
end
```

Copy the Sounds folder from NavDataPlugin to the root of your mod folder, this should configure the sounders as DCS expects
The module expects the following sound paths:

Aircrafts/Morse/dit
Aircrafts/Morse/dash

### Playing a signal

```lua
morse:playSignal("NZAA", true, 5.0)
```

MorseString is the text to play.
isLoop controls whether the signal repeats.
loopSpacing is the silence time in seconds before the first signal and between repeats.

Calling playSignal repeatedly with the same string while active does nothing.
Calling it with a new string restarts playback.

### Update loop

```lua
function update()
    morse:update()
end
```

### Stopping playback

```lua
morse:stopSignal()
```

### Adjusting volume

```lua
morse:adjustVolume(0.5)
```

Volume range is 0 to 1.

## Timing Model

Timing follows standard Morse code rules.

Dit is one unit.
Dash is three units.
Intra symbol gap is one unit.
Inter letter gap is three units.
Inter word gap is seven units.

Unit time is calculated as 1.2 divided by WPM.

Loop spacing is specified directly in seconds.

## License

MIT License

Copyright (c) 2025 OpenFlight Community
