#pragma once

#include <Base/Always.h>
#include <Memory.h>
#include <assert.h>
#include <type_traits>
#include <utility>

template<typename T, typename Pr = std::less<T>>
requires std::is_trivially_destructible<T>::value && std::is_destructible<T>::value
class TPriorityQueueClass
{
public:
	TPriorityQueueClass(int capacity = 0) :
		Count(0),
		Capacity(capacity),
		Nodes((T**)Allocate(sizeof(T*)* (capacity + 1))),
		LMost(0),
		RMost(-1)
	{
		if constexpr (!std::is_pointer<T>())
			memset(Nodes, 0, sizeof(T*) * (Count + 1));
		else
		{
			for (int i = 0; i < Count; ++i)
			{
				Nodes[i] = nullptr;
			}
		}
	}

	~TPriorityQueueClass()
	{
		Clear();
		Deallocate(Nodes);
		Nodes = (T**)nullptr;
	}

	void Clear()
	{
		if constexpr (!std::is_pointer<T>())
			memset(Nodes, 0, sizeof(T*) * (Count + 1));
		else
		{
			for (int i = 0; i < Count; ++i) {
				Nodes[i] = nullptr;
			}
		}
		Count = 0;
	}

	T* Top()
	{
		return Count == 0 ? nullptr : Nodes[1];
	}

	bool Pop()
	{
		if (Count == 0)
			return false;

		Nodes[1] = Nodes[Count--];
		int now = 1;
		while (now * 2 <= Count)
		{
			int next = now * 2;
			if (next < Count && Comp(Nodes[next + 1], Nodes[next]))
				++next;
			if (Comp(Nodes[now], Nodes[next]))
				break;

			// Westwood did Nodes[now] = Nodes[next] here
			std::swap(Nodes[now], Nodes[next]);

			now = next;
		}

		return true;
	}

	bool Push(T* pValue)
	{
		if (Count >= Capacity)
			return false;

		Nodes[++Count] = pValue;
		int now = Count;
		while (now != 1)
		{
			int next = now / 2;
			if (!Comp(Nodes[now], Nodes[next]))
				break;

			// Westwood did Nodes[now] = Nodes[next] here
			std::swap(Nodes[now], Nodes[next]);

			now = next;
		}

		return true;
	}

	bool WWPop()
	{
		if (Pop())
		{
			for (int i = 1; i <= Count; ++i)
				WWPointerUpdate(Nodes[i]);

			return true;
		}

		return false;
	}

	bool WWPush(T* pValue)
	{
		if (Push(pValue))
		{
			WWPointerUpdate(pValue);

			return true;
		}

		return false;
	}

	bool Comp(T* p1, T* p2)
	{
		return Pr()(*p1, *p2);
	}

	void WWPointerUpdate(T* pValue)
	{
		if ((uintptr_t)pValue > RMost)
			RMost = (uintptr_t)pValue;
		if ((uintptr_t)pValue < LMost)
			LMost = (uintptr_t)pValue;
	}

public:
	int Count; //capacity
	int Capacity; //HeapSize
	T** Nodes; //Heap
	uintptr_t LMost; //ptr1
	uintptr_t RMost; //ptr2
};

#ifdef compilerErr
template<typename TElement, typename TPriority, bool IsMinHeap = true>
class PriorityQueueClassNode final
{
public:
	PriorityQueueClassNode();
	PriorityQueueClassNode(const TElement& element, const TPriority& priority);
	PriorityQueueClassNode(TElement&& element, const TPriority& priority);
	PriorityQueueClassNode(const TElement& element, TPriority&& priority);
	PriorityQueueClassNode(TElement&& element, TPriority&& priority);
	PriorityQueueClassNode(const PriorityQueueClassNode<TElement, TPriority, IsMinHeap>& that) = default;
	PriorityQueueClassNode(PriorityQueueClassNode<TElement, TPriority, IsMinHeap>&& that) noexcept = default;

	~PriorityQueueClassNode() = default;

	PriorityQueueClassNode<TElement, TPriority, IsMinHeap>& operator=(const PriorityQueueClassNode<TElement, TPriority, IsMinHeap>& that) = default;
	PriorityQueueClassNode<TElement, TPriority, IsMinHeap>& operator=(PriorityQueueClassNode<TElement, TPriority, IsMinHeap>&& that) noexcept = default;

	const TElement& Get_Element() const { return Element; }
	TElement& Get_Element() { return Element; }

	const TPriority& Get_Priority() const { return Priority; }
	TPriority& Get_Priority() { return Priority; }

