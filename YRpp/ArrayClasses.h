#pragma once

#include <Memory.h>

#include <algorithm>

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

static_assert(sizeof(DummyDynamicVectorClass) == 0x18, "Invalid Size !");

template<typename T, class V /*= DynamicVectorClass<T>*/>
class VectorCursor
{
public:
	VectorCursor(V* vector, int pos = 0);
	VectorCursor(const V* vector, int pos = 0);
	virtual ~VectorCursor();

	VectorCursor& operator=(const VectorCursor& that);

	bool operator==(const VectorCursor& that) const;
	bool operator!=(const VectorCursor& that) const;

	T& operator*() { return *Get_Element(); }
	const T& operator*() const { return *Get_Element(); }

	T* operator->() { return Get_Element(); }
	const T* operator->() const { return Get_Element(); }

	operator bool() { return Can_Loop(); }

	VectorCursor& operator++() { Next(); return *this; }
	VectorCursor& operator--() { Prev(); return *this; }

	VectorCursor operator++(int) { VectorCursor tmp; tmp.Next(); return tmp; }
	VectorCursor operator--(int) { VectorCursor tmp; tmp.Prev(); return tmp; }

protected:
	T* Get_Element();
	const T* Get_Element() const;

	const V& Get_Vector() const { return *VectorPtr; }
	int Get_Position() const { return Position; }

	virtual bool Can_Loop() const;

	virtual void Next() { ++Position; }
	virtual void Prev() { --Position; }

	bool Next_Match(T* that_elem);
	bool Next_Match(const T* that_elem);
	void Next_Valid();

protected:
	const V* VectorPtr;
	int Position;
};

template<typename T, class V>
VectorCursor<T, V>::VectorCursor(V* vector, int pos) :
	VectorPtr(vector),
	Position(pos)
{
}

template<typename T, class V>
VectorCursor<T, V>::VectorCursor(const V* vector, int pos) :
	VectorPtr(vector),
	Position(pos)
{
}

template<typename T, class V>
VectorCursor<T, V>::~VectorCursor()
{
}

template<typename T, class V>
VectorCursor<T, V>& VectorCursor<T, V>::operator=(const VectorCursor<T, V>& that)
{
	if (this != &that)
	{
		VectorPtr = that.VectorPtr;
		Position = that.Position;
	}
	return *this;
}

template<typename T, class V>
bool VectorCursor<T, V>::operator==(const VectorCursor<T, V>& that) const
{
	return VectorPtr == that.VectorPtr
		&& Position == that.Position;
}

template<typename T, class V>
bool VectorCursor<T, V>::operator!=(const VectorCursor<T, V>& that) const
{
	return VectorPtr != that.VectorPtr
		&& Position != that.Position;
}

template<typename T, class V>
T* VectorCursor<T, V>::Get_Element()
{
	if (!VectorCursor::Can_Loop())
	{
		return nullptr;
	}

	if (!VectorPtr)
	{
		return false;
	}

	return &const_cast<T&>((*VectorPtr)[Position]);
}

template<typename T, class V>
const T* VectorCursor<T, V>::Get_Element() const
{
	if (!VectorCursor::Can_Loop())
	{
		return nullptr;
	}

	if (!VectorPtr)
	{
		return nullptr;
	}

	return &(*VectorPtr)[Position];
}

template<typename T, class V>
bool VectorCursor<T, V>::Can_Loop() const
{
	if (!VectorPtr)
	{
		return false;
	}

	return Position < VectorPtr->Count;
}

template<typename T, class V>
bool VectorCursor<T, V>::Next_Match(T* that_elem)
{
	while (Can_Loop())
	{
		const T* elem = &(*VectorPtr)[Position];
		if (elem == that_elem)
		{
			break;
		}
		Next();
	}
}

template<typename T, class V>
bool VectorCursor<T, V>::Next_Match(const T* that_elem)
{
	while (Can_Loop())
	{
		const T* elem = &(*VectorPtr)[Position];
		if (elem == that_elem)
		{
			break;
		}
		Next();
	}
}

template<typename T, class V>
void VectorCursor<T, V>::Next_Valid()
{
	while (Can_Loop())
	{
		const T* v3 = &(*VectorPtr)[Position];
		if (v3 != nullptr)
		{
			break;
		}
		Next();
	}
}
enum class ArrayType : int
{
	Vector , DynamicVector , TypeList , Counter
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
	{
	}

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
		if (capacity != 0)
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
		if (!this->IsInitialized)
		{
			return 0;
		}

		for (auto i = 0; i < this->Capacity; ++i)
		{
			if (this->Items[i] == item)
			{
				return i;
			}
		}

