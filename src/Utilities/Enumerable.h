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

	static int FindIndex(const char* Title)
	{
		auto result = std::find_if(Array.begin(), Array.end(), [Title](std::unique_ptr<T>& Item)
			{
				return _strcmpi(Item->Name.data(), Title) == 0;
			});

		if (result == Array.end())
			return -1;

		return std::distance(Array.begin(), result);
	}

	static int FindIndexFromType(T* pType)
	{
		return (pType) ? FindIndex(pType->Name.data()) : -1;
	}

	static T* Find(const char* Title)
	{
		int result = FindIndex(Title);
		result = result > (int)Array.size() ? (Array.empty() ? -1 : 0) : result;
		return (result < 0) ? nullptr : Array[static_cast<size_t>(result)].get();
	}

	static T* FindFromIndex(int Idx)
	{
		Idx = Idx > (int)Array.size() ? (Array.empty() ? -1 : 0) : Idx;
		return ((Idx < 0)) ? nullptr : Array[static_cast<size_t>(Idx)].get();
	}

	static T* FindFromIndexFix(int Idx) {
		Idx = Idx > (int)Array.size() || Idx < 0 ? 0 : Idx;
		return Array[static_cast<size_t>(Idx)].get();
	}

	static T* FindOrAllocate(const char* Title)
	{
		if (T* find = Find(Title))
			return find;

		Array.push_back(std::make_unique<T>(Title));

		return Array.back().get();
	}

	static void Clear()
	{
		Debug::Log("%s Clearing Array Count [%d] ! \n", typeid(T).name(), Array.size());

		if (!Array.empty())
		{
			for (size_t i = 0; i < Array.size(); ++i)
			{
				if (Array[i]) {
					Array[i].release();
				}
			}

			Array.clear();
		}
	}

	static void LoadFromINIList(CCINIClass* pINI, bool bDebug = false)
	{
		if (!pINI)
			return;

		const char* section = GetMainSection();

		if (!pINI->GetSection(section))
			return;

		for (int i = 0; i < pINI->GetKeyCount(section); ++i)
		{
			if (pINI->ReadString(section, pINI->GetKeyName(section, i), "", Phobos::readBuffer))
			{
				if (auto const pItem = FindOrAllocate(Phobos::readBuffer))
				{
					if (bDebug)
						Debug::Log("%s Reading[%d] %s \"%s\".\n", typeid(T).name(), i, section, Phobos::readBuffer);

					pItem->LoadFromINI(pINI);
				}
				else
				{
					if (bDebug)
						Debug::Log("%s Error Creating[%d] %s \"%s\"!\n", typeid(T).name(), i, section, Phobos::readBuffer);
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

	Enumerable(const char* Title) : Name { }
	{ Name = Title; }

	virtual ~Enumerable() = default;

	virtual void LoadFromINI(CCINIClass* pINI) { }

	virtual void LoadFromStream(PhobosStreamReader& Stm) = 0;

	virtual void SaveToStream(PhobosStreamWriter& Stm) = 0;

	FixedString<32> Name;
};