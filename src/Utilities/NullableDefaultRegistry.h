#pragma once

#include <string>
#include <vector>

class NullableDefaultRegistry
{
public:
	using IsBoundFn = bool(*)(const void*);

	struct Entry
	{
		const void* Obj;
		IsBoundFn   IsBound;
		const char* Section;
		const char* Key;
		std::string ID;
	};

	static NullableDefaultRegistry& Instance()
	{
		static NullableDefaultRegistry inst;
		return inst;
	}

	// parse is single-threaded -> no lock. No-op once disabled.
	void Register(const void* obj, IsBoundFn fn);
	void Deregister(const void* obj);

	// called from ReadDefault so the log can name the offending tag
	void SetIdentity(const void* obj, const char* sec, const char* key, std::string_view id);

	// ONE call after parse: log every unbound member, fatal once if any,
	// then free the array and disable further registration.
	void ValidateAndFatal();

	// swap-to-empty actually releases capacity (clear() would not) + disables
	void Free();

	// re-arm to validate another parse pass
	void Reset();

private:
	std::vector<Entry> Entries;
	bool Enabled { true };
};
