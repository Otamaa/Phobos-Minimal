#pragma once

#include "Savegame.h"

#include <type_traits>
#include <utility>
#include <vector>
#include <algorithm>

struct Handles
{
	static std::vector<Handles*> Array;

	// Index into Array, for O(1) swap-and-pop deregister. -1 = not registered.
	std::size_t RegistryIndex = static_cast<std::size_t>(-1);

	virtual ~Handles() noexcept = default;

	// Drop the owned resource WITHOUT deleting it (shutdown safety).
	virtual void detachptr() = 0;

	// Tell EVERY live handle to let go of its resource. Call once, at game
	// termination, BEFORE the game's own teardown frees things underneath us.
	// VERIFY: hook this at your process-shutdown point (mirrors your
	// ExceptionHandler "TerminateProcess bypasses cleanup" philosophy).
	static void DetachAll() noexcept
	{
		for (auto* pHandle : Array)
			pHandle->detachptr();
	}

protected:
	void RegistrySelf() noexcept
	{
		this->RegistryIndex = Array.size();
		Array.emplace_back(this);
	}

	void UnregistrySelf() noexcept
	{
		if (this->RegistryIndex == static_cast<std::size_t>(-1))
			return;

		const std::size_t last = Array.size() - 1;
		if (this->RegistryIndex != last)
		{
			// Move the tail element into our slot and fix its index.
			Array[this->RegistryIndex] = Array[last];
			Array[this->RegistryIndex]->RegistryIndex = this->RegistryIndex;
		}

		Array.pop_back();
		this->RegistryIndex = static_cast<std::size_t>(-1);
	}
};

// owns a resource. not copyable, but movable.
template <typename T, typename Deleter, T Default = T()>
struct Handle final : public Handles
{
	Handle() noexcept
		: Handles()
		, Value(Default)
	{
		this->RegistrySelf();
	}

	explicit Handle(T value) noexcept
		: Handles()
		, Value(value)
	{
		this->RegistrySelf();
	}

	Handle(const Handle&) = delete;

	// BUGFIX: original stole the value but never registered `this`.
	Handle(Handle&& other) noexcept
		: Handles()
		, Value(other.release())
	{
		this->RegistrySelf();
		// `other` stays registered with Value == Default; its dtor is a safe
		// no-op (no delete) and correctly deregisters itself.
	}

	~Handle() noexcept override
	{
		if (this->Value != Default)
			Deleter {}(this->Value);

		this->Value = Default;
		this->UnregistrySelf(); // BUGFIX + PERF: O(1), single element only.
	}

	Handle& operator=(const Handle&) = delete;

	Handle& operator=(Handle&& other) noexcept
	{
		if (this != &other)
			this->reset(other.release());

		return *this;
	}

	COMPILETIMEEVAL explicit operator bool() const noexcept { return this->Value != Default; }
	COMPILETIMEEVAL operator T() const noexcept { return this->Value; }
	COMPILETIMEEVAL T get() const noexcept { return this->Value; }
	COMPILETIMEEVAL T operator->() const noexcept { return this->get(); }

	COMPILETIMEEVAL T release() noexcept { return std::exchange(this->Value, Default); }

	void reset(T value = Default) noexcept
	{
		if (this->Value != Default)
			Deleter {}(this->Value);

		this->Value = value;
	}

	// release the reference WITHOUT deleting - avoids double-delete on shutdown.
	void detachptr() noexcept override { this->Value = Default; }

	void swap(Handle& other) noexcept
	{
		using std::swap;
		swap(this->Value, other.Value);
	}

	friend void swap(Handle& lhs, Handle& rhs) noexcept { lhs.swap(rhs); }

	bool load(PhobosStreamReader& Stm, bool RegisterForChange)
	{
		return Stm.Process(this->Value, RegisterForChange).Success();
	}

	bool save(PhobosStreamWriter& Stm) const
	{
		return Stm.Process(this->Value).Success();
	}

private:
	T Value { Default };
};

struct WeakRefNodeBase;
// Lives on the TARGET's extension data - one head per observable object.
struct WeakRefList
{
	WeakRefNodeBase* Head = nullptr;

