#pragma once

#include <Base/Always.h>

// vroom vroom
// Westwood uses if(((1 << HouseClass::ArrayIndex) & TechnoClass::DisplayProductionToHouses) != 0) and other bitfields like this (esp. in CellClass, omg optimized). helper wrapper just because
template <typename T>
class IndexBitfield
{
public:
	COMPILETIMEEVAL IndexBitfield() = default;
	COMPILETIMEEVAL explicit IndexBitfield(DWORD const defVal) noexcept : data(defVal) { };

	COMPILETIMEEVAL IndexBitfield<T>& operator=(DWORD other)
	{
		std::swap(data, other);
		return *this;
	}

	COMPILETIMEEVAL bool Contains(const T obj) const
	{
		return Contains(obj->ArrayIndex);
	}

	COMPILETIMEEVAL bool Contains(int index) const
	{
		return (this->data & (1u << index)) != 0u;
	}

	COMPILETIMEEVAL void Add(int index)
	{
		this->data |= (1u << index);
	}

	COMPILETIMEEVAL void Add(const T obj)
	{
		Add(obj->ArrayIndex);
	}

	COMPILETIMEEVAL void Remove(int index)
	{
		this->data &= ~(1u << index);
	}

	COMPILETIMEEVAL void Remove(const T obj)
	{
		Remove(obj->ArrayIndex);
	}

	COMPILETIMEEVAL void Clear()
	{
		this->data = 0u;
	}

	COMPILETIMEEVAL operator DWORD() const
	{
		return this->data;
	}

	DWORD data { 0u };
};
