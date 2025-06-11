#pragma once

#include <Base/Always.h>

#include <ASMMacros.h>

#include <stdlib.h>

#include <memory>
#include <type_traits>
#include <utility>
#include <Helpers/CompileTime.h>
/*
 * The memory (de)allocators have to match!
 * Do not allocate memory in the DLL and hand it to the game to deallocate, or vice versa.
 * Kapiche?

 * A simple |auto foo = new IngameClass();| allocates memory from the DLL's pool.
 * But |delete foo;| deallocates memory from the game's own pool. (assuming the class you're freeing has a virtual SDDTOR)

 * So use the macros to make sure game classes go to the game's pool.
 * The custom classes like ExtMap do not need this treatment so you can use the plain old new/delete on them.

 * For the ObjectClass derivates, if you use the game's built-in allocators like | Type->CreateObject() | ,
 * you can use plain | delete |;

 */

/*
 * OK, new plan - the game's operator new/delete has been hooked to redirect to the DLL's
 * so GAME_(DE)ALLOC is now just a wrapper. Don't remove it though, just in case this fails
 * and I need to run those allocations differently.
 */

/*
 * Newer plan - previous hook screwed performance, so going back
 */

/*
* Yet a newer plan - use variadic templates
*/

// provides access to the game's operator new and operator delete.
namespace YRMemory {

	OPTIONALINLINE NAKED void* __cdecl Allocate(size_t sz , int flag= 1) {
		JMP(0x7C9442);
    }

	OPTIONALINLINE void* __cdecl MAllocate(size_t sz) {
		return Allocate(sz , *reinterpret_cast<int*>(0xB782C4));
	}

    OPTIONALINLINE NAKED void __cdecl Deallocate( void* mem) {
		JMP(0x7C93E8);
    }

  //  OPTIONALINLINE void* AllocateChecked(size_t sz) {
  //     // if (auto const ptr = YRMemory::Allocate(sz)) {
  //     //     return ptr;
  //     // }
  //     // std::exit(static_cast<int>(0x30000000u | sz));
		//return YRMemory::Allocate(sz);
  //  }
}

template<typename T>
struct needs_vector_delete : std::integral_constant<bool,
	!std::is_scalar<T>::value && !std::is_trivially_destructible<T>::value> {};

enum class AllocatorType : unsigned char
{
	GameAllocator = 0 ,
	DllAllocator = 1
};
// this is a stateless basic allocator definition that manages memory using the
// game's operator new and operator delete methods. do not use it directly,
// though. use std::allocator_traits, which will fill in the blanks.
template <typename T>
struct GameAllocator {
	using value_type = T;

	static_assert(!std::is_const_v<T>, "The C++ Standard forbids containers of const elements "
								"because allocator<const T> is ill-formed.");
	static_assert(!std::is_function_v<T>, "The C++ Standard forbids allocators for function elements "
									   "because of [allocator.requirements].");
	static_assert(!std::is_reference_v<T>, "The C++ Standard forbids allocators for reference elements "
										"because of [allocator.requirements].");

	static const AllocatorType AllocType = AllocatorType::GameAllocator;
	COMPILETIMEEVAL GameAllocator() noexcept = default;

	template <typename U>
	COMPILETIMEEVAL GameAllocator(const GameAllocator<U>&) noexcept {}

	_CONSTEXPR20 ~GameAllocator() = default;
	_CONSTEXPR20 GameAllocator& operator=(const GameAllocator&) = default;

	COMPILETIMEEVAL bool operator == (const GameAllocator&) const noexcept { return true; }
	COMPILETIMEEVAL bool operator != (const GameAllocator&) const noexcept { return false; }

	T* allocate(const size_t count) const noexcept {
		return static_cast<T*>(YRMemory::Allocate(count * sizeof(T)));
	}

	void destroy(T* const ptr) const noexcept {
		std::destroy_at(ptr);
	}

	void deallocate(T* const ptr, size_t count) const noexcept {
		YRMemory::Deallocate(ptr);
	}

	template<class U, class... Args>
	void construct(U* p, Args&&... args)  const noexcept {
		std::construct_at(p, std::forward<Args>(args)...);
	}

};

template <typename T>
struct DllAllocator : public std::allocator<T>{
	static const AllocatorType AllocType = AllocatorType::DllAllocator;
};

// construct or destroy objects using an allocator.
class Memory {
public:
	// construct scalars
	template <typename T, typename TAlloc, typename... TArgs>
	static OPTIONALINLINE T* Create(TAlloc& alloc, TArgs&&... args) {
		auto const ptr = std::allocator_traits<TAlloc>::allocate(alloc, 1);
		std::allocator_traits<TAlloc>::construct(alloc, ptr, std::forward<TArgs>(args)...);
		return ptr;
	};