	void Set_Element(const TElement& element) { Element = element; }
	void Set_Element(TElement&& element) { Element = std::move(element); }

	void Set_Priority(const TPriority& priority) { Priority = priority; }
	void Set_Priority(TPriority&& priority) { Priority = std::move(priority); }

	template<typename bool IsMinHeap2 = IsMinHeap>
	static typename std::enable_if_t<IsMinHeap2, bool> Compare_Priority(const TPriority& lhs, const TPriority& rhs)
	{
		return std::greater<TPriority>()(lhs, rhs);
	}

	template<typename bool IsMinHeap2 = IsMinHeap>
	static typename std::enable_if_t<!IsMinHeap2, bool> Compare_Priority(const TPriority& lhs, const TPriority& rhs)
	{
		return std::less<TPriority>()(lhs, rhs);
	}

private:
	TElement Element;
	TPriority Priority;
};

template<typename TElement, typename TPriority, bool IsMinHeap = true>
class PriorityQueueClass final
{
	static_assert(std::is_default_constructible_v<TElement>, "TElement must be default constructible.");
	static_assert(std::is_default_constructible_v<TPriority>, "TPriority must be default constructible.");
	static_assert(std::is_copy_constructible_v<TElement>, "TElement must be copy constructible.");
	static_assert(std::is_copy_constructible_v<TPriority>, "TPriority must be copy constructible.");
	static_assert(std::is_move_constructible_v<TElement>, "TElement must be move constructible.");
	static_assert(std::is_move_constructible_v<TPriority>, "TPriority must be move constructible.");
	static_assert(std::is_copy_assignable_v<TElement>, "TElement must be copy assignable.");
	static_assert(std::is_copy_assignable_v<TPriority>, "TPriority must be copy assignable.");
	static_assert(std::is_move_assignable_v<TElement>, "TElement must be move assignable.");
	static_assert(std::is_move_assignable_v<TPriority>, "TPriority must be move assignable.");
	static_assert(std::equality_comparable<TElement>, "TElement must be equality comparable with itself.");

public:
	PriorityQueueClass(int size);
	PriorityQueueClass(const PriorityQueueClass<TElement, TPriority, IsMinHeap>& that);
	PriorityQueueClass(PriorityQueueClass<TElement, TPriority, IsMinHeap>&& that) noexcept;

	~PriorityQueueClass();

	PriorityQueueClass<TElement, TPriority, IsMinHeap>& operator=(const PriorityQueueClass<TElement, TPriority, IsMinHeap>& that);
	PriorityQueueClass<TElement, TPriority, IsMinHeap>& operator=(PriorityQueueClass<TElement, TPriority, IsMinHeap>&& that) noexcept;

	int Get_Count() const;
	int Get_Size() const;
	void Clear();
	bool Is_Empty() const;

	bool Insert(PriorityQueueClassNode<TElement, TPriority, IsMinHeap>* node);
	PriorityQueueClassNode<TElement, TPriority, IsMinHeap>* Extract();
	void Resize(int new_size);
	PriorityQueueClassNode<TElement, TPriority, IsMinHeap>* Find(const TElement& element) const;
	bool Remove_Matching(const TElement& element);
	bool Update_Priority(const TElement& element, const TPriority& priority);
	bool Update_Priority(const TElement& element, TPriority&& priority);

private:
	void Release_Heap();
	void Heapify_Up(int index);
	void Heapify_Down(int index);
	void Internal_Copy_Heap(const PriorityQueueClass<TElement, TPriority, IsMinHeap>& that);
	int Internal_Find(const TElement& element) const;

	static int Get_Parent_Index(int index);
	static int Get_Left_Index(int index);
	static int Get_Right_Index(int index);

private:
	int Count;
	int Size;
	PriorityQueueClassNode<TElement, TPriority, IsMinHeap>** Heap;
	std::uintptr_t MaxNodePointer;
	std::uintptr_t MinNodePointer;
};

template<typename TElement, typename TPriority, bool IsMinHeap>
PriorityQueueClassNode<TElement, TPriority, IsMinHeap>::PriorityQueueClassNode()
	: Element(), Priority()
{
}

template<typename TElement, typename TPriority, bool IsMinHeap>
PriorityQueueClassNode<TElement, TPriority, IsMinHeap>::PriorityQueueClassNode(const TElement& element, const TPriority& priority)
	: Element(element), Priority(priority)
{
}

template<typename TElement, typename TPriority, bool IsMinHeap>
PriorityQueueClassNode<TElement, TPriority, IsMinHeap>::PriorityQueueClassNode(TElement&& element, const TPriority& priority)
	: Element(std::move(element)), Priority(priority)
{
}