		return -1;
	}

	virtual int GetItemIndex(const T* pItem) const final
	{
		if (!this->IsInitialized)
		{
			return 0;
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

	int Length() const { return Capacity; }

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
	{
	}

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
		if (!this->IsInitialized) {
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

	bool ValidIndex(int index) const
	{
		return static_cast<unsigned int>(index) < static_cast<unsigned int>(this->Count);
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

	T* begin() const
	{
		return (&this->Items[0]);
	}

	T* end() const
	{
		return (&this->Items[this->Count]);
	}

	T* begin()
	{
		return (&this->Items[0]);
	}

	T* end()
	{
		return (&this->Items[this->Count]);
	}

	bool AddItem(T item)
	{
		if (!this->IsValidArray())
			return false;

		this->Items[this->Count++] = std::move_if_noexcept(item);
		return true;
	}

	bool Insert(int index, const T& object)
	{
		if (!this->ValidIndex(index) || !this->IsValidArray()) {
			return false;
		}

		if (index < this->Count)
		{
			std::memmove(&this->Items[index + 1], &this->Items[index], (this->Count - index) * sizeof(T));
		}

		this->Items[index] = std::move_if_noexcept(object);
		++this->Count;

		return true;
	}
	
	template <class... _Valty>
	constexpr decltype(auto) emplace_back(_Valty&&... _Val) {
		AddItem(T{ _Val... });
		return *back();
	}
	
	bool AddUnique(const T& item)
	{
		int idx = this->FindItemIndex(item);
		return idx < 0 && this->AddItem(item);
	}

	bool RemoveAt(int index)
	{
		if (!this->ValidIndex(index)) {
			return false;
		}

		auto nBegin = (&this->Items[index]);
		auto nEnd = (&this->Items[this->Count]);

		if (nBegin != nEnd)
		{
			auto nNext = std::next(nBegin, 1); //pick next item after target
			const auto nDistance = nEnd - nNext; // calculate the distance
			std::memmove(nBegin, nNext, static_cast<size_t>(sizeof(T) * nDistance)); // move all the items from next to current pos
			--this->Count;//decrease the count
			return true;
		}

		return false;
	}

	bool Remove(const T& item)
	{
		auto nBegin = (&this->Items[0]);
		auto nEnd = (&this->Items[this->Count]);

		if (nBegin != nEnd)
		{
			while (*nBegin != item)
			{
				if (++nBegin == nEnd)
				{
					return false;
				}
			}

			auto nNext = std::next(nBegin, 1); //pick next item after target
			const auto nDistance = nEnd - nNext; // calculate the distance
			std::memmove(nBegin, nNext, static_cast<size_t>(sizeof(T) * nDistance)); // move all the items from next to current pos
			--this->Count;//decrease the count
			return true;
		}

		return false;
	}

	bool FindAndRemove(const T& item) {
		return this->RemoveItemAt(this->FindItemIndex(item));
	}

	void Swap(DynamicVectorClass<T>& other) noexcept
	{
		VectorClass<T>::Swap(other);
		using std::swap;
		swap(this->Count, other.Count);
		swap(this->CapacityIncrement, other.CapacityIncrement);
	}

	size_t Size() const
	{
		return static_cast<size_t>(Count);
	}

public:
	int Count { 0 };
	int CapacityIncrement { 10 };

protected:

	bool IsValidArray() {
		if (this->Count >= this->Capacity) {
			if ((this->IsAllocated || !this->Capacity) && this->CapacityIncrement > 0) {
				return this->SetCapacity(this->Capacity + this->CapacityIncrement, nullptr);
			} else {
				return false;
			}
		}

		return true;
	}
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
	using VecInt_type = VectorClass<int>;

	CounterClass(const CounterClass& other)
		: VecInt_type(other), Total(other.Total)
	{ }

	CounterClass(CounterClass&& other) noexcept
		: VecInt_type(std::move(other)), Total(other.Total)
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
		return (index < this->Capacity) ? this->Items[index] : 0;
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
		VecInt_type::Swap(other);
		using std::swap;
		swap(this->Total, other.Total);
	}

	//HRESULT Load(IStream* pStm) { JMP_THIS(0x49FBE0); }
	//HRESULT Save(IStream* pStm) { JMP_THIS(0x49FB70); }
public:
	int Total { 0 };
};

template<typename T, const int size>
class ArrayHelper
{
public:
	operator T* () { return (T*)this; }
	operator const T* () const { return (T*)this; }
	T* operator&() { return (T*)this; }
	const T* operator&() const { return (T*)this; }
	T& operator[](int index) { return ((T*)this)[index]; }
	const T& operator[](int index) const { return ((T*)this)[index]; }

	T& begin() { return ((T*)this)[0]; }
	T& end() { return ((T*)this)[size]; }

	const T& begin() const { return ((T*)this)[0]; }
	const T& end() const { return ((T*)this)[size]; }

	int Size() const {
		return size;
	}

protected:
	char _dummy[size * sizeof(T)];
};

template<typename T, const int y, const int x>
class ArrayHelper2D
{
public:
	operator ArrayHelper<T, x>* () { return (ArrayHelper<T, x> *)this; }
	operator const ArrayHelper<T, x>* () const { return (ArrayHelper<T, x> *)this; }
	ArrayHelper<T, x>* operator&() { return (ArrayHelper<T, x> *)this; }
	const ArrayHelper<T, x>* operator&() const { return (ArrayHelper<T, x> *)this; }
	ArrayHelper<T, x>& operator[](int index) { return _dummy[index]; }
	const ArrayHelper<T, x>& operator[](int index) const { return _dummy[index]; }

	ArrayHelper<T, x>& begin() { return _dummy[0]; }
	ArrayHelper<T, x>& end() { return _dummy[y]; }

	const ArrayHelper<T, x>& begin() const  { return _dummy[0]; }
	const ArrayHelper<T, x>& end() const { return _dummy[y]; }

	int Size() const {
		return y;
	}

protected:
	ArrayHelper<T, x> _dummy[y];
};
