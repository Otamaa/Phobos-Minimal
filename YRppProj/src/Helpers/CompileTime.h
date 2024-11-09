#pragma once

#include <Base/Always.h>
#include <type_traits>

struct noinit_t;

// defines a compile time pointer to a known memory address
template <typename T, unsigned int Address>
struct constant_ptr {
	using value_type = T*;

	constexpr constant_ptr() noexcept = default;
	constant_ptr(constant_ptr&) = delete;
private:
	// mere presence "fixes" C2100: illegal indirection
	constexpr constant_ptr(noinit_t) noexcept {}
public:

	constexpr DWORD getAddrs() const noexcept {
		return Address;
	}

	FORCEINLINE constexpr value_type get() const noexcept {
		return reinterpret_cast<value_type>(Address);
	}

	FORCEINLINE constexpr operator value_type() const noexcept {
		return get();
	}

	FORCEINLINE constexpr value_type operator()() const noexcept {
		return get();
	}

	FORCEINLINE constexpr value_type operator->() const noexcept {
		return get();
	}

	constexpr T& operator*() const noexcept {
		return *get();
	}
};

// defines a compile time reference to a known memory address
template <typename T, unsigned int Address, size_t Count = 0>
struct reference {
	using value_type = T[Count];

	static const auto Size = Count;

	constexpr reference() noexcept = default;
	reference(reference&) = delete;
private:
	// mere presence "fixes" C2100: illegal indirection
	constexpr reference(noinit_t) noexcept {}
public:

	constexpr DWORD getAddrs() const noexcept {
		return Address;
	}

	constexpr value_type& get() const noexcept {
		// fixes" C2101: '&' on constant
		static auto const address = Address;
		return *reinterpret_cast<value_type*>(address);
	}

	constexpr operator value_type&() const noexcept {
		return get();
	}

	constexpr value_type& operator()() const noexcept {
		return get();
	}

	constexpr decltype(auto) operator&() const noexcept {
		return &get();
	}

	constexpr decltype(auto) operator*() const noexcept {
		return *get();
	}

	constexpr T& operator[](int index) const noexcept {
		return get()[index];
	}

	constexpr value_type& data() const noexcept {
		return get();
	}

	constexpr size_t size() const noexcept {
		return Size;
	}

	constexpr size_t c_size() const noexcept {
		return Count;
	}

	constexpr T* begin() const noexcept {
		return data();
	}

	constexpr T* end() const noexcept {
		return begin() + Size;
	}
};

//TODO : operators
template <typename T, unsigned int Address, size_t CountX , size_t CountY>
struct reference2D {
	using value_type = T[CountX][CountY];
	constexpr reference2D() noexcept = default;
	reference2D(reference2D&) = delete;
private:
	// mere presence "fixes" C2100: illegal indirection
	constexpr reference2D(noinit_t) noexcept {}
public:

	constexpr DWORD getAddrs() const noexcept {
		return Address;
	}

	value_type& get() const noexcept {
		// fixes" C2101: '&' on constant
		static auto const address = Address;
		return *reinterpret_cast<value_type*>(address);
	}

	operator value_type& () const noexcept {
		return get();
	}

	value_type& operator()() const noexcept {
		return get();
	}

	decltype(auto) operator&() const noexcept {
		return &get();
	}

	decltype(auto) operator*() const noexcept {
		return *get();
	}

	value_type& data() const noexcept {
		return get();
	}
};

// specializations for non-array references
template <typename T, unsigned int Address>
struct reference<T, Address, 0> {
	using value_type = T;

	constexpr reference() noexcept = default;
	reference(reference&) = delete;
private:
	// mere presence "fixes" C2100: illegal indirection
	constexpr reference(noinit_t) noexcept {}
public:

	FORCEINLINE constexpr DWORD getAddrs() const noexcept {
		return Address;
	}

	FORCEINLINE constexpr value_type& get() const noexcept {
		return *reinterpret_cast<value_type*>(Address);
	}

	template <typename T2, typename = std::enable_if_t<std::is_assignable<T&, T2>::value>>
	FORCEINLINE value_type& operator=(T2&& rhs) const {
		return get() = std::forward<T2>(rhs);
	}

	constexpr operator value_type&() const noexcept {
		return get();
	}

	constexpr T& operator()() const noexcept {
		return get();
	}

	constexpr decltype(auto) operator&() const noexcept {
		return &get();
	}

	FORCEINLINE constexpr decltype(auto) operator->() const noexcept {
		if constexpr (std::is_pointer<T>::type())
			return get();
		else
			return &get();
	}

	constexpr decltype(auto) operator*() const noexcept {
		return *get();
	}

	constexpr decltype(auto) operator[](int index) const noexcept {
		return get()[index];
	}
};

template <typename T, unsigned int Address>
struct referencefunc {
	using value_type = T;

	constexpr referencefunc() noexcept = default;
	referencefunc(referencefunc&) = delete;
private:
	// mere presence "fixes" C2100: illegal indirection
	constexpr referencefunc(noinit_t) noexcept {}
public:
	constexpr DWORD getAddrs() const noexcept {
		return Address;
	}

	FORCEINLINE value_type& get() const noexcept {
		return *reinterpret_cast<value_type*>(Address);
	}

	FORCEINLINE uintptr_t getAddress() {
		return Address;
	}

	template <typename T2>
	FORCEINLINE bool operator=(T2 rhs) const {

		DWORD protection = PAGE_EXECUTE_READWRITE;
		if(VirtualProtect((LPVOID)Address, sizeof(LPVOID), protection, &protection) == TRUE) {
			*reinterpret_cast<LPVOID*>(Address) = rhs;
			VirtualProtect((LPVOID)Address, sizeof(LPVOID), protection, &protection);
			return true;
		}

		return false;
	}
};