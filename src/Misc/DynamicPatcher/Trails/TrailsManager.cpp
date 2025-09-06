#include "TrailsManager.h"

#include <Ext/TechnoType/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/Bullet/Body.h>
#include <Ext/BulletType/Body.h>

#include <Ext/Particle/Body.h>
#include <Ext/ParticleType/Body.h>

#include <Ext/VoxelAnim/Body.h>
#include <Ext/VoxelAnimType/Body.h>
#include <Ext/WeaponType/Body.h>

#pragma region TemplatedStuffs
template<typename T, typename Ttype, typename Tbase, typename TbaseType, bool IsTechno = false>
static bool CheckAndContruct(Tbase* pClass, TbaseType* pClassType, bool Clear = false)
{
	const auto pExt = T.Find(pClass);
	const auto pTypeExt = Ttype.Find(pClassType);

	if (!pExt->Trails.empty())
	{
		if (Clear)
			pExt->Trails.clear();
		else
			return false;
	}

	if (pTypeExt->Trails.CurrentData.empty())
		return false;

	for (const auto& pTrails : pTypeExt->Trails.CurrentData)
	{
		const auto pType = TrailType::Array[pTrails.CurrentType].get();
		if (pType->Mode != TrailMode::NONE)
		{
			bool OnTurrent = false;
			if COMPILETIMEEVAL (IsTechno) OnTurrent = pTrails.Onturrents;

			auto& pBackTrail = pExt->Trails.emplace_back(pType, pTrails.FLHs, OnTurrent);
			pBackTrail.OnLandTypes = pTrails.OnLand;
			pBackTrail.OnTileTypes = pTrails.OnTileTypes;
		}
	}

	return true;
}

template<typename Text, typename Tbase>
static bool ClearLastLoc(Tbase* pBase)
{
	const auto  pExt = Text.Find(pBase);

	if (pExt->Trails.empty())
		return false;

	for (auto& pTrail : pExt->Trails) {
		pTrail.ClearLastLocation();
	}

	return true;
}

template <typename Text, typename Tbase>
static bool ClearVector(Tbase* pBase)
{
	Text.Find(pBase)->Trails.clear();

	return true;
}
#pragma endregion

#pragma region Construct
template<typename T>
void TrailsManager::Construct(T* pOwner, bool IsConverted) { }

template<>
void TrailsManager::Construct(TechnoClass* pOwner, bool IsConverted)
{
	if (pOwner->WhatAmI() == BuildingClass::AbsID || TrailType::Array.empty())
		return;

	const auto pClassType = pOwner->GetTechnoType();

	if (pClassType->Invisible)
		return;

	const auto pExt = TechnoExtContainer::Instance.Find(pOwner);
	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pClassType);

	if (!pExt->Trails.empty())
	{
		if (IsConverted)
			pExt->Trails.clear();
		else
			return;
	}

	if (pTypeExt->Trails.CurrentData.empty())
		return;

	for (const auto& pTrails : pTypeExt->Trails.CurrentData)
	{
		const auto pType = TrailType::Array[pTrails.CurrentType].get();

		if (pType->Mode != TrailMode::NONE)
		{
			pExt->Trails.emplace_back(pType, pTrails.FLHs, pTrails.Onturrents);
			auto& pBackTrail = pExt->Trails.back();
			std::copy(pTrails.OnLand.begin() , pTrails.OnLand.end() ,std::back_inserter(pBackTrail.OnLandTypes));
			std::copy(pTrails.OnTileTypes.begin(), pTrails.OnTileTypes.end(), std::back_inserter(pBackTrail.OnTileTypes));
		}
	}
}

template<>
void TrailsManager::Construct(BulletClass* pOwner, bool IsConverted)
{
	if (TrailType::Array.empty())
		return;

	const auto pClassType = pOwner->Type;
	const auto pExt = BulletExtContainer::Instance.Find(pOwner);
	const auto pTypeExt = BulletTypeExtContainer::Instance.Find(pClassType);

	if (!pExt->Trails.empty()) {
		return;
	}

	if (pTypeExt->Trails.CurrentData.empty())
		return;

	for (const auto & pTrails : pTypeExt->Trails.CurrentData)
	{
		const auto pType = TrailType::Array[pTrails.CurrentType].get();

		if (pType->Mode != TrailMode::NONE)
		{
			pExt->Trails.emplace_back(pType, pTrails.FLHs, false);
			auto& pBackTrail = pExt->Trails.back();
			std::copy(pTrails.OnLand.begin(), pTrails.OnLand.end(), std::back_inserter(pBackTrail.OnLandTypes));
			std::copy(pTrails.OnTileTypes.begin(), pTrails.OnTileTypes.end(), std::back_inserter(pBackTrail.OnTileTypes));

		}
	}
}