	// Called from the target's death path. Walks the list ONCE, nulls every
	// observer, and clears the head. Does NOT per-element unlink (that would
	// mutate the list mid-walk); it detaches wholesale so no node keeps a
	// pointer back into this soon-to-be-freed head.
	void DetachAll() noexcept;
};

struct WeakRefNodeBase
{
	WeakRefNodeBase* Prev = nullptr;
	WeakRefNodeBase* Next = nullptr;
	WeakRefList* Owner = nullptr;

	virtual ~WeakRefNodeBase() noexcept = default;

	// Derived nulls its own stored value here.
	virtual void detachptr() noexcept = 0;

	void Link(WeakRefList* list) noexcept
	{
		if (list == nullptr)
			return; // target has no ref-list (e.g. no ext data) - stay unlinked.

		this->Owner = list;
		this->Prev = nullptr;
		this->Next = list->Head;

		if (list->Head != nullptr)
			list->Head->Prev = this;

		list->Head = this;
	}

	void Unlink() noexcept
	{
		if (this->Owner == nullptr)
			return; // not linked - nothing to do.

		if (this->Prev != nullptr)
			this->Prev->Next = this->Next;
		else
			this->Owner->Head = this->Next; // we were the head.

		if (this->Next != nullptr)
			this->Next->Prev = this->Prev;

		this->Prev = nullptr;
		this->Next = nullptr;
		this->Owner = nullptr;
	}
};

inline void WeakRefList::DetachAll() noexcept
{
	WeakRefNodeBase* node = this->Head;
	while (node != nullptr)
	{
		WeakRefNodeBase* next = node->Next;

		node->detachptr();   // null the observer's stored value
		node->Prev = nullptr;
		node->Next = nullptr;
		node->Owner = nullptr; // must not reference this head after we return

		node = next;
	}

	this->Head = nullptr;
}

// -----------------------------------------------------------------------------
// Traits: maps a target value -> its WeakRefList*. Specialize / replace for
// your extension containers.
//
// VERIFY: wire GetList to your real ext-data accessor, e.g. for AbstractClass*
//   return &AbstractExt::ExtMap.Find(value)->WeakRefs;
// Return nullptr when the target has no ext data - Link() guards for that.
// -----------------------------------------------------------------------------
template <typename T>
struct WeakHandleTraits
{
	static WeakRefList* GetList(T /*value*/) noexcept
	{
		// VERIFY: replace with the real per-target list lookup. Returning
		// nullptr here means the handle simply won't self-invalidate.
		return nullptr;
	}
};

template <typename T, typename Traits = WeakHandleTraits<T>, T Default = T()>
struct WeakHandle final : public WeakRefNodeBase
{
	WeakHandle() noexcept
		: WeakRefNodeBase()
		, Value(Default)
	{}

	explicit WeakHandle(T value) noexcept
		: WeakRefNodeBase()
		, Value(value)
	{
		this->Attach();
	}

	WeakHandle(const WeakHandle& other) noexcept
		: WeakRefNodeBase()
		, Value(other.Value)
	{
		this->Attach(); // link into the SAME target's list.
	}

	WeakHandle(WeakHandle&& other) noexcept
		: WeakRefNodeBase()
		, Value(other.Value)
	{
		this->Attach();
		other.Clear(); // unlink + null the source.
	}

	~WeakHandle() noexcept override { this->Unlink(); }

	WeakHandle& operator=(const WeakHandle& other) noexcept
	{
		if (this != &other)
			this->reset(other.Value);

		return *this;
	}

	WeakHandle& operator=(WeakHandle&& other) noexcept
	{
		if (this != &other)
		{
			this->reset(other.Value);
			other.Clear();
		}

		return *this;
	}

	COMPILETIMEEVAL explicit operator bool() const noexcept { return this->Value != Default; }
	COMPILETIMEEVAL operator T() const noexcept { return this->Value; }
	COMPILETIMEEVAL T get() const noexcept { return this->Value; }
	COMPILETIMEEVAL T operator->() const noexcept { return this->get(); }

