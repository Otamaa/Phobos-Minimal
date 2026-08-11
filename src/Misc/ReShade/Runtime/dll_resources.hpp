/*
 * Copyright (C) 2014 Patrick Mours
 * SPDX-License-Identifier: BSD-3-Clause OR MIT
 */

#pragma once

#include <res/Reshade.Resource.h>
#include <string>
#include <vector>

namespace reshade::resources
{
	struct data_resource
	{
		size_t data_size;
		const void *data;
	};

	/// <summary>
	/// Loads the raw data associated with the specified <paramref name="id"/> from the DLL resources.
	/// </summary>
	/// <param name="id">Resource identifier of the raw data resource.</param>
	data_resource load_data_resource(unsigned short id);
}
