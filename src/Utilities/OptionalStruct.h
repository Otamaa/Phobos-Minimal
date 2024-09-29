#pragma once

#include <Utilities/SavegameDef.h>

// a wrapper for an optional value
template <typename T, bool Persistable = false>
struct OptionalStruct
{
	constexpr OptionalStruct() = default;
	explicit OptionalStruct(T value) noexcept : Value(std::move(value)), HasValue(true) { }

	OptionalStruct& operator= (T value)
	{
		this->Value = std::move(value);
		this->HasValue = true;
		return *this;
	}

	~OptionalStruct() noexcept { this->clear(); }

	constexpr OptionalStruct(const OptionalStruct& other) = default;
	constexpr OptionalStruct& operator=(const OptionalStruct& other) = default;

	constexpr operator T& () noexcept
	{
		return this->Value;
	}

	constexpr operator const T& () const noexcept
	{
		return this->Value;
	}

	constexpr void clear()
	{
		this->Value = T();
		this->HasValue = false;
	}

	constexpr bool empty() const noexcept
	{
		return !this->HasValue;
	}

	constexpr explicit operator bool() const noexcept
	{
		return this->HasValue;
	}

	constexpr bool has_value() const noexcept
	{
		return this->HasValue;
	}

	constexpr bool isset() const noexcept
	{
		return this->HasValue;
	}

	constexpr const T& get() const noexcept
	{
		return this->Value;
	}

	bool load(PhobosStreamReader& Stm, bool RegisterForChange)
	{
		this->clear();

		if constexpr (!Persistable)
			return true;
		else
		{
			if (Stm.Load(this->HasValue))
			{
				if (!this->HasValue || Savegame::ReadPhobosStream(Stm, this->Value, RegisterForChange))
				{
					return true;
				}
			}

			return false;
		}
	}
	bool save(PhobosStreamWriter& Stm) const
	{
		if constexpr (!Persistable)
			return true;
		else
		{
			Stm.Save(this->HasValue);
			if (this->HasValue)
			{
				Savegame::WritePhobosStream(Stm, this->Value);
			}
			return true;
		}
	}

private:
	T Value {};
	bool HasValue { false };
};