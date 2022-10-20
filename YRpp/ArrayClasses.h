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

//========================================================================
//=== VectorClass ========================================================
//========================================================================

template <typename T>
class VectorClass
{
public:
	// the hidden element count messes with alignment. only applies to align 8, 16, ...
	static_assert(!needs_vector_delete<T>::value || (__alignof(T) <= 4), "Alignment of T needs to be less than or equal to 4.");

	constexpr VectorClass() noexcept = default;

	explicit VectorClass(int capacity, T* pMem = nullptr) {
		if(capacity != 0) {
			this->Capacity = capacity;

			if(pMem) {
				this->Items = pMem;
			} else {
				this->Items = GameCreateArray<T>(static_cast<size_t>(capacity));
				this->IsAllocated = true;
			}
		}
	}

	VectorClass(const VectorClass<T>&other) {
		if(other.Capacity > 0) {
			this->Items = GameCreateArray<T>(static_cast<size_t>(other.Capacity));
			this->IsAllocated = true;
			this->Capacity = other.Capacity;
			for(auto i = 0; i < other.Capacity; ++i) {
				this->Items[i] = other.Items[i];
			}
		}
	}

	VectorClass(VectorClass<T>&&other) noexcept :
		Items(other.Items),
		Capacity(other.Capacity),
		IsInitialized(other.IsInitialized),
		IsAllocated(std::exchange(other.IsAllocated, false))
	{ }

	virtual ~VectorClass() noexcept {
		if(this->IsAllocated) {
			GameDeleteArray(this->Items, static_cast<size_t>(this->Capacity));
		}
	}

	VectorClass<T>& operator = (const VectorClass<T>&other) {
		VectorClass<T>(other).Swap(*this);
		return *this;
	}

	VectorClass<T>& operator = (VectorClass<T>&&other) noexcept {
		VectorClass<T>(std::move(other)).Swap(*this);
		return *this;
	}

	virtual bool operator == (const VectorClass<T>&other) const {
		if(this->Capacity != other.Capacity) {
			return false;
		}

		for(auto i = 0; i < this->Capacity; ++i) {
			if(this->Items[i] == other.Items[i]) {
				continue; // kapow! don't rewrite this to != unless you know why you're doing it
			}
			return false;
		}

		return true;
	}

	bool operator != (const VectorClass<T>&other) const {
		return !(*this == other);
	}

	virtual bool SetCapacity(int capacity, T* pMem = nullptr) {
		if(capacity != 0) {
			this->IsInitialized = false;

			bool bMustAllocate = (pMem == nullptr);
			if(!pMem) {
				pMem = GameCreateArray<T>(static_cast<size_t>(capacity));
			}

			this->IsInitialized = true;

			if(!pMem) {
				return false;
			}

			if(this->Items) {
				auto n = (capacity < this->Capacity) ? capacity : this->Capacity;
				for(auto i = 0; i < n; ++i) {
					pMem[i] = std::move(this->Items[i]);
				}

				if(this->IsAllocated) {
					GameDeleteArray(this->Items, static_cast<size_t>(this->Capacity));
					this->Items = nullptr;
				}
			}

			this->IsAllocated = bMustAllocate;
			this->Items = pMem;
			this->Capacity = capacity;
		} else {
			Clear();
		}
		return true;
	}

	virtual void Clear() {
		VectorClass<T>(std::move(*this));
		this->Items = nullptr;
		this->Capacity = 0;
	}

	virtual int FindItemIndex(const T& item) const {
		if(!this->IsInitialized) {
			return 0;
		}

		for(auto i = 0; i < this->Capacity; ++i) {
			if(this->Items[i] == item) {
				return i;
			}
		}

		return -1;
	}

	virtual int GetItemIndex(const T* pItem) const final {
		if(!this->IsInitialized) {
			return 0;
		}

		return pItem - this->Items;
	}

	virtual T GetItem(int i) const final {
		return this->Items[i];
	}

	T& operator [] (int i) {
		return this->Items[i];
	}

	const T& operator [] (int i) const {
		return this->Items[i];
	}

	bool Reserve(int capacity) {
		if(!this->IsInitialized) {
			return false;
		}

		if(this->Capacity >= capacity) {
			return true;
		}

		return SetCapacity(capacity, nullptr);
	}

