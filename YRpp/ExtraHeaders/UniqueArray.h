#pragma once

#include <memory>
#include <assert.h>

OPTIONALINLINE void nodeleter(void*) { }

/// Array of T with ownership. Like \see std::unique_ptr<T[]> but with size tracking.
/// @tparam T Element type.
template <typename T>
class unique_array : public std::unique_ptr<T[], void (*)(void*)>
{
	size_t Size;
private:
	typedef std::unique_ptr<T[], void (*)(void*)> base;
protected:
	unique_array(T* ptr, size_t size, void (*deleter)(void*)) noexcept : base(ptr, deleter), Size(size) { }
	void reset(T* ptr, size_t size) noexcept { base::reset(ptr); Size = size; }
public:
	COMPILETIMEEVAL unique_array() noexcept : base(nullptr, operator delete[]), Size(0) { }
	explicit unique_array(size_t size) : base(new T[size], operator delete[]), Size(size) { }
	template <size_t N> unique_array(T(&arr)[N]) : base(arr, &nodeleter), Size(N) { }
	unique_array(unique_array<T>&& r) : base(move(r)), Size(r.Size) { r.Size = 0; }
	void reset(size_t size = 0) { base::reset(size ? new T[size] : nullptr); Size = size; }
	void swap(unique_array<T>&& other) noexcept { base::swap(other); std::swap(Size, other.Size); }
	void assign(const unique_array<T>& r) const { assert(Size == r.Size); std::copy(r.begin(), r.end(), begin()); }
	const unique_array<T>& operator =(const unique_array<T>& r) const { assign(r); return *this; }
	size_t size() const noexcept { return Size; }
	T* begin() const noexcept { return base::get(); }
	T* end() const noexcept { return begin() + Size; }
	T& operator[](size_t i) const { assert(i < Size); return base::operator[](i); }
	unique_array<T> slice(size_t start, size_t count) const noexcept
	{ assert(start + count <= Size); return unique_array<T>(begin() + start, count, &nodeleter); }
};