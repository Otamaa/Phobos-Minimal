#ifdef COMPILE_PORTED_DP_FEATURES
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
template<typename T, typename Ttype, typename Tbase, typename TbaseType>
static bool CheckAndContruct(Tbase* pClass, TbaseType* pClassType, bool Clear = false, bool IsTechno = true)
{
	if (!pClassType)
		return false;

	auto const pExt = T::ExtMap.Find<false>(pClass);
	auto const pTypeExt = Ttype::ExtMap.Find<false>(pClassType);

	if (!pExt || !pTypeExt)
		return false;

	if (!pExt->Trails.empty())
	{
		if (Clear)
			pExt->Trails.clear();
		else
			return false;
	}

	if (pTypeExt->Trails.CurrentData.empty())
		return false;

	for (auto const& pTrails : pTypeExt->Trails.CurrentData)
	{
		if (auto const pType = TrailType::Array[pTrails.CurrentType].get())
		{
			if (pType->Mode != TrailMode::NONE)
			{
				pExt->Trails.emplace_back(std::make_unique<UniversalTrail>(pType, pTrails.FLHs, IsTechno ? pTrails.Onturrents : false));
				auto const& pBackTrail = pExt->Trails.back();
				pBackTrail->OnLandTypes = pTrails.OnLand;
				pBackTrail->OnTileTypes = pTrails.OnTileTypes;
			}
		}
	}

	return true;
}

template<typename Text, typename Tbase>
static bool ClearLastLoc(Tbase* pBase)
{
	auto const& pExt = Text::ExtMap.Find<false>(pBase);

	if (!pExt)
		return false;

	if (!pExt->Trails.empty())
	{
		for (auto const& pTrail : pExt->Trails)
		{
			pTrail->ClearLastLocation();
		}
	}

	return true;
}

template <typename Text, typename Tbase>
static bool ClearVector(Tbase* pBase)
{
	auto const& pExt = Text::ExtMap.Find<false>(pBase);

	if (!pExt)
		return false;

	pExt->Trails.clear();

	return true;
}
#pragma endregion

#pragma region Construct
template<typename T>
void TrailsManager::Construct(T* pOwner, bool IsConverted) {
	Debug::Log(__FUNCTION__" %s Not have specified Function to use ! \n" , typeid(T).name());
}

template<>
void TrailsManager::Construct(TechnoClass* pOwner, bool IsConverted)
{
	if (!pOwner || pOwner->WhatAmI() == AbstractType::Building || TrailType::Array.empty())
		return;
	auto pClassType = pOwner->GetTechnoType();

	if (!pClassType || pClassType->Invisible)
		return;

	auto const pExt = TechnoExt::ExtMap.Find<false>(pOwner);
	auto const pTypeExt = TechnoTypeExt::ExtMap.Find<false>(pClassType);

	if (!pExt || !pTypeExt)
		return;

	if (!pExt->Trails.empty())
	{
		if (IsConverted)
			pExt->Trails.clear();
		else
			return;
	}

	if (pTypeExt->Trails.CurrentData.empty())
		return;

	for (auto const& pTrails : pTypeExt->Trails.CurrentData)
	{
		if (auto const pType = TrailType::Array[pTrails.CurrentType].get())
		{
			if (pType->Mode != TrailMode::NONE)
			{
				pExt->Trails.emplace_back(std::make_unique<UniversalTrail>(pType, pTrails.FLHs, pTrails.Onturrents));
				auto const& pBackTrail = pExt->Trails.back();
				pBackTrail->OnLandTypes = pTrails.OnLand;
				pBackTrail->OnTileTypes = pTrails.OnTileTypes;
			}
		}
	}
}