	void Swap(VectorClass<T>& other) noexcept {
		using std::swap;
		swap(this->Items, other.Items);
		swap(this->Capacity, other.Capacity);
		swap(this->IsInitialized, other.IsInitialized);
		swap(this->IsAllocated, other.IsAllocated);
	}

	int Length() const { return Capacity; }

	T* Items{ nullptr };
	int Capacity{ 0 };
	bool IsInitialized{ true };
	bool IsAllocated{ false };
};

//========================================================================
//=== DynamicVectorClass =================================================
//========================================================================

template <typename T>
class DynamicVectorClass : public VectorClass<T>
{
public:
	constexpr DynamicVectorClass() noexcept = default;

	explicit DynamicVectorClass(int capacity, T* pMem = nullptr)
		: VectorClass<T>(capacity, pMem)
	{ }

	DynamicVectorClass(const DynamicVectorClass<T> &other) {
		if(other.Capacity > 0) {
			this->Items = GameCreateArray<T>(static_cast<size_t>(other.Capacity));
			this->IsAllocated = true;
			this->Capacity = other.Capacity;
			this->Count = other.Count;
			this->CapacityIncrement = other.CapacityIncrement;
			for(auto i = 0; i < other.Count; ++i) {
				this->Items[i] = other.Items[i];
			}
		}
	}

	DynamicVectorClass(DynamicVectorClass<T>&&other) noexcept
		: VectorClass<T>(std::move(other)), Count(other.Count),
		CapacityIncrement(other.CapacityIncrement)
	{ }

	DynamicVectorClass<T>& operator = (const DynamicVectorClass<T>&other) {
		DynamicVectorClass<T>(other).Swap(*this);
		return *this;
	}

	DynamicVectorClass<T>& operator = (DynamicVectorClass<T>&&other) noexcept {
		DynamicVectorClass<T>(std::move(other)).Swap(*this);
		return *this;
	}

	virtual bool SetCapacity(int capacity, T* pMem = nullptr) override {
		bool bRet = VectorClass<T>::SetCapacity(capacity, pMem);

		if(this->Capacity < this->Count) {
			this->Count = this->Capacity;
		}

		return bRet;
	}

	virtual void Clear() override {
		VectorClass<T>::Clear();
		this->Count = 0;
	}

	virtual int FindItemIndex(const T& item) const override final {
		if(!this->IsInitialized) {
			return 0;
		}

		for(int i = 0; i < this->Count; i++) {
			if(this->Items[i] == item) {
				return i;
			}
		}

		return -1;
	}

	bool ValidIndex(int index) const {
		return static_cast<unsigned int>(index) < static_cast<unsigned int>(this->Count);
	}

	T GetItemOrDefault(int i) const {
		return this->GetItemOrDefault(i, T());
	}

	T GetItemOrDefault(int i, T def) const {
		if(this->ValidIndex(i)) {
			return this->Items[i];
		}
		return def;
	}

	T* begin() const {
		return (&this->Items[0]);
	}

	T* end() const {
		return (&this->Items[this->Count]);
	}

	T* begin() {
		return (&this->Items[0]);
	}

	T* end() {
		return (&this->Items[this->Count]);
	}

	bool AddItem(T item) {
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

		this->Items[this->Count++] = std::move(item);
		return true;
	}

	bool Insert(int index, const T& object)
	{
		if (index < 0 || index > this->Count) {
			return false;
		}

		if (this->Count >= this->Capacity) {
			if ((this->IsAllocated || !this->Capacity) && this->CapacityIncrement > 0) {
				if (!this->SetCapacity(this->Capacity + this->CapacityIncrement, nullptr)) {
					return false;
				}
			}
			else {
				return false;
			}
		}

		if (index < this->Count) {
			std::memmove(&(*this)[index + 1], &(*this)[index], (this->Count - index) * sizeof(T));
		}

		(*this)[index] = std::move(object);
		++this->Count;

		return true;
	}

	bool AddUnique(const T &item) {
		int idx = this->FindItemIndex(item);
		return idx < 0 && this->AddItem(item);
	}

	bool RemoveItem(int index) {
		if(!this->ValidIndex(index)) {
			return false;
		}

		--this->Count;
		for(int i = index; i < this->Count; ++i) {
			this->Items[i] = std::move(this->Items[i + 1]);
		}

		return true;
	}

