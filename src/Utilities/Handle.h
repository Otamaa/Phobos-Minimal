#pragma once

#include "Savegame.h"

// owns a resource. not copyable, but movable.
template <typename T, typename Deleter, T Default = T()>
struct Handle
{
	constexpr Handle() noexcept = default;

	constexpr explicit Handle(T value) noexcept
		: Value(value)
	{
	}

	Handle(const Handle&) = delete;

	constexpr Handle(Handle&& other) noexcept
		: Value(other.release())
	{
	}

	~Handle() noexcept
	{
		if (this->destroy)
		{
			Deleter {}(this->Value);
		}

		this->Value = Default;
	}

	constexpr void SetDestroyCondition(bool val)
	{
		this->destroy = val;
	}

	Handle& operator = (const Handle&) = delete;

	Handle& operator = (Handle&& other) noexcept
	{
		this->reset(other.release());
		return *this;
	}

	constexpr explicit operator bool() const noexcept
	{
		return this->Value != Default;
	}

	constexpr operator T () const noexcept
	{
		return this->Value;
	}

	constexpr T get() const noexcept
	{
		return this->Value;
	}

	constexpr T operator->() const noexcept
	{
		return get();
	}

	constexpr T release() noexcept
	{
		return std::exchange(this->Value, Default);
	}

	void reset(T value) noexcept
	{
		Handle(this->Value);
		this->Value = value;
	}

	void clear() noexcept
	{
		Handle(std::move(*this));
	}

	void swap(Handle& other) noexcept
	{
		using std::swap;
		swap(this->Value, other.Value);
	}

	friend void swap(Handle& lhs, Handle& rhs) noexcept
	{
		lhs.swap(rhs);
	}

	bool load(PhobosStreamReader& Stm, bool RegisterForChange)
	{
		return
			Savegame::ReadPhobosStream(Stm, this->destroy, false)
			&& Savegame::ReadPhobosStream(Stm, this->Value, RegisterForChange)
			;

	}
	bool save(PhobosStreamWriter& Stm) const
	{
		return Savegame::WritePhobosStream(Stm, this->destroy)
			&& Savegame::WritePhobosStream(Stm, this->Value)
			;
	}

private:
	bool destroy { true };
	T Value { Default };
};