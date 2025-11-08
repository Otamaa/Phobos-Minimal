#pragma once

#include <DynamicVectorClass.h>

//========================================================================
//=== TypeList ===========================================================
//========================================================================

template <typename T>
class TypeList : public DynamicVectorClass<T>
{
public:
	// Type aliases for STL compatibility
	using typename DynamicVectorClass<T>::value_type;
	using typename DynamicVectorClass<T>::size_type;
	using typename DynamicVectorClass<T>::reference;
	using typename DynamicVectorClass<T>::const_reference;
	using typename DynamicVectorClass<T>::pointer;
	using typename DynamicVectorClass<T>::const_pointer;
	using typename DynamicVectorClass<T>::iterator;
	using typename DynamicVectorClass<T>::const_iterator;
	using typename DynamicVectorClass<T>::reverse_iterator;
	using typename DynamicVectorClass<T>::const_reverse_iterator;

	static constexpr ArrayType Type = ArrayType::TypeList;

	constexpr TypeList() noexcept = default;

	explicit TypeList(int capacity, T* pMem = nullptr)
		: DynamicVectorClass<T>(capacity, pMem)
	{ }

	TypeList(const TypeList<T>& other)
		: DynamicVectorClass<T>(other), unknown_18(other.unknown_18)
	{ }

	TypeList(TypeList<T>&& other) noexcept
		: DynamicVectorClass<T>(std::move(other)), unknown_18(other.unknown_18)
	{ }

	// Initializer list constructor
	TypeList(std::initializer_list<T> init)
		: DynamicVectorClass<T>(init)
	{ }

	TypeList<T>& operator=(const TypeList<T>& other)
	{
		if (this != &other)
		{
			TypeList<T>(other).swap(*this);
		}
		return *this;
	}

	TypeList<T>& operator=(TypeList<T>&& other) noexcept
	{
		if (this != &other)
		{
			TypeList<T>(std::move(other)).swap(*this);
		}
		return *this;
	}

	TypeList<T>& operator=(std::initializer_list<T> init)
	{
		TypeList<T>(init).swap(*this);
		return *this;
	}

	void swap(TypeList<T>& other) noexcept
	{
		DynamicVectorClass<T>::swap(other);
		using std::swap;
		swap(this->unknown_18, other.unknown_18);
	}

	// Additional accessor for the unknown field (if needed)
	[[nodiscard]] constexpr int get_unknown_18() const noexcept
	{
		return unknown_18;
	}

	void set_unknown_18(int value) noexcept
	{
		unknown_18 = value;
	}

	/*
	// Legacy interface (deprecated - kept for reference during migration)
	// Use modern STL-style methods instead:
	//
	// Swap(other)          → swap(other)
	// GetUnknown18()       → get_unknown_18()
	// SetUnknown18(value)  → set_unknown_18(value)
	*/

	int unknown_18 { 0 };
};

// Swap specialization for ADL
template <typename T>
void swap(TypeList<T>& lhs, TypeList<T>& rhs) noexcept
{
	lhs.swap(rhs);
}