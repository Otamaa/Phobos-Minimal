#pragma once
#include <atomic>
#include <utility>
#include <type_traits>

// Forward declarations
class PhobosStreamReader;
class PhobosStreamWriter;

// Base control block
struct GameObjectControlBlockBase
{
	std::atomic<int> refCount { 1 };
	std::atomic<int> weakCount { 0 };
	void* ptr = nullptr;
	std::atomic<bool> destructed { false };

	GameObjectControlBlockBase(void* p) : ptr(p) { }
	virtual ~GameObjectControlBlockBase() = default;

	void MarkDestructed()
	{
		destructed.store(true, std::memory_order_release);
	}

	bool IsDestructed() const
	{
		return destructed.load(std::memory_order_acquire);
	}

	virtual void CallDeleter() = 0;
};

// Templated control block with deleter
template<typename T, typename Deleter>
struct GameObjectControlBlock : public GameObjectControlBlockBase
{
	Deleter deleter;

	GameObjectControlBlock(T* p, Deleter d)
		: GameObjectControlBlockBase(p), deleter(std::move(d)) { }

	void CallDeleter() override
	{
		if (ptr && !IsDestructed())
		{
			MarkDestructed();
			deleter(static_cast<T*>(ptr));
			ptr = nullptr;
		}
	}
};

// Weak pointer forward declaration
template<typename T>
class GameObjectWeakPtr;

// Shared pointer for GameObject
template<typename T>
class GameObjectSharedPtr
{
private:
	GameObjectControlBlockBase* control = nullptr;
	T* ptr = nullptr;

	friend class GameObjectWeakPtr<T>;

	void addRef()
	{
		if (control)
		{
			control->refCount.fetch_add(1, std::memory_order_relaxed);
		}
	}

	void release()
	{
		if (!control) return;

		if (control->refCount.fetch_sub(1, std::memory_order_acq_rel) == 1)
		{
			// Last reference - call custom deleter if provided
			control->CallDeleter();

			// Check if we should delete the control block
			if (control->weakCount.load(std::memory_order_acquire) == 0)
			{
				delete control;
				control = nullptr;
			}
		}
	}

public:
	// Default constructor
	GameObjectSharedPtr() = default;

	// Constructor from raw pointer (non-owning, no deleter)
	explicit GameObjectSharedPtr(T* p) : ptr(p)
	{
		if (p)
		{
			// Use NoopDeleter by default
			struct NoopDel { void operator()(T*) const noexcept { } };
			control = new GameObjectControlBlock<T, NoopDel>(p, NoopDel {});
		}
	}

	// Constructor with deleter functor (like your DefaultDeleter, MarkForDeathDeleter, etc.)
	template<typename Deleter>
	GameObjectSharedPtr(T* p, Deleter deleter) : ptr(p)
	{
		if (p)
		{
			control = new GameObjectControlBlock<T, Deleter>(p, std::move(deleter));
		}
	}

	// Constructor from nullptr
	GameObjectSharedPtr(std::nullptr_t) : GameObjectSharedPtr() { }

	// Copy constructor
	GameObjectSharedPtr(const GameObjectSharedPtr& other)
		: control(other.control), ptr(other.ptr)
	{
		addRef();
	}

	// Move constructor
	GameObjectSharedPtr(GameObjectSharedPtr&& other) noexcept
		: control(other.control), ptr(other.ptr)
	{
		other.control = nullptr;
		other.ptr = nullptr;
	}

	// Copy assignment
	GameObjectSharedPtr& operator=(const GameObjectSharedPtr& other)
	{
		if (this != &other)
		{
			release();
			control = other.control;
			ptr = other.ptr;
			addRef();
		}
		return *this;
	}

	// Move assignment
	GameObjectSharedPtr& operator=(GameObjectSharedPtr&& other) noexcept
	{
		if (this != &other)
		{
			release();
			control = other.control;
			ptr = other.ptr;
			other.control = nullptr;
			other.ptr = nullptr;
		}
		return *this;
	}

	// Assignment from nullptr
	GameObjectSharedPtr& operator=(std::nullptr_t)
	{
		reset();
		return *this;
	}

	// Assignment from raw pointer (uses NoopDeleter)
	GameObjectSharedPtr& operator=(T* p)
	{
		reset(p);
		return *this;
	}

	// Destructor
	~GameObjectSharedPtr()
	{
		release();
	}

	// Dereference operators
	T* operator->() const { return ptr; }
	T& operator*() const { return *ptr; }

	// Get raw pointer
	T* get() const { return ptr; }

	// Check validity
	explicit operator bool() const { return ptr != nullptr && IsValid(); }

	// Implicit conversion to raw pointer (like MarkPtr)
	operator T* () const { return ptr; }

	// Get reference count
	int useCount() const
	{
		return control ? control->refCount.load(std::memory_order_relaxed) : 0;
	}

	// Reset to nullptr
	void reset()
	{
		release();
		control = nullptr;
		ptr = nullptr;
	}

	// Reset with new pointer (no deleter)
	void reset(T* p)
	{
		release();
		control = nullptr;
		ptr = nullptr;

		if (p)
		{
			ptr = p;
			struct NoopDel { void operator()(T*) const noexcept { } };
			control = new GameObjectControlBlock<T, NoopDel>(p, NoopDel {});
		}
	}

	// Reset with new pointer and custom deleter
	template<typename Deleter>
	void reset(T* p, Deleter deleter)
	{
		release();
		control = nullptr;
		ptr = nullptr;

		if (p)
		{
			ptr = p;
			control = new GameObjectControlBlock<T, Deleter>(p, std::move(deleter));
		}
	}

