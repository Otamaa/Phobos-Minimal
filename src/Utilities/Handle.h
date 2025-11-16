#pragma once

#include "Savegame.h"

#include <type_traits>
#include <utility>
#include <vector>
#include <algorithm>

struct Handles
{
	static std::vector<Handles*> Array;
	virtual void detachptr() = 0;
};

// owns a resource. not copyable, but movable.
template <typename T, typename Deleter, T Default = T()>
struct Handle : public Handles
{
	Handle() noexcept : Handles()
		, Value()
	{
		Handles::Array.emplace_back(this);
	};

	explicit Handle(T value) noexcept
		: Handles()
		, Value(value)
	{
		Handles::Array.emplace_back(this);
	}

	Handle(const Handle&) = delete;

	Handle(Handle&& other) noexcept
		: Handles()
		, Value(other.release())
	{ }

	~Handle() noexcept
	{
		if (this->Value != Default)
		{
			Deleter {}(this->Value);
		}

		this->Value = Default;
		auto find = std::ranges::find_if(Handles::Array, [this](auto ptr)
 {
	 return this == ptr;
		});

		if (find != Handles::Array.end())
			Handles::Array.erase(find, Handles::Array.end());
	}

	Handle& operator = (const Handle&) = delete;

	Handle& operator = (Handle&& other) noexcept
	{
		if (this != &other)
		{
			this->reset(other.release());
		}
		return *this;
	}

	COMPILETIMEEVAL explicit operator bool() const noexcept
	{
		return this->Value != Default;
	}

	COMPILETIMEEVAL operator T () const noexcept
	{
		return this->Value;
	}

	COMPILETIMEEVAL T get() const noexcept
	{
		return this->Value;
	}

	COMPILETIMEEVAL T operator->() const noexcept
	{
		return get();
	}

	COMPILETIMEEVAL T release() noexcept
	{
		return std::exchange(this->Value, Default);
	}

	void reset(T value = Default) noexcept
	{
		if (this->Value != Default)
		{
			Deleter {}(this->Value);
		}

		this->Value = value;
	}

	//release all references to avoid double delete issues
	virtual void detachptr() override
	{
		this->Value = Default;
	}

	void swap(Handle& other) noexcept
	{
		using std::swap;
		swap(this->Value, other.Value);
	}

	friend void swap(Handle& lhs, Handle& rhs) noexcept
	{
		lhs.swap(rhs);
	}

	bool load(PhobosStreamReader& Stm, bool RegisterForChange)
	{
		return Stm
			.Process(this->Value, RegisterForChange)
			.Success()
			;
	}

	bool save(PhobosStreamWriter& Stm) const
	{
		return Stm
			.Process(this->Value)
			.Success()
			;
	}

private:
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