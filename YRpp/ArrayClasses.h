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

	VectorClass(noinit_t const&) { };
	COMPILETIMEEVAL VectorClass<T>() noexcept = default;

	explicit VectorClass<T>(int capacity, T* pMem = nullptr) :
		Items(0),
		Capacity(capacity),
		IsInitialized(true),
		IsAllocated(false)
	{
		if (capacity != 0)
		{
			if (pMem) {
				this->Items = pMem;
			}
			else{
				GameAllocator<T> alloc {};
				this->Items = Memory::CreateArray<T>(alloc, static_cast<size_t>(capacity));
				this->IsAllocated = true;
			}
		}
	}

	VectorClass<T>(const VectorClass<T>& other)
	{
		if (other.Capacity > 0)
		{
			GameAllocator<T> alloc {};
			this->Items = Memory::CreateArray<T>(alloc, static_cast<size_t>(other.Capacity));
			this->IsAllocated = true;
			this->Capacity = other.Capacity;
			for (auto i = 0; i < other.Capacity; ++i) {
				this->Items[i] = other.Items[i];
			}
		}
	}

	VectorClass<T> (VectorClass<T>&& other) noexcept :
		Items(other.Items),
		Capacity(other.Capacity),
		IsInitialized(other.IsInitialized),
		IsAllocated(std::exchange(other.IsAllocated, false))
	{
	}

	virtual ~VectorClass<T>() noexcept
	{
		if (this->IsAllocated) {
			GameAllocator<T> alloc {};
			Memory::DeleteArray(alloc, this->Items, static_cast<size_t>(this->Capacity));
		}

		this->Items = nullptr;
		this->IsAllocated = false;
		this->Capacity = 0;
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
				GameAllocator<T> alloc {};
				pMem = Memory::CreateArray<T>(alloc, (size_t)capacity);
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
					GameAllocator<T> alloc {};
					Memory::DeleteArray(alloc, this->Items,(size_t)this->Capacity);
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
		if (this->IsAllocated) {
			GameAllocator<T> alloc {};
			Memory::DeleteArray(alloc, this->Items, static_cast<size_t>(this->Capacity));
		}

		this->IsAllocated = false;
		this->Items = nullptr;
		this->Capacity = 0;
	}

	virtual int FindItemIndex(const T& item) const
	{
		static_assert(direct_comparable<T>, "Missing equality operator !");

		if (!this->IsInitialized) {
			return 0;
		}

		for (auto i = 0; i < this->Capacity; ++i) {
			if (this->Items[i] == item) {
				return i;
			}
		}

		return -1;
	}

	virtual int GetItemIndex(const T* pItem) const final
	{
		if (!this->IsInitialized) {
			return 0;
		}

		return(((unsigned long)pItem - (unsigned long)&(*this)[0]) / sizeof(T));
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

protected:
	bool VectorClassPad[2];
};

//========================================================================
//=== DynamicVectorClass =================================================
//========================================================================

//TODO : unify the naming !
template <typename T>
class DynamicVectorClass : public VectorClass<T>
{
public:
	static const ArrayType Type = ArrayType::DynamicVector;

#pragma region constructorandoperators
	COMPILETIMEEVAL DynamicVectorClass<T>() noexcept = default;

	explicit DynamicVectorClass<T>(int capacity, T* pMem = nullptr)
		: VectorClass<T>(capacity, pMem) , Count { 0 }, CapacityIncrement { 10 }
	{ }

	DynamicVectorClass<T>(const DynamicVectorClass<T>& other)
	{
		if (other.Capacity > 0)
		{
			GameAllocator<T> alloc {};
			this->Items = Memory::CreateArray<T>(alloc, static_cast<size_t>(other.Capacity));
			this->IsAllocated = true;
			this->Capacity = other.Capacity;
			this->Count = other.Count;
			this->CapacityIncrement = other.CapacityIncrement;
			for (auto i = 0; i < other.Count; ++i) {
				this->Items[i] = other.Items[i];
			}
		}
	}

	DynamicVectorClass<T>(DynamicVectorClass<T>&& other) noexcept
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

#pragma endregion

#pragma region virtuals

	virtual ~DynamicVectorClass<T>() = default;

	virtual bool SetCapacity(int capacity, T* pMem = nullptr) override
	{
		if (VectorClass<T>::SetCapacity(capacity, pMem) && this->Capacity < this->Count) {
			this->Count = this->Capacity;
			return true;
		}

		return false;
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

		T* iter = this->Find(item);
		return iter != this->end() ?  std::distance(this->begin() , iter) : -1;
	}

#pragma endregion

#pragma region iteratorpointer
	int FindItemIndexFromIterator(T* iter) {
		if (!this->IsInitialized) {
			return 0;
		}

		if (iter == this->end())
			return -1;

		return std::distance(this->begin(), iter);
	}

