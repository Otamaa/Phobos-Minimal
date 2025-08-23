#pragma once

#include <Utilities/SavegameDef.h>

// a wrapper for an optional value
template <typename T, bool Persistable = false>
struct OptionalStruct
{
	COMPILETIMEEVAL OptionalStruct() = default;
	explicit OptionalStruct(T value) noexcept : Value(std::move(value)), HasValue(true) { }

	OptionalStruct& operator= (T value)
	{
		this->Value = std::move(value);
		this->HasValue = true;
		return *this;
	}

	~OptionalStruct() noexcept { this->clear(); }

	COMPILETIMEEVAL OptionalStruct(const OptionalStruct& other) = default;
	COMPILETIMEEVAL OptionalStruct& operator=(const OptionalStruct& other) = default;

	COMPILETIMEEVAL operator T& () noexcept
	{
		return this->Value;
	}

	COMPILETIMEEVAL operator const T& () const noexcept
	{
		return this->Value;
	}

	COMPILETIMEEVAL void clear()
	{
		this->Value = T();
		this->HasValue = false;
	}

	COMPILETIMEEVAL bool empty() const noexcept
	{
		return !this->HasValue;
	}

	COMPILETIMEEVAL explicit operator bool() const noexcept
	{
		return this->HasValue;
	}

	COMPILETIMEEVAL bool has_value() const noexcept
	{
		return this->HasValue;
	}

	COMPILETIMEEVAL bool isset() const noexcept
	{
		return this->HasValue;
	}

	COMPILETIMEEVAL const T& get() const noexcept
	{
		return this->Value;
	}

	bool load(PhobosStreamReader& Stm, bool RegisterForChange)
	{
		this->clear();

		if COMPILETIMEEVAL (!Persistable)
			return true;
		else
		{
			if (Stm.Load(this->HasValue))
			{
				if (!this->HasValue || Stm.Process(this->Value, RegisterForChange))
				{
					return true;
				}
			}

			return false;
		}
	}
	bool save(PhobosStreamWriter& Stm) const
	{
		if COMPILETIMEEVAL (!Persistable)
			return true;
		else
		{
			Stm.Save(this->HasValue);
			if (this->HasValue)
			{
				Stm.Process(this->Value);
			}
			return true;
		}
	}

private:
	T Value {};
	bool HasValue { false };
};