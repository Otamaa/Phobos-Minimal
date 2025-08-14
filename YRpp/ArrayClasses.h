#pragma once

#include <Memory.h>

#include <algorithm>
#include <Helpers/Concepts.h>
#include <YRPPCore.h>

struct __declspec(align(4)) DummyDynamicVectorClass
{
	void* vftble;
	void** Vector_Item;
	int VectorMax;
	char IsValid;
	char IsAllocated;
	char VectorClassPad[2];
	int ActiveCount;
	int GrowthStep;
};

template<typename T>
struct TDummyDynamicVectorClass
{
	void* vftble;
	T* Vector_Item;
	int VectorMax;
	char IsValid;
	char IsAllocated;
	char VectorClassPad[2];
	int ActiveCount;
	int GrowthStep;
};

static_assert(sizeof(DummyDynamicVectorClass) == 0x18, "Invalid Size !");

enum class ArrayType : int
{
	Vector, DynamicVector, TypeList, Counter
};

//========================================================================
//=== VectorClass ========================================================
//========================================================================

template <typename T>
class VectorClass
{
public:
	// the hidden element count messes with alignment. only applies to align 8, 16, ...
	static_assert(!needs_vector_delete<T>::value || (__alignof(T) <= 4), "Alignment of T needs to be less than or equal to 4.");
	static const ArrayType Type = ArrayType::Vector;

	constexpr VectorClass() noexcept = default;

	explicit VectorClass(int capacity, T* pMem = nullptr)
	{
		if (capacity != 0)
		{
			this->Capacity = capacity;

			if (pMem)
			{
				this->Items = pMem;
			}
			else
			{
				this->Items = GameCreateArray<T>(static_cast<size_t>(capacity));
				this->IsAllocated = true;
			}
		}
	}

	VectorClass(const VectorClass<T>& other)
	{
		if (other.Capacity > 0)
		{
			this->Items = GameCreateArray<T>(static_cast<size_t>(other.Capacity));
			this->IsAllocated = true;
			this->Capacity = other.Capacity;
			for (auto i = 0; i < other.Capacity; ++i)
			{
				this->Items[i] = other.Items[i];
			}
		}
	}

	VectorClass(VectorClass<T>&& other) noexcept :
		Items(other.Items),
		Capacity(other.Capacity),
		IsInitialized(other.IsInitialized),
		IsAllocated(std::exchange(other.IsAllocated, false))
	{ }

	virtual ~VectorClass() noexcept
	{
		if (this->IsAllocated)
		{
			GameDeleteArray(this->Items, static_cast<size_t>(this->Capacity));
		}
	}

	VectorClass<T>& operator = (const VectorClass<T>& other)
	{
		VectorClass<T>(other).Swap(*this);
		return *this;
	}

	VectorClass<T>& operator = (VectorClass<T>&& other) noexcept
	{
		VectorClass<T>(std::move(other)).Swap(*this);
		return *this;
	}

	virtual bool operator == (const VectorClass<T>& other) const
	{
		if (this->Capacity != other.Capacity)
		{
			return false;
		}

		for (auto i = 0; i < this->Capacity; ++i)
		{
			if (this->Items[i] == other.Items[i])
			{
				continue; // kapow! don't rewrite this to != unless you know why you're doing it
			}
			return false;
		}

		return true;
	}

	bool operator != (const VectorClass<T>& other) const
	{
		return !(*this == other);
	}

	virtual bool SetCapacity(int capacity, T* pMem = nullptr)
	{
		if (capacity > 0)
		{
			this->IsInitialized = false;

			bool bMustAllocate = (pMem == nullptr);
			if (!pMem)
			{
				pMem = GameCreateArray<T>(static_cast<size_t>(capacity));
			}

			this->IsInitialized = true;

			if (!pMem)
			{
				return false;
			}

			if (this->Items)
			{
				auto n = (capacity < this->Capacity) ? capacity : this->Capacity;
				for (auto i = 0; i < n; ++i)
				{
					pMem[i] = std::move_if_noexcept(this->Items[i]);
				}

				if (this->IsAllocated)
				{
					GameDeleteArray(this->Items, static_cast<size_t>(this->Capacity));
					this->Items = nullptr;
				}
			}

			this->IsAllocated = bMustAllocate;
			this->Items = pMem;
			this->Capacity = capacity;
		}
		else
		{
			Clear();
		}
		return true;
	}

