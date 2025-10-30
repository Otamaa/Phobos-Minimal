#pragma once

#include "Constructs.h"
#include <Phobos.CRT.h>

#include "Enum.h"
#ifdef aaa
struct EnumCompareMode {
	bool operator()(const std::string& s1, const std::string& s2) {
		return IS_SAME_STR_(s1.c_str() , s2.c_str());
	}
};

template <typename TValue, class Comp>
	requires std::is_enum_v<TValue>
class EnumMap
{
	using pair_t = std::pair<std::string, TValue>;
	using container_t = std::vector<pair_t>;
public:
	COMPILETIMEEVAL auto get_key_iterator(const std::string& key) {
		return std::ranges::find_if(this->values, [&](const container_t::value_type& item) {
			return EnumCompareMode()(item.first , key);
		});
	}

	COMPILETIMEEVAL void emplace_back(const char* pKey, TValue val) {
		auto back = &values.emplace_back();
		back->first = pKey;
		back->second = val;
	}

	COMPILETIMEEVAL [[nodiscard]] auto begin() noexcept
	{
		return values.begin();
	}

	COMPILETIMEEVAL [[nodiscard]] auto begin() const noexcept
	{
		return values.begin();
	}

	COMPILETIMEEVAL [[nodiscard]] auto end() noexcept
	{
		return values.end();
	}

	COMPILETIMEEVAL [[nodiscard]] auto end() const noexcept
	{
		return values.end();
	}

private:

	container_t values;
};

template<typename TEnum> requires std::is_enum_v<TEnum>
COMPILETIMEEVAL OPTIONALINLINE EnumMap<TEnum, EnumCompareMode> GetEnumMapping() {
	static_assert(true, "Not Implemented!");
}

template<>
COMPILETIMEEVAL OPTIONALINLINE EnumMap<ExpireWeaponCondition, EnumCompareMode> GetEnumMapping() {
	EnumMap<ExpireWeaponCondition, EnumCompareMode> ret;
	ret.emplace_back("none" , ExpireWeaponCondition::None);
	ret.emplace_back("expire", ExpireWeaponCondition::Expire);
	ret.emplace_back("remove", ExpireWeaponCondition::Remove);
	ret.emplace_back("death", ExpireWeaponCondition::Death);
	ret.emplace_back("all", ExpireWeaponCondition::All);
	return ret;
}

// Unificate enum parsing using this please. -Multfinite
template<typename TEnum , bool OrOperator = false> requires std::is_enum_v<TEnum>
COMPILETIMEEVAL OPTIONALINLINE bool ParseEnum(const std::string& value, TEnum& result)
{
	auto mappings = GetEnumMapping<TEnum>();
	auto iter = mappings.get_key_iterator(value);

	if (iter != mappings.end()) {

		if COMPILETIMEEVAL (!OrOperator)
			result = iter->second;
		else
			result |= iter->second;

		return true;
	}

	return false;
}
#endif