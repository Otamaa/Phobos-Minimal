#pragma once

#include <vector>
#include <Utilities/Concepts.h>
#include <Utilities/Savegame.h>

// a poor man's map with contiguous storage
template <typename TKey, typename TValue, class customMem = std::allocator<std::pair<TKey, TValue>>>
class PhobosMap
{
	using pair_t = std::pair<TKey, TValue>;
	using container_t = std::vector<pair_t, customMem>;
public:

	TValue& operator[] (const TKey& key)
	{
		if (auto pValue = this->tryfind(key))
		{
			return *pValue;
		}
		return this->insert_unchecked(key, TValue());
	}

	//TValue* find__(const TKey& key) {
	//	auto pValue = static_cast<const PhobosMap*>(this)->tryfind(key);
	//	return const_cast<TValue*>(pValue);
	//}

	constexpr TValue* tryfind(const TKey& key)
	{
		auto it = this->get_key_iterator(key);
		if (it != this->values.end())
		{
			return &it->second;
		}

		return nullptr;
	}

	// nonmodifiable
	constexpr const TValue* tryfind(const TKey& key) const
	{
		auto it = this->get_key_iterator(key);

		if (it != this->values.end())
		{
			return &it->second;
		}

		return nullptr;
	}

	// nonmodifiable
	constexpr TValue get_or_default(const TKey& key) const
	{
		if (auto pValue = this->tryfind(key))
		{
			return *pValue;
		}
		return TValue();
	}

	// nonmodifiable
	constexpr TValue get_or_default(const TKey& key, TValue def) const
	{
		if (auto pValue = this->tryfind(key))
		{
			return *pValue;
		}
		return def;
	}

	constexpr void erase(container_t::iterator iter)
	{
		this->values.erase(iter, this->values.end());
	}

	constexpr bool erase(const TKey& key)
	{
		auto it = this->get_key_iterator(key);
		if (it != this->values.end())
		{
			this->values.erase(it);
			return true;
		}
		return false;
	}

	constexpr bool contains(const TKey& key) const
	{
		return this->get_key_iterator(key) != values.end();
	}

	bool insert(const TKey& key, TValue value)
	{
		if (!this->tryfind(key))
		{
			this->insert_unchecked(key, std::move(value));
			return true;
		}
		return false;
	}

	void emplace_unchecked(const TKey& key, TValue value)
	{
		this->insert_unchecked(key, std::move(value));
	}

	constexpr size_t size() const
	{
		return values.size();
	}

	constexpr bool empty() const
	{
		return values.empty();
	}

	constexpr void clear()
	{
		values.clear();
	}

	constexpr void reserve(size_t newsize)
	{
		values.reserve(newsize);
	}

	constexpr void resize(size_t newsize)
	{
		values.resize(newsize);
	}

	bool load(PhobosStreamReader& Stm, bool RegisterForChange)
	{
		this->clear();

		size_t size = 0;
		auto ret = Stm.Load(size);

		if (ret && size)
		{
			this->values.resize(size);
			for (size_t i = 0; i < size; ++i)
			{
				if (!Savegame::ReadPhobosStream(Stm, this->values[i].first, RegisterForChange)
					|| !Savegame::ReadPhobosStream(Stm, this->values[i].second, RegisterForChange))
				{
					return false;
				}
			}
		}

		return ret;
	}
	bool save(PhobosStreamWriter& Stm) const
	{
		Stm.Save(this->values.size());

		for (const auto& [first, second] : this->values)
		{
			Savegame::WritePhobosStream(Stm, first);
			Savegame::WritePhobosStream(Stm, second);
		}

		return true;
	}

	constexpr [[nodiscard]] auto begin() noexcept
	{
		return values.begin();
	}

	constexpr [[nodiscard]] auto begin() const noexcept
	{
		return values.begin();
	}

	constexpr [[nodiscard]] auto end() noexcept
	{
		return values.end();
	}

	constexpr [[nodiscard]] auto end() const noexcept
	{
		return values.end();
	}

	constexpr auto back() const
	{
		return values.back();
	}

	constexpr auto back()
	{
		return values.back();
	}

	constexpr auto get_key_iterator(const TKey& key)
	{
		if constexpr (direct_comparable<TKey>)
		{
			return std::find_if(this->values.begin(), this->values.end(), [&](const container_t::value_type& item)
 {
	 return item.first == key;
			});
		}
		else
		{
			return std::find(this->values.begin(), this->values.end(), key);
		}
	}

	// nonmodifiable
	constexpr auto get_key_iterator(const TKey& key) const
	{
		if constexpr (direct_comparable<TKey>)
		{
			return std::find_if(this->values.begin(), this->values.end(), [&](const container_t::value_type& item)
 {
	 return item.first == key;
			});
		}
		else
		{
			return std::find(this->values.begin(), this->values.end(), key);
		}
	}

	TValue& insert_unchecked(const TKey& key, TValue value)
	{
		this->values.emplace_back(key, std::move(value));
		return this->values.back().second;
	}

private:

	container_t values;
};