	template <typename T, typename TAlloc, typename... TArgs>
	static OPTIONALINLINE void ConstructAt(TAlloc& alloc, T* ptr , TArgs&&... args)  {
		std::allocator_traits<TAlloc>::construct(alloc, ptr, std::forward<TArgs>(args)...);
	};

	// destruct scalars
	template<bool calldtor = false , typename T, typename TAlloc>
	static OPTIONALINLINE void Delete(std::true_type,TAlloc& alloc, T* ptr) {
		if(ptr) {
			if COMPILETIMEEVAL (calldtor)
			  std::allocator_traits<TAlloc>::destroy(alloc, ptr);

			std::allocator_traits<TAlloc>::deallocate(alloc, ptr, 1);
		}
	};

	template<bool calldtor = false, typename T, typename TAlloc>
	static OPTIONALINLINE void Delete(std::false_type ,TAlloc& alloc, T* ptr)
	{
		if COMPILETIMEEVAL (calldtor)
			std::allocator_traits<TAlloc>::destroy(alloc, ptr);

		std::allocator_traits<TAlloc>::deallocate(alloc, ptr, 1);
	};

	// construct vectors
	template <typename T, typename TAlloc, typename... TArgs>
	static OPTIONALINLINE T* CreateArray(TAlloc& alloc, size_t capacity, TArgs&&... args) {

		auto const ptr = std::allocator_traits<TAlloc>::allocate(alloc, capacity);

		if (capacity)
		{
			if COMPILETIMEEVAL (!(sizeof...(args)))
			{
				std::memset(ptr, 0, capacity * sizeof(T));
			} else {
				for (size_t i = 0; i < capacity; ++i) {
					// use args... here. can't move args, because we need to reuse them
					std::allocator_traits<TAlloc>::construct(alloc, &ptr[i], args...);
				}
			}
		}

		return ptr;
	}

	// construct vectors
	template <typename T, typename TAlloc, typename... TArgs>
	static OPTIONALINLINE T* CreateArrayAt(TAlloc& alloc, T* ptr, size_t capacity, TArgs&&... args)
	{
		if (capacity)
		{
			if COMPILETIMEEVAL (!(sizeof...(args)))
			{
				std::memset(ptr, 0, capacity * sizeof(T));
			}
			else
			{
				for (size_t i = 0; i < capacity; ++i)
				{
					// use args... here. can't move args, because we need to reuse them
					std::allocator_traits<TAlloc>::construct(alloc, &ptr[i], args...);
				}
			}
		}

		return ptr;
	}


	// destruct vectors
	template<typename T, typename TAlloc>
	static OPTIONALINLINE void DeleteArray(TAlloc& alloc, T* ptr, size_t capacity) {
		if(ptr) {
			// call the destructor if required
			if(capacity && !std::is_trivially_destructible<T>::value) {
				for(size_t i = 0; i < capacity; ++i) {
					std::allocator_traits<TAlloc>::destroy(alloc, &ptr[i]);
				}
			}

			std::allocator_traits<TAlloc>::deallocate(alloc, ptr, capacity);
		}
	};
};

// helper methods as free functions.

template <typename T, typename... TArgs>
static OPTIONALINLINE T* GameCreateUnchecked(TArgs&&... args)
{
	static_assert(std::is_constructible<T, TArgs...>::value, "Cannot construct T from TArgs.");

	GameAllocator<T> alloc {};
	return Memory::Create<T>(alloc, std::forward<TArgs>(args)...);
}

template <typename T, typename... TArgs>
static OPTIONALINLINE T* GameCreate(TArgs&&... args) {
	static_assert(std::is_constructible<T, TArgs...>::value, "Cannot construct T from TArgs.");

	GameAllocator<T> alloc {};
	return Memory::Create<T>(alloc, std::forward<TArgs>(args)...);
}

//Construct an This Object to an avaible memort space !
//be aware that this calling `new` on debug mode
//it will get optimize out on release mode !
template <typename T, typename... TArgs>
static OPTIONALINLINE void GameConstruct(T* AllocatedSpace , TArgs&&... args)
{
	static_assert(std::is_constructible<T, TArgs...>::value, "Cannot construct T from TArgs.");

	//AllocatedSpace->T::template T(std::forward<TArgs>(args)...);
	GameAllocator<T> alloc {};
	Memory::ConstructAt<T>(alloc, AllocatedSpace, std::forward<TArgs>(args)...);
}

