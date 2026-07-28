#include "NullableDefaultRegistry.h"
#include <Utilities/Debug.h>

void NullableDefaultRegistry::Register(const void* obj, IsBoundFn fn)
{
	if (!this->Enabled)
		return;

	this->Entries.emplace_back(obj, fn, nullptr, nullptr, "");
}

void NullableDefaultRegistry::Deregister(const void* obj)
{
	if (this->Entries.empty())
		return;

	for (auto it = this->Entries.begin(); it != this->Entries.end(); ++it)
	{
		if (it->Obj == obj)
		{
			// swap-erase the single match -- never erase(find, end())
			*it = this->Entries.back();
			this->Entries.pop_back();
			return;
		}
	}
}

void NullableDefaultRegistry::SetIdentity(const void* obj, const char* sec, const char* key, std::string_view id)
{
	for (auto& e : this->Entries)
	{
		if (e.Obj == obj)
		{
			e.Section = sec;
			e.Key = key;
			e.ID = id;
			return;
		}
	}
}

void NullableDefaultRegistry::ValidateAndFatal()
{
	int unbound = 0;

	for (const auto& e : this->Entries)
	{
		if (e.IsBound && e.IsBound(e.Obj))
			continue;

		++unbound;
		Debug::Log("NullableDefault [T %s] never bound: [%s] %s\n", // VERIFY: logger
			e.ID,
			e.Section ? e.Section : "?",
			e.Key ? e.Key : "?");
	}

	if (unbound > 0)
		Debug::FatalErrorAndExit(                            // VERIFY: fatal
			"%d NullableDefault member(s) had no default bound.\n", unbound);

	this->Free();
}

void NullableDefaultRegistry::Free()
{
	std::vector<Entry>().swap(this->Entries);
	this->Enabled = false;
}

void NullableDefaultRegistry::Reset()
{
	this->Free();
	this->Enabled = true;
}