#pragma once

#include <Utilities/SavegameDef.h>

// a wrapper for an optional value
template <typename T, bool Persistable = false>
struct OptionalStruct : public std::optional<T>
{
	using Base = std::optional<T>;

	// Constructors - forward to base class
	COMPILETIMEEVAL OptionalStruct() = default;
	COMPILETIMEEVAL OptionalStruct(const OptionalStruct& other) = default;
	COMPILETIMEEVAL OptionalStruct& operator=(const OptionalStruct& other) = default;
	COMPILETIMEEVAL OptionalStruct(OptionalStruct&& other) = default;
	COMPILETIMEEVAL OptionalStruct& operator=(OptionalStruct&& other) = default;

	// Constructor from value
	explicit COMPILETIMEEVAL OptionalStruct(const T& value) noexcept : Base(value) { }
	explicit COMPILETIMEEVAL OptionalStruct(T&& value) noexcept : Base(std::move(value)) { }

	// Assignment operator from T - no reinterpret_cast needed!
	OptionalStruct& operator=(const T& value)
	{
		Base::operator=(value);
		return *this;
	}

	OptionalStruct& operator=(T&& value)
	{
		Base::operator=(std::move(value));
		return *this;
	}

	// Conversion operators for compatibility with original code
	COMPILETIMEEVAL operator T& () noexcept
	{
		return this->value();
	}

	COMPILETIMEEVAL operator const T& () const noexcept
	{
		return this->value();
	}

	// Compatibility methods
	COMPILETIMEEVAL void clear() noexcept
	{
		this->reset();
	}

	COMPILETIMEEVAL bool empty() const noexcept
	{
		return !this->has_value();
	}

	COMPILETIMEEVAL bool isset() const noexcept
	{
		return this->has_value();
	}

	COMPILETIMEEVAL const T& get() const noexcept
	{
		return this->value();
	}

	bool load(PhobosStreamReader& Stm, bool RegisterForChange)
	{
		this->reset(); // Clear existing value

		if COMPILETIMEEVAL(!Persistable)
			return true;
		else {
			bool hasval;
			if (Stm.Process(hasval)) {
				if (!hasval) {
					return true;
				} else {
					T buffer {};
					if (Stm.Process(buffer, RegisterForChange)) {
						this->emplace(std::move(buffer)); // Use emplace for efficiency
						return true;
					}
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
			const bool hasValue = this->has_value();
			Stm.Process(hasValue);
			if (hasValue)
			{
				Stm.Process(this->value());
			}
			return true;
		}
	}

};