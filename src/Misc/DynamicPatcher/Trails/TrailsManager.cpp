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
template<typename T, typename Ttype, typename Tbase, typename TbaseType, bool IsTechno = false>
static bool CheckAndContruct(Tbase* pClass, TbaseType* pClassType, bool Clear = false)
{
	if (!pClassType)
		return false;

	auto const pExt = T::ExtMap.Find(pClass);
	auto const pTypeExt = Ttype::ExtMap.Find(pClassType);

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
		const auto pType = TrailType::Array[pTrails.CurrentType].get();
		if (pType->Mode != TrailMode::NONE)
		{
			bool OnTurrent = false;
			if constexpr (IsTechno) OnTurrent = pTrails.Onturrents;

			pExt->Trails.emplace_back(pType, pTrails.FLHs, OnTurrent);
			auto& pBackTrail = pExt->Trails.back();
			pBackTrail.OnLandTypes = pTrails.OnLand;
			pBackTrail.OnTileTypes = pTrails.OnTileTypes;
		}
	}

	return true;
}

template<typename Text, typename Tbase>
static bool ClearLastLoc(Tbase* pBase)
{
	auto const& pExt = Text::ExtMap.Find(pBase);

	if (!pExt)
		return false;

	if (!pExt->Trails.empty())
	{
		for (auto& pTrail : pExt->Trails)
		{
			pTrail.ClearLastLocation();
		}
	}

	return true;
}

template <typename Text, typename Tbase>
static bool ClearVector(Tbase* pBase)
{
	Text::ExtMap.Find(pBase)->Trails.clear();

	return true;
}
#pragma endregion

#pragma region Construct
template<typename T>
void TrailsManager::Construct(T* pOwner, bool IsConverted)
{
	Debug::Log(__FUNCTION__" %s Not have specified Function to use ! \n", typeid(T).name());
}

template<>
void TrailsManager::Construct(TechnoClass* pOwner, bool IsConverted)
{
	if (!pOwner || Is_Building(pOwner) || TrailType::Array.empty())
		return;

	auto pClassType = pOwner->GetTechnoType();

	if (!pClassType || pClassType->Invisible)
		return;

	auto const pExt = TechnoExt::ExtMap.Find(pOwner);
	auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pClassType);

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
		const auto pType = TrailType::Array[pTrails.CurrentType].get();

		if (pType->Mode != TrailMode::NONE)
		{
			pExt->Trails.emplace_back(pType, pTrails.FLHs, pTrails.Onturrents);
			auto& pBackTrail = pExt->Trails.back();
			pBackTrail.OnLandTypes = pTrails.OnLand;
			pBackTrail.OnTileTypes = pTrails.OnTileTypes;
		}
	}
}

template<>
void TrailsManager::Construct(BulletClass* pOwner, bool IsConverted)
{
	if (!pOwner || TrailType::Array.empty())
		return;

	if (!pOwner->Type || pOwner->Type->Inviso)
		return;

	auto pClassType = pOwner->Type;
	auto const pExt = BulletExt::ExtMap.Find(pOwner);
	auto const pTypeExt = BulletTypeExt::ExtMap.Find(pClassType);

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
		const auto pType = TrailType::Array[pTrails.CurrentType].get();

		if (pType->Mode != TrailMode::NONE)
		{
			pExt->Trails.emplace_back(pType, pTrails.FLHs, false);
			auto& pBackTrail = pExt->Trails.back();
			pBackTrail.OnLandTypes = pTrails.OnLand;
			pBackTrail.OnTileTypes = pTrails.OnTileTypes;
		}
	}
}

template<>
void TrailsManager::Construct(VoxelAnimClass* pOwner, bool IsConverted)
{
	if (!pOwner || TrailType::Array.empty())
		return;

	if (!pOwner->Type)
		return;

	auto pClassType = pOwner->Type;
	auto const pExt = VoxelAnimExt::ExtMap.Find(pOwner);
	auto const pTypeExt = VoxelAnimTypeExt::ExtMap.Find(pClassType);

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
		const auto pType = TrailType::Array[pTrails.CurrentType].get();

		if (pType->Mode != TrailMode::NONE)
		{
			pExt->Trails.emplace_back(pType, pTrails.FLHs, false);
			auto& pBackTrail = pExt->Trails.back();
			pBackTrail.OnLandTypes = pTrails.OnLand;
			pBackTrail.OnTileTypes = pTrails.OnTileTypes;
		}
	}
}

template<>
void TrailsManager::Construct(ParticleClass* pOwner, bool IsConverted)
{
	if (!pOwner || TrailType::Array.empty())
		return;

	CheckAndContruct<ParticleExt, ParticleTypeExt>(pOwner, pOwner->Type, false);
}
#pragma endregion

#pragma region AI
template<typename T>
void TrailsManager::AI(T* pOwner)
{
	Debug::Log(__FUNCTION__" %s Not have specified Function to use ! \n", typeid(T).name());
}

