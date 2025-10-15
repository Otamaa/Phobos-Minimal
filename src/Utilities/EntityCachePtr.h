#pragma once

#include <Base/Always.h>
#include "SavegameDef.h"

struct EntityCachePtrCallbacks
{
	// User must define this function to handle component removal notifications
	// Example implementation:
	// static void onComponentRemoved(entt::entity entity) { /* refresh all pointers */ }
	static void onComponentRemoved(entt::entity entity);

	// Optional: callback for component relocation (e.g., after storage compaction)
	// static void onComponentRelocated(entt::entity entity) { /* refresh pointer */ }
};

template<typename T, bool check>
class EntityCachePtr
{
private:
	T* ptr_;

	// Validation check - only compiled when check=true
	bool isValid(entt::entity entity) const
	{
		if constexpr (check)
		{
			if (!ptr_) return false;

			// Verify entity still exists and has the component
			if (!Phobos::gEntt->valid(entity))
				return false;

			if (!Phobos::gEntt->all_of<T>(entity))
				return false;

			// Verify pointer still points to the correct component
			T* current = Phobos::gEntt->try_get<T>(entity);
			return current == ptr_;
		}
		return ptr_ != nullptr;
	}

public:
	// Constructors
	EntityCachePtr() : ptr_(nullptr) { }
	explicit EntityCachePtr(T* p) : ptr_(p) { }
	EntityCachePtr(std::nullptr_t) : ptr_(nullptr) { }

	// Copy and move - deleted to prevent issues
	EntityCachePtr(const EntityCachePtr& other) = delete;
	EntityCachePtr& operator=(const EntityCachePtr& other) = delete;
	EntityCachePtr(EntityCachePtr&& other) noexcept = delete;

	// Assignment from entity - replaces component
	EntityCachePtr& operator=(entt::entity entity)
	{
		if (ptr_)
		{
			Phobos::gEntt->remove<T>(entity);
			//NotifyEntityCacheRefresh(entity); // Notify system
			ptr_ = nullptr;
		}

		ptr_ = &Phobos::gEntt->emplace<T>(entity);
		return *this;
	}

	// Dereference operators
	T& operator*() const
	{
		if constexpr (check)
		{
			if (!ptr_)
			{
				throw std::runtime_error("EntityCachePtr: Dereferencing null pointer");
			}
		}
		return *ptr_;
	}

	T* operator->() const
	{
		if constexpr (check)
		{
			if (!ptr_)
			{
				throw std::runtime_error("EntityCachePtr: Dereferencing null pointer");
			}
		}
		return ptr_;
	}

	// Array subscript operator
	T& operator[](size_t idx) const { return ptr_[idx]; }

	// Implicit conversion to raw pointer
	operator T* () const { return ptr_; }

	// Boolean conversion (for if statements)
	explicit operator bool() const { return ptr_ != nullptr; }

	// Comparison operators
	bool operator==(const EntityCachePtr& other) const { return ptr_ == other.ptr_; }
	bool operator!=(const EntityCachePtr& other) const { return ptr_ != other.ptr_; }
	bool operator==(std::nullptr_t) const { return ptr_ == nullptr; }
	bool operator!=(std::nullptr_t) const { return ptr_ != nullptr; }
	bool operator==(T* other) const { return ptr_ == other; }
	bool operator!=(T* other) const { return ptr_ != other; }

	// Less than operators (for maps/sets)
	bool operator<(const EntityCachePtr& other) const { return ptr_ < other.ptr_; }
	bool operator<=(const EntityCachePtr& other) const { return ptr_ <= other.ptr_; }
	bool operator>(const EntityCachePtr& other) const { return ptr_ > other.ptr_; }
	bool operator>=(const EntityCachePtr& other) const { return ptr_ >= other.ptr_; }

	// Get raw pointer (explicit method)
	T* get() const { return ptr_; }

	// Check if pointer is valid (uses validation if check=true)
	bool valid(entt::entity entity) const
	{
		return isValid(entity);
	}

	// Refresh pointer from entity (call this after component relocations)
	bool refresh(entt::entity entity)
	{
		if (!Phobos::gEntt->valid(entity))
			return false;

		ptr_ = Phobos::gEntt->try_get<T>(entity);
		return ptr_ != nullptr;
	}

	// Construct component on entity with forwarded arguments
	template<typename... Args>
	void construct(entt::entity entity, Args&&... args)
	{
		ptr_ = &Phobos::gEntt->emplace<T>(entity, std::forward<Args>(args)...);
	}

	template<typename... Args>
	void constructmove(entt::entity entity)
	{
		T temp;
		ptr_ = &Phobos::gEntt->emplace<T>(entity, std::move(temp));
	}

	// Destroy (remove) component from entity
	void destroy(entt::entity entity)
	{
		Phobos::gEntt->remove<T>(entity);
		//NotifyEntityCacheRefresh(entity); // Notify system
		ptr_ = nullptr;
	}

	bool save(PhobosStreamWriter& Stm)
	{
		if constexpr (!check)
		{
			if (!Savegame::WritePhobosStream(Stm, (long)this->ptr_))
				return false;

			static_assert(Savegame::detail::HasSave<T> || Savegame::detail::Hassave<T>,
				"Savegame::PhobosStreamObject<EntityCachePtr<T>>: Type must implement Load/load returning bool");

			return Savegame::WritePhobosStream(Stm, *this->ptr_);
		}
		else
		{
			const bool hasValue = this->ptr_ != nullptr;

			if (!Savegame::WritePhobosStream(Stm, hasValue))
				return false;

			if (hasValue)
			{
				if (!Savegame::WritePhobosStream(Stm, (long)this->ptr_))
					return false;

				static_assert(Savegame::detail::HasSave<T> || Savegame::detail::Hassave<T>,
					"Savegame::PhobosStreamObject<EntityCachePtr<T>>: Type must implement Load/load returning bool");

				return Savegame::WritePhobosStream(Stm, *this->ptr_);
			}

			return true;
		}
	}

	bool load(PhobosStreamReader& Stm, entt::entity holder)
	{
		if constexpr (check)
		{
			bool hasValue = false;
			if (!Savegame::ReadPhobosStream(Stm, hasValue))
				return false;

			if (hasValue)
			{
				long ptrOld = 0l;
				if (!Savegame::ReadPhobosStream(Stm, ptrOld))
					return false;

				this->operator=(holder);

				if (!Savegame::ReadPhobosStream(Stm, *this->ptr_, true))
					return false;

				PHOBOS_SWIZZLE_REGISTER_POINTER(ptrOld, this->ptr_, PhobosCRT::GetTypeIDName<T>().c_str())
			}

			return true;
		}
		else
		{
			long ptrOld = 0l;
			if (!Savegame::ReadPhobosStream(Stm, ptrOld))
				return false;

			this->operator=(holder);

			if (!Savegame::ReadPhobosStream(Stm, *this->ptr_, true))
				return false;

			PHOBOS_SWIZZLE_REGISTER_POINTER(ptrOld, this->ptr_, PhobosCRT::GetTypeIDName<T>().c_str())
				return true;
		}
	}
};