template<>
void TrailsManager::Construct(VoxelAnimClass* pOwner, bool IsConverted)
{
	if (TrailType::Array.empty())
		return;

	const auto pClassType = pOwner->Type;
	const auto  pExt = VoxelAnimExtContainer::Instance.Find(pOwner);
	const auto pTypeExt = VoxelAnimTypeExtContainer::Instance.Find(pClassType);

	if (!pExt->Trails.empty()) {
		return;
	}

	if (pTypeExt->Trails.CurrentData.empty())
		return;

	for (const auto& pTrails : pTypeExt->Trails.CurrentData)
	{
		const auto pType = TrailType::Array[pTrails.CurrentType].get();

		if (pType->Mode != TrailMode::NONE)
		{
			pExt->Trails.emplace_back(pType, pTrails.FLHs, false);
			auto& pBackTrail = pExt->Trails.back();
			std::copy(pTrails.OnLand.begin(), pTrails.OnLand.end(), std::back_inserter(pBackTrail.OnLandTypes));
			std::copy(pTrails.OnTileTypes.begin(), pTrails.OnTileTypes.end(), std::back_inserter(pBackTrail.OnTileTypes));

		}
	}
}

template<>
void TrailsManager::Construct(ParticleClass* pOwner, bool IsConverted)
{
	if (TrailType::Array.empty())
		return;

	const auto pExt = ParticleExtContainer::Instance.Find(pOwner);
	const auto pTypeExt = ParticleTypeExtContainer::Instance.Find(pOwner->Type);

	if (!pExt->Trails.empty()) {
		return ;
	}

	if (pTypeExt->Trails.CurrentData.empty())
		return ;

	for (const auto& pTrails : pTypeExt->Trails.CurrentData)
	{
		const auto pType = TrailType::Array[pTrails.CurrentType].get();
		if (pType->Mode != TrailMode::NONE)
		{
			pExt->Trails.emplace_back(pType, pTrails.FLHs, false);
			auto& pBackTrail = pExt->Trails.back();
			std::copy(pTrails.OnLand.begin(), pTrails.OnLand.end(), std::back_inserter(pBackTrail.OnLandTypes));
			std::copy(pTrails.OnTileTypes.begin(), pTrails.OnTileTypes.end(), std::back_inserter(pBackTrail.OnTileTypes));

		}
	}

	return;
}
#pragma endregion

#pragma region AI
template<typename T>
void TrailsManager::AI(T* pOwner)
{
	Debug::LogInfo(__FUNCTION__" {} Not have specified Function to use ! ", typeid(T).name());
}

template<>
void TrailsManager::AI(FootClass* pOwner)
{
	const auto pExt = TechnoExtContainer::Instance.Find((TechnoClass*)pOwner);

	for (auto& pTrails : pExt->Trails)
	{
		if (((TechnoClass*)pOwner)->IsInCloakState() && pTrails.Type->HideWhenCloak.Get()) {
			continue;
		}

		if (!pExt->IsInTunnel)
			pTrails.Visible = true;

		if (pTrails.Type->Mode == TrailMode::ANIM)
		{
			switch (pExt->MyDriveData.nState)
			{
			case DrivingState::Start:
			case DrivingState::Stop:
				pTrails.SetDrivingState(pExt->MyDriveData.nState);
				break;
			}
		}

		auto nSource = TechnoExtData::GetFLHAbsoluteCoords((TechnoClass*)pOwner, pTrails.FLH, pTrails.IsOnTurret);
		pTrails.DrawTrail(pOwner->GetOwningHouse(), nSource, CoordStruct::Empty);
	}
}

template<>
void TrailsManager::AI(BulletClass* pOwner)
{
	const auto pExt = BulletExtContainer::Instance.Find(pOwner);
	HouseClass* Invoker = pOwner->Owner ? pOwner->Owner->GetOwningHouse() : (pExt->Owner) ? pExt->Owner : nullptr;
	HouseClass* victim = pOwner->Target ? pOwner->Target->GetOwningHouse() : nullptr;
	auto pTechnoInvoker = pOwner->Owner;

	for (auto& pTrails : pExt->Trails)
	{
		CoordStruct location = (pOwner)->Location;
		VelocityClass& velocity = (pOwner)->Velocity;

		// We adjust LaserTrails to account for vanilla bug of drawing stuff one frame ahead.
		// Pretty meh solution but works until we fix the bug - Kerbiter
		CoordStruct drawnCoords
		{
			(int)(location.X + velocity.X),
			(int)(location.Y + velocity.Y),
			(int)(location.Z + velocity.Z)
		};

		auto nSource = TechnoExtData::GetFLHAbsoluteCoords((TechnoClass*)pOwner, pTrails.FLH, pTrails.IsOnTurret);
		pTrails.DrawTrail(Invoker, location, pTrails.FLH ,pTechnoInvoker,victim);
	}
}