	COMPILETIMEEVAL T* begin() const
	{
		return &this->Items[0];
	}

	COMPILETIMEEVAL T* end() const
	{
		return &this->Items[this->Count];
	}

	COMPILETIMEEVAL T* front() const {
		return &this->Items[0];
	}

	COMPILETIMEEVAL T* back() const {
		return  &this->Items[(this->Count - 1)];
	}

	COMPILETIMEEVAL T* begin()
	{
		return &this->Items[0];
	}

	COMPILETIMEEVAL T* end()
	{
		return &this->Items[this->Count];
	}


	COMPILETIMEEVAL size_t size() const {
		return static_cast<size_t>(Count);
	}

#pragma endregion

#pragma region Funcs

	// this one doesnt destroy the memory , just reset the count
	// the vector memory may still contains dangling pointer if it vector of pointer
	COMPILETIMEEVAL void FORCEDINLINE Reset(int resetCount = 0) {
		this->Count = resetCount;
	}

	COMPILETIMEEVAL bool FORCEDINLINE ValidIndex(int index) const {
		return static_cast<size_t>(index) < static_cast<size_t>(this->Count);
	}

	COMPILETIMEEVAL bool FORCEDINLINE ValidIndex(size_t index) const {
		return index < static_cast<size_t>(this->Count);
	}

	T GetItemOrDefault(size_t i) const
	{
		return this->GetItemOrDefault(i, T());
	}

	COMPILETIMEEVAL T GetItemOrDefault(size_t i, T def) const
	{
		if (!this->ValidIndex(i))
			return def;

		return this->Items[i];
	}

	bool Contains(const T& item) const
	{
		if (this->Count <= 0) {
			return false;
		}

		return this->Find(item) != this->end();
	}

	bool AddItem(T&& item)
	{
		if (!this->IsValidArray())
			return false;

		this->Items[this->Count++] = std::move_if_noexcept(item);

		return true;
	}

	template< class... Args >
	void EmpalaceItem(Args&&... args) {
		AddItem(T(std::forward<Args>(args)...));
	}

	bool AddItem(const T& item)
	{
		if (!this->IsValidArray())
			return false;

		this->Items[this->Count++] = std::move_if_noexcept(item);

		return true;
	}

	bool AddHead(const T& object)
	{
		if (!this->ValidArray())
			return false;

		if (this->Count)
		{
			T* next = std::next(this->begin());
			std::memmove(next, this->begin(), this->Count * sizeof(T));
		}

		this->Items[0] = std::move_if_noexcept(object);
		this->Count++;

		return true;
	}

	bool InsertAt(int index, const T& object)
	{
		if (!this->ValidIndex(index))
			return false;

		if (this->IsValidArray()) {

			T* nSource = this->Items + index;
			T* nDest = std::next(nSource);
			std::memmove(nDest, nSource, (this->begin() - nSource) * sizeof(T));

			this->Items[index] = std::move_if_noexcept(object);
			++this->Count;

			return true;
		}

		return false;
	}

	T* UninitializedAdd() {
		return this->IsValidArray() ?
			&(this->Items[this->Count++]) : nullptr;
	}

	bool AddUnique(const T& item)
	{
		const int count = this->Count;
		return count <= 0 || this->Find(item) == this->end() ? this->AddItem(item) : false;
	}

	bool FORCEDINLINE COMPILETIMEEVAL Empty() { return this->Count <= 0;}

	template<bool avoidmemcpy = false>
	bool RemoveAt(int index)
	{
		if (this->ValidIndex(index)) {
			if COMPILETIMEEVAL (!avoidmemcpy) {
				T* find = this->Items + index;
				T* end = this->Items + this->Count;

				if (find != end)
				{

					T* next = std::next(find);
					// move all the items from next to current pos
					std::memmove(find, next, (end - next) * sizeof(T));
					--this->Count;//decrease the count
					return true;
				}
			} else {

				--this->Count;
				for (int i = index; i < this->Count; ++i) {
					this->Items[i] = std::move_if_noexcept(this->Items[i + 1]);
				}

				return true;
			}
		}
		return false;
	}

	template<bool avoidmemcpy = false>
	bool Remove(const T& item)
	{
		if COMPILETIMEEVAL (!avoidmemcpy)
		{
			T* end = this->Items + this->Count;
			T* iter = this->Find(item);

			if (iter != this->end())
			{
				T* next = std::next(iter);
				std::memmove(iter, next, (end - next) * sizeof(T));
				--this->Count;
				return true;
			}

			return false;
		}
		else
		{
			return this->RemoveAt<true>(this->FindItemIndex(item));
		}
	}

