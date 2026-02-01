MORSE = {
    -- Letters
    A = ".-",
    B = "-...",
    C = "-.-.",
    D = "-..",
    E = ".",
    F = "..-.",
    G = "--.",
    H = "....",
    I = "..",
    J = ".---",
    K = "-.-",
    L = ".-..",
    M = "--",
    N = "-.",
    O = "---",
    P = ".--.",
    Q = "--.-",
    R = ".-.",
    S = "...",
    T = "-",
    U = "..-",
    V = "...-",
    W = ".--",
    X = "-..-",
    Y = "-.--",
    Z = "--..",

    -- Numbers
    ["0"] = "-----",
    ["1"] = ".----",
    ["2"] = "..---",
    ["3"] = "...--",
    ["4"] = "....-",
    ["5"] = ".....",
    ["6"] = "-....",
    ["7"] = "--...",
    ["8"] = "---..",
    ["9"] = "----.",
}

durations = {
    dit  = 1,           -- dot length
    dash = 3,           -- dash length
    intra_symbol = 1,   -- gap between dit/dash within a character
    inter_letter = 3,   -- gap between letters
    inter_word   = 7,   -- gap between words
}

-- local wpm = 18
-- local dit_time = 1.2 / wpm
-- local t = durations.dit * dit_time

