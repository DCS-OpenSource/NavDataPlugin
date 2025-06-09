#pragma once
#include <windows.h>
#include <errors.h>
#include <iostream>

//params handling
typedef void* (*PFN_ED_COCKPIT_GET_PARAMETER_HANDLE)  (const char* name);
typedef void   (*PFN_ED_COCKPIT_UPDATE_PARAMETER_WITH_STRING)  (void* handle, const char* string_value);
typedef void   (*PFN_ED_COCKPIT_UPDATE_PARAMETER_WITH_NUMBER)  (void* handle, double   number_value);
typedef bool   (*PFN_ED_COCKPIT_PARAMETER_VALUE_TO_NUMBER)  (const void* handle, double& res, bool interpolated);
typedef bool   (*PFN_ED_COCKPIT_PARAMETER_VALUE_TO_STRING)  (const void* handle, char* buffer, unsigned buffer_size);
typedef int    (*PFN_ED_COCKPIT_COMPARE_PARAMETERS)  (void* handle_1, void* handle_2);
typedef void   (*PFN_ED_COCKPIT_DISPATCH_ACTION_TO_DEVICE)  (unsigned __int8 device, int action, float value);

typedef void   (*PFN_ED_COCKPIT_SET_DRAW_ARGUMENT)  (int arg, float value);
typedef void   (*PFN_ED_COCKPIT_SET_EXTERNAL_DRAW_ARGUMENT)  (int arg, float value);
typedef float  (*PFN_ED_COCKPIT_GET_EXTERNAL_DRAW_ARGUMENT)  (int arg);

typedef bool(__cdecl* PFN_COCKPIT__AVSYNCABLE__IS_FM_MASTER)(void); // Use __cdecl
typedef bool(__cdecl* PFN_COCKPIT__AVSYNCABLE__IS_MULTIPLAYER)(void); // Use __cdecl
typedef bool(__cdecl* PFN_COCKPIT__AVSYNCABLE__IS_NET_MASTER)(void); // Use __cdecl
typedef unsigned int(__cdecl* PFN_COCKPIT__AVSYNCABLE__NUM_SLOT)(void); // Use __cdecl

struct cockpit_param_api
{
	PFN_ED_COCKPIT_GET_PARAMETER_HANDLE				 pfn_ed_cockpit_get_parameter_handle;
	PFN_ED_COCKPIT_UPDATE_PARAMETER_WITH_STRING		 pfn_ed_cockpit_update_parameter_with_string;
	PFN_ED_COCKPIT_UPDATE_PARAMETER_WITH_NUMBER		 pfn_ed_cockpit_update_parameter_with_number;
	PFN_ED_COCKPIT_PARAMETER_VALUE_TO_NUMBER		 pfn_ed_cockpit_parameter_value_to_number;
	PFN_ED_COCKPIT_PARAMETER_VALUE_TO_STRING		 pfn_ed_cockpit_parameter_value_to_string;
	PFN_ED_COCKPIT_COMPARE_PARAMETERS				 pfn_ed_cockpit_compare_parameters;
	PFN_ED_COCKPIT_DISPATCH_ACTION_TO_DEVICE		 pfn_ed_cockpit_dispatch_action_to_device;

	PFN_ED_COCKPIT_SET_DRAW_ARGUMENT				 pfn_ed_cockpit_set_draw_argument;
	PFN_ED_COCKPIT_SET_EXTERNAL_DRAW_ARGUMENT		 pfn_ed_cockpit_set_external_draw_argument;
	PFN_ED_COCKPIT_GET_EXTERNAL_DRAW_ARGUMENT		 pfn_ed_cockpit_get_external_draw_argument;

	PFN_COCKPIT__AVSYNCABLE__IS_FM_MASTER			 pfn_cockpit__avsyncable__is_fm_master;
	PFN_COCKPIT__AVSYNCABLE__IS_MULTIPLAYER			 pfn_cockpit__avsyncable__is_multiplayer;
	PFN_COCKPIT__AVSYNCABLE__IS_NET_MASTER			 pfn_cockpit__avsyncable__is_net_master;
	PFN_COCKPIT__AVSYNCABLE__NUM_SLOT				 pfn_cockpit__avsyncable__num_slot;
};