template<bool calldtor = false,bool check = true, typename T>
static OPTIONALINLINE void GameDelete(T* ptr) {
	GameAllocator<T> alloc {};
	Memory::Delete<calldtor>(std::bool_constant<check>::type(), alloc, ptr);
}

template<bool check = true , typename T>
static OPTIONALINLINE void CallDTOR(T* ptr) {
	GameAllocator<T> alloc {};
	Memory::Delete<true>(std::bool_constant<check>::type(), alloc, ptr);
}

template <typename T, typename... TArgs>
static OPTIONALINLINE T* GameCreateArray(size_t capacity, TArgs&&... args) {
	static_assert(std::is_constructible<T, TArgs...>::value, "Cannot construct T from TArgs.");

	GameAllocator<T> alloc {};
	return Memory::CreateArray<T>(alloc, capacity, std::forward<TArgs>(args)...);
}

template <typename T, typename... TArgs>
static OPTIONALINLINE T* GameConstructArray(T* AllocatedSpace, size_t capacity, TArgs&&... args)
{
	static_assert(std::is_constructible<T>::value, "Cannot construct T from TArgs.");

	GameAllocator<T> alloc {};
	return Memory::CreateArrayAt<T>(alloc, AllocatedSpace , capacity, std::forward<TArgs>(args)...);
}

template<typename T>
static OPTIONALINLINE void GameDeleteArray(T* ptr, size_t capacity) {
	GameAllocator<T> alloc {};
	Memory::DeleteArray(alloc, ptr, capacity);
}

template <typename T, typename... TArgs>
static OPTIONALINLINE T* DLLCreate(TArgs&&... args) {
	static_assert(std::is_constructible<T, TArgs...>::value, "Cannot construct T from TArgs.");

	DllAllocator<T> alloc {};
	return Memory::Create<T>(alloc, std::forward<TArgs>(args)...);
}


template <typename T, typename... TArgs>
static OPTIONALINLINE T* DLLCreateArray(size_t capacity, TArgs&&... args)
{
	static_assert(std::is_constructible<T, TArgs...>::value, "Cannot construct T from TArgs.");

	DllAllocator<T> alloc {};
	return Memory::CreateArray<T>(alloc, capacity, std::forward<TArgs>(args));
}

template<typename T>
static OPTIONALINLINE void DLLDeleteArray(T* ptr, size_t capacity)
{
	DllAllocator<T> alloc {};
	Memory::DeleteArray(alloc, ptr, capacity);
}

template<bool check = true , typename T>
static OPTIONALINLINE void DLLDelete(T* ptr) {
	DllAllocator<T> alloc {};
	Memory::Delete(std::bool_constant<check>::type(), alloc, ptr);
}

template<bool check = true, typename T>
static OPTIONALINLINE void DLLCallDTOR(T* ptr)
{
	DllAllocator<T> alloc {};
	Memory::Delete<true>(std::bool_constant<check>::type(), alloc, ptr);
}

enum class DeleterType : int
{
	DllDeleter = 0,
	DllDTorCaller,
	GameDeleter,
	GameDleterWithDTOR,
	GameDTORCaller
};

struct GameDeleter {
	static COMPILETIMEEVAL DeleterType DeleterType = DeleterType::GameDeleter;

	template <typename T>
	void operator ()(T* ptr) noexcept {
		GameDelete(ptr);
		ptr = nullptr;
	}
};

struct GameDeleterWithDTOR {
	static COMPILETIMEEVAL DeleterType DeleterType = DeleterType::GameDleterWithDTOR;

	template <typename T>
	void operator ()(T* ptr) noexcept
	{
		GameDelete<true,true>(ptr);
		ptr = nullptr;
	}
};

struct GameDTORCaller {
	static COMPILETIMEEVAL DeleterType DeleterType = DeleterType::GameDTORCaller;

	template <typename T>
	void operator ()(T* ptr) noexcept {
		CallDTOR(ptr);
		ptr = nullptr;
	}
};

struct DLLDeleter
{
	static COMPILETIMEEVAL DeleterType DeleterType = DeleterType::DllDeleter;

	template <typename T>
	void operator ()(T* ptr) noexcept {
		if (ptr) {
			DLLDelete(ptr);
			ptr = nullptr;
		}
	}
};

struct DLLDTORCaller
{
	static COMPILETIMEEVAL DeleterType DeleterType = DeleterType::DllDTorCaller;

	template <typename T>
	void operator ()(T* ptr) noexcept {
		DLLCallDTOR(ptr);
		ptr = nullptr;
	}
};

template <typename T>
static FORCEDINLINE T* DLLAllocWithoutCTOR() {
	DllAllocator<T> alloc {};
	return (T*)std::allocator_traits<DllAllocator<T>>::allocate(alloc, 1);
}