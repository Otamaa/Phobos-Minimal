#pragma once
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

	TrailData() noexcept = default;

	TrailData(int cur, CoordStruct flh, bool nTur) noexcept :
		CurrentType { cur }
		, OnLand { }
		, OnTileTypes { }
		, FLHs { flh }
		, Onturrents { nTur }
	{ }

	~TrailData() = default;

	TrailData(const TrailData& other) = default;
	TrailData& operator=(const TrailData& other) = default;

	OPTIONALINLINE bool Load(PhobosStreamReader& Stm, bool RegisterForChange)
	{
		//Debug::LogInfo("Processing Element From TrailData ! ");
		return Stm
			.Process(CurrentType)
			.Process(OnLand)
			.Process(OnTileTypes)
			.Process(FLHs)
			.Process(Onturrents)
			.Success() //&& Stm.RegisterChange(this)
			;
	}

	OPTIONALINLINE bool Save(PhobosStreamWriter& Stm) const
	{
		//Debug::LogInfo("Processing Element From TrailData ! ");
		return Stm
			.Process(CurrentType)
			.Process(OnLand)
			.Process(OnTileTypes)
			.Process(FLHs)
			.Process(Onturrents)
			.Success() //&& Stm.RegisterChange(this)
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

	~TrailsReader() = default;

	TrailsReader(const TrailsReader& other) = default;
	TrailsReader& operator=(const TrailsReader& other) = default;

	void Read(INI_EX& nParser, const char* pSection, bool IsForTechno)
	{
		if (TrailType::Array.empty())
			return;

		//char tempBuffer[0x50];
		for (size_t i = 0; ; ++i)
		{
			NullableIdx <TrailType> trail;
			std::string _base  = "Trail";
			_base += std::to_string(i);

			trail.Read(nParser, pSection, (_base + ".Type").c_str());

			if (!trail.isset() || trail == -1)
				break;

			auto& Back = CurrentData.emplace_back();
			Back.CurrentType = trail;

			detail::read(Back.FLHs, nParser, pSection, (_base + ".FLH").c_str());

			if (IsForTechno) {
				detail::read(Back.Onturrents, nParser, pSection, (_base + ".IsOnTurret").c_str());
			}

			detail::ReadVectors(Back.OnLand, nParser, pSection, (_base + ".OnLands").c_str());
			detail::ReadVectors(Back.OnTileTypes, nParser, pSection, (_base + ".OnTiles").c_str());
		}
	}

	bool Load(PhobosStreamReader& Stm, bool RegisterForChange)
	{ return Serialize(Stm); }

	bool Save(PhobosStreamWriter& Stm) const
	{ return const_cast<TrailsReader*>(this)->Serialize(Stm); }

private:

	template <typename T>
	bool Serialize(T& Stm)
	{
		return Stm
			.Process(CurrentData)
			;

	}

};