	void reset(T value = Default) noexcept
	{
		this->Unlink();
		this->Value = value;
		this->Attach();
	}

	// WeakRefList::DetachAll() calls this on target death.
	void detachptr() noexcept override { this->Value = Default; }

	void swap(WeakHandle& other) noexcept
	{
		// Re-link both so each ends up on the other's target list.
		const T tmp = this->Value;
		this->reset(other.Value);
		other.reset(tmp);
	}

	friend void swap(WeakHandle& lhs, WeakHandle& rhs) noexcept { lhs.swap(rhs); }

	bool load(PhobosStreamReader& Stm, bool RegisterForChange)
	{
		const bool ok = Stm.Process(this->Value, RegisterForChange).Success();
		this->Attach(); // list is runtime-only: re-link after load.
		return ok;
	}

	bool save(PhobosStreamWriter& Stm) const
	{
		return Stm.Process(this->Value).Success();
	}

private:
	void Attach() noexcept
	{
		if (this->Value == Default)
			return; // nothing to observe.

		this->Link(Traits::GetList(this->Value));
	}

	void Clear() noexcept
	{
		this->Unlink();
		this->Value = Default;
	}

	T Value { Default };
};

template<typename T> class GameObjectLifetime;
class AbstractClass;

template<typename T>
struct DefaultDeleter
{
	void operator()(T* ptr) const noexcept
	{
		if (!ptr) return;

		if constexpr (std::is_base_of_v<AbstractClass, T>)
		{
			ptr->UnInit();
		}
		else
		{
			delete ptr;
		}
	}
};

template<typename T>
struct PutDeleter
{
	void operator()(T* ptr) const noexcept
	{
		if (ptr)
		{
			ptr->Put(nullptr, 0);
		}
	}
};

template<typename T>
struct RemoveDeleter
{
	void operator()(T* ptr) const noexcept
	{
		if (ptr)
		{
			ptr->Remove();
		}
	}
};

template<typename T>
struct MarkForDeathDeleter
{
	void operator()(T* ptr) const noexcept
	{
		if (ptr && ptr->Type && !ptr->TimeToDie)
		{
			ptr->TimeToDie = true;
			ptr->UnInit();
		}
	}
};

template<typename T>
struct MarkForDeathDeleterB
{
	void operator()(T* ptr) const noexcept
	{
		if (ptr && ptr->Type && !ptr->TimeToDie)
		{
			ptr->Owner = nullptr;
			ptr->UnInit();
			ptr->TimeToDie = true;
		}
	}
};


template<typename T>
struct DetachDeleter
{
	void operator()(T* ptr) const noexcept
	{
		if (ptr)
		{
			ptr->Detach();
			ptr->UnInit();
		}
	}
};

template<typename T>
struct MarkOnlyDeleter
{
	void operator()(T* ptr) const noexcept
	{
		if (ptr)
		{
			ptr->TimeToDie = true;
		}
	}
};

template<typename T>
struct NoopDeleter
{
	void operator()(T*) const noexcept { }
};

template<typename T, void (T::* DestroyFunc)()>
struct MemberFunctionDeleter
{
	void operator()(T* ptr) const noexcept
	{
		if (ptr)
		{
			(ptr->*DestroyFunc)();
		}
	}
};

template<typename T, typename Deleter = DefaultDeleter<T>>
class OwningPtr
{
private:
	T* ptr_;
	bool owns_;
	[[no_unique_address]] Deleter deleter_;

	void destroy() noexcept
	{
		if (ptr_ && owns_)
		{
			deleter_(ptr_);
		}
		ptr_ = nullptr;
	}

public:
	using element_type = T;
	using deleter_type = Deleter;

	OwningPtr() noexcept : ptr_(nullptr), owns_(false), deleter_() { }

	explicit OwningPtr(T* ptr, bool take_ownership = true) noexcept
		: ptr_(ptr), owns_(take_ownership), deleter_() { }

	OwningPtr(T* ptr, bool take_ownership, const Deleter& d) noexcept
		: ptr_(ptr), owns_(take_ownership), deleter_(d) { }

