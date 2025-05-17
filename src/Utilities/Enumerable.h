#pragma once

#include <Phobos.CRT.h>

#include "Savegame.h"
#include "Swizzle.h"

#include <algorithm>
#include <CCINIClass.h>

#include <Utilities/PhobosFixedString.h>

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
		for (auto pos = Array.begin();
			pos != Array.end();
			++pos) {
			if (IS_SAME_STR_(pos->get()->Name.data(), Title)) {
				return std::distance(Array.begin(), pos);
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

	static OPTIONALINLINE COMPILETIMEEVAL int FindIndexFromType(T* pType)
	{
		if (pType) {
			for (size_t i = 0; i < Array.size(); ++i) {
				if (Array[i].get() == pType)
					return i;
			}
		}

		return -1;
	}

	static OPTIONALINLINE COMPILETIMEEVAL T* TryFindFromIndex(int Idx) {

		if (size_t(Idx) > Array.size())
			return nullptr;

		return Array[static_cast<size_t>(Idx)].get();
	}

	// Warning : no Idx validation !
	static OPTIONALINLINE COMPILETIMEEVAL T* FindFromIndex(int Idx)
	{
		return Array[static_cast<size_t>(Idx)].get();
	}

	// With Idx validation ,return to the first item if Idx is invalid
	static OPTIONALINLINE COMPILETIMEEVAL T* FindFromIndexFix(int Idx)
	{
		if (Array.empty())
			return nullptr;

		return Array[size_t(Idx) > Array.size() ? 0 : Idx].get();
	}

	static OPTIONALINLINE COMPILETIMEEVAL T* Allocate(const char* Title)
	{
		AllocateNoCheck(Title);
		return Array.back().get();
	}

	static OPTIONALINLINE COMPILETIMEEVAL void AllocateNoCheck(const char* Title) {
		Array.emplace_back(std::move(std::make_unique<T>(Title)));
	}

	static OPTIONALINLINE COMPILETIMEEVAL T* FindOrAllocate(const char* Title)
	{
		if (T* find = Find(Title))
			return find;

		return Allocate(Title);
	}

	static OPTIONALINLINE COMPILETIMEEVAL void Clear()
	{
		Array.clear();
	}

	// pre-allocate all keys and read them later
	static void FindOrAllocateKeysFromINI(CCINIClass* pINI, bool bDebug = false)
	{
		const char* section = GetMainSection();

		if (!pINI->GetSection(section))
			return;

		for (int i = 0; i < pINI->GetKeyCount(section); ++i)
		{
			if (pINI->ReadString(section, pINI->GetKeyName(section, i),
				Phobos::readDefval, Phobos::readBuffer) > 0)
			{
				FindOrAllocate(Phobos::readBuffer);
			}
		}
	}

	static void ReadListFromINI(CCINIClass* pINI, bool bDebug = false)
	{
		for (auto& pItem : Array)
			pItem->LoadFromINI(pINI);
	}

	static void LoadFromINIOnlyTheList(CCINIClass* pINI, bool bDebug = false)
	{
		const char* section = GetMainSection();

		if (!pINI->GetSection(section))
			return;

		auto const pKeyCount = pINI->GetKeyCount(section);

		if (!pKeyCount)
			return;

		if (pKeyCount > (int)Array.size())
		{
			Array.reserve(pKeyCount);
		}

		for (int i = 0; i < pKeyCount; ++i)
		{
			if (pINI->ReadString(section, pINI->GetKeyName(section, i),
				Phobos::readDefval, Phobos::readBuffer) > 0)
			{
				FindOrAllocate(Phobos::readBuffer);
			}
		}
	}

	static void LoadFromINIList(CCINIClass* pINI, bool bDebug = false)
	{
		const char* section = GetMainSection();

		if (!pINI->GetSection(section))
			return;

		auto const pKeyCount = pINI->GetKeyCount(section);

		if (!pKeyCount)
			return;

		if (pKeyCount > (int)Array.size())
		{
			Array.reserve(pKeyCount);
		}

		for (int i = 0; i < pKeyCount; ++i)
		{
			if (pINI->ReadString(section, pINI->GetKeyName(section, i),
				Phobos::readDefval, Phobos::readBuffer) > 0)
			{

				if (auto const pFind = FindOrAllocate(Phobos::readBuffer))
				{
					pFind->LoadFromINI(pINI);
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

			if (!Stm.Load(oldPtr))
				return false;

			decltype(Name) name;
			if (!Stm.Load(name))
				return false;

			auto newPtr = FindOrAllocate(name);
			SwizzleManagerClass::Instance->Here_I_Am((long)oldPtr, newPtr);
			newPtr->LoadFromStream(Stm);
		}

		return true;
	}

	static bool SaveGlobals(PhobosStreamWriter& Stm)
	{
		Stm.Save(Array.size());

		for (auto& item : Array) {
			// write old pointer and name, then delegate
			Stm.Save(item.get());
			Stm.Save(item->Name);
			item->SaveToStream(Stm);
		}

		return true;
	}

	static const char* GetMainSection();

	PhobosFixedString<0x18> Name {};

	COMPILETIMEEVAL Enumerable(const char* name) : Name { name } {}

	virtual ~Enumerable() = default;
};

#define CREATEENUMTYPECLASS(x) class x##TypeClass final : public Enumerable<x##TypeClass>