	template<typename Func>
	COMPILETIMEEVAL bool FORCEINLINE remove_if(Func&& act)
	{
		if (!this->IsAllocated) return false;
		this->Reset(std::distance(this->begin() ,std::remove_if(this->begin(), this->end(), act)));
		return true;
	}

	bool FORCEDINLINE FindAndRemove(const T& item) {
		return this->RemoveAt(this->FindItemIndex(item));
	}

	void Swap(DynamicVectorClass<T>& other) noexcept
	{
		VectorClass<T>::Swap(other);
		using std::swap;
		swap(this->Count, other.Count);
		swap(this->CapacityIncrement, other.CapacityIncrement);
	}

	FORCEDINLINE T* Find(const T& item) const {
		if COMPILETIMEEVAL (direct_comparable<T>) {
			return this->find_if([item](const auto item_here) { return item_here == item; });
		} else {
			return std::find(this->begin(), this->end(), item);
		}
	}
#pragma endregion

#pragma region WrappedSTD
	template <typename Func>
	COMPILETIMEEVAL auto FORCEDINLINE find_if(Func&& act) const {
	    for (auto i = this->begin(); i != this->end(); ++i) {
			if (act(*i)) {
				return i;
			}
   		}

		return this->end();
	}

	template <typename Func>
	COMPILETIMEEVAL auto FORCEDINLINE find_if(Func&& act) {
		for (auto i = this->begin(); i != this->end(); ++i) {
			if (act(*i)) {
				return i;
			}
   		}

		return this->end();
	}

	template <typename Func>
	COMPILETIMEEVAL void FORCEDINLINE for_each(Func&& act) const {
		for (auto i = this->begin(); i != this->end(); ++i) {
        	act(*i);
    	}
	}

	template <typename Func>
	COMPILETIMEEVAL void FORCEDINLINE for_each(Func&& act) {
		for (auto i = this->begin(); i != this->end(); ++i) {
        	act(*i);
    	}
	}

	template<typename func>
	COMPILETIMEEVAL bool FORCEDINLINE none_of(func&& fn) const {
		for (auto i = this->begin(); i != this->end(); ++i) {
       	 	if (fn(*i)) {
           	 	return false;
        	}
    	}

    	return true;
	}

	template<typename func>
	COMPILETIMEEVAL bool FORCEDINLINE none_of(func&& fn) {
		for (auto i = this->begin(); i != this->end(); ++i) {
       	 	if (fn(*i)) {
           	 	return false;
        	}
    	}

    	return true;
	}

	template<typename func>
	COMPILETIMEEVAL bool FORCEDINLINE any_of(func&& fn) const {
		for (auto i = this->begin(); i != this->end(); ++i) {
       		if (fn(*i)) {
            	return true;
			}
        }

		return false;
	}

	template<typename func>
	COMPILETIMEEVAL bool FORCEDINLINE any_of(func&& fn) {
		for (auto i = this->begin(); i != this->end(); ++i) {
       		if (fn(*i)) {
            	return true;
			}
        }

		return false;
	}
#pragma endregion

	bool FORCEDINLINE IsValidArray()
	{
		if (this->Count >= this->Capacity)
		{
			if ((this->IsAllocated || !this->Capacity) && this->CapacityIncrement > 0)
			{
				return this->SetCapacity(this->Capacity + this->CapacityIncrement, nullptr);
			}
			else
			{
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
	COMPILETIMEEVAL TypeList<T>() noexcept = default;
	static const ArrayType Type = ArrayType::TypeList;
	using VectType = DynamicVectorClass<T>;

	explicit TypeList<T>(int capacity, T* pMem = nullptr)
		: VectType(capacity, pMem)
	{ }

	TypeList<T>(const TypeList<T>& other)
		: VectType(other), unknown_18(other.unknown_18)
	{ }

	TypeList<T>(TypeList<T>&& other) noexcept
		: VectType(std::move(other)), unknown_18(other.unknown_18)
	{ }

	virtual ~TypeList<T>() = default;

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
		VectType::Swap(other);
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
	COMPILETIMEEVAL CounterClass() noexcept = default;
	static const ArrayType Type = ArrayType::Counter;
	using VectType = VectorClass<int>;

	CounterClass(const CounterClass& other)
		: VectType(other), Total(other.Total)
	{ }

	CounterClass(CounterClass&& other) noexcept
		: VectType(std::move(other)), Total(other.Total)
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

	virtual ~CounterClass() = default;

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

	COMPILETIMEEVAL int GetItemCount(int index) const
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
		VectorClass<int>::Swap(other);
		using std::swap;
		swap(this->Total, other.Total);
	}

	//HRESULT LoadFromStream(IStream* pStm) { JMP_THIS(0x49FBE0); }
	//HRESULT SaveFromStream(IStream* pStm) { JMP_THIS(0x49FB70); }
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
