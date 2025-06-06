local NavData_dll_path = LockOn_Options.script_path.."NavDataPlugin/"

package.cpath = package.cpath .. ";" .. NavData_dll_path .. "/?.dll"
success, result = pcall(require, 'NavDataPluginExtension')
if not success then
    print_message_to_user("NavDataPluginExtension: Failed to load NavDataPluginExtension. Please check the path: " .. NavData_dll_path)
end
