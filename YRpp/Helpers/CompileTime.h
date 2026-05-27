#pragma once

#include <Base/Always.h>
#include <type_traits>

struct noinit_t;

// ============================================================================
// Trait: is_mem_ref<T>
// Detects whether T is one of the wrapper types defined in this file.
// Use is_mem_ref_v<T> or the NotAMemRef concept to guard generic APIs.
// ============================================================================
template<typename T>
struct is_mem_ref : std::false_type {};

template<typename T>
inline constexpr bool is_mem_ref_v = is_mem_ref<std::remove_cvref_t<T>>::value;

#if __cplusplus >= 202002L
template<typename T>
concept NotAMemRef = !is_mem_ref_v<T>;
#endif

// ============================================================================
// mem_ref_base
//
// Private base inherited by every wrapper.
// Deleting all copy/move paths ensures:
//   auto x  = VoxelSurface;     // error: deleted copy-ctor
//   auto x  = std::move(...);   // error: deleted move-ctor
//
// The wrappers are intentionally zero-size immovable sentinels.
// The ONLY ways to get data out are:
//   .get()          -> T&      (reference to the value at Address)
//   .ptr()          -> T*      (pointer  to the value at Address)
//   operator T&()              (implicit reference conversion)
//   operator T*()              (implicit pointer   conversion)
//   operator()()               (same as get())
//   operator->()               (delegates to get/ptr)
//   operator*()                (dereferences get())
//   operator[](i)              (array access via get())
//
// operator&() is intentionally ABSENT everywhere.
// It was the root cause of the crash: in a template context,
//   &wrapper  ->  std::addressof() path bypasses operator&(),
//                 returning the wrapper's own stack/static address instead
//                 of the intended game memory address.
// Removing it forces all pointer-taking paths through operator T*() which
// is unambiguous regardless of how the call site deduces types.
// ============================================================================
struct mem_ref_base
{
protected:
	constexpr mem_ref_base() noexcept = default;

	mem_ref_base(mem_ref_base const&) = delete;
	mem_ref_base(mem_ref_base&&) = delete;
	mem_ref_base& operator=(mem_ref_base const&) = delete;
	mem_ref_base& operator=(mem_ref_base&&) = delete;

	// ── Block all arithmetic/comparison/bitwise operators ────────────────
	// Prevents silent implicit-conversion-then-operate bugs.
	// If you hit one of these errors, use .get() or .ptr() explicitly.
	template<typename T> T& operator++() = delete;
	template<typename T> T& operator--() = delete;
	template<typename T> T operator++(T) = delete;
	template<typename T> T operator--(T) = delete;
	template<typename T> bool operator== (T&&) const = delete;
	template<typename T> bool operator!= (T&&) const = delete;
	template<typename T> bool operator<  (T&&) const = delete;
	template<typename T> bool operator>  (T&&) const = delete;
	template<typename T> bool operator<= (T&&) const = delete;
	template<typename T> bool operator>= (T&&) const = delete;
	template<typename T> auto operator+  (T&&) const = delete;
	template<typename T> auto operator-  (T&&) const = delete;
	template<typename T> auto operator*  (T&&) const = delete; // binary multiply, not unary deref
	template<typename T> auto operator/  (T&&) const = delete;
	template<typename T> auto operator%  (T&&) const = delete;
	template<typename T> auto operator&  (T&&) const = delete; // binary AND, not unary addr-of
	template<typename T> auto operator|  (T&&) const = delete;
	template<typename T> auto operator^  (T&&) const = delete;
	template<typename T> auto operator<< (T&&) const = delete;
	template<typename T> auto operator>> (T&&) const = delete;
	operator bool() const = delete;
};

// ============================================================================
// Forward declarations (needed for is_mem_ref specialisations)
// ============================================================================
template <typename T, unsigned int Address>
struct constant_ptr;

template <typename T, unsigned int Address, size_t Count = 0>
struct reference;

template <typename T, unsigned int Address, size_t CountX, size_t CountY>
struct reference2D;

template <typename T, unsigned int Address>
struct referencefunc;

// Register all wrappers with the trait
template<typename T, unsigned int A>
struct is_mem_ref<constant_ptr<T, A>> : std::true_type {};

template<typename T, unsigned int A, size_t C>
struct is_mem_ref<reference<T, A, C>> : std::true_type {};

template<typename T, unsigned int A, size_t CX, size_t CY>
struct is_mem_ref<reference2D<T, A, CX, CY>> : std::true_type {};

template<typename T, unsigned int A>
struct is_mem_ref<referencefunc<T, A>> : std::true_type {};


// ============================================================================
// constant_ptr<T, Address>
//
// A compile-time typed pointer to a fixed address.
// Use when the game stores a pointer at a known address.
// ============================================================================
template <typename T, unsigned int Address>
struct constant_ptr : private mem_ref_base
{
	using value_type = T*;

