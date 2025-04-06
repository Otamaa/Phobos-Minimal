#pragma once

#include "UUID.h"

namespace Phobos
{
	struct IDComponent
	{
		UUID ID;

		IDComponent() = default;
		IDComponent(const IDComponent&) = default;
	};
}