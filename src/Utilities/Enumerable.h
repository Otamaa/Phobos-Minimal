#pragma once

#include <Phobos.CRT.h>

#include "Savegame.h"
#include "Swizzle.h"

#include <algorithm>
#include <CCINIClass.h>

#include <Utilities/PhobosFixedString.h>
#include <Utilities/Debug.h>

// an wrapper class to make `Type` like in the game
// remember to not modify the array ouside allocation new item(s) from the back
// it will mess upt the `ArrayIndex` !
template <typename T> class Enumerable
{
	typedef std::vector<std::unique_ptr<T>> container_t;
public:
	MOVEABLE_ONLY(Enumerable<T>);
public:

	static FORCEDINLINE COMPILETIMEEVAL const container_t& GetArray() { return Array; }
	static FORCEDINLINE COMPILETIMEEVAL size_t Count() { return Array.size(); }
	static FORCEDINLINE COMPILETIMEEVAL bool Empty() { return Array.empty(); }
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

	static OPTIONALINLINE int FindIndexById(const char* Title)
	{
		for (auto& eNum : Array) {
			if (IS_SAME_STR_(eNum->Name.c_str(), Title)) {
				return eNum->ArrayIndex;
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
			return pType->ArrayIndex;
		}

		return -1;
	}

	static OPTIONALINLINE COMPILETIMEEVAL T* TryFindFromIndex(int Idx) {

		if (size_t(Idx) >= Array.size())
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

		return Array[size_t(Idx) >= Array.size() ? 0 : Idx].get();
	}

	static OPTIONALINLINE COMPILETIMEEVAL T* Allocate(const char* Title)
	{
		AllocateNoCheck(Title);
		return Array.back().get();
	}

	static OPTIONALINLINE COMPILETIMEEVAL void AllocateNoCheck(const char* Title) {
		Array.emplace_back((std::make_unique<T>(Title)));
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
		const char* section = T::MainSection;

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
		const char* section = T::MainSection;

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
		const char* section = T::MainSection;

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

				if (auto const pFind = FindOrAllocate(Phobos::readBuffer)) {
					pFind->LoadFromINI(pINI);
				}
			}
		}
	}

	static bool LoadGlobals(PhobosStreamReader& Stm)
	{
		Clear();

		int Count = 0;
		if (Stm.Process(Count)) {
			if (Count > 0) {
				Array.reserve(Count);
				for (int i = 0; i < Count; ++i) {
					uintptr_t oldPtr = 0l;

					if (!Stm.Process(oldPtr))
						return false;

					decltype(Name) name {};
					if (!Stm.Process(name))
						return false;

					auto newPtr = FindOrAllocate(name.data());
					PHOBOS_SWIZZLE_REGISTER_POINTER(oldPtr, newPtr, T::ClassName);
					newPtr->LoadFromStream(Stm);
				}
			}

			return true;
		}

		return false;
	}

	static bool SaveGlobals(PhobosStreamWriter& Stm) {

		//save it as int instead of size_t
		const int Count = (int)Array.size();
		if (!Stm.Process(Count))
			return false;

		for (int i = 0; i < Count; ++i) {
			Debug::Log("Saving %s [%s - %x] to stream\n", T::ClassName, Array[i]->Name.data(), (long)Array[i].get());

			if(!Stm.Save((uintptr_t)Array[i].get()))
				return false;

			if(!Stm.Process(Array[i]->Name))
				return false;

			Array[i]->SaveToStream(Stm);
		}

		return true;
	}

	PhobosFixedString<0x18> Name {};
	int ArrayIndex { -1 }; //Array index are not serialized , it will re-new every creation

	COMPILETIMEEVAL Enumerable(const char* name) :
		Name { name }
		// capturing the array size before item added so this always correct
		, ArrayIndex { static_cast<int>(Array.size()) }
	{}

	virtual ~Enumerable() = default;
};