template<>
void TrailsManager::AI(VoxelAnimClass* pOwner)
{
	const auto pExt = VoxelAnimExtContainer::Instance.Find(pOwner);
	//const auto pTypeExt = VoxelAnimTypeExtContainer::Instance.Find(pOwner->Type);
	auto const pTechnoOwner = VoxelAnimExtData::GetTechnoOwner(pOwner);
	auto bounceCoords = (pOwner)->Bounce.GetCoords();

	for (auto& pTrails : pExt->Trails) {
		auto const pHouseOwner = pTechnoOwner ? pTechnoOwner->GetOwningHouse() : pOwner->GetOwningHouse();
		pTrails.DrawTrail(pHouseOwner, bounceCoords, pTrails.FLH);
	}
}

template<>
void TrailsManager::AI(ParticleClass* pOwner)
{
	const auto pExt = ParticleExtContainer::Instance.Find(pOwner);

	for (auto& pTrails : pExt->Trails) {
		pTrails.DrawTrail((pOwner)->GetOwningHouse(), pOwner->Location, pTrails.FLH);
	}
}

#pragma endregion

#pragma region Hide

template<typename T>
void TrailsManager::Hide(T* pOwner)
{
	Debug::LogInfo(__FUNCTION__" {} Not have specified Function to use ! ", typeid(T).name());
}
template<>
void TrailsManager::Hide(TechnoClass* pOwner)
{
	if (pOwner->WhatAmI() == BuildingClass::AbsID)
		return;

	const auto pExt = TechnoExtContainer::Instance.Find((TechnoClass*)pOwner);

	if (pExt->Trails.empty())
		return;

	for (auto& pTrail : pExt->Trails)
	{
		pTrail.Visible = !(((TechnoClass*)pOwner)->InLimbo);
		pTrail.ClearLastLocation();
	}
}

template<>
void TrailsManager::Hide(BulletClass* pOwner)
{
	const auto pExt = BulletExtContainer::Instance.Find((BulletClass*)pOwner);

	if (pExt->Trails.empty())
		return;

	for (auto& pTrail : pExt->Trails)
	{
		pTrail.ClearLastLocation();
	}
}

template<>
void TrailsManager::Hide(VoxelAnimClass* pOwner)
{
	const auto pExt = VoxelAnimExtContainer::Instance.Find((VoxelAnimClass*)pOwner);

	if (pExt->Trails.empty())
		return;

	for (auto& pTrail : pExt->Trails) {
		pTrail.ClearLastLocation();
	}
}

template<>
void TrailsManager::Hide(ParticleClass* pOwner)
{
	const auto  pExt = ParticleExtContainer::Instance.Find(pOwner);

	if (pExt->Trails.empty())

	for (auto& pTrail : pExt->Trails) {
		pTrail.ClearLastLocation();
	}
}

#pragma endregion

#pragma region CleanUp
template<typename T>
void TrailsManager::CleanUp(T* pOwner)
{
	Debug::LogInfo(__FUNCTION__" {} Not have specified Function to use ! ", typeid(T).name());
}

template<>
void TrailsManager::CleanUp(TechnoClass* pOwner)
{
	if (pOwner->WhatAmI() == BuildingClass::AbsID)
		return;

	const auto  pExt = TechnoExtContainer::Instance.Find(pOwner);

	if (pExt->Trails.empty())
		return;

	for (auto& pTrail : pExt->Trails)
	{
		pTrail.ClearLastLocation();
	}

}

template<>
void TrailsManager::CleanUp(BulletClass* pOwner)
{
	const auto  pExt = BulletExtContainer::Instance.Find(pOwner);

	if (pExt->Trails.empty())
		return;

	for (auto& pTrail : pExt->Trails)
	{
		pTrail.ClearLastLocation();
	}

}

template<>
void TrailsManager::CleanUp(VoxelAnimClass* pOwner)
{
	const auto  pExt = VoxelAnimExtContainer::Instance.Find(pOwner);

	if (pExt->Trails.empty())
		return;

	for (auto& pTrail : pExt->Trails)
	{
		pTrail.ClearLastLocation();
	}
}

template<>
void TrailsManager::CleanUp(ParticleClass* pOwner)
{
	const auto  pExt = ParticleExtContainer::Instance.Find(pOwner);

	if (pExt->Trails.empty())
		return;

	for (auto& pTrail : pExt->Trails)
	{
		pTrail.ClearLastLocation();
	}
}
#pragma endregion