template<>
void TrailsManager::Construct(BulletClass* pOwner, bool IsConverted) {
	if (!pOwner || TrailType::Array.empty())
		return;

	if (!pOwner->Type || pOwner->Type->Inviso)
		return;

	auto pClassType = pOwner->Type;
	auto const pExt = BulletExt::ExtMap.Find<false>(pOwner);
	auto const pTypeExt = BulletTypeExt::ExtMap.Find<false>(pClassType);

	if (!pExt || !pTypeExt)
		return;

	if (!pExt->Trails.empty()) {
		return;
	}

	if (pTypeExt->Trails.CurrentData.empty())
		return;

	for (auto const& pTrails : pTypeExt->Trails.CurrentData)
	{
		if (auto pType = TrailType::Array[pTrails.CurrentType].get())
		{
			if (pType->Mode != TrailMode::NONE)
			{
				pExt->Trails.emplace_back(std::make_unique<UniversalTrail>(pType , pTrails.FLHs , false));
				auto const& pBackTrail = pExt->Trails.back();
				pBackTrail->OnLandTypes = pTrails.OnLand;
				pBackTrail->OnTileTypes = pTrails.OnTileTypes;
			}
		}
	}
}

template<>
void TrailsManager::Construct(VoxelAnimClass* pOwner, bool IsConverted) {
	if (!pOwner || TrailType::Array.empty())
		return;

	if (!pOwner->Type)
		return;

	auto pClassType = pOwner->Type;
	auto const pExt = VoxelAnimExt::ExtMap.Find<false>(pOwner);
	auto const pTypeExt = VoxelAnimTypeExt::ExtMap.Find<false>(pClassType);

	if (!pExt || !pTypeExt)
		return;

	if (!pExt->Trails.empty())
	{
		return;
	}

	if (pTypeExt->Trails.CurrentData.empty())
		return;

	for (auto const& pTrails : pTypeExt->Trails.CurrentData)
	{
		if (auto const pType = TrailType::Array[pTrails.CurrentType].get())
		{
			if (pType->Mode != TrailMode::NONE)
			{
				pExt->Trails.emplace_back(std::make_unique<UniversalTrail>(pType, pTrails.FLHs, false));
				auto const& pBackTrail = pExt->Trails.back();
				pBackTrail->OnLandTypes = pTrails.OnLand;
				pBackTrail->OnTileTypes = pTrails.OnTileTypes;
			}
		}
	}
}

template<>
void TrailsManager::Construct(ParticleClass* pOwner, bool IsConverted) {
	if (!pOwner || TrailType::Array.empty())
		return;

	CheckAndContruct<ParticleExt, ParticleTypeExt>(pOwner, pOwner->Type, false, false);
}
#pragma endregion

#pragma region AI
template<typename T>
void TrailsManager::AI(T* pOwner) {
	Debug::Log(__FUNCTION__" %s Not have specified Function to use ! \n", typeid(T).name());
}

template<>
void TrailsManager::AI(TechnoClass* pOwner)
{
	if (!pOwner || pOwner->WhatAmI() == AbstractType::Building || TrailType::Array.empty())
		return;

	auto const pExt = TechnoExt::ExtMap.Find<false>((TechnoClass*)pOwner);

	if (!pExt)
		return;

	if (!pExt->Trails.empty())
	{
		for (auto const& pTrails : pExt->Trails)
		{
			if (((TechnoClass*)pOwner)->CloakState == CloakState::Cloaking ||
				((TechnoClass*)pOwner)->CloakState == CloakState::Cloaked)
			{
				if (pTrails->Type->HideWhenCloak.Get())
					continue;
			}

			if (pTrails->Type->Mode == TrailMode::ANIM)
			{
				switch (pExt->MyDriveData.nState)
				{
				case DrivingState::Start:
				case DrivingState::Stop:
					pTrails->SetDrivingState(pExt->MyDriveData.nState);
					break;
				}
			}

			auto nSource = TechnoExt::GetFLHAbsoluteCoords((TechnoClass*)pOwner, pTrails->FLH, pTrails->IsOnTurret);
			pTrails->DrawTrail(pOwner->GetOwningHouse(), nSource , CoordStruct::Empty);
		}
	}
}

template<>
void TrailsManager::AI(BulletClass* pOwner)
{
	if (!pOwner)
		return;

	auto const pExt = BulletExt::ExtMap.Find<false>(pOwner);

	if (!pExt)
		return;

	if (!pExt->Trails.empty())
	{
		for (auto const& pTrails : pExt->Trails)
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

			pTrails->DrawTrail((pOwner)->GetOwningHouse(), location, pTrails->FLH);
		}
	}
}

