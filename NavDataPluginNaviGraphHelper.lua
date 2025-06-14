local NavData_dll_path = LockOn_Options.script_path.."NavDataPlugin/"

local function load_dll()
    package.cpath = package.cpath .. ";" .. NavData_dll_path .. "/?.dll"
    success, result = pcall(require, 'NavDataPluginNaviGraph')
    if not success then
        print_message_to_user("NavDataPluginNaviGraph: Failed to load NavDataPluginNaviGraph. Please check the path: " .. NavData_dll_path)
    end
    return result
end

return load_dll()