	virtual void Clear()
	{
		VectorClass<T>(std::move(*this));
		this->Items = nullptr;
		this->Capacity = 0;
	}

	virtual int FindItemIndex(const T& item) const
	{
		if (this->IsInitialized)
		{
			for (auto i = 0; i < this->Capacity; ++i)
			{
				if (this->Items[i] == item)
				{
					return i;
				}
			}
		}

		return -1;
	}

	virtual int GetItemIndex(const T* pItem) const final
	{
		if (!this->IsInitialized)
		{
			return -1;
		}

		return pItem - this->Items;
	}

	virtual T GetItem(int i) const final
	{
		return this->Items[i];
	}

	T& operator [] (int i)
	{
		return this->Items[i];
	}

	const T& operator [] (int i) const
	{
		return this->Items[i];
	}

	bool Reserve(int capacity)
	{
		if (!this->IsInitialized)
		{
			return false;
		}

		if (this->Capacity >= capacity)
		{
			return true;
		}

		return SetCapacity(capacity, nullptr);
	}

	void Swap(VectorClass<T>& other) noexcept
	{
		using std::swap;
		swap(this->Items, other.Items);
		swap(this->Capacity, other.Capacity);
		swap(this->IsInitialized, other.IsInitialized);
		swap(this->IsAllocated, other.IsAllocated);
	}

	T* Items { nullptr };
	int Capacity { 0 };
	bool IsInitialized { true };
	bool IsAllocated { false };
};

//========================================================================
//=== DynamicVectorClass =================================================
//========================================================================

template <typename T>
class DynamicVectorClass : public VectorClass<T>
{
public:
	constexpr DynamicVectorClass() noexcept = default;
	static const ArrayType Type = ArrayType::DynamicVector;
	explicit DynamicVectorClass(int capacity, T* pMem = nullptr)
		: VectorClass<T>(capacity, pMem)
	{ }

	DynamicVectorClass(const DynamicVectorClass<T>& other)
	{
		if (other.Capacity > 0)
		{
			this->Items = GameCreateArray<T>(static_cast<size_t>(other.Capacity));
			this->IsAllocated = true;
			this->Capacity = other.Capacity;
			this->Count = other.Count;
			this->CapacityIncrement = other.CapacityIncrement;
			for (auto i = 0; i < other.Count; ++i)
			{
				this->Items[i] = other.Items[i];
			}
		}
	}

	DynamicVectorClass(DynamicVectorClass<T>&& other) noexcept
		: VectorClass<T>(std::move(other)), Count(other.Count),
		CapacityIncrement(other.CapacityIncrement)
	{ }

	DynamicVectorClass<T>& operator = (const DynamicVectorClass<T>& other)
	{
		DynamicVectorClass<T>(other).Swap(*this);
		return *this;
	}

	DynamicVectorClass<T>& operator = (DynamicVectorClass<T>&& other) noexcept
	{
		DynamicVectorClass<T>(std::move(other)).Swap(*this);
		return *this;
	}

	virtual bool SetCapacity(int capacity, T* pMem = nullptr) override
	{
		bool bRet = VectorClass<T>::SetCapacity(capacity, pMem);

		if (this->Capacity < this->Count)
		{
			this->Count = this->Capacity;
		}

		return bRet;
	}

	virtual void Clear() override
	{
		VectorClass<T>::Clear();
		this->Count = 0;
	}

	virtual int FindItemIndex(const T& item) const override final
	{
		if (!this->IsInitialized)
		{
			return 0;
		}

		for (int i = 0; i < this->Count; i++)
		{
			if (this->Items[i] == item)
			{
				return i;
			}
		}

		return -1;
	}

