#pragma once
#ifdef COMPILE_PORTED_DP_FEATURES
#include "Trails.h"
#include <IsometricTileTypeClass.h>
#include <memory>

class ObjectClass;
struct TrailData
{
	int CurrentType;
	std::vector<LandType> OnLand;
	std::vector<TileType> OnTileTypes;
	CoordStruct FLHs;
	bool Onturrents;

	TrailData() :
		CurrentType { 0 }
		, OnLand {  }
		, OnTileTypes {  }
		, FLHs { 0,0,0 }
		, Onturrents { false }
	{ }

	TrailData(int cur, CoordStruct flh, bool nTur) :
		CurrentType { cur }
		, OnLand { }
		, OnTileTypes { }
		, FLHs { flh }
		, Onturrents { nTur }
	{ }

	inline bool Load(PhobosStreamReader& Stm, bool RegisterForChange)
	{
		Debug::Log("Processing Element From TrailData ! \n");
		return Stm
			.Process(CurrentType)
			.Process(OnLand)
			.Process(OnTileTypes)
			.Process(FLHs)
			.Process(Onturrents)
			.Success()
			;
	}

	inline bool Save(PhobosStreamWriter& Stm) const
	{
		Debug::Log("Processing Element From TrailData ! \n");
		return Stm
			.Process(CurrentType)
			.Process(OnLand)
			.Process(OnTileTypes)
			.Process(FLHs)
			.Process(Onturrents)
			.Success()
			;
	}

};

//managing the vector
class TrailsManager
{
	NO_CONSTRUCT_CLASS(TrailsManager)
public:

	template<typename T>
	static void Construct(T* pOwner, bool IsConverted = false);

	template<typename T>
	static void AI(T* pOwner);

	static void Hide(ObjectClass* pOwner);
	static void CleanUp(ObjectClass* pOwner);

};

//reading
struct TrailsReader
{
	ValueableVector<TrailData> CurrentData;

	explicit TrailsReader() : CurrentData {} { CurrentData.reserve(1); };

	void Read(INI_EX& nParser, const char* pSection, bool IsForTechno)
	{
		char tempBuffer[32];

		if (TrailType::Array.empty())
			return;

		size_t nTotal = 0;
		for (size_t i = 0; ; ++i)
		{
			NullableIdx <TrailType> trail;
			_snprintf_s(tempBuffer, sizeof(tempBuffer), "Trail%d.Type", i);
			trail.Read(nParser, pSection, tempBuffer);

			if (!trail.isset() || trail == -1)
				break;

			Valueable<CoordStruct> flh;
			_snprintf_s(tempBuffer, sizeof(tempBuffer), "Trail%d.FLH", i);
			flh.Read(nParser, pSection, tempBuffer);

			Nullable<bool> isOnTurret {};
			if (IsForTechno) {
				_snprintf_s(tempBuffer, sizeof(tempBuffer), "Trail%d.IsOnTurret", i);
				isOnTurret.Read(nParser, pSection, tempBuffer);
			}

			ValueableVector<LandType> land;
			_snprintf_s(tempBuffer, sizeof(tempBuffer), "Trail%d.OnLands", i);
			land.Read(nParser, pSection, tempBuffer);

			ValueableVector<TileType> nTiles;
			_snprintf_s(tempBuffer, sizeof(tempBuffer), "Trail%d.OnTiles", i);
			nTiles.Read(nParser, pSection, tempBuffer);

			CurrentData.emplace_back(trail.Get(),flh.Get(),isOnTurret.Get(false));
			auto &Back = CurrentData.back();
			Back.OnLand.assign(land.begin(), land.end());
			Back.OnTileTypes.assign(nTiles.begin(), nTiles.end());

			++nTotal;
		}

		if (nTotal > 0)
			CurrentData.resize(nTotal);
		else
			CurrentData.clear();
	}

	template <typename T>
	void Serialize(T& Stm)
	{
		Stm
			.Process(CurrentData)
			;
	}

};
#endif