#pragma once

template<class T>
class RefPtr
{
public:
	RefPtr(T* realptr = nullptr) : Pointer(realptr) { }
	RefPtr(const RefPtr& that) : Pointer(that.Pointer) { }
	~RefPtr() { Pointer = nullptr; }

	operator T* () const { refptr_assert(Pointer != nullptr); return Pointer; }
	operator T& () const { refptr_assert(Pointer != nullptr); return (*Pointer); }
	// operator uintptr_t() const { return uintptr_t(Pointer); }

	bool operator!() const { refptr_assert(Pointer != nullptr); return Pointer == nullptr; }

	RefPtr& operator=(const T* that)
	{
		refptr_assert(that != nullptr);

		Pointer = const_cast<T*>(that);
		return (*this);
	}

	RefPtr& operator=(const RefPtr& that)
	{
		Pointer = that.Pointer;
		return (*this);
	}

	T** operator &() { refptr_assert(Pointer != nullptr); return &Pointer; } // Address-of operator.
	T* operator->() const { refptr_assert(Pointer != nullptr); return Pointer; }
	T& operator*() const { refptr_assert(Pointer != nullptr); return (*Pointer); }

	bool Is_Valid() const { return Pointer != nullptr; }

private:
	/**
	 *  Raw pointer to the object.
	 */
	T* Pointer;
};