	OwningPtr(T* ptr, bool take_ownership, Deleter&& d) noexcept
		: ptr_(ptr), owns_(take_ownership), deleter_(std::move(d)) { }

	OwningPtr(const OwningPtr&) = delete;
	OwningPtr& operator=(const OwningPtr&) = delete;

	OwningPtr(OwningPtr&& other) noexcept
		: ptr_(other.ptr_), owns_(other.owns_), deleter_(std::move(other.deleter_))
	{
		other.ptr_ = nullptr;
		other.owns_ = false;
	}

	OwningPtr& operator=(OwningPtr&& other) noexcept
	{
		if (this != &other)
		{
			destroy();
			ptr_ = other.ptr_;
			owns_ = other.owns_;
			deleter_ = std::move(other.deleter_);
			other.ptr_ = nullptr;
			other.owns_ = false;
		}
		return *this;
	}

	~OwningPtr() noexcept
	{
		destroy();
	}

	T* get() const noexcept { return ptr_; }
	T* operator->() const noexcept { return ptr_; }
	T& operator*() const noexcept { return *ptr_; }
	explicit operator bool() const noexcept { return ptr_ != nullptr; }
	operator T* () const noexcept { return ptr_; }

	bool owns() const noexcept { return owns_; }
	void set_ownership(bool own) noexcept { owns_ = own; }
	Deleter& get_deleter() noexcept { return deleter_; }
	const Deleter& get_deleter() const noexcept { return deleter_; }

	T* release() noexcept
	{
		owns_ = false;
		return std::exchange(ptr_, nullptr);
	}

	void reset(T* ptr = nullptr, bool take_ownership = true) noexcept
	{
		destroy();
		ptr_ = ptr;
		owns_ = take_ownership;
	}

	void swap(OwningPtr& other) noexcept
	{
		using std::swap;
		swap(ptr_, other.ptr_);
		swap(owns_, other.owns_);
		swap(deleter_, other.deleter_);
	}

	bool operator==(const OwningPtr& other) const { return ptr_ == other.ptr_; }
	bool operator!=(const OwningPtr& other) const { return ptr_ != other.ptr_; }
	bool operator==(std::nullptr_t) const { return ptr_ == nullptr; }
	bool operator!=(std::nullptr_t) const { return ptr_ != nullptr; }

	bool save(PhobosStreamWriter& Stm) const
	{
		return Stm
			.Process(this->ptr_)
			.Process(this->owns_);
	}

	bool load(PhobosStreamReader& Stm, bool RegisterForChange)
	{
		return Stm
			.Process(this->ptr_, RegisterForChange)
			.Process(this->owns_);
	}
};

template<typename T, typename Deleter = DefaultDeleter<T>>
class LinkedPtr
{
private:
	T* ptr_;
	bool owns_;
	[[no_unique_address]] Deleter deleter_;

	void destroy() noexcept
	{
		if (ptr_ && owns_)
		{
			GameObjectLifetime<T>::unregister_object(ptr_);
			deleter_(ptr_);
		}
		ptr_ = nullptr;
	}

public:
	using element_type = T;
	using deleter_type = Deleter;

	LinkedPtr() noexcept : ptr_(nullptr), owns_(false), deleter_() { }

	explicit LinkedPtr(T* ptr, bool take_ownership = true) noexcept
		: ptr_(ptr), owns_(take_ownership), deleter_()
	{
		if (ptr_ && owns_)
		{
			GameObjectLifetime<T>::register_object(ptr_);
			GameObjectLifetime<T>::on_destroy(ptr_, [this]()
 {
	 ptr_ = nullptr;
	 owns_ = false;
			});
		}
	}

	LinkedPtr(T* ptr, bool take_ownership, const Deleter& d) noexcept
		: ptr_(ptr), owns_(take_ownership), deleter_(d)
	{
		if (ptr_ && owns_)
		{
			GameObjectLifetime<T>::register_object(ptr_);
		}
	}

	LinkedPtr(const LinkedPtr&) = delete;
	LinkedPtr& operator=(const LinkedPtr&) = delete;