template<>
void TrailsManager::AI(VoxelAnimClass* pOwner)
{
	if (!pOwner)
		return;

	auto  pExt = VoxelAnimExt::ExtMap.Find<false>(pOwner);
	auto pTypeExt = VoxelAnimTypeExt::ExtMap.Find<false>(pOwner->Type);

	if (!pExt || !pTypeExt)
		return;

	if (!pExt->Trails.empty())
	{
		CoordStruct location = (pOwner)->Bounce.GetCoords();

		for (auto const& pTrails : pExt->Trails) {
			auto const pTechnoOwner = VoxelAnimExt::GetTechnoOwner(pOwner, pTypeExt->ExplodeOnWater);
			auto const pHouseOwner = pTechnoOwner ? pTechnoOwner->GetOwningHouse() : pOwner->GetOwningHouse();
			pTrails->DrawTrail(pHouseOwner, location, pTrails->FLH);
		}
	}
}

template<>
void TrailsManager::AI(ParticleClass* pOwner)
{
	if (!pOwner)
		return;

	auto const pExt = ParticleExt::ExtMap.Find<false>(pOwner);

	if (!pExt)
		return;

	if (!pExt->Trails.empty())
	{
		CoordStruct location = (pOwner)->GetCoords();

		for (auto const& pTrails : pExt->Trails) {
			pTrails->DrawTrail((pOwner)->GetOwningHouse(), location, pTrails->FLH);
		}
	}
}

#pragma endregion

#pragma region Hide
void TrailsManager::Hide(ObjectClass* pOwner)
{
	if (!pOwner || pOwner->WhatAmI() == AbstractType::Building || TrailType::Array.empty())
		return;

	switch (pOwner->WhatAmI())
	{
	case AbstractType::Unit:
	case AbstractType::Infantry:
	case AbstractType::Aircraft:
	{
		auto const pExt = TechnoExt::ExtMap.Find<false>((TechnoClass*)pOwner);

		if (!pExt)
			return;

		if (!pExt->Trails.empty())
		{
			for (auto const& pTrail : pExt->Trails)
			{
				pTrail->ClearLastLocation();
			}
		}

	}
	break;
	case AbstractType::Bullet:
	{
		auto const pExt = BulletExt::ExtMap.Find<false>((BulletClass*)pOwner);

		if (!pExt)
			return;

		if (!pExt->Trails.empty())
		{
			for (auto const& pTrail : pExt->Trails)
			{
				pTrail->ClearLastLocation();
			}
		}
	}
	break;
	case AbstractType::VoxelAnim:
	{
		auto const pExt = VoxelAnimExt::ExtMap.Find<false>((VoxelAnimClass*)pOwner);

		if (!pExt)
			return;

		if (!pExt->Trails.empty())
		{
			for (auto const& pTrail : pExt->Trails)
			{
				pTrail->ClearLastLocation();
			}
		}
	}
	break;
	case AbstractType::Particle:
	{
		if (!ClearLastLoc<ParticleExt>((ParticleClass*)pOwner))
			return;
	}
	break;
	}
}

#pragma endregion

#pragma region CleanUp
void TrailsManager::CleanUp(ObjectClass* pOwner)
{
	if (!pOwner || pOwner->WhatAmI() == AbstractType::Building || TrailType::Array.empty())
		return;

	switch (pOwner->WhatAmI())
	{
	case AbstractType::Unit:
	case AbstractType::Infantry:
	case AbstractType::Aircraft:
	{
		auto const pExt = TechnoExt::ExtMap.Find<false>((TechnoClass*)pOwner);

		if (!pExt)
			return;

		pExt->Trails.clear();
	}
	break;
	case AbstractType::Bullet:
	{
		auto const pExt = BulletExt::ExtMap.Find<false>((BulletClass*)pOwner);

		if (!pExt)
			return;

		pExt->Trails.clear();
	}
	break;
	case AbstractType::VoxelAnim:
	{
		auto const pExt = VoxelAnimExt::ExtMap.Find<false>((VoxelAnimClass*)pOwner);

		if (!pExt)
			return;

		pExt->Trails.clear();
	}
	break;
	case AbstractType::Particle:
	{
		if (!ClearVector<ParticleExt>((ParticleClass*)pOwner))
			return;
	}
	break;
	}
}

#pragma endregion
#endif