template<typename TElement, typename TPriority, bool IsMinHeap>
PriorityQueueClassNode<TElement, TPriority, IsMinHeap>::PriorityQueueClassNode(const TElement& element, TPriority&& priority)
	: Element(element), Priority(std::move(priority))
{
}

template<typename TElement, typename TPriority, bool IsMinHeap>
PriorityQueueClassNode<TElement, TPriority, IsMinHeap>::PriorityQueueClassNode(TElement&& element, TPriority&& priority)
	: Element(std::move(element)), Priority(std::move(priority))
{
}

template<typename TElement, typename TPriority, bool IsMinHeap>
PriorityQueueClass<TElement, TPriority, IsMinHeap>::PriorityQueueClass(int size)
	: Count(0), Size(size), MaxNodePointer(std::numeric_limits<decltype(MaxNodePointer)>::min()), MinNodePointer(std::numeric_limits<decltype(MinNodePointer)>::max())
{
	Heap = Allocate(PriorityQueueClassNode<TElement, TPriority, IsMinHeap> * [size + 1]);
	std::memset(Heap, 0, (size + 1) * sizeof(PriorityQueueClassNode<TElement, TPriority, IsMinHeap> *));
}

template<typename TElement, typename TPriority, bool IsMinHeap>
PriorityQueueClass<TElement, TPriority, IsMinHeap>::PriorityQueueClass(const PriorityQueueClass<TElement, TPriority, IsMinHeap>& that)
	: Count(that.Count), Size(that.Size), Heap(nullptr), MaxNodePointer(that.MaxNodePointer), MinNodePointer(that.MinNodePointer)
{
	Internal_Copy_Heap(that);
}

template<typename TElement, typename TPriority, bool IsMinHeap>
PriorityQueueClass<TElement, TPriority, IsMinHeap>::PriorityQueueClass(PriorityQueueClass<TElement, TPriority, IsMinHeap>&& that) noexcept
	: Count(that.Count), Size(that.Size), Heap(that.Heap), MaxNodePointer(that.MaxNodePointer), MinNodePointer(that.MinNodePointer)
{
	that.Heap = nullptr;
}

template<typename TElement, typename TPriority, bool IsMinHeap>
PriorityQueueClass<TElement, TPriority, IsMinHeap>::~PriorityQueueClass()
{
	/**
	  *  #OPTIMIZATION:
	  *  Original called Clear here and then deleted the heap.
	  *  Only releasing the heap suffices...
	 */
	Release_Heap();
}

template<typename TElement, typename TPriority, bool IsMinHeap>
PriorityQueueClass<TElement, TPriority, IsMinHeap>& PriorityQueueClass<TElement, TPriority, IsMinHeap>::operator=(const PriorityQueueClass<TElement, TPriority, IsMinHeap>& that)
{
	if (this != &that)
	{
		Release_Heap();

		Count = that.Count;
		Size = that.Size;
		MaxNodePointer = that.MaxNodePointer;
		MinNodePointer = that.MinNodePointer;

		Internal_Copy_Heap(that);
	}

	return *this;
}

template<typename TElement, typename TPriority, bool IsMinHeap>
PriorityQueueClass<TElement, TPriority, IsMinHeap>& PriorityQueueClass<TElement, TPriority, IsMinHeap>::operator=(PriorityQueueClass<TElement, TPriority, IsMinHeap>&& that) noexcept
{
	if (this != &that)
	{
		Release_Heap();

		Count = that.Count;
		Size = that.Size;
		Heap = that.Heap;
		MaxNodePointer = that.MaxNodePointer;
		MinNodePointer = that.MinNodePointer;

		that.Heap = nullptr;
	}

	return *this;
}

template<typename TElement, typename TPriority, bool IsMinHeap>
int PriorityQueueClass<TElement, TPriority, IsMinHeap>::Get_Count() const
{
	return Count;
}

template<typename TElement, typename TPriority, bool IsMinHeap>
int PriorityQueueClass<TElement, TPriority, IsMinHeap>::Get_Size() const
{
	return Size;
}

template<typename TElement, typename TPriority, bool IsMinHeap>
void PriorityQueueClass<TElement, TPriority, IsMinHeap>::Clear()
{
	std::memset(Heap, 0, (Count + 1) * sizeof(PriorityQueueClassNode<TElement, TPriority, IsMinHeap> *));
	Count = 0;
}

template<typename TElement, typename TPriority, bool IsMinHeap>
bool PriorityQueueClass<TElement, TPriority, IsMinHeap>::Is_Empty() const
{
	return (Count <= 0);
}

