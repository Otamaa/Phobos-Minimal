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

	inline bool Load(PhobosStreamReader& Stm, bool RegisterForChange)
	{
		//Debug::Log("Processing Element From TrailData ! \n");
		return Stm
			.Process(CurrentType)
			.Process(OnLand)
			.Process(OnTileTypes)
			.Process(FLHs)
			.Process(Onturrents)
			.Success() //&& Stm.RegisterChange(this)
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
			std::string _buffer = std::format("Trail{}.Type" , i);
			//IMPL_SNPRNINTF(tempBuffer, sizeof(tempBuffer), "Trail%d.Type", i);
			trail.Read(nParser, pSection, _buffer.c_str());

			if (!trail.isset() || trail == -1)
				break;

			auto& Back = CurrentData.emplace_back();
			Back.CurrentType = trail;

			_buffer = std::format("Trail{}.FLH" , i);
			//IMPL_SNPRNINTF(tempBuffer, sizeof(tempBuffer), "Trail%d.FLH", i);
			detail::read(Back.FLHs, nParser, pSection, _buffer.c_str());

			if (IsForTechno) {
				_buffer = std::format("Trail{}.IsOnTurret" , i);
				//IMPL_SNPRNINTF(tempBuffer, sizeof(tempBuffer), "Trail%d.IsOnTurret", i);
				detail::read(Back.Onturrents, nParser, pSection, _buffer.c_str());
			}

			_buffer = std::format("Trail{}.OnLands" , i);
			//IMPL_SNPRNINTF(tempBuffer, sizeof(tempBuffer), "Trail%d.OnLands", i);
			detail::ReadVectors(Back.OnLand, nParser, pSection, _buffer.c_str());

			_buffer = std::format("Trail{}.OnTiles" , i);
			//IMPL_SNPRNINTF(tempBuffer, sizeof(tempBuffer), "Trail%d.OnTiles", i);
			detail::ReadVectors(Back.OnTileTypes, nParser, pSection, _buffer.c_str());
		}
	}

	template <typename T>
	void Serialize(T& Stm)
	{
		Stm
			.Process(CurrentData)
			;

		//Stm.RegisterChange(this);
	}

};
