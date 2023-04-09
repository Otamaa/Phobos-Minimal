#pragma once
#ifdef COMPILE_PORTED_DP_FEATURES
#include <CoordStruct.h>
#include <GeneralStructures.h>
#include <CellClass.h>
#include "TrailType.h"

#include <Ext/House/Body.h>
#include <Misc/DynamicPatcher/Techno/DriveData/DriveData.h>
#include "../Helpers/EffectHelpers.h"

//class TrailsManager;
class UniversalTrail
{
public:

	TrailType* Type;
	OptionalStruct<CoordStruct,true> LastLocation;
	CDTimerClass DelayTimer;
	CoordStruct FLH;
	int initialDelay;
	bool canDraw;
	bool forceDraw;
	bool IsOnTurret;
	bool Visible;
	DrivingState drivingState;
	Iterator<LandType> OnLandTypes;
	Iterator<TileType> OnTileTypes;

	UniversalTrail(TrailType* type ,CoordStruct flh, bool onturret) :
		Type { type }
		, LastLocation { }
		, DelayTimer { }
		, FLH { flh }
		, initialDelay { type->InitialDelay > 0 ? type->InitialDelay:0 }
		, canDraw { !(type->InitialDelay > 0) }
		, forceDraw { false }
		, IsOnTurret { onturret }
		, Visible { true }
		, drivingState { DrivingState::Moving }
		, OnLandTypes { }
		, OnTileTypes { }
	{ }

	UniversalTrail() :
		Type { nullptr }
		, LastLocation { }
		, DelayTimer { }
		, FLH { CoordStruct::Empty }
		, initialDelay { 0 }
		, canDraw { false }
		, forceDraw { false }
		, IsOnTurret { false }
		, Visible { true }
		, drivingState { DrivingState::Stop }
		, OnLandTypes { }
		, OnTileTypes { }
	{ }

	~UniversalTrail() = default;

	void ClearLastLocation() {
		LastLocation.clear();
	}

	void SetDrivingState(DrivingState& state)
	{
		drivingState = state;
		if (state == DrivingState::Stop)
		{
			forceDraw = true;
		}
	}

	void RealDrawTrail(CoordStruct& sourcePos, CoordStruct& targetPos, HouseClass* pHouse)
	{
		switch (Type->Mode)
		{
		case TrailMode::LASER:

			ColorStruct houseColor = { 0,0,0 };
			if (pHouse && Type->LaserTrailType.IsHouseColor)
				houseColor = pHouse->LaserColor;

			EffectHelpers::DrawLine(sourcePos, targetPos, Type->LaserTrailType, houseColor);
			break;
		case TrailMode::ELECTIRIC:
			EffectHelpers::DrawBolt(sourcePos, targetPos, Type->BoltTrailType);
			break;
		case TrailMode::BEAM:
			EffectHelpers::DrawBeam(sourcePos, targetPos, Type->BeamTrailType, ColorStruct::Empty);
			break;
		case TrailMode::PARTICLE:
			EffectHelpers::DrawParticle(pHouse,sourcePos, targetPos, Type->ParticleTrailType.ParticleSystem.Get());
			break;
		case TrailMode::ANIM:
			DrawAnimTrail(sourcePos, pHouse);
			break;
		}
	}

	void DrawAnimTrail(CoordStruct& sourcePos, HouseClass* pHouse)
	{
		auto animType = Type->AnimTrailType.WhileDrivingAnim;

		switch (drivingState)
		{
		case DrivingState::Start:
			animType = Type->AnimTrailType.StartDrivingAnim;
			break;
		case DrivingState::Stop:
			animType = Type->AnimTrailType.StopDrivingAnim;
			break;
		}

		if (animType)
			if (auto pAnim = GameCreate<AnimClass>(animType, sourcePos))
				pAnim->Owner = pHouse;
	}

	void DrawTrail(HouseClass* pHouse, CoordStruct& sourcePos, const CoordStruct& createOffset)
	{
		if (!pHouse)
			pHouse = HouseExt::FindCivilianSide();

		if (sourcePos)
		{
			if (LastLocation.has_value() && LastLocation.get())
			{
				CoordStruct targetPos = LastLocation.get();
				int distance = Type->Distance;

				if (sourcePos.DistanceFromI(targetPos) > distance || forceDraw)
				{
					if ((CanDraw() && CheckVertical(sourcePos, targetPos)) || forceDraw)
					{
						forceDraw = false;
						if (IsOnLand(sourcePos))
						{
							RealDrawTrail(sourcePos, targetPos, pHouse);
						}
						drivingState = DrivingState::Moving;
					}
					LastLocation = sourcePos;
				}
			}
			else
			{
				LastLocation = sourcePos - createOffset;
			}
		}
	}

private:

	bool CanDraw()
	{
		if (!canDraw)
		{
			if (initialDelay > 0)
			{
				DelayTimer.Start(initialDelay);
				initialDelay = 0;
			}
			canDraw = DelayTimer.Expired();
		}
		return canDraw && Visible;
	}

	bool CheckVertical(CoordStruct& sourcePos, CoordStruct& targetPos)
	{
		return (
			Type->IgnoreVertical ?
			(abs(sourcePos.X - targetPos.X) > 32 || abs(sourcePos.Y - targetPos.Y) > 32) : true);
	}

	bool IsOnTile(CellClass* pCell)
	{
		auto const pCount = std::count_if(OnTileTypes.begin(), OnTileTypes.end(), [pCell](TileType const& nTile) {
			if (nTile == TileType::Unk || ((int)nTile >= 21))
				return false;

			return (pCell)->TileIs(nTile);
		});

		return pCount > 0;
	}

	bool IsOnLand(CoordStruct& sourcePos)
	{
		if (!OnLandTypes.empty())
		{
			if (auto pCell = MapClass::Instance->TryGetCellAt(sourcePos))
			{
				LandType landType = pCell->LandType;

				if (OnLandTypes.contains(landType))
				{
					if (!OnTileTypes.empty()) {
						return IsOnTile(pCell);
					}

					return true;
				}
			}

			return false;
		}

		return true;
	}

public:
	bool Load(PhobosStreamReader& Stm, bool RegisterForChange)
	{ return Serialize(Stm); }

	bool Save(PhobosStreamWriter& Stm) const
	{ return  const_cast<UniversalTrail*>(this)->Serialize(Stm); }

private:
	template <typename T>
	bool Serialize(T& Stm)
	{
		//Debug::Log("Processing Element From UniversalTrail ! \n");

		return Stm
		.Process(Type)
		.Process(LastLocation)
		.Process(canDraw)
		.Process(initialDelay)
		.Process(DelayTimer)
		.Process(forceDraw)
		.Process(drivingState)
		.Process(FLH)
		.Process(IsOnTurret)
		.Process(OnLandTypes)
		.Process(OnTileTypes)
		.Success()
			;
	}
};


template <>
struct Savegame::ObjectFactory<UniversalTrail>
{
	std::unique_ptr<UniversalTrail> operator() (PhobosStreamReader& Stm) const
	{
		return std::make_unique<UniversalTrail>();
	}
};

#endif