	LinkedPtr(LinkedPtr&& other) noexcept
		: ptr_(other.ptr_), owns_(other.owns_), deleter_(std::move(other.deleter_))
	{
		other.ptr_ = nullptr;
		other.owns_ = false;
	}

	LinkedPtr& operator=(LinkedPtr&& other) noexcept
	{
		if (this != &other)
		{
			destroy();
			ptr_ = other.ptr_;
			owns_ = other.owns_;
			deleter_ = std::move(other.deleter_);
			other.ptr_ = nullptr;
			other.owns_ = false;
		}
		return *this;
	}

	~LinkedPtr() noexcept
	{
		destroy();
	}

	T* get() const noexcept
	{
		return ptr_ && GameObjectLifetime<T>::is_alive(ptr_) ? ptr_ : nullptr;
	}

	T* operator->() const noexcept { return get(); }
	T& operator*() const noexcept { return *get(); }
	explicit operator bool() const noexcept { return get() != nullptr; }
	operator T* () const noexcept { return get(); }

	bool owns() const noexcept { return owns_; }
	Deleter& get_deleter() noexcept { return deleter_; }
	const Deleter& get_deleter() const noexcept { return deleter_; }

	T* release() noexcept
	{
		owns_ = false;
		return std::exchange(ptr_, nullptr);
	}

	void reset(T* ptr = nullptr, bool take_ownership = true) noexcept
	{
		destroy();
		ptr_ = ptr;
		owns_ = take_ownership;
		if (ptr_ && owns_)
		{
			GameObjectLifetime<T>::register_object(ptr_);
		}
	}

	bool save(PhobosStreamWriter& Stm) const
	{
		return Stm
			.Process(this->ptr_)
			.Process(this->owns_);
	}

	bool load(PhobosStreamReader& Stm, bool RegisterForChange)
	{
		return Stm
			.Process(this->ptr_, RegisterForChange)
			.Process(this->owns_);
	}
};

template<typename T>
class ObservingPtr
{
private:
	T* ptr_;

public:
	ObservingPtr() noexcept : ptr_(nullptr) { }
	explicit ObservingPtr(T* ptr) noexcept : ptr_(ptr) { }

	ObservingPtr(const ObservingPtr&) = delete;
	ObservingPtr& operator=(const ObservingPtr&) = delete;

	ObservingPtr(ObservingPtr&& other) noexcept
		: ptr_(std::exchange(other.ptr_, nullptr)) { }

	ObservingPtr& operator=(ObservingPtr&& other) noexcept
	{
		ptr_ = std::exchange(other.ptr_, nullptr);
		return *this;
	}

	~ObservingPtr() = default;

	T* get() const noexcept { return ptr_; }
	T* operator->() const noexcept { return ptr_; }
	T& operator*() const noexcept { return *ptr_; }
	explicit operator bool() const noexcept { return ptr_ != nullptr; }
	operator T* () const noexcept { return ptr_; }

	bool operator==(const ObservingPtr& other) const { return ptr_ == other.ptr_; }
	bool operator!=(const ObservingPtr& other) const { return ptr_ != other.ptr_; }
	bool operator==(std::nullptr_t) const { return ptr_ == nullptr; }
	bool operator!=(std::nullptr_t) const { return ptr_ != nullptr; }

	void reset(T* ptr = nullptr) noexcept { ptr_ = ptr; }
	T* release() noexcept { return std::exchange(ptr_, nullptr); }

	bool save(PhobosStreamWriter& Stm) const
	{
		return Stm.Process(this->ptr_);
	}

	bool load(PhobosStreamReader& Stm, bool RegisterForChange)
	{
		return Stm.Process(this->ptr_, RegisterForChange);
	}
};

template<typename T>
class MoveOnlyPtr
{
private:
	T* ptr_;

public:
	MoveOnlyPtr() noexcept : ptr_(nullptr) { }
	explicit MoveOnlyPtr(T* ptr) noexcept : ptr_(ptr) { }

	MoveOnlyPtr(const MoveOnlyPtr&) = delete;
	MoveOnlyPtr& operator=(const MoveOnlyPtr&) = delete;

