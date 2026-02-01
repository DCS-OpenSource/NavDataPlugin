-- MIT License
-- 
-- Copyright (c) 2025 OpenFlight Community
--
-- Permission is hereby granted, free of charge, to any person obtaining a copy
-- of this software and associated documentation files (the "Software"), to deal
-- in the Software without restriction, including without limitation the rights
-- to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
-- copies of the Software, and to permit persons to whom the Software is
-- furnished to do so, subject to the following conditions:
-- 
-- The above copyright notice and this permission notice shall be included in all
-- copies or substantial portions of the Software.
-- 

-- THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
-- IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
-- FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
-- AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
-- LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
-- OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
-- SOFTWARE.

package.path = package.path..";"..LockOn_Options.script_path.."NavDataPlugin/MorseCodePlayer/?.lua"
require("Morse")


local MorsePlayer = {}
MorsePlayer.__index = MorsePlayer


--- Create the MorsePlayer
--- @param update_rate number same number parsed to make_default_activity()
--- @param WPM number|nil Morse Words Per Minute, default 18
function MorsePlayer:new(update_rate, WPM)
    local self = setmetatable({}, MorsePlayer)

    self.update_rate = update_rate
    self.WPM = WPM or 18

    -- timing
    self.dit_time = 1.2 / self.WPM

    -- sound handles
    self.dit = nil
    self.dash = nil

    -- playback state
    self.sequence = {}
    self.sequence_index = 1
    self.timer = 0
    self.active = false
    self.current_string = nil

    -- loop
    self.loop = false
    self.loop_spacing_units = 0

    return self
end


--- Load Dit and Dash sounders
--- Must be called from post_initialize
function MorsePlayer:loadSounds()
    local sndhost = create_sound_host("COCKPIT_ARMS", "HEADPHONES", 0, 0, 0)
    self.dit  = sndhost:create_sound("Aircrafts/Morse/dit")
    self.dash = sndhost:create_sound("Aircrafts/Morse/dash")
end


--- Play the parsed string as a morse code signal
--- @param MorseString string
--- @param isLoop boolean|nil
--- @param loopSpacing number|nil Extra spacing (seconds) before repeat
function MorsePlayer:playSignal(MorseString, isLoop, loopSpacing)
    if not MorseString or MorseString == "" then
        return
    end

    MorseString = MorseString:upper()

    -- If already playing the same signal, do nothing
    if self.active and self.current_string == MorseString then
        return
    end

    -- Store the currently playing string
    self.current_string = MorseString

    -- Reset state
    self.sequence = {}
    self.sequence_index = 1
    self.timer = loopSpacing or 0


    self.loop = isLoop or false
    self.loop_spacing_time = loopSpacing or (durations.inter_word * self.dit_time * 2)

    for char in MorseString:gmatch(".") do
        if char == " " then
            table.insert(self.sequence, {
                type = "gap",
                units = durations.inter_word
            })
        else
            local morse = MORSE[char]
            if morse then
                for symbol in morse:gmatch(".") do
                    if symbol == "." then
                        table.insert(self.sequence, { type = "dit", units = durations.dit })
                    elseif symbol == "-" then
                        table.insert(self.sequence, { type = "dash", units = durations.dash })
                    end

                    table.insert(self.sequence, { type = "gap", units = durations.intra_symbol })
                end

                -- replace last intra-symbol gap with inter-letter gap
                self.sequence[#self.sequence].units = durations.inter_letter
            end
        end
    end

    self.active = (#self.sequence > 0)
end



--- Function to stop the signal from playing
function MorsePlayer:stopSignal()
    self.sequence = {}
    self.sequence_index = 1
    self.timer = 0
    self.active = false
    self.current_string = nil
end



--- MorseCodePlayer Update function
function MorsePlayer:update()
    if not self.active then return end

    local event = self.sequence[self.sequence_index]
    if not event then
        if self.loop then
            self.sequence_index = 1
            self.timer = self.loop_spacing_time
        else
            self.active = false
        end
        return
    end


    if self.timer <= 0 then
        if event.type == "dit" and self.dit then
            self.dit:play_once()
        elseif event.type == "dash" and self.dash then
            self.dash:play_once()
        end

        self.timer = event.units * self.dit_time
        self.sequence_index = self.sequence_index + 1
    else
        self.timer = self.timer - self.update_rate
    end
end


--- Set Volume of the Dits and Dashes
--- @param value number 0-1 value.
function MorsePlayer:adjustVolume(value)
    value = value / 2 -- Volume is wierd, Top end seems high so I've scaled it
    if self.dit then self.dit:update(nil, value, nil) end
    if self.dash then self.dash:update(nil, value, nil) end
end


-- Debug helpers

function MorsePlayer:playDit()
    if self.dit then self.dit:play_once() end
end

function MorsePlayer:playDash()
    if self.dash then self.dash:play_once() end
end

function MorsePlayer:getSequenceAsMorse()
    local out = {}

    for _, e in ipairs(self.sequence) do
        if e.type == "dit" then
            table.insert(out, ".")
        elseif e.type == "dash" then
            table.insert(out, "-")
        elseif e.type == "gap" then
            table.insert(out, " ")
        end
    end

    return table.concat(out)
end


return MorsePlayer