	COMPILETIMEEVAL constant_ptr() noexcept = default;
	constant_ptr(constant_ptr&) = delete;
private:
	COMPILETIMEEVAL constant_ptr(noinit_t) noexcept {}
public:

	FORCEDINLINE COMPILETIMEEVAL DWORD getAddrs() const noexcept
	{
		return Address;
	}

	// Returns the pointer stored at Address
	FORCEDINLINE COMPILETIMEEVAL value_type get() const noexcept
	{
		return reinterpret_cast<value_type>(Address);
	}

	// ptr() mirrors get() for consistency with reference<>
	FORCEDINLINE COMPILETIMEEVAL value_type ptr() const noexcept
	{
		return get();
	}

	FORCEDINLINE COMPILETIMEEVAL operator value_type() const noexcept { return get(); }
	FORCEDINLINE COMPILETIMEEVAL value_type operator()() const noexcept { return get(); }
	FORCEDINLINE COMPILETIMEEVAL value_type operator->() const noexcept { return get(); }
	FORCEDINLINE COMPILETIMEEVAL T& operator*() const noexcept { return *get(); }

	// operator&() intentionally absent — see mem_ref_base comment
};


// ============================================================================
// reference<T, Address, Count>   — ARRAY specialisation
//
// Use when the game stores a fixed-size array at a known address.
// e.g. reference<WORD, 0x12345, 256>
// ============================================================================
template <typename T, unsigned int Address, size_t Count>
struct reference : private mem_ref_base
{
	using value_type = T[Count];
	using pointer = T*;

	static const auto Size = Count;

	COMPILETIMEEVAL reference() noexcept = default;
	reference(reference&) = delete;
private:
	COMPILETIMEEVAL reference(noinit_t) noexcept {}
public:

	FORCEDINLINE COMPILETIMEEVAL DWORD getAddrs() const noexcept
	{
		return Address;
	}

	FORCEDINLINE COMPILETIMEEVAL value_type& get() const noexcept
	{
		static auto const address = Address;
		return *reinterpret_cast<value_type*>(address);
	}

	// ptr() — unambiguous way to obtain T* regardless of call context
	FORCEDINLINE COMPILETIMEEVAL pointer ptr() const noexcept
	{
		static auto const address = Address;
		return reinterpret_cast<pointer>(address);
	}

	FORCEDINLINE COMPILETIMEEVAL operator value_type& () const noexcept { return get(); }

	// Implicit T* conversion — safe replacement for the removed operator&()
	// Ensures template helpers get the game address, never the wrapper address
	FORCEDINLINE COMPILETIMEEVAL operator pointer() const noexcept { return ptr(); }

	FORCEDINLINE COMPILETIMEEVAL value_type& operator()()  const noexcept { return get(); }
	FORCEDINLINE COMPILETIMEEVAL decltype(auto) operator*() const noexcept { return *get(); }
	FORCEDINLINE COMPILETIMEEVAL T& operator[](int index)  const noexcept { return get()[index]; }

	FORCEDINLINE COMPILETIMEEVAL value_type& data()  const noexcept { return get(); }
	FORCEDINLINE COMPILETIMEEVAL size_t size()       const noexcept { return Size; }
	FORCEDINLINE COMPILETIMEEVAL size_t c_size()     const noexcept { return Count; }
	FORCEDINLINE COMPILETIMEEVAL T* begin()          const noexcept { return ptr(); }
	FORCEDINLINE COMPILETIMEEVAL T* end()            const noexcept { return ptr() + Size; }

	// operator&() intentionally absent — see mem_ref_base comment
};


// ============================================================================
// reference<T, Address, 0>   — SCALAR specialisation  (most common)
//
// Use when the game stores a single value at a known address.
// e.g. reference<int,    0x8205D0u>  RGBMode
//      reference<DSurface*, 0x887300u>  Sidebar
//      reference<RectangleStruct, 0x886FA0u>  ViewBounds
// ============================================================================
template <typename T, unsigned int Address>
struct reference<T, Address, 0> : private mem_ref_base
{
	using value_type = T;
	using pointer = T*;

	COMPILETIMEEVAL reference() noexcept = default;
	reference(reference&) = delete;
private:
	COMPILETIMEEVAL reference(noinit_t) noexcept {}
public:

	FORCEDINLINE COMPILETIMEEVAL DWORD getAddrs() const noexcept
	{
		return Address;
	}

	// get() — the canonical way to obtain a T& reference to the game value
	FORCEDINLINE COMPILETIMEEVAL value_type& get() const noexcept
	{
		return *reinterpret_cast<value_type*>(Address);
	}

	// ptr() — the canonical way to obtain a T* pointer to the game value.
	//
	// CRASH STORY: the original code had operator&() overloaded to return this.
	// In a plain call like &ViewBounds it worked correctly.
	// But inside any template/generic helper, the compiler routes & through
	// std::addressof(), which bypasses operator&() and returns the wrapper's
	// own address — not the game address. Crash guaranteed.
	//
	// The fix: remove operator&() entirely and expose ptr() + operator T*()
	// so there is exactly one unambiguous pointer path regardless of context.
	FORCEDINLINE COMPILETIMEEVAL pointer ptr() const noexcept
	{
		return reinterpret_cast<pointer>(Address);
	}

