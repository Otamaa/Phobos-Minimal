#pragma once

#include <Base/Always.h>
#include <type_traits>

struct noinit_t;

// defines a compile time pointer to a known memory address
template <typename T, unsigned int Address>
struct constant_ptr {
	using value_type = T*;

	COMPILETIMEEVAL constant_ptr() noexcept = default;
	constant_ptr(constant_ptr&) = delete;
private:
	// mere presence "fixes" C2100: illegal indirection
	COMPILETIMEEVAL constant_ptr(noinit_t) noexcept {}
public:

	COMPILETIMEEVAL DWORD getAddrs() const noexcept {
		return Address;
	}

	FORCEDINLINE COMPILETIMEEVAL value_type get() const noexcept {
		return reinterpret_cast<value_type>(Address);
	}

	FORCEDINLINE COMPILETIMEEVAL operator value_type() const noexcept {
		return get();
	}

	FORCEDINLINE COMPILETIMEEVAL value_type operator()() const noexcept {
		return get();
	}

	FORCEDINLINE COMPILETIMEEVAL value_type operator->() const noexcept {
		return get();
	}

	COMPILETIMEEVAL T& operator*() const noexcept {
		return *get();
	}
};

// defines a compile time reference to a known memory address
template <typename T, unsigned int Address, size_t Count = 0>
struct reference {
	using value_type = T[Count];

	static const auto Size = Count;

	COMPILETIMEEVAL reference() noexcept = default;
	reference(reference&) = delete;
private:
	// mere presence "fixes" C2100: illegal indirection
	COMPILETIMEEVAL reference(noinit_t) noexcept {}
public:

	COMPILETIMEEVAL DWORD getAddrs() const noexcept {
		return Address;
	}

	COMPILETIMEEVAL value_type& get() const noexcept {
		// fixes" C2101: '&' on constant
		static auto const address = Address;
		return *reinterpret_cast<value_type*>(address);
	}

	COMPILETIMEEVAL operator value_type&() const noexcept {
		return get();
	}

	COMPILETIMEEVAL value_type& operator()() const noexcept {
		return get();
	}

	COMPILETIMEEVAL decltype(auto) operator&() const noexcept {
		return &get();
	}

	COMPILETIMEEVAL decltype(auto) operator*() const noexcept {
		return *get();
	}

	COMPILETIMEEVAL T& operator[](int index) const noexcept {
		return get()[index];
	}

	COMPILETIMEEVAL value_type& data() const noexcept {
		return get();
	}

	COMPILETIMEEVAL size_t size() const noexcept {
		return Size;
	}

	COMPILETIMEEVAL size_t c_size() const noexcept {
		return Count;
	}

	COMPILETIMEEVAL T* begin() const noexcept {
		return data();
	}

	COMPILETIMEEVAL T* end() const noexcept {
		return begin() + Size;
	}
};

//TODO : operators
template <typename T, unsigned int Address, size_t CountX , size_t CountY>
struct reference2D {
	using value_type = T[CountX][CountY];
	COMPILETIMEEVAL reference2D() noexcept = default;
	reference2D(reference2D&) = delete;
private:
	// mere presence "fixes" C2100: illegal indirection
	COMPILETIMEEVAL reference2D(noinit_t) noexcept {}
public:

	COMPILETIMEEVAL DWORD getAddrs() const noexcept {
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

	COMPILETIMEEVAL reference() noexcept = default;
	reference(reference&) = delete;
private:
	// mere presence "fixes" C2100: illegal indirection
	COMPILETIMEEVAL reference(noinit_t) noexcept {}
public:

	FORCEDINLINE COMPILETIMEEVAL DWORD getAddrs() const noexcept {
		return Address;
	}

	FORCEDINLINE COMPILETIMEEVAL value_type& get() const noexcept {
		return *reinterpret_cast<value_type*>(Address);
	}

	template <typename T2, typename = std::enable_if_t<std::is_assignable<T&, T2>::value>>
	FORCEDINLINE value_type& operator=(T2&& rhs) const {
		return get() = std::forward<T2>(rhs);
	}

	COMPILETIMEEVAL operator value_type&() const noexcept {
		return get();
	}

	COMPILETIMEEVAL T& operator()() const noexcept {
		return get();
	}

	COMPILETIMEEVAL decltype(auto) operator&() const noexcept {
		return &get();
	}

	FORCEDINLINE COMPILETIMEEVAL decltype(auto) operator->() const noexcept {
		if COMPILETIMEEVAL (std::is_pointer<T>::type())
			return get();
		else
			return &get();
	}

	COMPILETIMEEVAL decltype(auto) operator*() const noexcept {
		return *get();
	}

	COMPILETIMEEVAL decltype(auto) operator[](int index) const noexcept {
		return get()[index];
	}
};

template <typename T, unsigned int Address>
struct referencefunc {
	using value_type = T;

	COMPILETIMEEVAL referencefunc() noexcept = default;
	referencefunc(referencefunc&) = delete;
private:
	// mere presence "fixes" C2100: illegal indirection
	COMPILETIMEEVAL referencefunc(noinit_t) noexcept {}
public:
	COMPILETIMEEVAL DWORD getAddrs() const noexcept {
		return Address;
	}

	FORCEDINLINE value_type invoke() const noexcept {
		return *reinterpret_cast<value_type*>(Address);
	}

	FORCEDINLINE uintptr_t getAddress() {
		return Address;
	}

	template <typename T2>
	FORCEDINLINE bool operator=(T2 rhs) const {

		DWORD protection = PAGE_EXECUTE_READWRITE;
		DWORD protectionb {};
		if(VirtualProtect((LPVOID)Address, sizeof(LPVOID), protection, &protection) == TRUE) {
			*reinterpret_cast<LPVOID*>(Address) = rhs;
			VirtualProtect((LPVOID)Address, sizeof(LPVOID), protection, &protectionb);
			FlushInstructionCache(Game::hInstance, (LPVOID)Address, sizeof(LPVOID));
			return true;
		}

		return false;
	}
};