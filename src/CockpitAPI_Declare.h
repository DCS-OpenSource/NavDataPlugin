#pragma once
#include "ccParametersAPI.h"
#include <iostream>


class CockpitAPI
{
public:
	cockpit_param_api ed_param_api;

	CockpitAPI()
	{
		ed_param_api = ed_get_cockpit_param_api();
	}

	void* getParamHandle(const char* name)
	{
		return ed_param_api.pfn_ed_cockpit_get_parameter_handle(name);
	}

	void setParamNumber(void* handle, double value)
	{
		ed_param_api.pfn_ed_cockpit_update_parameter_with_number(handle, value);
	}

	void setParamString(void* handle, const char* string)
	{
		ed_param_api.pfn_ed_cockpit_update_parameter_with_string(handle, string);
	}

	double getParamNumber(void* handle)
	{
		double res = 0;
		ed_param_api.pfn_ed_cockpit_parameter_value_to_number(handle, res, false);
		return res;
	}

	const char* getParamString(void* handle, unsigned buffer_size)
	{
		char buffer[256];
		ed_param_api.pfn_ed_cockpit_parameter_value_to_string(handle, buffer, 256);
		return &buffer[0];
	}

	int compareParams(void* handle1, void* handle2)
	{
		return ed_param_api.pfn_ed_cockpit_compare_parameters(handle1, handle2);
	}

	void dispatchActionToDevice(unsigned __int8 device, int command, float value)
	{
		return ed_param_api.pfn_ed_cockpit_dispatch_action_to_device(device, command, value);
	}

	/// <summary>
	/// Untested
	/// </summary>
	/// <param name="arg"></param>
	/// <param name="value"></param>
	void setDrawArgument(int arg, float value)
	{
		ed_param_api.pfn_ed_cockpit_set_draw_argument(arg, value);
	}

	/// <summary>
	/// Seems to be nonop currently - always sets 0
	/// </summary>
	/// <param name="arg"></param>
	/// <param name="value"></param>
	void setExternalDrawArgument(int arg, float value)
	{
		ed_param_api.pfn_ed_cockpit_set_external_draw_argument(arg, value);
	}

	/// <summary>
	/// Seems to be nonop currently - always returns 0
	/// </summary>
	/// <param name="arg"></param>
	/// <returns></returns>
	float getExternalDrawArgument(int arg)
	{
		return ed_param_api.pfn_ed_cockpit_get_external_draw_argument(arg);
	}

	/// <summary>
	/// Seems to be nonop currently - access violation
	/// </summary>
	/// <returns></returns>
	bool isNetMaster()
	{
		return ed_param_api.pfn_cockpit__avsyncable__is_net_master();
	}

	/// <summary>
	/// Seems to be nonop currently - access violation
	/// </summary>
	/// <returns></returns>
	bool isFmMaster() // seems to be non op currently
	{
		return ed_param_api.pfn_cockpit__avsyncable__is_fm_master();
	}

	bool isMultiplayer()
	{
		return ed_param_api.pfn_cockpit__avsyncable__is_multiplayer();
	}

	/// <summary>
	/// returns the slot number as listed by plane.lua
	/// </summary>
	/// <returns></returns>
	int numSlot()
	{
		return ed_param_api.pfn_cockpit__avsyncable__num_slot();
	}
};