template<typename TElement, typename TPriority, bool IsMinHeap>
bool PriorityQueueClass<TElement, TPriority, IsMinHeap>::Insert(PriorityQueueClassNode<TElement, TPriority, IsMinHeap>* node)
{
	int new_count = Count + 1;
	if (new_count >= Size)
	{
		return false;
	}
	++Count;

	Heap[new_count] = node;

	Heapify_Up(new_count);

	MaxNodePointer = MaxImpl(MaxNodePointer, reinterpret_cast<std::uintptr_t>(node));
	MinNodePointer = MinImpl(MinNodePointer, reinterpret_cast<std::uintptr_t>(node));

	return true;
}

template<typename TElement, typename TPriority, bool IsMinHeap>
PriorityQueueClassNode<TElement, TPriority, IsMinHeap>* PriorityQueueClass<TElement, TPriority, IsMinHeap>::Extract()
{
	if (Is_Empty())
	{
		return nullptr;
	}

	PriorityQueueClassNode<TElement, TPriority, IsMinHeap>* root_node = Heap[1];
	Heap[1] = Heap[Count];
	Heap[Count] = nullptr;
	--Count;

	Heapify_Down(1);

	return root_node;
}

template<typename TElement, typename TPriority, bool IsMinHeap>
void PriorityQueueClass<TElement, TPriority, IsMinHeap>::Resize(int new_size)
{
	if (Size == new_size)
	{
		return;
	}

	PriorityQueueClassNode<TElement, TPriority, IsMinHeap>** new_heap =
		Allocate(PriorityQueueClassNode<TElement, TPriority, IsMinHeap> *[new_size + 1]);

	std::memset(new_heap, 0, (new_size + 1) * sizeof(PriorityQueueClassNode<TElement, TPriority, IsMinHeap> *));

	std::memcpy(new_heap, Heap, (Count + 1) * sizeof(PriorityQueueClassNode<TElement, TPriority, IsMinHeap> *));
	Release_Heap();
	Heap = new_heap;
	Size = new_size;

	if (Count >= new_size)
	{
		Count = (new_size <= 0 ? 0 : new_size - 1);
		MaxNodePointer = std::numeric_limits<decltype(MaxNodePointer)>::min();
		MinNodePointer = std::numeric_limits<decltype(MinNodePointer)>::max();

		for (int i = 1; i <= Count; ++i)
		{
			PriorityQueueClassNode<TElement, TPriority, IsMinHeap>* current_node = Heap[i];
			MaxNodePointer = MaxImpl(MaxNodePointer, reinterpret_cast<std::uintptr_t>(current_node));
			MinNodePointer = MinImpl(MinNodePointer, reinterpret_cast<std::uintptr_t>(current_node));
		}
	}
}

template<typename TElement, typename TPriority, bool IsMinHeap>
PriorityQueueClassNode<TElement, TPriority, IsMinHeap>* PriorityQueueClass<TElement, TPriority, IsMinHeap>::Find(const TElement& element) const
{
	int i = Internal_Find(element);
	return (i == -1 ? nullptr : Heap[i]);
}

template<typename TElement, typename TPriority, bool IsMinHeap>
bool PriorityQueueClass<TElement, TPriority, IsMinHeap>::Remove_Matching(const TElement& element)
{
	bool result = false;

	int i = Internal_Find(element);

	if (i != -1)
	{
		/**
		 *  #BUGFIX:
		 *  Set the last element to nullptr to keep our Heap clean.
		 */
		std::swap(Heap[i], Heap[Count]);
		Heap[Count] = nullptr;
		--Count;

		if (i <= Count)
		{
			int parent_index = Get_Parent_Index(i);
			if (i > 1 && PriorityQueueClassNode<TElement, TPriority, IsMinHeap>::Compare_Priority(Heap[i]->Get_Priority(), Heap[parent_index]->Get_Priority()))
			{
				Heapify_Up(i);
			}
			else
			{
				Heapify_Down(i);
			}
		}
	}

	return result;
}

template<typename TElement, typename TPriority, bool IsMinHeap>
bool PriorityQueueClass<TElement, TPriority, IsMinHeap>::Update_Priority(const TElement& element, const TPriority& priority)
{
	int i = Internal_Find(element);
	if (i == -1)
	{
		return false;
	}

	if (PriorityQueueClassNode<TElement, TPriority, IsMinHeap>::Compare_Priority(priority, Heap[i]->Get_Priority()))
	{
		Heap[i]->Set_Priority(priority);
		Heapify_Down(i);
	}
	else
	{
		Heap[i]->Set_Priority(priority);
		Heapify_Up(i);
	}

	return true;
}

