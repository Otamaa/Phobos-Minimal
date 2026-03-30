#pragma once

#include <Utilities/Enumerable.h>
#include <Utilities/GameUniquePointers.h>
#include <Utilities/Handle.h>
#include <Utilities/INIparser.h>

class PaletteManager final : public Enumerable<PaletteManager>
{
public:
	static COMPILETIMEEVAL const char* MainSection = "Palettes";
	static COMPILETIMEEVAL const char* ClassName = "PaletteManager";

public:
	enum class Mode : unsigned int
	{
		Default = 0,
		Temperate = 1
	};

	FixedString<32> CachedName;
	ConvertClass* Convert_Temperate;
	ConvertClass* Convert;
	BytePalette* Palette;
	DynamicVectorClass<ColorScheme*>* ColorschemeDataVector;
	bool NoTemperate;

	PaletteManager(const char* const pTitle);

	void LoadFromStream(PhobosStreamReader& Stm);
	void SaveToStream(PhobosStreamWriter& Stm);

public:

	template<PaletteManager::Mode nMode>
	COMPILETIMEEVAL ConvertClass* GetConvert() const
	{
		return nMode == Mode::Default ? Convert : Convert_Temperate;
	}

	template<PaletteManager::Mode nMode>
	COMPILETIMEEVAL ConvertClass* GetOrDefaultConvert(ConvertClass* const& pDefault) const
	{
		const auto nRet = GetConvert<nMode>();
		return nRet ? nRet : pDefault;
	}

	bool LoadFromCachedName();

	void Clear_Internal();
	void CreateConvert();

	static void InitDefaultConverts();

private:

	void LoadFromName(const char* PaletteName);
};


class CustomPalette
{
public:

	enum class PaletteMode : unsigned int
	{
		Default = 0,
		Temperate = 1
	};

	PaletteMode Mode;
	std::string Name;
	UniqueGamePtr<ConvertClass> Convert;
	UniqueGamePtr<BytePalette> Palette;
	DynamicVectorClass<ColorScheme*>* ColorschemeDataVector;

	public:

	CustomPalette() : Mode(PaletteMode::Default), Name(), Convert(), Palette(), ColorschemeDataVector() {};
	explicit CustomPalette(PaletteMode mode) noexcept : Mode(mode), Name(), Convert(), Palette(), ColorschemeDataVector() {};

	MOVEABLE_ONLY(CustomPalette);

	ConvertClass* GetConvert() const {
		return this->Convert.get();
	}

	ConvertClass* GetOrDefaultConvert(ConvertClass* pDefault) const {
		return this->Convert.get() ? this->Convert.get() : pDefault;
	}

	bool Read(INI_EX& parser, const char* pSection, const char* pKey);
	bool Allocate(std::string name);

	bool Load(PhobosStreamReader& Stm, bool RegisterForChange);
	bool Save(PhobosStreamWriter& Stm) const;

	void Clear();

private:
	void CreateConvert();
};