	// Release without deleting (like MarkPtr::release)
	T* release_ptr()
	{
		T* temp = ptr;

		if (control)
		{
			// Just release our reference, don't mark as destructed
			control->refCount.fetch_sub(1, std::memory_order_acq_rel);
			control = nullptr;
		}

		ptr = nullptr;
		return temp;
	}

	// Invalidate - game is deleting the object, just clear our reference
	// This is for external notifications (like InvalidateAnimPointer)
	void Invalidate()
	{
		if (control)
		{
			control->MarkDestructed();
			// Don't touch the pointer, game will delete it
		}
		ptr = nullptr;
		control = nullptr;
	}

	// Check if the object is still valid (not destructed)
	bool IsValid() const
	{
		return ptr && control && !control->IsDestructed();
	}

	// Comparison operators
	bool operator==(const GameObjectSharedPtr& other) const
	{
		return ptr == other.ptr;
	}

	bool operator!=(const GameObjectSharedPtr& other) const
	{
		return ptr != other.ptr;
	}

	bool operator==(std::nullptr_t) const
	{
		return ptr == nullptr;
	}

	bool operator!=(std::nullptr_t) const
	{
		return ptr != nullptr;
	}

	bool operator==(T* other) const
	{
		return ptr == other;
	}

	bool operator!=(T* other) const
	{
		return ptr != other;
	}

	// Phobos Save
	bool save(PhobosStreamWriter& Stm) const
	{
		return Stm.Process(this->ptr);
	}

	// Phobos Load
	bool load(PhobosStreamReader& Stm, bool RegisterForChange)
	{
		T* loadedPtr = nullptr;

		if (!Stm.Process(loadedPtr, RegisterForChange))
		{
			return false;
		}

		// Release current pointer
		release();
		control = nullptr;
		ptr = nullptr;

		// Set new pointer (no deleter on load, objects are managed by game)
		if (loadedPtr)
		{
			ptr = loadedPtr;
			struct NoopDel { void operator()(T*) const noexcept { } };
			control = new GameObjectControlBlock<T, NoopDel>(loadedPtr, NoopDel {});
		}

		return true;
	}
};

// Weak pointer for GameObject
template<typename T>
class GameObjectWeakPtr
{
private:
	GameObjectControlBlockBase* control = nullptr;

	void addWeakRef()
	{
		if (control)
		{
			control->weakCount.fetch_add(1, std::memory_order_relaxed);
		}
	}

	void releaseWeak()
	{
		if (!control) return;

		if (control->weakCount.fetch_sub(1, std::memory_order_acq_rel) == 1)
		{
			// Last weak reference
			if (control->refCount.load(std::memory_order_acquire) == 0)
			{
				delete control;
				control = nullptr;
			}
		}
	}

public:
	// Default constructor
	GameObjectWeakPtr() = default;

	// Constructor from shared pointer
	GameObjectWeakPtr(const GameObjectSharedPtr<T>& shared)
		: control(shared.control)
	{
		addWeakRef();
	}

	// Copy constructor
	GameObjectWeakPtr(const GameObjectWeakPtr& other)
		: control(other.control)
	{
		addWeakRef();
	}

	// Move constructor
	GameObjectWeakPtr(GameObjectWeakPtr&& other) noexcept
		: control(other.control)
	{
		other.control = nullptr;
	}

	// Copy assignment
	GameObjectWeakPtr& operator=(const GameObjectWeakPtr& other)
	{
		if (this != &other)
		{
			releaseWeak();
			control = other.control;
			addWeakRef();
		}
		return *this;
	}

	// Assignment from shared pointer
	GameObjectWeakPtr& operator=(const GameObjectSharedPtr<T>& shared)
	{
		releaseWeak();
		control = shared.control;
		addWeakRef();
		return *this;
	}

	// Move assignment
	GameObjectWeakPtr& operator=(GameObjectWeakPtr&& other) noexcept
	{
		if (this != &other)
		{
			releaseWeak();
			control = other.control;
			other.control = nullptr;
		}
		return *this;
	}

	// Destructor
	~GameObjectWeakPtr()
	{
		releaseWeak();
	}

	// Lock to get shared pointer
	GameObjectSharedPtr<T> lock() const
	{
		if (!control) return GameObjectSharedPtr<T>();

		// Try to increment refCount if it's not zero
		int refCount = control->refCount.load(std::memory_order_relaxed);
		while (refCount > 0)
		{
			if (control->refCount.compare_exchange_weak(
				refCount, refCount + 1,
				std::memory_order_acquire,
				std::memory_order_relaxed))
			{

				GameObjectSharedPtr<T> result;
				result.control = control;
				result.ptr = static_cast<T*>(control->ptr);
				return result;
			}
		}

		return GameObjectSharedPtr<T>();
	}

	// Check if expired
	bool expired() const
	{
		return !control || control->refCount.load(std::memory_order_relaxed) == 0;
	}

	// Get use count
	int useCount() const
	{
		return control ? control->refCount.load(std::memory_order_relaxed) : 0;
	}

	// Reset
	void reset()
	{
		releaseWeak();
		control = nullptr;
	}

	// Phobos Save
	bool save(PhobosStreamWriter& Stm) const
	{
		T* ptr = (control && control->ptr) ? static_cast<T*>(control->ptr) : nullptr;
		return Stm.Process(ptr);
	}

	// Phobos Load
	bool load(PhobosStreamReader& Stm, bool RegisterForChange)
	{
		T* loadedPtr = nullptr;

		if (!Stm.Process(loadedPtr, RegisterForChange))
		{
			return false;
		}

		// Weak pointers don't create control blocks on load
		// They will be valid only if a shared_ptr exists for this object
		releaseWeak();
		control = nullptr;

		return true;
	}
};