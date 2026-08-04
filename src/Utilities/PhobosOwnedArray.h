#pragma once

// ============================================================================
// PhobosOwnedArray<TBase>
//
// Single-owner polymorphic container. Replaces the
//   vector<shared_ptr<Base>> + vector<shared_ptr<Derived>>
// dual-registration pattern used by MapTextBoxClass / MapChoiceBoxClass.
//
// Rationale:
//   - There is exactly one owner, so unique_ptr is sufficient; the refcount in
//     shared_ptr was only there because the object sat in two arrays.
//   - The derived arrays existed purely to avoid a type check. Every access
//     through them was already O(n), so filtering on a Kind enum costs nothing.
//   - With one container there is no second array to fall out of sync with,
//     which structurally removes an entire bug class (see ANALYSIS.md B1-B5).
//
// Requirements on TBase:
//   - virtual destructor
//   - a nested/associated enum type `KindType`
//   - `virtual KindType GetKind() const`
// Requirements on each TDerived:
//   - `static constexpr TBase::KindType ClassKind = ...;`
//
// Kind is an int compare, replacing the strcmp() dispatch currently used in
// DrawAll() / LoadGlobals().
// ============================================================================

#include <algorithm>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

template <typename TBase>
class PhobosOwnedArray
{
public:
	using KindType = typename TBase::KindType;
	using Pointer = std::unique_ptr<TBase>;
	using Storage = std::vector<Pointer>;
	using iterator = typename Storage::iterator;
	using const_iterator = typename Storage::const_iterator;

	PhobosOwnedArray() = default;

	PhobosOwnedArray(const PhobosOwnedArray&) = delete;
	PhobosOwnedArray& operator=(const PhobosOwnedArray&) = delete;

	PhobosOwnedArray(PhobosOwnedArray&&) = default;
	PhobosOwnedArray& operator=(PhobosOwnedArray&&) = default;

	// ===== Kind-checked cast =====================================================
	// Cheap replacement for strcmp(GetTypeMarker(), "...") and for dynamic_cast.

	template <typename TDerived>
	static TDerived* KindCast(TBase* pItem)
	{
		static_assert(std::is_base_of_v<TBase, TDerived>, "TDerived must derive from TBase");

		if (pItem && pItem->GetKind() == TDerived::ClassKind)
			return static_cast<TDerived*>(pItem);

		return nullptr;
	}

	template <typename TDerived>
	static const TDerived* KindCast(const TBase* pItem)
	{
		static_assert(std::is_base_of_v<TBase, TDerived>, "TDerived must derive from TBase");

		if (pItem && pItem->GetKind() == TDerived::ClassKind)
			return static_cast<const TDerived*>(pItem);

		return nullptr;
	}

	// ===== Creation ==============================================================

	// Constructs a TDerived in place and returns a non-owning pointer to it.
	// Replaces the make_shared + push_back(both arrays) + Array.back().get() dance.
	template <typename TDerived, typename... TArgs>
	TDerived* Create(TArgs&&... args)
	{
		static_assert(std::is_base_of_v<TBase, TDerived>, "TDerived must derive from TBase");

		auto pOwned = std::make_unique<TDerived>(std::forward<TArgs>(args)...);
		TDerived* const pRaw = pOwned.get();
		this->Items.emplace_back(std::move(pOwned));
		return pRaw;
	}

	// Takes ownership of an already-constructed object (used by LoadGlobals,
	// where the object must be constructed and deserialized before insertion).
	TBase* Adopt(Pointer pOwned)
	{
		TBase* const pRaw = pOwned.get();
		this->Items.emplace_back(std::move(pOwned));
		return pRaw;
	}

	// ===== Lookup ================================================================

	template <typename TPredicate>
	TBase* Find(TPredicate&& predicate)
	{
		for (const auto& pItem : this->Items)
		{
			if (pItem && predicate(*pItem))
				return pItem.get();
		}

		return nullptr;
	}

	// Finds the first item of the given Kind matching the predicate.
	// Replaces "search the derived array" without needing a derived array.
	template <typename TDerived, typename TPredicate>
	TDerived* FindOf(TPredicate&& predicate)
	{
		for (const auto& pItem : this->Items)
		{
			if (TDerived* const pTyped = KindCast<TDerived>(pItem.get()))
			{
				if (predicate(*pTyped))
					return pTyped;
			}
		}

		return nullptr;
	}

	bool Contains(const TBase* pItem) const
	{
		return std::any_of(this->Items.begin(), this->Items.end(),
			[pItem](const Pointer& pOwned) { return pOwned.get() == pItem; });
	}

	// ===== Iteration =============================================================

	// Visits every item of the given Kind.
	//
	// NOTE: the callback must not Create()/Remove() on this array - that would
	// invalidate the iterator. Set a flag and sweep with RemoveIf() afterwards
	// (this is the pattern DrawAll() already uses with its `expired` vector).
	template <typename TDerived, typename TFunc>
	void ForEachOf(TFunc&& func)
	{
		for (const auto& pItem : this->Items)
		{
			if (TDerived* const pTyped = KindCast<TDerived>(pItem.get()))
				func(*pTyped);
		}
	}

	template <typename TFunc>
	void ForEach(TFunc&& func)
	{
		for (const auto& pItem : this->Items)
		{
			if (pItem)
				func(*pItem);
		}
	}

	// ===== Removal ===============================================================

	// Removes and destroys a single item. Returns false if it wasn't present.
	bool Remove(const TBase* pItem)
	{
		auto const it = std::find_if(this->Items.begin(), this->Items.end(),
			[pItem](const Pointer& pOwned) { return pOwned.get() == pItem; });

		if (it == this->Items.end())
			return false;

		this->Items.erase(it);
		return true;
	}

	// Removes and destroys every item matching the predicate. Returns the count.
	template <typename TPredicate>
	size_t RemoveIf(TPredicate&& predicate)
	{
		auto const it = std::remove_if(this->Items.begin(), this->Items.end(),
			[&predicate](const Pointer& pOwned)
			{
				return pOwned && predicate(*pOwned);
			});

		size_t const removed = static_cast<size_t>(std::distance(it, this->Items.end()));
		this->Items.erase(it, this->Items.end());
		return removed;
	}

	// Removes and destroys every item of the given Kind matching the predicate.
	template <typename TDerived, typename TPredicate>
	size_t RemoveIfOf(TPredicate&& predicate)
	{
		return this->RemoveIf([&predicate](TBase& item)
			{
				TDerived* const pTyped = KindCast<TDerived>(&item);
				return pTyped && predicate(*pTyped);
			});
	}

	// Removes and destroys every item of the given Kind.
	// Replaces the broken WaypointXxx::ClearAll() / MapXxx::ClearAll() pair.
	template <typename TDerived>
	size_t RemoveAllOf()
	{
		return this->RemoveIf([](TBase& item)
			{
				return item.GetKind() == TDerived::ClassKind;
			});
	}

	void Clear() { this->Items.clear(); }

	// ===== Capacity / raw access =================================================

	size_t Size() const { return this->Items.size(); }
	bool IsEmpty() const { return this->Items.empty(); }
	void Reserve(size_t count) { this->Items.reserve(count); }

	// For serialization loops that need the owning pointers directly.
	Storage& Raw() { return this->Items; }
	const Storage& Raw() const { return this->Items; }

	iterator begin() { return this->Items.begin(); }
	iterator end() { return this->Items.end(); }
	const_iterator begin() const { return this->Items.begin(); }
	const_iterator end() const { return this->Items.end(); }

private:
	Storage Items;
};