template<>
void TrailsManager::AI(TechnoClass* pOwner)
{
	if (!pOwner || Is_Building(pOwner) || TrailType::Array.empty())
		return;

	auto const pExt = TechnoExt::ExtMap.Find((TechnoClass*)pOwner);

	if (!pExt)
		return;

	if (!pExt->Trails.empty())
	{
		for (auto& pTrails : pExt->Trails)
		{
			if (((TechnoClass*)pOwner)->CloakState == CloakState::Cloaking ||
				((TechnoClass*)pOwner)->CloakState == CloakState::Cloaked)
			{
				if (pTrails.Type->HideWhenCloak.Get())
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

			auto nSource = TechnoExt::GetFLHAbsoluteCoords((TechnoClass*)pOwner, pTrails.FLH, pTrails.IsOnTurret);
			pTrails.DrawTrail(pOwner->GetOwningHouse(), nSource, CoordStruct::Empty);
		}
	}
}

template<>
void TrailsManager::AI(BulletClass* pOwner)
{
	if (!pOwner)
		return;

	auto const pExt = BulletExt::ExtMap.Find(pOwner);

	if (!pExt)
		return;

	if (!pExt->Trails.empty())
	{
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

			pTrails.DrawTrail((pOwner)->GetOwningHouse(), location, pTrails.FLH);
		}
	}
}

template<>
void TrailsManager::AI(VoxelAnimClass* pOwner)
{
	if (!pOwner)
		return;

	auto  pExt = VoxelAnimExt::ExtMap.Find(pOwner);
	auto pTypeExt = VoxelAnimTypeExt::ExtMap.Find(pOwner->Type);

	if (!pExt || !pTypeExt)
		return;

	if (!pExt->Trails.empty())
	{
		CoordStruct location = (pOwner)->Bounce.GetCoords();

		for (auto& pTrails : pExt->Trails)
		{
			auto const pTechnoOwner = VoxelAnimExt::GetTechnoOwner(pOwner);
			auto const pHouseOwner = pTechnoOwner ? pTechnoOwner->GetOwningHouse() : pOwner->GetOwningHouse();
			pTrails.DrawTrail(pHouseOwner, location, pTrails.FLH);
		}
	}
}

template<>
void TrailsManager::AI(ParticleClass* pOwner)
{
	if (!pOwner)
		return;

	auto const pExt = ParticleExt::ExtMap.Find(pOwner);

	if (!pExt)
		return;

	if (!pExt->Trails.empty())
	{
		CoordStruct location = (pOwner)->GetCoords();

		for (auto& pTrails : pExt->Trails)
		{
			pTrails.DrawTrail((pOwner)->GetOwningHouse(), location, pTrails.FLH);
		}
	}
}

#pragma endregion

#pragma region Hide

template<typename T>
void TrailsManager::Hide(T* pOwner)
{
	Debug::Log(__FUNCTION__" %s Not have specified Function to use ! \n", typeid(T).name());
}
template<>
void TrailsManager::Hide(TechnoClass* pOwner)
{
	if (!pOwner || Is_Building(pOwner) || TrailType::Array.empty())
		return;

	auto const pExt = TechnoExt::ExtMap.Find((TechnoClass*)pOwner);

	if (!pExt->Trails.empty())
	{
		for (auto& pTrail : pExt->Trails)
		{
			pTrail.Visible = !(((TechnoClass*)pOwner)->InLimbo);
			pTrail.ClearLastLocation();
		}
	}
}

template<>
void TrailsManager::Hide(BulletClass* pOwner)
{
	if (!pOwner || TrailType::Array.empty())
		return;

	auto const pExt = BulletExt::ExtMap.Find((BulletClass*)pOwner);;

	if (!pExt->Trails.empty())
	{
		for (auto& pTrail : pExt->Trails)
		{
			pTrail.ClearLastLocation();
		}
	}
}

template<>
void TrailsManager::Hide(VoxelAnimClass* pOwner)
{
	if (!pOwner || TrailType::Array.empty())
		return;

	auto const pExt = VoxelAnimExt::ExtMap.Find((VoxelAnimClass*)pOwner);

	if (!pExt->Trails.empty())
	{
		for (auto& pTrail : pExt->Trails)
		{
			pTrail.ClearLastLocation();
		}
	}
}

template<>
void TrailsManager::Hide(ParticleClass* pOwner)
{
	if (!pOwner || TrailType::Array.empty())
		return;

	if (!ClearLastLoc<ParticleExt>((ParticleClass*)pOwner))
		return;
}

#pragma endregion

#pragma region CleanUp
template<typename T>
void TrailsManager::CleanUp(T* pOwner)
{
	Debug::Log(__FUNCTION__" %s Not have specified Function to use ! \n", typeid(T).name());
}

template<>
void TrailsManager::CleanUp(TechnoClass* pOwner)
{
	if (!pOwner || Is_Building(pOwner) || TrailType::Array.empty())
		return;

	if (!ClearVector<TechnoExt>((TechnoClass*)pOwner))
		return;
}

template<>
void TrailsManager::CleanUp(BulletClass* pOwner)
{
	if (!pOwner || TrailType::Array.empty())
		return;

	if (!ClearVector<BulletExt>((BulletClass*)pOwner))
		return;
}

template<>
void TrailsManager::CleanUp(VoxelAnimClass* pOwner)
{
	if (!pOwner || TrailType::Array.empty())
		return;

	if (!ClearVector<VoxelAnimExt>((VoxelAnimClass*)pOwner))
		return;
}

template<>
void TrailsManager::CleanUp(ParticleClass* pOwner)
{
	if (!pOwner || TrailType::Array.empty())
		return;

	if (!ClearVector<ParticleExt>((ParticleClass*)pOwner))
		return;
}
#pragma endregion
#endif