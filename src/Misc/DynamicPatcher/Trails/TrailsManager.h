#pragma once
#ifdef COMPILE_PORTED_DP_FEATURES
#include "Trails.h"
#include <IsometricTileTypeClass.h>
#include <memory>

class ObjectClass;
struct TrailData
{
	int CurrentType { 0 };
	std::vector<LandType> OnLand {  };
	std::vector<TileType> OnTileTypes {  };
	CoordStruct FLHs { 0,0,0 };
	bool Onturrents { false };

	TrailData() noexcept = default;

	TrailData(int cur, CoordStruct flh, bool nTur) noexcept :
		CurrentType { cur }
		, OnLand { }
		, OnTileTypes { }
		, FLHs { flh }
		, Onturrents { nTur }
	{ }

	virtual ~TrailData() = default;

	inline bool Load(PhobosStreamReader& Stm, bool RegisterForChange)
	{
		//Debug::Log("Processing Element From TrailData ! \n");
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
		//Debug::Log("Processing Element From TrailData ! \n");
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
namespace TrailsManager
{
	template<typename T>
	void Construct(T* pOwner, bool IsConverted = false);

	template<typename T>
	void AI(T* pOwner);

	template<typename T>
	void Hide(T* pOwner);

	template<typename T>
	void CleanUp(T* pOwner);

};

//reading
struct TrailsReader
{
	ValueableVector<TrailData> CurrentData;

	explicit TrailsReader() : CurrentData {} { CurrentData.reserve(2); };

	virtual ~TrailsReader() = default;

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