	MoveOnlyPtr(MoveOnlyPtr&& other) noexcept
		: ptr_(std::exchange(other.ptr_, nullptr)) { }

	MoveOnlyPtr& operator=(MoveOnlyPtr&& other) noexcept
	{
		ptr_ = std::exchange(other.ptr_, nullptr);
		return *this;
	}

	~MoveOnlyPtr() = default;

	T* get() const noexcept
	{
		return is_valid() ? ptr_ : nullptr;
	}

	T* get_unchecked() const noexcept
	{
		return ptr_;
	}

	T* operator->() const noexcept { return get(); }
	T& operator*() const noexcept { return *get(); }
	explicit operator bool() const noexcept { return is_valid(); }
	operator T* () const noexcept { return get(); }

	bool is_valid() const noexcept
	{
#ifdef USE_LIFETIME_TRACKING
		return ptr_ && GameObjectLifetime<T>::is_alive(ptr_);
#else
		return ptr_ != nullptr;
#endif
	}

	void reset(T* ptr = nullptr) noexcept { ptr_ = ptr; }
	T* release() noexcept { return std::exchange(ptr_, nullptr); }

	bool save(PhobosStreamWriter& Stm) const
	{
		return Stm.Process(this->ptr_);
	}

	bool load(PhobosStreamReader& Stm, bool RegisterForChange)
	{
		return Stm.Process(this->ptr_, RegisterForChange);
	}
};

template<typename T>
class ValueWrapper
{
private:
	T value_;

public:
	ValueWrapper() = default;

	template<typename... Args>
	explicit ValueWrapper(Args&&... args)
		: value_(std::forward<Args>(args)...) { }

	ValueWrapper(const ValueWrapper&) = delete;
	ValueWrapper& operator=(const ValueWrapper&) = delete;

	ValueWrapper(ValueWrapper&& other) noexcept(std::is_nothrow_move_constructible_v<T>)
		: value_(std::move(other.value_)) { }

	ValueWrapper& operator=(ValueWrapper&& other) noexcept(std::is_nothrow_move_assignable_v<T>)
	{
		value_ = std::move(other.value_);
		return *this;
	}

	T& get() noexcept { return value_; }
	const T& get() const noexcept { return value_; }
	T* operator->() noexcept { return &value_; }
	const T* operator->() const noexcept { return &value_; }
	T& operator*() noexcept { return value_; }
	const T& operator*() const noexcept { return value_; }

	operator T& () noexcept { return value_; }
	operator const T& () const noexcept { return value_; }

	bool save(PhobosStreamWriter& Stm) const
	{
		return Stm.Process(this->value_);
	}

	bool load(PhobosStreamReader& Stm, bool RegisterForChange)
	{
		return Stm.Process(this->value_, RegisterForChange);
	}
};

template<typename T>
using UninitPtr = OwningPtr<T, DefaultDeleter<T>>;

template<typename T>
using MarkPtr = OwningPtr<T, MarkForDeathDeleter<T>>;

template<typename T>
using PutPtr = OwningPtr<T, PutDeleter<T>>;

template<typename T>
using RemovePtr = OwningPtr<T, RemoveDeleter<T>>;

template<typename T>
using ObservePtr = OwningPtr<T, NoopDeleter<T>>;

template<typename T, typename Deleter = DefaultDeleter<T>, typename... Args>
OwningPtr<T, Deleter> make_owned(Args&&... args)
{
	T* obj = GameCreate<T>(std::forward<Args>(args)...);
	return OwningPtr<T, Deleter>(obj, true);
}

template<typename T, typename Deleter = DefaultDeleter<T>>
OwningPtr<T, Deleter> make_owned_from(T* ptr, Deleter d = Deleter {})
{
	return OwningPtr<T, Deleter>(ptr, true, std::move(d));
}

template<typename T, typename Deleter = DefaultDeleter<T>, typename... Args>
LinkedPtr<T, Deleter> make_linked(Args&&... args)
{
	T* obj = GameCreate<T>(std::forward<Args>(args)...);
	return LinkedPtr<T, Deleter>(obj, true);
}