inline cockpit_param_api  ed_get_cockpit_param_api()
{
	cockpit_param_api ret;

	HMODULE cockpit_dll = GetModuleHandleA("CockpitBase.dll"); //assume that we work inside same process
	if (!cockpit_dll)
	{
		std::cerr << "Failed to load CockpitBase.dll" << std::endl;
		throw std::runtime_error("Failed to load CockpitBase.dll from GetModuleHandleA");
	}

	ret.pfn_ed_cockpit_get_parameter_handle = (PFN_ED_COCKPIT_GET_PARAMETER_HANDLE)GetProcAddress(cockpit_dll, "ed_cockpit_get_parameter_handle");
	ret.pfn_ed_cockpit_update_parameter_with_number = (PFN_ED_COCKPIT_UPDATE_PARAMETER_WITH_NUMBER)GetProcAddress(cockpit_dll, "ed_cockpit_update_parameter_with_number");
	ret.pfn_ed_cockpit_update_parameter_with_string = (PFN_ED_COCKPIT_UPDATE_PARAMETER_WITH_STRING)GetProcAddress(cockpit_dll, "ed_cockpit_update_parameter_with_string");
	ret.pfn_ed_cockpit_parameter_value_to_number = (PFN_ED_COCKPIT_PARAMETER_VALUE_TO_NUMBER)GetProcAddress(cockpit_dll, "ed_cockpit_parameter_value_to_number");
	ret.pfn_ed_cockpit_parameter_value_to_string = (PFN_ED_COCKPIT_PARAMETER_VALUE_TO_STRING)GetProcAddress(cockpit_dll, "ed_cockpit_parameter_value_to_string");
	ret.pfn_ed_cockpit_compare_parameters = (PFN_ED_COCKPIT_COMPARE_PARAMETERS)GetProcAddress(cockpit_dll, "ed_cockpit_compare_parameters");
	ret.pfn_ed_cockpit_dispatch_action_to_device = (PFN_ED_COCKPIT_DISPATCH_ACTION_TO_DEVICE)GetProcAddress(cockpit_dll, "ed_cockpit_dispatch_action_to_device");

	ret.pfn_ed_cockpit_set_draw_argument = (PFN_ED_COCKPIT_SET_DRAW_ARGUMENT)GetProcAddress(cockpit_dll, "ed_cockpit_set_draw_argument");
	ret.pfn_ed_cockpit_set_external_draw_argument = (PFN_ED_COCKPIT_SET_EXTERNAL_DRAW_ARGUMENT)GetProcAddress(cockpit_dll, "ed_cockpit_set_external_draw_argument");
	ret.pfn_ed_cockpit_get_external_draw_argument = (PFN_ED_COCKPIT_GET_EXTERNAL_DRAW_ARGUMENT)GetProcAddress(cockpit_dll, "ed_cockpit_get_external_draw_argument");

	ret.pfn_cockpit__avsyncable__is_fm_master = (PFN_COCKPIT__AVSYNCABLE__IS_FM_MASTER)GetProcAddress(cockpit_dll, "?is_fm_master@avSyncable@cockpit@@SA_NXZ");
	ret.pfn_cockpit__avsyncable__is_multiplayer = (PFN_COCKPIT__AVSYNCABLE__IS_MULTIPLAYER)GetProcAddress(cockpit_dll, "?is_multiplayer@avSyncable@cockpit@@SA_NXZ");
	ret.pfn_cockpit__avsyncable__is_net_master = (PFN_COCKPIT__AVSYNCABLE__IS_NET_MASTER)GetProcAddress(cockpit_dll, "?is_net_master@avSyncable@cockpit@@SA_NXZ"); // net_master not working?
	ret.pfn_cockpit__avsyncable__num_slot = (PFN_COCKPIT__AVSYNCABLE__NUM_SLOT)GetProcAddress(cockpit_dll, "?num_slot@avSyncable@cockpit@@SAIXZ");
	return ret;
}