	COMPILETIMEEVAL bool ValidIndex(int index) const
	{
		return static_cast<size_t>(index) < static_cast<size_t>(this->Count);
	}

	T GetItemOrDefault(int i) const
	{
		return this->GetItemOrDefault(i, T());
	}

	T GetItemOrDefault(int i, T def) const
	{
		if (this->ValidIndex(i))
		{
			return this->Items[i];
		}
		return def;
	}

	COMPILETIMEEVAL T* begin() const
	{
		return &this->Items[0];
	}

	COMPILETIMEEVAL T* end() const
	{

		return &this->Items[this->Count];
	}

	COMPILETIMEEVAL T* front() const
	{
		return begin();
	}

	COMPILETIMEEVAL T* back() const
	{
		return end() - 1;
	}

	COMPILETIMEEVAL size_t size() const
	{
		return (size_t)Count;
	}

	// this one doesnt destroy the memory , just reset the count
	// the vector memory may still contains dangling pointer if it vector of pointer
	COMPILETIMEEVAL void FORCEDINLINE Reset(int resetCount = 0)
	{
		this->Count = (size_t)resetCount < (size_t)this->Capacity ? resetCount : 0;
	}

	COMPILETIMEEVAL bool Contains(const T& item) const
	{
		if (this->Count <= 0)
		{
			return false;
		}

		return this->Find(item) != this->end();
	}

	bool AddItem(T item)
	{
		if (this->Count >= this->Capacity)
		{
			if (!this->IsAllocated && this->Capacity != 0)
			{
				return false;
			}

			if (this->CapacityIncrement <= 0)
			{
				return false;
			}

			if (!this->SetCapacity(this->Capacity + this->CapacityIncrement, nullptr))
			{
				return false;
			}
		}

		this->Items[Count++] = std::move(item);
		return true;
	}

	template <class... _Valty>
	COMPILETIMEEVAL decltype(auto) emplace_back(_Valty&&... _Val)
	{
		AddItem(T { _Val... });
		return *back();
	}

	bool AddUnique(const T& item)
	{
		int idx = this->FindItemIndex(item);
		return idx < 0 && this->AddItem(item);
	}

	bool RemoveAt(int index)
	{
		if (!this->ValidIndex(index))
		{
			return false;
		}

		--this->Count;
		for (int i = index; i < this->Count; ++i)
		{
			this->Items[i] = std::move_if_noexcept(this->Items[i + 1]);
		}

		return true;
	}

	template<typename Func>
	COMPILETIMEEVAL bool FORCEINLINE remove_if(Func&& act)
	{
		if (!this->IsAllocated) return false;
		T* newEnd = std::remove_if(this->begin(), this->end(), act);
		this->Count = newEnd ? std::distance(this->begin(), newEnd) : 0;
		return true;
	}

	bool Remove(const T& item)
	{
		int idx = this->FindItemIndex(item);
		return idx >= 0 && this->RemoveAt(idx);
	}

	void Swap(DynamicVectorClass& other) noexcept
	{
		VectorClass<T>::Swap(other);
		using std::swap;
		swap(this->Count, other.Count);
		swap(this->CapacityIncrement, other.CapacityIncrement);
	}

	FORCEDINLINE T* Find(const T& item) const
	{
		if (this->Count <= 0)
		{
			return this->end();
		}
		return std::find(this->begin(), this->end(), item);
	}

#pragma region WrappedSTD
	template <typename Func>
	COMPILETIMEEVAL auto FORCEDINLINE find_if(Func&& act) const
	{
		if (this->Count <= 0) {
			return this->end();
		}

		for (auto i = this->begin(); i != this->end(); ++i)
		{
			if (act(*i))
			{
				return i;
			}
		}

		return this->end();
	}

	template <typename Func>
	COMPILETIMEEVAL auto FORCEDINLINE find_if(Func&& act)
	{
		if (this->Count <= 0) {
			return this->end();
		}

		for (auto i = this->begin(); i != this->end(); ++i)
		{
			if (act(*i))
			{
				return i;
			}
		}

		return this->end();
	}