// Central palette storage — owns all unique palette/convert data
// Keyed by the actual palette filename, so duplicates are impossible
//class PaletteStore
//{
//public:
//	struct Entry
//	{
//		BytePalette* Palette { nullptr };
//		ConvertClass* Convert { nullptr };
//		ConvertClass* Convert_Temperate { nullptr };
//		DynamicVectorClass<ColorScheme*>* ColorschemeDataVector { nullptr };
//		bool OwnsPalette { false };
//		bool NoTemperate { false };
//		int RefCount { 0 };
//	};
//
//	// Returns a shared handle — never allocates a duplicate
//	// The key is the resolved (theater-suffixed) palette filename
//	static int GetOrCreate(const char* resolvedName, bool noTemperate)
//	{
//		// Check if we already have this palette loaded
//		auto it = NameToIndex.find_or_default(resolvedName, -1);
//		if (it >= 0)
//		{
//			Entries[it].RefCount++;
//			return it;
//		}
//
//		// New palette — allocate and register
//		Entry entry {};
//		entry.NoTemperate = noTemperate;
//
//		// Try the normal allocator first (we own it)
//		if (auto pPal = FileSystem::AllocatePalette(resolvedName))
//		{
//			entry.Palette = pPal;
//			entry.OwnsPalette = true;
//		}
//		// Fallback to file loader (it owns the memory)
//		else if (auto pPal = (BytePalette*)FakeFileLoader::_Retrieve(resolvedName, false))
//		{
//			for (auto& color : pPal->Entries)
//			{
//				color.R <<= 2;
//				color.G <<= 2;
//				color.B <<= 2;
//			}
//			entry.Palette = pPal;
//			entry.OwnsPalette = false;
//		}
//
//		if (!entry.Palette)
//			return -1;
//
//		entry.RefCount = 1;
//		auto index = static_cast<int>(Entries.size());
//		Entries.push_back(std::move(entry));
//		NameToIndex[resolvedName] = index;
//
//		// Build converts after storing, so the entry is stable
//		BuildConverts(index);
//
//		return index;
//	}
//
//	static void BuildConverts(int index)
//	{
//		auto& e = Entries[index];
//		if (!e.Palette)
//			return;
//
//		if (!e.NoTemperate)
//		{
//			e.Convert_Temperate = GameCreate<ConvertClass>(
//				e.Palette, &FileSystem::TEMPERAT_PAL(),
//				DSurface::Primary(), 53, false);
//			e.Convert = GameCreate<ConvertClass>(
//				e.Palette, e.Palette,
//				DSurface::Alternate(), 1, false);
//		}
//		else
//		{
//			e.Convert = GameCreate<ConvertClass>(
//				e.Palette, e.Palette,
//				DSurface::Alternate(), 1, false);
//			e.Convert_Temperate = e.Convert;
//		}
//	}
//
//	static void Release(int index)
//	{
//		if (index < 0 || index >= (int)Entries.size())
//			return;
//
//		auto& e = Entries[index];
//		if (--e.RefCount > 0)
//			return;
//
//		// Last reference gone — clean up
//		if (e.Convert_Temperate && !e.NoTemperate)
//			GameDelete(e.Convert_Temperate);
//
//		if (e.Convert)
//			GameDelete(e.Convert);
//
//		if (e.Palette && e.OwnsPalette)
//			GameDelete(e.Palette);
//
//		e = {}; // zero out, slot can be reused
//	}
//
//	// Fast runtime access — no string lookups
//	static ConvertClass* GetConvert(int index)
//	{
//		return (index >= 0) ? Entries[index].Convert : nullptr;
//	}
//
//	static ConvertClass* GetConvertTemperate(int index)
//	{
//		return (index >= 0) ? Entries[index].Convert_Temperate : nullptr;
//	}
//
//	static void ClearAll()
//	{
//		for (int i = 0; i < (int)Entries.size(); ++i)
//		{
//			Entries[i].RefCount = 1; // force cleanup
//			Release(i);
//		}
//		Entries.clear();
//		NameToIndex.clear();
//	}
//
//private:
//	static std::vector<Entry> Entries;
//	static PhobosMap<std::string, int> NameToIndex;
//};
//
//// Tile or palette user — lightweight, no ownership
//class IsoPaletteRef
//{
//	int StoreIndex { -1 };
//
//public:
//	void Load(const char* paletteName, bool noTemperate = false)
//	{
//		if (StoreIndex >= 0)
//			PaletteStore::Release(StoreIndex);
//
//		StoreIndex = PaletteStore::GetOrCreate(paletteName, noTemperate);
//	}
//
//	// Called every frame during rendering — just an array index lookup
//	ConvertClass* GetConvert() const
//	{
//		return PaletteStore::GetConvert(StoreIndex);
//	}
//
//	ConvertClass* GetConvertTemperate() const
//	{
//		return PaletteStore::GetConvertTemperate(StoreIndex);
//	}
//
//	~IsoPaletteRef()
//	{
//		if (StoreIndex >= 0)
//			PaletteStore::Release(StoreIndex);
//	}
//};