template<typename TElement, typename TPriority, bool IsMinHeap>
bool PriorityQueueClass<TElement, TPriority, IsMinHeap>::Update_Priority(const TElement& element, TPriority&& priority)
{
	int i = Internal_Find(element);
	if (i == -1)
	{
		return false;
	}

	if (PriorityQueueClassNode<TElement, TPriority, IsMinHeap>::Compare_Priority(priority, Heap[i]->Get_Priority()))
	{
		Heap[i]->Set_Priority(std::move(priority));
		Heapify_Down(i);
	}
	else
	{
		Heap[i]->Set_Priority(std::move(priority));
		Heapify_Up(i);
	}

	return true;
}

template<typename TElement, typename TPriority, bool IsMinHeap>
void PriorityQueueClass<TElement, TPriority, IsMinHeap>::Release_Heap()
{
	Deallocate(Heap);
	Heap = nullptr;
}

template<typename TElement, typename TPriority, bool IsMinHeap>
void PriorityQueueClass<TElement, TPriority, IsMinHeap>::Heapify_Up(int index)
{
	while (index > 1)
	{
		int parent_index = Get_Parent_Index(index);
		PriorityQueueClassNode<TElement, TPriority, IsMinHeap>* current_node = Heap[index];
		PriorityQueueClassNode<TElement, TPriority, IsMinHeap>* parent_node = Heap[parent_index];

		if (PriorityQueueClassNode<TElement, TPriority, IsMinHeap>::Compare_Priority(current_node->Get_Priority(), parent_node->Get_Priority()))
		{
			break;
		}

		std::swap(Heap[index], Heap[parent_index]);
		index = parent_index;
	}
}

template<typename TElement, typename TPriority, bool IsMinHeap>
void PriorityQueueClass<TElement, TPriority, IsMinHeap>::Heapify_Down(int index)
{
	for (;;)
	{
		int absolute_node_index = index;

		int left_index = Get_Left_Index(index);
		if (left_index <= Count)
		{
			if (PriorityQueueClassNode<TElement, TPriority, IsMinHeap>::Compare_Priority(Heap[index]->Get_Priority(), Heap[left_index]->Get_Priority()))
			{
				absolute_node_index = left_index;
			}
		}

		int right_index = Get_Right_Index(index);
		if (right_index <= Count)
		{
			if (PriorityQueueClassNode<TElement, TPriority, IsMinHeap>::Compare_Priority(Heap[absolute_node_index]->Get_Priority(), Heap[right_index]->Get_Priority()))
			{
				absolute_node_index = right_index;
			}
		}

		if (absolute_node_index == index)
		{
			break;
		}

		std::swap(Heap[index], Heap[absolute_node_index]);
		index = absolute_node_index;
	}
}

template<typename TElement, typename TPriority, bool IsMinHeap>
void PriorityQueueClass<TElement, TPriority, IsMinHeap>::Internal_Copy_Heap(const PriorityQueueClass<TElement, TPriority, IsMinHeap>& that)
{
	Heap = Allocate(PriorityQueueClassNode<TElement, TPriority, IsMinHeap> *[Size + 1]);
	std::memset(Heap, 0, (Size + 1) * sizeof(PriorityQueueClassNode<TElement, TPriority, IsMinHeap> *));
	std::memcpy(Heap, that.Heap, (Count + 1) * sizeof(PriorityQueueClassNode<TElement, TPriority, IsMinHeap> *));
}

template<typename TElement, typename TPriority, bool IsMinHeap>
int PriorityQueueClass<TElement, TPriority, IsMinHeap>::Internal_Find(const TElement& element) const
{
	int i = 1;
	for (; i <= Count; ++i) {
		if (Heap[i]->Get_Element() == element)
		{
			return i;
		}
	}

	return -1;
}

template<typename TElement, typename TPriority, bool IsMinHeap>
int PriorityQueueClass<TElement, TPriority, IsMinHeap>::Get_Parent_Index(int index)
{
	return (index / 2);
}

template<typename TElement, typename TPriority, bool IsMinHeap>
int PriorityQueueClass<TElement, TPriority, IsMinHeap>::Get_Left_Index(int index)
{
	return (index * 2);
}

template<typename TElement, typename TPriority, bool IsMinHeap>
int PriorityQueueClass<TElement, TPriority, IsMinHeap>::Get_Right_Index(int index)
{
	return (index * 2 + 1);
}
#endif