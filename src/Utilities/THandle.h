#pragma once
#include <Base/Always.h>

#include <utility>
#include <type_traits>
#include <algorithm>

template <typename T>
struct THandleTraits
{
	using type = T;

	static type default_value() noexcept
	{
		return T {};
	}
};

struct TModuleHandleTraits : public THandleTraits<HMODULE>
{
	static void close(HMODULE handle) noexcept
	{
		if (handle)
		{
			FreeLibrary(handle);
		}
	}
};

template <typename Traits>
struct THandle
{
	using value_type = typename Traits::type;

	THandle() noexcept = default;

	explicit THandle(value_type value) noexcept : Value(value)
	{}

	THandle(THandle const&) = delete;

	THandle(THandle&& other) noexcept : Value(other.release())
	{}

	~THandle() noexcept
	{
		if (*this)
		{
			Traits::close(this->Value);
		}
	}

	THandle& operator=(THandle const&) = delete;

	THandle& operator=(THandle&& other) noexcept
	{
		this->reset(other.release());
		return *this;
	}

	explicit operator bool() const noexcept
	{
		return this->Value != Traits::default_value();
	}

	operator value_type() const noexcept
	{
		return this->Value;
	}

	value_type get() const noexcept
	{
		return this->Value;
	}

	value_type release() noexcept
	{
		return std::exchange(this->Value, Traits::default_value());
	}

	void reset(value_type value) noexcept
	{
		THandle(this->Value);
		this->Value = value;
	}

	void clear() noexcept
	{
		THandle(std::move(*this));
	}

	value_type* set() noexcept
	{
		this->clear();
		return &this->Value;
	}

	void swap(THandle& other) noexcept
	{
		using std::swap;
		swap(this->Value, other.Value);
	}

	friend void swap(THandle& lhs, THandle& rhs) noexcept
	{
		lhs.swap(rhs);
	}

private:
	value_type Value { Traits::default_value() };
};

using ModuleTHandle = THandle<TModuleHandleTraits>;
