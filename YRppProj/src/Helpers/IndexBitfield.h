#pragma once

// vroom vroom
// Westwood uses if(((1 << HouseClass::ArrayIndex) & TechnoClass::DisplayProductionToHouses) != 0) and other bitfields like this (esp. in CellClass, omg optimized). helper wrapper just because
#include <algorithm>

template <typename T>
class IndexBitfield
{
public:
	constexpr IndexBitfield() = default;
	constexpr explicit IndexBitfield(DWORD const defVal) noexcept : data(defVal) { };

	constexpr IndexBitfield<T>& operator=(DWORD other)
	{
		std::swap(data, other);
		return *this;
	}

	constexpr bool Contains(const T obj) const
	{
		return Contains(obj->ArrayIndex);
	}

	constexpr bool Contains(int index) const
	{
		return (this->data & (1u << index)) != 0u;
	}

	constexpr void Add(int index)
	{
		this->data |= (1u << index);
	}

	constexpr void Add(const T obj)
	{
		Add(obj->ArrayIndex);
	}

	constexpr void Remove(int index)
	{
		this->data &= ~(1u << index);
	}

	constexpr void Remove(const T obj)
	{
		Remove(obj->ArrayIndex);
	}

	constexpr void Clear()
	{
		this->data = 0u;
	}

	constexpr operator DWORD() const
	{
		return this->data;
	}

	DWORD data { 0u };
};