	template <typename Func>
	COMPILETIMEEVAL void FORCEDINLINE for_each(Func&& act) const
	{
		if (this->Count <= 0) {
			return;
		}

		for (auto i = this->begin(); i != this->end(); ++i)
		{
			act(*i);
		}
	}

	template <typename Func>
	COMPILETIMEEVAL void FORCEDINLINE for_each(Func&& act)
	{
		if (this->Count <= 0) {
			return;
		}

		for (auto i = this->begin(); i != this->end(); ++i)
		{
			act(*i);
		}
	}

	template<typename func>
	COMPILETIMEEVAL bool FORCEDINLINE none_of(func&& fn) const
	{
		if (this->Count <= 0) {
			return true;
		}

		for (auto i = this->begin(); i != this->end(); ++i)
		{
			if (fn(*i))
			{
				return false;
			}
		}

		return true;
	}

	template<typename func>
	COMPILETIMEEVAL bool FORCEDINLINE none_of(func&& fn)
	{
		if (this->Count <= 0) {
			return true;
		}

		for (auto i = this->begin(); i != this->end(); ++i)
		{
			if (fn(*i))
			{
				return false;
			}
		}

		return true;
	}

	template<typename func>
	COMPILETIMEEVAL bool FORCEDINLINE any_of(func&& fn) const
	{
		if (this->Count <= 0) {
			return false;
		}

		for (auto i = this->begin(); i != this->end(); ++i)
		{
			if (fn(*i))
			{
				return true;
			}
		}

		return false;
	}

	template<typename func>
	COMPILETIMEEVAL bool FORCEDINLINE any_of(func&& fn) {

		if (this->Count <= 0) {
			return false;
		}

		for (auto i = this->begin(); i != this->end(); ++i)
		{
			if (fn(*i))
			{
				return true;
			}
		}

		return false;
	}
#pragma endregion

	bool FORCEDINLINE COMPILETIMEEVAL Empty() { return this->Count <= 0; }

	bool FORCEDINLINE IsValidArray() {
		if (this->Count >= this->Capacity) {
			if ((this->IsAllocated || !this->Capacity) && this->CapacityIncrement > 0) {
				return this->SetCapacity(this->Capacity + this->CapacityIncrement, nullptr);
			} else {
				return false;
			}
		}
		return true;
	}

public:

	int Count { 0 };
	int CapacityIncrement { 10 };
};

//========================================================================
//=== TypeList ===========================================================
//========================================================================

template <typename T>
class TypeList : public DynamicVectorClass<T>
{
public:
	constexpr TypeList() noexcept = default;
	static const ArrayType Type = ArrayType::TypeList;
	explicit TypeList(int capacity, T* pMem = nullptr)
		: DynamicVectorClass<T>(capacity, pMem)
	{ }

	TypeList(const TypeList<T>& other)
		: DynamicVectorClass<T>(other), unknown_18(other.unknown_18)
	{ }

	TypeList(TypeList<T>&& other) noexcept
		: DynamicVectorClass<T>(std::move(other)), unknown_18(other.unknown_18)
	{ }

	TypeList<T>& operator = (const TypeList<T>& other)
	{
		TypeList<T>(other).Swap(*this);
		return *this;
	}

	TypeList<T>& operator = (TypeList<T>&& other) noexcept
	{
		TypeList<T>(std::move(other)).Swap(*this);
		return *this;
	}

	void Swap(TypeList<T>& other) noexcept
	{
		DynamicVectorClass<T>::Swap(other);
		using std::swap;
		swap(this->unknown_18, other.unknown_18);
	}

	int unknown_18 { 0 };
};

//========================================================================
//=== CounterClass =======================================================
//========================================================================

class CounterClass : public VectorClass<int>
{
public:
	constexpr CounterClass() noexcept = default;
	static const ArrayType Type = ArrayType::Counter;
	CounterClass(const CounterClass& other)
		: VectorClass<int>(other), Total(other.Total)
	{ }

