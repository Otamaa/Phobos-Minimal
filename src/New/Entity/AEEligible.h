#pragma once

#include <algorithm>

// Shared allow/disallow eligibility check for AE data entries.
//
// Logic:
//   1. null `who` → always eligible (no filter target)
//   2. If allow list is non-empty, `who` must appear in it
//   3. If `who` appears in disallow list, reject
//
// Container-generic: works with VectorSet, ValueableVector, std::vector, etc.

// By-reference overload (owned containers)
template <typename T, typename ContainerA, typename ContainerD>
COMPILETIMEEVAL bool AEIsEligible(
	T* who,
	const ContainerA& allow,
	const ContainerD& disallow)
{
	if (!who)
		return true;

	if (!allow.empty()
		&& std::find(allow.begin(), allow.end(), who) == allow.end())
		return false;

	if (!disallow.empty()
		&& std::find(disallow.begin(), disallow.end(), who) != disallow.end())
		return false;

	return true;
}

// By-pointer overload (zero-copy AE data entries)
template <typename T, typename ContainerA, typename ContainerD>
COMPILETIMEEVAL bool AEIsEligible(
	T* who,
	const ContainerA* allow,
	const ContainerD* disallow)
{
	if (!who)
		return true;

	if (allow && !allow->empty()
		&& std::find(allow->begin(), allow->end(), who) == allow->end())
		return false;

	if (disallow && !disallow->empty()
		&& std::find(disallow->begin(), disallow->end(), who) != disallow->end())
		return false;

	return true;
}
