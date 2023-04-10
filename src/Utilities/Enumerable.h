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

//#include <format>

template <typename T> class Enumerable
{
	typedef std::vector<std::unique_ptr<T>> container_t;

public:
	static container_t Array;

	static int FindOrAllocateIndex(const char* Title)
	{
		const auto nResult = FindIndexById(Title);

		if (nResult < 0) {
			Array.push_back(std::move(std::make_unique<T>(Title)));
			return Array.size() - 1;
		}

		return nResult;
	}

	static int FindIndexById(const char* Title)
	{
		if (!Title || !strlen(Title))
			return -1;

		const auto nResult = std::find_if(Array.begin(), Array.end(),
			[Title](std::unique_ptr<T>& Item) {
				return IS_SAME_STR_(Item->Name.data(), Title);
			});

		if (nResult == Array.end())
			return -1;

		return std::distance(Array.begin(), nResult);
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
		const auto result = std::find_if(Array.begin(), Array.end(), 
			[Title](std::unique_ptr<T>& Item) {
				return tem.get() == pType;
			});

		return result == Array.end() ?
			-1 : std::distance(Array.begin(), result);
	}

	static T* FindFromIndex(int Idx)
	{
		return Array[static_cast<size_t>(Idx)].get();
	}

	static T* FindFromIndexFix(int Idx) {
		Idx = Idx > (int)Array.size() || Idx < 0 ? 0 : Idx;
		return Array[static_cast<size_t>(Idx)].get();
	}

	static T* Allocate(const char* Title)
	{
		if (!Title || !strlen(Title))
			return nullptr;

		Array.push_back(std::move(std::make_unique<T>(Title)));
		return Array.back().get();
	}

	static T* FindOrAllocate(const char* Title)
	{
		if (T* find = Find(Title))
			return find;

		return Allocate(Title);
	}

	static void Clear()
	{
		//Debug::Log("%s Clearing Array Count [%d] ! \n", typeid(T).name(), Array.size());
		Array.clear();
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

		Array.reserve(pKeyCount);

		for (int i = 0; i < pKeyCount; ++i) {
			if (pINI->ReadString(section, pINI->GetKeyName(section, i), Phobos::readDefval, Phobos::readBuffer)) {
				if (auto const pItem = FindOrAllocate(Phobos::readBuffer)) {
					//if (bDebug)
					//	Debug::Log("%s Reading[%d] %s \"%s\".\n", typeid(T).name(), i, section, Phobos::readBuffer);

					pItem->LoadFromINI(pINI);
				}
				else
				{
					//if (bDebug)
						//Debug::Log("%s Error Creating[%d] %s \"%s\"!\n", typeid(T).name(), i, section, Phobos::readBuffer);
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

		for (size_t i = 0; i < Count; ++i) {
			void* oldPtr = nullptr;
			decltype(Name) name;

			if (!Stm.Load(oldPtr) || !Stm.Load(name))
				return false;

			auto newPtr = FindOrAllocate(name);
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