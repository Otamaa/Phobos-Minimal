#pragma once

#include <Base/Macros.h>
#include <Helpers/CompileTime.h>

struct TheirSync
{
	static constexpr reference<TheirSync, 0xAFA358, 8> const Array {};

	int frame;
	int __send;
	int __recv;
	int timing_C;
	int __router_resp;
	int timing_14;
};
static_assert(sizeof(TheirSync) == 0x18);