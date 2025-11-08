#pragma region Ares Copyrights
/*
 *Copyright (c) 2008+, All Ares Contributors
 *All rights reserved.
 *
 *Redistribution and use in source and binary forms, with or without
 *modification, are permitted provided that the following conditions are met:
 *1. Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 *2. Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 *3. All advertising materials mentioning features or use of this software
 *   must display the following acknowledgement:
 *   This product includes software developed by the Ares Contributors.
 *4. Neither the name of Ares nor the
 *   names of its contributors may be used to endorse or promote products
 *   derived from this software without specific prior written permission.
 *
 *THIS SOFTWARE IS PROVIDED BY ITS CONTRIBUTORS ''AS IS'' AND ANY
 *EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 *WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *DISCLAIMED. IN NO EVENT SHALL THE ARES CONTRIBUTORS BE LIABLE FOR ANY
 *DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 *(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#pragma endregion

#pragma once

/*
	Iterator is used for bridging implementation between different kinds of array classes.
	Provides a unified read-only view over various container types.
*/

#include <ArrayClasses.h>
#include <vector>
#include <array>
#include <span>
#include <algorithm>
#include <optional>
#include <type_traits>
#include <Helpers/Concepts.h>

template<typename T>
class Iterator
{
public:
	// Type aliases for STL compatibility
	using value_type = T;
	using size_type = std::size_t;
	using difference_type = std::ptrdiff_t;
	using const_reference = const T&;
	using const_pointer = const T*;
	using const_iterator = const T*;

	// Default constructor
	constexpr Iterator() noexcept = default;

	// Raw pointer + size constructor
	constexpr Iterator(const T* first, size_type count) noexcept
		: items(first), count(count) { }

	// std::vector constructor
	constexpr Iterator(const std::vector<T>& vec) noexcept
		: items(vec.data()), count(vec.size()) { }

	// std::array constructor
	template<size_type N>
	constexpr Iterator(const std::array<T, N>& arr) noexcept
		: items(arr.data()), count(N) { }

	// std::span constructor
	constexpr Iterator(std::span<const T> sp) noexcept
		: items(sp.data()), count(sp.size()) { }

	// VectorClass constructor
	constexpr Iterator(const VectorClass<T>& vec) noexcept
		: items(vec.Items), count(static_cast<size_type>(vec.Capacity)) { }

	// DynamicVectorClass constructor
	constexpr Iterator(const DynamicVectorClass<T>& vec) noexcept
		: items(vec.Items), count(static_cast<size_type>(vec.Count)) { }

	// TypeList constructor
	constexpr Iterator(const TypeList<T>& vec) noexcept
		: items(vec.Items), count(static_cast<size_type>(vec.Count)) { }

	// Element access with bounds checking
	[[nodiscard]] constexpr const_reference at(size_type index) const
	{
		if (index >= count)
		{
			throw std::out_of_range("Iterator::at: index out of range");
		}
		return items[index];
	}

	// Unchecked element access (for performance)
	[[nodiscard]] constexpr const_reference operator[](size_type index) const noexcept
	{
		return items[index];
	}

	// Get element with optional
	[[nodiscard]] constexpr std::optional<T> get(size_type index) const noexcept
	{
		if (valid_index(index))
		{
			return items[index];
		}
		return std::nullopt;
	}

	// Get element or default value
	[[nodiscard]] constexpr T get_or(size_type index, T default_value) const noexcept
	{
		if (valid_index(index))
		{
			return items[index];
		}
		return default_value;
	}

	// Get element or last valid element
	[[nodiscard]] constexpr T get_or_last(size_type index) const noexcept
	{
		if (!valid())
		{
			return T();
		}
		if (!valid_index(index))
		{
			index = count - 1;
		}
		return items[index];
	}

	// Size queries
	[[nodiscard]] constexpr size_type size() const noexcept
	{
		return count;
	}

	[[nodiscard]] constexpr bool empty() const noexcept
	{
		return count == 0 || items == nullptr;
	}