	bool Remove(const T &item) {
#ifdef Causing_Crash
		auto const iter = std::find_if(&Items[0], &Items[Count], [&](auto const& arritem) { return arritem == item; });

		if (iter != (&Items[Count]))
		{
			memmove(iter, (iter + 1), (&Items[Count]) - (iter + 1));
			--Count;
			return true;
		}
		return false;
#else
		int idx = this->FindItemIndex(item);
		return idx >= 0 && this->RemoveItem(idx);
#endif
	}


	void Swap(DynamicVectorClass<T>& other) noexcept {
		VectorClass<T>::Swap(other);
		using std::swap;
		swap(this->Count, other.Count);
		swap(this->CapacityIncrement, other.CapacityIncrement);
	}

	size_t Size() const {
		return static_cast<size_t>(Count);
	}

	// is this even right ?
	int GetSizeMax() const {
		return (sizeof(T) * Count) + 4;
	}

public:
	int Count{ 0 };
	int CapacityIncrement{ 10 };
};

//========================================================================
//=== TypeList ===========================================================
//========================================================================

template <typename T>
class TypeList : public DynamicVectorClass<T>
{
public:
	constexpr TypeList() noexcept = default;

	explicit TypeList(int capacity, T* pMem = nullptr)
		: DynamicVectorClass<T>(capacity, pMem)
	{ }

	TypeList(const TypeList<T> &other)
		: DynamicVectorClass<T>(other), unknown_18(other.unknown_18)
	{ }

	TypeList(TypeList<T>&&other) noexcept
		: DynamicVectorClass<T>(std::move(other)), unknown_18(other.unknown_18)
	{ }

	TypeList<T>& operator = (const TypeList<T>&other) {
		TypeList<T>(other).Swap(*this);
		return *this;
	}

	TypeList<T>& operator = (TypeList<T>&&other) noexcept {
		TypeList<T>(std::move(other)).Swap(*this);
		return *this;
	}

	void Swap(TypeList<T>& other) noexcept {
		DynamicVectorClass<T>::Swap(other);
		using std::swap;
		swap(this->unknown_18, other.unknown_18);
	}

	int unknown_18{ 0 };
};

//========================================================================
//=== CounterClass =======================================================
//========================================================================

class CounterClass : public VectorClass<int>
{
public:
	constexpr CounterClass() noexcept = default;

	CounterClass(const CounterClass& other)
		: VectorClass<int>(other), Total(other.Total)
	{ }

	CounterClass(CounterClass &&other) noexcept
		: VectorClass<int>(std::move(other)), Total(other.Total)
	{ }

	CounterClass& operator = (const CounterClass &other) {
		CounterClass(other).Swap(*this);
		return *this;
	}

	CounterClass& operator = (CounterClass &&other) noexcept {
		CounterClass(std::move(other)).Swap(*this);
		return *this;
	}

	virtual void Clear() override {
		for(int i = 0; i < this->Capacity; ++i) {
			this->Items[i] = 0;
		}

		this->Total = 0;
	}

	int GetTotal() const {
		return this->Total;
	}

	bool EnsureItem(int index) {
		if(index < this->Capacity) {
			return true;
		}

		int count = this->Capacity;
		if(this->SetCapacity(index + 10, nullptr)) {
			for(auto i = count; i < this->Capacity; ++i) {
				this->Items[i] = 0;
			}
			return true;
		}

		return false;
	}

	int operator[] (int index) const {
		return this->GetItemCount(index);
	}

	int GetItemCount(int index) {
		return this->EnsureItem(index) ? this->Items[index] : 0;
	}

	int GetItemCount(int index) const {
		return (index < this->Capacity) ? this->Items[index] : 0;
	}

	int Increment(int index) {
		if(this->EnsureItem(index)) {
			++this->Total;
			return ++this->Items[index];
		}
		return 0;
	}

	int Decrement(int index) {
		if(this->EnsureItem(index)) {
			--this->Total;
			return --this->Items[index];
		}
		return 0;
	}

	void Swap(CounterClass& other) noexcept {
		VectorClass<int>::Swap(other);
		using std::swap;
		swap(this->Total, other.Total);
	}

	int Total{ 0 };
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

protected:
	ArrayHelper<T, x> _dummy[y];
};
