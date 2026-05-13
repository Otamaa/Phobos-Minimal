#pragma once

#include <Utilities/Macro.h>
#include <Utilities/Debug.h>

#include <AStarClass.h>
#include <FootClass.h>
#include <TechnoClass.h>
#include <Memory.h>

template<typename T, typename Pr = std::less<T>>
class SafePriorityQueueClass
{
public:
	SafePriorityQueueClass(int capacity) :
		Count(0),
		Capacity(capacity),
		MaxPointer(nullptr),
		MinPointer(reinterpret_cast<T*>(~0ULL))
	{
		if (capacity <= 0)
		{
			Debug::FatalError("PriorityQueue created with invalid capacity: %d\n", capacity);
			capacity = 1;
		}

		Nodes = static_cast<T**>(YRMemory::AllocateChecked(sizeof(T*) * (capacity + 1)));
		std::memset(Nodes, 0, sizeof(T*) * (capacity + 1));
	}

	~SafePriorityQueueClass()
	{
		Clear();
		YRMemory::Deallocate(Nodes);
		Nodes = nullptr;
	}

	bool Push(T* pValue)
	{
		// Null check
		if (!pValue)
		{
			Debug::Log("Attempt to push null into PriorityQueue\n");
			return false;
		}

		// Capacity check
		if (Count >= Capacity)
		{
			Debug::Log("PriorityQueue overflow: Count=%d, Capacity=%d\n", Count, Capacity);
			return false;
		}

		// Insert at end
		Nodes[++Count] = pValue;

		// Bubble up
		int current = Count;
		while (current > 1)
		{
			int parent = current / 2;

			// Null safety
			if (!Nodes[current] || !Nodes[parent])
			{
				Debug::Log("Null node during bubble-up at index %d\n", current);
				break;
			}

			if (!Comp(Nodes[current], Nodes[parent]))
				break;

			std::swap(Nodes[current], Nodes[parent]);
			current = parent;
		}

		// Update bounds
		UpdatePointerBounds(pValue);

		return true;
	}

	T* Pop()
	{
		if (Count <= 0)
			return nullptr;

		T* result = Nodes[1];

		// Move last to root
		Nodes[1] = Nodes[Count];
		Nodes[Count] = nullptr;
		--Count;

		// Heapify down
		if (Count > 0)
			HeapifyDown(1);

		return result;
	}

	T* Top() const
	{
		return (Count > 0) ? Nodes[1] : nullptr;
	}

	void Clear()
	{
		int clearCount = std::min(Count + 1, Capacity + 1);
		for (int i = 0; i < clearCount; ++i)
		{
			Nodes[i] = nullptr;
		}
		Count = 0;
		MaxPointer = nullptr;
		MinPointer = reinterpret_cast<T*>(~0ULL);
	}

	bool IsEmpty() const { return Count <= 0; }
	int GetCount() const { return Count; }
	int GetCapacity() const { return Capacity; }

	// Debug validation
	bool ValidateHeap() const
	{
		for (int i = 1; i <= Count; ++i)
		{
			if (!Nodes[i])
			{
				Debug::Log("Null node at index %d (Count=%d)\n", i, Count);
				return false;
			}

			int left = i * 2;
			int right = left + 1;

			if (left <= Count && Nodes[left] && Comp(Nodes[left], Nodes[i]))
			{
				Debug::Log("Heap property violated: left child %d < parent %d\n", left, i);
				return false;
			}

			if (right <= Count && Nodes[right] && Comp(Nodes[right], Nodes[i]))
			{
				Debug::Log("Heap property violated: right child %d < parent %d\n", right, i);
				return false;
			}
		}
		return true;
	}

private:
	void HeapifyDown(int index)
	{
		while (true)
		{
			int smallest = index;
			int left = index * 2;
			int right = left + 1;

			if (left <= Count && left <= Capacity)
			{
				if (Nodes[left] && Nodes[smallest] && Comp(Nodes[left], Nodes[smallest]))
					smallest = left;
			}

			if (right <= Count && right <= Capacity)
			{
				if (Nodes[right] && Nodes[smallest] && Comp(Nodes[right], Nodes[smallest]))
					smallest = right;
			}

			if (smallest == index)
				break;

			std::swap(Nodes[index], Nodes[smallest]);
			index = smallest;
		}
	}

	bool Comp(T* p1, T* p2) const
	{
		if (!p1 || !p2)
			return p1 < p2;
		return Pr()(*p1, *p2);
	}

	void UpdatePointerBounds(T* pValue)
	{
		if (pValue > MaxPointer)
			MaxPointer = pValue;
		if (pValue < MinPointer)
			MinPointer = pValue;
	}

public:
	int Count;
	int Capacity;
	T** Nodes;
	T* MaxPointer;
	T* MinPointer;
};

class NOVTABLE FakeAStarPathFinderClass : public AStarPathFinderClass
{
public:
	PathType* __AStarClass__Find_Path(CellStruct* a2,
		CellStruct* dest,
		TechnoClass* a4,
		int* path,
		int max_count,
		MovementZone a7,
		ZoneType cellPath);

	bool __Find_Path_Hierarchical(CellStruct* from, CellStruct* to, MovementZone mzone, FootClass* foot);
};