	[[nodiscard]] constexpr bool valid() const noexcept
	{
		return items != nullptr;
	}

	[[nodiscard]] constexpr bool valid_index(size_type index) const noexcept
	{
		return index < count;
	}

	// Iterator interface
	[[nodiscard]] constexpr const_iterator begin() const noexcept
	{
		return items;
	}

	[[nodiscard]] constexpr const_iterator end() const noexcept
	{
		return items ? items + count : nullptr;
	}

	[[nodiscard]] constexpr const_iterator cbegin() const noexcept
	{
		return begin();
	}

	[[nodiscard]] constexpr const_iterator cend() const noexcept
	{
		return end();
	}

	// Front and back access
	[[nodiscard]] constexpr const_reference front() const
	{
		if (empty())
		{
			throw std::out_of_range("Iterator::front: empty iterator");
		}
		return items[0];
	}

	[[nodiscard]] constexpr const_reference back() const
	{
		if (empty())
		{
			throw std::out_of_range("Iterator::back: empty iterator");
		}
		return items[count - 1];
	}

	// Data pointer access
	[[nodiscard]] constexpr const_pointer data() const noexcept
	{
		return items;
	}

	// Search operations
	[[nodiscard]] constexpr bool contains(const T& value) const noexcept
	{
		if constexpr (direct_comparable<T>)
		{
			// Optimized path for directly comparable types
			for (size_type i = 0; i < count; ++i)
			{
				if (items[i] == value)
				{
					return true;
				}
			}
			return false;
		}
		else
		{
			return std::find(begin(), end(), value) != end();
		}
	}

	// Find index of element
	[[nodiscard]] constexpr std::optional<size_type> find_index(const T& value) const noexcept
	{
		for (size_type i = 0; i < count; ++i)
		{
			if (items[i] == value)
			{
				return i;
			}
		}
		return std::nullopt;
	}

	// Find with predicate
	template<typename Pred>
	[[nodiscard]] constexpr const_iterator find_if(Pred&& pred) const noexcept
	{
		return std::find_if(begin(), end(), std::forward<Pred>(pred));
	}

	// Count occurrences
	[[nodiscard]] constexpr size_type count_if(auto&& pred) const noexcept
	{
		return std::count_if(begin(), end(), std::forward<decltype(pred)>(pred));
	}

	// Algorithm wrappers
	template<typename Func>
	constexpr void for_each(Func&& func) const
	{
		std::for_each(begin(), end(), std::forward<Func>(func));
	}

	template<typename Pred>
	[[nodiscard]] constexpr bool any_of(Pred&& pred) const noexcept
	{
		return std::any_of(begin(), end(), std::forward<Pred>(pred));
	}

	template<typename Pred>
	[[nodiscard]] constexpr bool all_of(Pred&& pred) const noexcept
	{
		return std::all_of(begin(), end(), std::forward<Pred>(pred));
	}

	template<typename Pred>
	[[nodiscard]] constexpr bool none_of(Pred&& pred) const noexcept
	{
		return std::none_of(begin(), end(), std::forward<Pred>(pred));
	}

	// Conversion to std::span
	[[nodiscard]] constexpr operator std::span<const T>() const noexcept
	{
		return std::span<const T>(items, count);
	}

	// Boolean conversion
	[[nodiscard]] constexpr explicit operator bool() const noexcept
	{
		return !empty();
	}

	[[nodiscard]] constexpr bool operator!() const noexcept
	{
		return empty();
	}