	CounterClass(CounterClass&& other) noexcept
		: VectorClass<int>(std::move(other)), Total(other.Total)
	{ }

	CounterClass& operator = (const CounterClass& other)
	{
		CounterClass(other).Swap(*this);
		return *this;
	}

	CounterClass& operator = (CounterClass&& other) noexcept
	{
		CounterClass(std::move(other)).Swap(*this);
		return *this;
	}

	virtual void Clear() override
	{
		for (int i = 0; i < this->Capacity; ++i)
		{
			this->Items[i] = 0;
		}

		this->Total = 0;
	}

	int GetTotal() const
	{
		return this->Total;
	}

	bool EnsureItem(int index)
	{
		if (index < this->Capacity)
		{
			return true;
		}

		int count = this->Capacity;
		if (this->SetCapacity(index + 10, nullptr))
		{
			for (auto i = count; i < this->Capacity; ++i)
			{
				this->Items[i] = 0;
			}
			return true;
		}

		return false;
	}

	int operator[] (int index) const
	{
		return this->GetItemCount(index);
	}

	int GetItemCount(int index)
	{
		return this->EnsureItem(index) ? this->Items[index] : 0;
	}

	int GetItemCount(int index) const
	{
		return ((size_t)index < (size_t)this->Capacity) ? this->Items[index] : 0;
	}

	int Increment(int index)
	{
		if (this->EnsureItem(index))
		{
			++this->Total;
			return ++this->Items[index];
		}
		return 0;
	}

	int Decrement(int index)
	{
		if (this->EnsureItem(index))
		{
			--this->Total;
			return --this->Items[index];
		}
		return 0;
	}

	void Swap(CounterClass& other) noexcept
	{
		VectorClass<int>::Swap(other);
		using std::swap;
		swap(this->Total, other.Total);
	}

	int Total { 0 };
};

template<typename T, const int size>
class ArrayHelper
{
public:
	operator T* () { return reinterpret_cast<T*>(this); }
	operator const T* () const { return reinterpret_cast<const T*>(this); }
	T* operator&() { return reinterpret_cast<T*>(this); }
	const T* operator&() const { return reinterpret_cast<const T*>(this); }
	T& operator[](int index)
	{
		return reinterpret_cast<T*>(this)[index];
	}
	const T& operator[](int index) const
	{
		return reinterpret_cast<const T*>(this)[index];
	}

	T* begin() { return reinterpret_cast<T*>(this); }
	T* end() { return reinterpret_cast<T*>(this) + size; }

	const T* begin() const { return reinterpret_cast<const T*>(this); }
	const T* end() const { return reinterpret_cast<const T*>(this) + size; }

	constexpr int Size() const
	{
		return size;
	}

protected:
	alignas(T) char _dummy[size * sizeof(T)];
};

template<typename T, const int y, const int x>
class ArrayHelper2D
{
public:
	operator ArrayHelper<T, x>* () { return reinterpret_cast<ArrayHelper<T, x>*>(this); }
	operator const ArrayHelper<T, x>* () const { return reinterpret_cast<const ArrayHelper<T, x>*>(this); }
	ArrayHelper<T, x>* operator&() { return reinterpret_cast<ArrayHelper<T, x>*>(this); }
	const ArrayHelper<T, x>* operator&() const { return reinterpret_cast<const ArrayHelper<T, x>*>(this); }
	ArrayHelper<T, x>& operator[](int index)
	{
		return _dummy[index];
	}
	const ArrayHelper<T, x>& operator[](int index) const
	{
		return _dummy[index];
	}

	ArrayHelper<T, x>* begin() { return _dummy; }
	ArrayHelper<T, x>* end() { return _dummy + y; }

	const ArrayHelper<T, x>* begin() const { return _dummy; }
	const ArrayHelper<T, x>* end() const { return _dummy + y; }

	constexpr int Size() const
	{
		return y;
	}

protected:
	ArrayHelper<T, x> _dummy[y];
};
