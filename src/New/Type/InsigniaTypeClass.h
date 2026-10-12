#pragma once

#include <Utilities/Enumerable.h>
#include <Utilities/TemplateDef.h>
#include <New/Type/PaletteManager.h>

class InsigniaTypeClass final : public Enumerable<InsigniaTypeClass>
{
public:
	static COMPILETIMEEVAL const char* MainSection = "InsigniaTypes";
	static COMPILETIMEEVAL const char* ClassName = "InsigniaTypeClass";

public:
	Promotable<SHPCaches*> Insignia { };
	CustomPalette Palette { CustomPalette::PaletteMode::Temperate };
	CustomPalette Palette_Rookie { CustomPalette::PaletteMode::Temperate };
	CustomPalette Palette_Veteran { CustomPalette::PaletteMode::Temperate };
	CustomPalette Palette_Elite { CustomPalette::PaletteMode::Temperate };
	Promotable<int> InsigniaFrame { -1 };

	InsigniaTypeClass(const char* const pTitle) : Enumerable<InsigniaTypeClass>(pTitle)	{ }
	virtual ~InsigniaTypeClass() = default;

	void LoadFromINI(CCINIClass* pINI);

	void LoadFromStream(PhobosStreamReader& Stm);
	void SaveToStream(PhobosStreamWriter& Stm);

private:
	template <typename T>
	void Serialize(T& Stm);
};

struct InsigniaDataEntry
{
	Valueable<InsigniaTypeClass*> Type;
	Promotable<SHPCaches*> Shape;
	CustomPalette Palette;
	CustomPalette Palette_Rookie;
	CustomPalette Palette_Veteran;
	CustomPalette Palette_Elite;
	Promotable<int> Frame;
	Valueable<Vector3D<int>> Frames;

	InsigniaDataEntry()
		: Type { nullptr }
		, Shape { nullptr }
		, Palette { CustomPalette::PaletteMode::Temperate }
		, Palette_Rookie { CustomPalette::PaletteMode::Temperate }
		, Palette_Veteran { CustomPalette::PaletteMode::Temperate }
		, Palette_Elite { CustomPalette::PaletteMode::Temperate }
		, Frame { -1 }
		, Frames { { -1, -1, -1 } }
	{}

	inline SHPCaches* GetShape(Rank rank) const
	{
		const auto pType = this->Type.Get();
		return (pType ? pType->Insignia : this->Shape).GetFromSpecificRank(rank);
	}

	inline ConvertClass* GetPalette(Rank rank) const
	{
		const auto pType = this->Type.Get();
		const auto pDefault = (pType ? pType->Palette : this->Palette).GetConvert();

		switch (rank)
		{
		case Rank::Elite:
			return (pType ? pType->Palette_Elite : this->Palette_Elite).GetOrDefaultConvert(pDefault);

		case Rank::Veteran:
			return (pType ? pType->Palette_Veteran : this->Palette_Veteran).GetOrDefaultConvert(pDefault);

		default:
			return (pType ? pType->Palette_Rookie : this->Palette_Rookie).GetOrDefaultConvert(pDefault);
		}
	}

	inline int GetFrame(Rank rank) const
	{
		const auto pType = this->Type.Get();
		return (pType ? pType->InsigniaFrame : this->Frame).GetFromSpecificRank(rank);
	}

	inline Vector3D<int> GetFrames() const
	{
		return this->Type ? Vector3D<int> {-1, -1, -1} : this->Frames;
	}

	bool Load(PhobosStreamReader& stm, bool registerForChange);
	bool Save(PhobosStreamWriter& stm) const;

	static void Read(INI_EX exINI, const char* pSection,
		bool gunner,
		size_t passengers,
		size_t weaponCount,
		InsigniaDataEntry& entry,
		std::vector<InsigniaDataEntry>& weaponsData,
		std::vector<InsigniaDataEntry>& PassengersData
	);

private:
	template <typename T>
	bool Serialize(T& stm);
};