	// ── assignment ────────────────────────────────────────────────────────
	template <typename T2, typename = std::enable_if_t<std::is_assignable<T&, T2>::value>>
	FORCEDINLINE value_type& operator=(T2&& rhs) const
	{
		return get() = std::forward<T2>(rhs);
	}

	// ── implicit conversions ──────────────────────────────────────────────
	FORCEDINLINE COMPILETIMEEVAL operator value_type& () const noexcept { return get(); }

	// This is what fixes the viewport pointer bug:
	// Any function taking T* will now receive the game address via this
	// conversion, even when the call goes through a template that has
	// deduced T = reference<...>.
	FORCEDINLINE COMPILETIMEEVAL operator pointer() const noexcept { return ptr(); }

	FORCEDINLINE COMPILETIMEEVAL T& operator()() const noexcept { return get(); }

	FORCEDINLINE COMPILETIMEEVAL decltype(auto) operator->() const noexcept
	{
		if COMPILETIMEEVAL(std::is_pointer<T>::value)
			return get();
		else
			return ptr();
	}

	FORCEDINLINE COMPILETIMEEVAL decltype(auto) operator*()           const noexcept { return *get(); }
	FORCEDINLINE COMPILETIMEEVAL decltype(auto) operator[](int index) const noexcept { return get()[index]; }

	// operator&() intentionally absent — see mem_ref_base comment
};


// ============================================================================
// reference2D<T, Address, CountX, CountY>
//
// Use when the game stores a 2D array at a known address.
// ============================================================================
template <typename T, unsigned int Address, size_t CountX, size_t CountY>
struct reference2D : private mem_ref_base
{
	using value_type = T[CountX][CountY];
	using pointer = T*;

	COMPILETIMEEVAL reference2D() noexcept = default;
	reference2D(reference2D&) = delete;
private:
	COMPILETIMEEVAL reference2D(noinit_t) noexcept {}
public:

	FORCEDINLINE COMPILETIMEEVAL DWORD getAddrs() const noexcept
	{
		return Address;
	}

	FORCEDINLINE value_type& get() const noexcept
	{
		static auto const address = Address;
		return *reinterpret_cast<value_type*>(address);
	}

	FORCEDINLINE pointer ptr() const noexcept
	{
		static auto const address = Address;
		return reinterpret_cast<pointer>(address);
	}

	FORCEDINLINE operator value_type& ()          const noexcept { return get(); }
	FORCEDINLINE operator pointer() const noexcept { return ptr(); }
	FORCEDINLINE value_type& operator()()        const noexcept { return get(); }
	FORCEDINLINE decltype(auto) operator*()      const noexcept { return *get(); }
	FORCEDINLINE value_type& data()              const noexcept { return get(); }

	// operator&() intentionally absent — see mem_ref_base comment
};


// ============================================================================
// referencefunc<T, Address>
//
// Use when you need to read or patch a function pointer at a known address.
// T should be a function pointer type, e.g. void(__cdecl*)(int, int)
// ============================================================================
template <typename T, unsigned int Address>
struct referencefunc : private mem_ref_base
{
	using value_type = T;

	COMPILETIMEEVAL referencefunc() noexcept = default;
	referencefunc(referencefunc&) = delete;
private:
	COMPILETIMEEVAL referencefunc(noinit_t) noexcept {}
public:

	COMPILETIMEEVAL FORCEDINLINE DWORD getAddrs()       const noexcept { return Address; }
	COMPILETIMEEVAL FORCEDINLINE uintptr_t getAddress()       noexcept { return Address; }

	// Cast Address directly to T (when Address IS the function)
	FORCEDINLINE value_type asT() const noexcept
	{
		return reinterpret_cast<value_type>(Address);
	}

	// Dereference Address as a T* (when Address holds a pointer TO the function)
	FORCEDINLINE value_type invoke() const noexcept
	{
		return *reinterpret_cast<value_type*>(Address);
	}

	// Patch the function pointer at Address with RWX memory protection
	template <typename T2>
	FORCEDINLINE bool operator=(T2 rhs) const
	{
		DWORD protection = PAGE_EXECUTE_READWRITE;
		DWORD protectionb {};
		if (VirtualProtect((LPVOID)Address, sizeof(LPVOID), protection, &protection) == TRUE)
		{
			*reinterpret_cast<LPVOID*>(Address) = rhs;
			VirtualProtect((LPVOID)Address, sizeof(LPVOID), protection, &protectionb);
			FlushInstructionCache(
				*reinterpret_cast<HINSTANCE*>(0xB732F0u),
				(LPVOID)Address,
				sizeof(LPVOID)
			);
			return true;
		}
		return false;
	}

	// operator&() intentionally absent — see mem_ref_base comment
};