	// Type conversion for derived types
	template<typename Out>
	[[nodiscard]] constexpr operator Iterator<Out>() const noexcept
	{
		if constexpr(std::is_pointer<Out>::value && std::is_pointer<T>::value) {
			using typeOut = std::remove_pointer_t<Out>;
			using typeT = std::remove_pointer_t<T>;

			static_assert(sizeof(T) == sizeof(Out),
					"Pointer sizes must match for safe conversion");

			static_assert(std::is_base_of_v<typeOut, typeT> || std::is_base_of_v<typeT, typeOut>,
			"Types must be related by inheritance");

			return Iterator<Out>(reinterpret_cast<const Out*>(items), count);
		} else {
			// Note: this only works if pointer-to-derived equals pointer-to-base.
			// Safe for simple inheritance, unsafe with multiple/virtual inheritance.
			static_assert(sizeof(T*) == sizeof(Out*),
				"Pointer sizes must match for safe conversion");

			static_assert(std::is_base_of_v<Out, T> || std::is_base_of_v<T, Out>,
				"Types must be related by inheritance");

			return Iterator<Out>(reinterpret_cast<const Out*>(items), count);
		}
	}

	// Legacy interface (commented for migration)
	/*
	T GetItemAt(int nIdx) const { return get(nIdx).value_or(T()); }
	T GetItemAtOrMax(int nIdx) const { return get_or_last(nIdx); }
	T GetItemAtOrDefault(int nIdx, const T& other) const { return get_or(nIdx, other); }
	bool ValidIndex(int index) const { return valid_index(static_cast<size_type>(index)); }
	size_t end_idx() const { return count > 0 ? count - 1 : 0; }
	*/

private:
	const T* items { nullptr };
	size_type count { 0 };
};

// Deduction guides (C++17)
template<typename T>
Iterator(const T*, std::size_t) -> Iterator<T>;

template<typename T>
Iterator(const std::vector<T>&) -> Iterator<T>;

template<typename T, std::size_t N>
Iterator(const std::array<T, N>&) -> Iterator<T>;

template<typename T>
Iterator(const VectorClass<T>&) -> Iterator<T>;

template<typename T>
Iterator(const DynamicVectorClass<T>&) -> Iterator<T>;

template<typename T>
Iterator(const TypeList<T>&) -> Iterator<T>;

// Factory functions with better names
template<typename T>
[[nodiscard]] constexpr Iterator<T> make_iterator_single(const T& value) noexcept
{
	return Iterator<T>(&value, 1);
}

template<typename T, std::size_t Size>
[[nodiscard]] constexpr Iterator<T> make_iterator(const T(&arr)[Size]) noexcept
{
	return Iterator<T>(arr, Size);
}

template<typename T>
[[nodiscard]] constexpr Iterator<T> make_iterator(const T* ptr, std::size_t size) noexcept
{
	return Iterator<T>(ptr, size);
}

// Container overloads
template<typename T>
[[nodiscard]] constexpr Iterator<T> make_iterator(const std::vector<T>& value) noexcept
{
	return Iterator<T>(value);
}

template<typename T, std::size_t N>
[[nodiscard]] constexpr Iterator<T> make_iterator(const std::array<T, N>& value) noexcept
{
	return Iterator<T>(value);
}

template<typename T>
[[nodiscard]] constexpr Iterator<T> make_iterator(const VectorClass<T>& value) noexcept
{
	return Iterator<T>(value);
}

template<typename T>
[[nodiscard]] constexpr Iterator<T> make_iterator(const DynamicVectorClass<T>& value) noexcept
{
	return Iterator<T>(value);
}

template<typename T>
[[nodiscard]] constexpr Iterator<T> make_iterator(const TypeList<T>& value) noexcept
{
	return Iterator<T>(value);
}

template<typename T>
[[nodiscard]] constexpr Iterator<T> make_iterator(std::span<const T> value) noexcept
{
	return Iterator<T>(value);
}

// Delete rvalue overloads to prevent dangling references
template<typename T>
void make_iterator_single(const T&&) = delete;

template<typename T>
void make_iterator(const std::vector<T>&&) = delete;

template<typename T, std::size_t N>
void make_iterator(const std::array<T, N>&&) = delete;

template<typename T>
void make_iterator(const VectorClass<T>&&) = delete;

template<typename T>
void make_iterator(const DynamicVectorClass<T>&&) = delete;

template<typename T>
void make_iterator(const TypeList<T>&&) = delete;

template<typename T>
void make_iterator(std::span<const T>&&) = delete;