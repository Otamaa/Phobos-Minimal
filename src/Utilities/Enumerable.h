#pragma once

#include <Phobos.CRT.h>
#include "Savegame.h"
#include "Constructs.h"
#include "Swizzle.h"

#include <algorithm>
#include <memory>
#include <vector>

#include <ArrayClasses.h>
#include <CCINIClass.h>

// an wrapper class to make `Type` like in the game
// remember to not modify the array ouside allocation new item(s) from the back
// it will mess upt the `ArrayIndex` !
template <typename T> class Enumerable
{
	typedef std::vector<std::unique_ptr<T>> container_t;

public:

	static container_t Array;

	static int FindOrAllocateIndex(const char* Title)
	{
		if (!*Title || !strlen(Title))
			return -1;

		const auto nResult = FindIndexById(Title);

		if (nResult < 0)
		{
			AllocateNoCheck(Title);
			return Array.size() - 1;
		}

		return nResult;
	}

	static int FindIndexById(const char* Title)
	{
		if (!*Title || !strlen(Title))
			return -1;

		for (auto pos = Array.begin();
			pos != Array.end();
			++pos) {
			if (IS_SAME_STR_((*pos)->Name.data(), Title)) {
				return std::distance(Array.begin() , pos);
			}
		}

		return -1;
	}

	static T* Find(const char* Title)
	{
		const auto nResult = FindIndexById(Title);

		if (nResult < 0)
			return nullptr;

		return Array[nResult].get();
	}

	static int FindIndexFromType(T* pType)
	{
		if (!pType)
			return -1;

		return FindIndexById(pType->Name.data());
	}

	// Warning : no Idx validation !
	static T* FindFromIndex(int Idx)
	{
		return Array[static_cast<size_t>(Idx)].get();
	}

	// With Idx validation ,return to the first item if Idx is invalid
	static T* FindFromIndexFix(int Idx)
	{
		const auto aIdx = size_t(Idx) > Array.size() ? 0 : Idx;
		return Array[aIdx].get();
	}

	static T* Allocate(const char* Title)
	{
		AllocateNoCheck(Title);
		return Array.back().get();
	}

	static void AllocateNoCheck(const char* Title)
	{
		Array.push_back(std::move(std::make_unique<T>(Title)));
	}

	static T* FindOrAllocate(const char* Title)
	{
		if (!*Title || !strlen(Title))
			return nullptr;

		if (T* find = Find(Title))
			return find;

		return Allocate(Title);
	}

	static void Clear()
	{
		Array.clear();
	}

	// pre-allocate all keys and read them later
	static void FindOrAllocateKeysFromINI(CCINIClass* pINI, bool bDebug = false)
	{
		const char* section = GetMainSection();

		if (!pINI->GetSection(section))
			return;

		auto const pKeyCount = pINI->GetKeyCount(section);

		if (!pKeyCount)
			return;

		for (int i = 0; i < pKeyCount; ++i) {
			if (pINI->ReadString(section, pINI->GetKeyName(section, i),
				Phobos::readDefval, Phobos::readBuffer) > 0) {
				FindOrAllocate(Phobos::readBuffer);
			}
		}
	}

	static void ReadListFromINI(CCINIClass* pINI, bool bDebug = false) {
		for (auto& pItem : Array)
			pItem->LoadFromINI(pINI);
	}

	static void LoadFromINIList(CCINIClass* pINI, bool bDebug = false)
	{
		if (!pINI)
			return;

		const char* section = GetMainSection();

		if (!pINI->GetSection(section))
			return;

		auto const pKeyCount = pINI->GetKeyCount(section);

		if (!pKeyCount)
			return;

		if (pKeyCount > (int)Array.size()) {
			Array.reserve(pKeyCount);
		}

		for (int i = 0; i < pKeyCount; ++i) {
			if (pINI->ReadString(section, pINI->GetKeyName(section, i),
				Phobos::readDefval, Phobos::readBuffer)  > 0) {

				if (auto const pFind = Find(Phobos::readBuffer)) {
					pFind->LoadFromINI(pINI);
				}
				else {
					Allocate(Phobos::readBuffer)->LoadFromINI(pINI);
				}
			}
		}
	}

	static bool LoadGlobals(PhobosStreamReader& Stm)
	{
		Clear();

		size_t Count = 0;
		if (!Stm.Load(Count))
			return false;

		Array.reserve(Count);

		for (size_t i = 0; i < Count; ++i)
		{
			void* oldPtr = nullptr;
			decltype(Name) name;

			if (!Stm.Load(oldPtr) ||
				!Stm.Load(name))
				return false;

			auto newPtr = Allocate(name);
			PhobosSwizzle::Instance.RegisterChange(oldPtr, newPtr);

			newPtr->LoadFromStream(Stm);
		}

		return true;
	}

	static bool SaveGlobals(PhobosStreamWriter& Stm)
	{
		Stm.Save(Array.size());

		for (const auto& item : Array)
		{
			// write old pointer and name, then delegate
			Stm.Save(item.get());
			Stm.Save(item->Name);
			item->SaveToStream(Stm);
		}

		return true;
	}

	static const char* GetMainSection();

	Enumerable(const char* Title)
	{
		Name = Title;
	}

	virtual ~Enumerable() = default;

	virtual void LoadFromINI(CCINIClass* pINI) { }

	virtual void LoadFromStream(PhobosStreamReader& Stm) = 0;

	virtual void SaveToStream(PhobosStreamWriter& Stm) = 0;

	FixedString<32> Name;
};