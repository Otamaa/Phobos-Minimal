#include <exception>
#include <Windows.h>

#include <AircraftClass.h>
#include <AircraftTypeClass.h>
#include <Kamikaze.h>
#include <Locomotor/JumpjetLocomotionClass.h>
#include <SpawnManagerClass.h>
#include <TechnoTypeClass.h>
#include <TechnoClass.h>

#include <Misc/Kratos/Kratos.h>
#include <Helpers/Macro.h>

#include <Misc/Kratos/Extension/TechnoExt.h>

#include <Misc/Kratos/Ext/Helper/Scripts.h>

#include <Misc/Kratos/Ext/ObjectType/AttachFire.h>
#include <Misc/Kratos/Ext/TechnoType/MissileHoming.h>
#include <Misc/Kratos/Ext/TechnoType/Spawn.h>
#include <Misc/Kratos/Ext/TechnoType/TechnoStatus.h>

#ifdef _ENABLE_HOOKS

#pragma region MultiSpawnType

static bool TryFindNewIdInSpwanType(std::string typeId, int index, std::string& newId)
{
	if (typeId.find(",") != std::string::npos)
	{
		std::vector<std::string> values = {};
		char* context = nullptr;
		for (auto cur = strtok_s(const_cast<char*>(typeId.c_str()), ",", &context); cur && *cur; cur = strtok_s(nullptr, ",", &context)) {
			std::string tmp{ cur };
			values.push_back(tmp);
		}
		int size = values.size();
		if (index < size)
		{
			newId = trim(values[index]);
			return true;
		}
	}
	return false;
}

namespace MultiSpawn
{
	SpawnManagerClass* pManager = nullptr;
	Spawn* pSpawn = nullptr;
}

ASMJIT_PATCH(0x6B6CDF, SpawnManagerClass_CreateSpawnNode, 0x7)
{
	GET(SpawnManagerClass*, pManager, ESI);
	GET(TechnoClass*, pTechno, EDX);
	GET(TechnoTypeClass*, pAircraftType, EAX);
	std::string typeId{ pAircraftType->ID };
	MultiSpawn::pManager = nullptr;
	MultiSpawn::pSpawn = nullptr;
	if (typeId.find(",") != std::string::npos)
	{
		// 是一条多行类型，如Spawns=HORNET,ASW，记录下来，并修正类型
		// 此时TechnoStatus已经创建，但未初始化，需要手动挂载Spawn组件
		MultiSpawn::pSpawn = FindOrAttachScript<TechnoExt, Spawn>(pTechno);
		if (MultiSpawn::pSpawn)
		{
			MultiSpawn::pManager = pManager;
		}
		// 修正管理器里记录的正确的Type
		std::string newId{ "" };
		TryFindNewIdInSpwanType(typeId, 0, newId);
		if (IsNotNone(newId))
		{
			AircraftTypeClass* pNewType = AircraftTypeClass::Find(newId.c_str());
			if (pNewType)
			{
				R->EAX(reinterpret_cast<unsigned int>(pNewType));
			}
		}
	}
	return 0;
}


ASMJIT_PATCH(0x6B6D4D, SpawnManagerClass_CreateSpawnNode_CreateAircraft, 0x6)
{
	GET(SpawnManagerClass*, pManager, ESI);
	GET_STACK(int, i, 0x20 + 0x4);

	if (MultiSpawn::pManager == pManager)
	{
		std::string newId{ "" };
		Spawn* spawnEx = MultiSpawn::pSpawn;
		// 如果没有设置SpwanData，就从ini名字中找
		if (!spawnEx || !spawnEx->TryGetSpawnType(i, newId))
		{
			GET(TechnoTypeClass*, pType, ECX);
			std::string typeId{ pType->ID };
			TryFindNewIdInSpwanType(typeId, i, newId);
		}
		if (IsNotNone(newId))
		{
			AircraftTypeClass* pNewType = AircraftTypeClass::Find(newId.c_str());
			if (pNewType)
			{
				pManager->SpawnType = pNewType;
				R->ECX(reinterpret_cast<unsigned int>(pNewType));
			}
		}
	}
	return 0;
}

ASMJIT_PATCH(0x6B78E4, SpawnManagerClass_Update_CreateAircraft, 0x6)
{
	GET(SpawnManagerClass*, pManager, ESI);
	GET(int, i, EBX);

	std::string newId{ "" };
	Spawn* spawnEx = GetScript<TechnoExt, Spawn>(pManager->Owner);
	// 如果没有设置SpwanData，就从ini名字中找
	if (!spawnEx || !spawnEx->TryGetSpawnType(i, newId))
	{
		GET(TechnoTypeClass*, pType, ECX);
		std::string typeId{ pType->ID };
		TryFindNewIdInSpwanType(typeId, i, newId);
	}
	if (IsNotNone(newId))
	{
		AircraftTypeClass* pNewType = AircraftTypeClass::Find(newId.c_str());
		if (pNewType)
		{
			pManager->SpawnType = pNewType;
			R->ECX(reinterpret_cast<unsigned int>(pNewType));
		}
	}

	return 0;
}

#pragma endregion // MultiSpawnType

ASMJIT_PATCH(0x6B7317, SpawnManagerClass_Update_MissileSpawn_SkipMovingCheck, 0x7)
{
	GET(SpawnManagerClass*, pManager, ESI);
	TechnoClass* pTechno = pManager->Owner;
	FootClass* pFoot = nullptr;
	if (pTechno && pTechno->GetTechnoType()->BalloonHover && CastToFoot(pTechno, pFoot))
	{
		GET(ILocomotion**, ppLoco, EBP);
		if (JumpjetLocomotionClass* pJJ = locomotion_cast<JumpjetLocomotionClass*>(*ppLoco))
		{
			// 检查是否站住了
			AbstractClass* pDest = pFoot->Destination;
			if (!pDest)
			{
				CoordStruct location = pFoot->GetCoords();
				pDest = MapClass::Instance->TryGetCellAt(location);
			}
			// 和目的地的距离小于16就算站住了
			if (pTechno->DistanceFrom(pDest) < 16)
			{
				// Launch spawn
				return 0x6B735C;
			}
		}
	}
	return 0;
}

ASMJIT_PATCH(0x6B743E, SpawnManagerClass_Update_PutSpawns, 0x6)
{
	GET(TechnoClass*, pTechno, ECX);
	GET(SpawnManagerClass*, pManager, ESI);
	GET(int, weaponIdx, EBP);

	bool customFLH = false;
	if (weaponIdx > 0)
	{
		// wwsb 主武器没有Spawner，所以用副武器，继续检查其他武器
		// 检查副武器有没有Spawner
		WeaponStruct* pWeapon = pTechno->GetWeapon(weaponIdx);
		bool spawner = false;
		if (pWeapon && pWeapon->WeaponType)
		{
			spawner = pWeapon->WeaponType->Spawner;
		}
		if (!spawner)
		{
			// 副武器上也没有Spawner，继续找盖特武器其他的武器
			int weaponCount = pTechno->GetTechnoType()->WeaponCount;
			if (weaponCount > 2)
			{
				for (int i = 2; i < weaponCount; i++)
				{
					pWeapon = pTechno->GetWeapon(i);
					WeaponTypeClass* pWeaponType = nullptr;
					if (pWeapon && (pWeaponType = pWeapon->WeaponType) != nullptr && pWeaponType->Spawner)
					{
						// 找到一个子机发射器
						spawner = true;
						weaponIdx = i;
						break;
					}
				}
			}
		}
		if (spawner)
		{
			// 找到另外的子机发射器，设置Index
			R->EBP(static_cast<unsigned int>(weaponIdx));
		}
		else if (AttachFire* pFire = FindOrAttachScript<TechnoExt, AttachFire>(pTechno))
		{
			if (!pFire->SpawnerBurstFLH.empty())
			{
				int index = 0;
				int count = pManager->SpawnCount;
				if (count > 1)
				{
					index = count - pManager->CountDockedSpawns() - 1;
				}
				auto it = pFire->SpawnerBurstFLH.find(index);
				customFLH = it != pFire->SpawnerBurstFLH.end();
				if (customFLH)
				{
					// 副武器和盖特武器上都没有子机发射器，检查子机是否由ExtraFire或者AutoWeapon发射
					GET(CoordStruct*, eax, EAX);
					*eax = pFire->SpawnerBurstFLH[index];
				}
			}
		}
	}
	// 重设子机发射延迟
	if (Spawn* spawnEx = GetScript<TechnoExt, Spawn>(pTechno))
	{
		// 重设子机发射延迟
		if (spawnEx->GetSpawnData()->SpawnDelay > -1)
		{
			pManager->SpawnTimer.Start(spawnEx->GetSpawnData()->SpawnDelay);
		}
	}
	if (customFLH)
	{
		return 0x6B7498;
	}
	return 0;
}

ASMJIT_PATCH(0x6B796A, SpawnManagerClass_Update_PutSpawns_FireOnce, 0x5)
{
	GET(SpawnManagerClass*, pManager, ESI);
	if (pManager->CountDockedSpawns() == 0)
	{
		if (Spawn* spawnEx = GetScript<TechnoExt, Spawn>(pManager->Owner))
		{
			if (spawnEx->GetSpawnData()->SpawnFireOnce)
			{
				pManager->Destination = nullptr;
				pManager->SetTarget(nullptr);
			}
		}
	}
	return 0;
}

// ASMJIT_PATCH(0x6B77B4, SpawnManagerClass_Update_Callback, 0x7)
// {
// 	GET(SpawnManagerClass*, pManager, ESI);
// 	GET_STACK(int, i, 0x14);
// 	pManager->SpawnedNodes[i]->Status = SpawnNodeStatus::Reloading;
// 	return 0x6B7838;
// }

#pragma region Rocket Homing

// 如果子机管理器输入的预设目标在天上，自动开启跟踪模式
ASMJIT_PATCH(0x6B7A32, SpawnManagerClass_Update_Add_Missile_Target, 0x5)
{
	// GET(SpawnManagerClass*, pManager, ESI);
	GET(TechnoClass*, pRocket, ECX);
	GET(AbstractClass*, pTarget, EAX); // 预设目标，pManager->Destination，之后会通过KamikazeTrackerClass_Add写入Node->Cell

	// 将预设目标写入导弹，如果目标在天上，则开启跟踪模式
	if (MissileHoming* homing = GetScript<TechnoExt, MissileHoming>(pRocket))
	{
		// 预设目标不是FootClass，则强制关闭跟踪模式
		if (pTarget->AbstractFlags & AbstractFlags::Foot)
		{
			homing->HomingTarget = pTarget;
			// 目标如果在空中，强制开启跟踪模式
			if (pTarget->IsInAir())
			{
				homing->IsHoming = true;
			}
		}
		else
		{
			homing->HomingTarget = nullptr;
			homing->IsHoming = false;
		}
	}

	return 0;
}

// SpawnManagerClass_Update 会调用KamikazeTrackerClass_Add，传入导弹和目标格子
// 创建KamikazeControl，关联导弹和目标格子，写入全局清单KamikazeContainer
// 之后便交给KamikazeContainer处理，在Update中更新导弹的目标
// 这里传进来的参数destination，是SpawnManagerClass中记录的目标，是Techno或者Cell
// 这里会按照传入的destination，获得导弹脚下的格子，或者目标的格子，写入KamikazeControl
// ASMJIT_PATCH(0x54E478, KamikazeTrackerClass_Add, 0x5)
// {
// 	GET(Kamikaze::KamikazeControl*, pKamikazeControl, EBX);
// 	AircraftClass* pRocket = pKamikazeControl->Item;
// 	AbstractClass* pCell = pKamikazeControl->Cell;
// 	AbstractType cellType = AbstractType::None;
// 	if (pCell)
// 	{
// 		cellType = pCell->WhatAmI();
// 	}

// 	AbstractClass* pTarget = pRocket->Target;

// 	Debug::Log("KamikazeTrackerClass_Add, pKamikaze = %p, %p -> %p(%u), RocketTarget = %p\n", pKamikazeControl, pRocket, pCell, static_cast<unsigned int>(cellType), pRocket->Target);
// 	if (pTarget)
// 	{
// 		pKamikazeControl->Cell = pTarget;
// 	}
// 	return 0;
// }

namespace KamikazeTracker
{
	Kamikaze::KamikazeControl* pKamikazeControl = nullptr;
}

// 此处记录下pKamikazeControl的指针，便于后续修改导弹的目标
ASMJIT_PATCH(0x54E51D, KamikazeContainer_Update_SetTargetBefore, 0x5)
{
	GET(Kamikaze::KamikazeControl*, pKamikazeControl, EAX);
	// 缓存
	KamikazeTracker::pKamikazeControl = pKamikazeControl;
	return 0;
}

// 不可以Hook此处，否则会破坏对ECX的检查，认为pCell为0，导致进入0x54E540，搜寻脚下的格子当做导弹的目标
// ASMJIT_PATCH(0x54E524, KamikazeContainer_Update_SetTarget, 0xA)
// {
// 	GET(Kamikaze::KamikazeControl*, pKamikazeControl, EAX);
// 	GET(AbstractClass*, pTarget, ECX);

// 	AircraftClass* pRocket = pKamikazeControl->Item;
// 	AbstractClass* pCell = pKamikazeControl->Cell;

// 	DWORD* pECX = R->ECX<DWORD*>();

// 	Debug::Log(" - KamikazeContainer_Update_SetTarget, pKamikaze = %p, %p -> %p, ECX = %p, %p\n", pKamikazeControl, pRocket, pCell, R->ECX(), *pECX);
// 	return 0;
// }

// 当KamikazeControl的Cell为0时，会搜寻脚下的格子作为目标
// ASMJIT_PATCH(0x54E540, KamikazeContainer_Update_SetTarget_Zero, 0x7)
// {
// 	Debug::Log(" - KamikazeContainer_Update_SetTarget_Zero\n");
// 	return 0;
// }

// 此处已设置完导弹的目标，准备执行导弹的任务，可以在此修改导弹的目标
ASMJIT_PATCH(0x54E56D, KamikazeContainer_Update_SetTargetAfter, 0x6)
{
	GET(AircraftClass*, pRocket, ESI);
	// 读取记录下的KamikazeControl，修改导弹的目标
	Kamikaze::KamikazeControl* pKamikazeControl = KamikazeTracker::pKamikazeControl;
	KamikazeTracker::pKamikazeControl = nullptr;

	// 如有必要修改导弹的目标，同时修改KamikazeControl的Cell
	if (MissileHoming* homing = GetScript<TechnoExt, MissileHoming>(pRocket))
	{
		// 此处尝试修改导弹的目标，同时修改KamikazeControl的Cell
		homing->KamikazeUpdateTarget(pKamikazeControl);
	}

	return 0;
}

// ASMJIT_PATCH(0x6622C0, RocketLocomotionClass_Process, 0x6)
// {
// 	GET(FootClass*, pFoot, ESI);
// 	RocketLocomotionClass* pLoco = locomotion_cast<RocketLocomotionClass*>(pFoot->Locomotor.get());

// 	AbstractClass* pTarget = pFoot->Target;
// 	AbstractType targetType = AbstractType::None;
// 	if (pTarget)
// 	{
// 		targetType = pTarget->WhatAmI();
// 	}
// 	Debug::Log("RocketLocomotionClass_Process, %d, %p -> %p, %u\n", pLoco->MissionState, pFoot, pTarget, static_cast<unsigned int>(targetType));

// 	// RocketLocomotionClass* pLoco = locomotion_cast<RocketLocomotionClass*>(pFoot->Locomotor.get());
// 	// If missile try to AA, it will block on ground. step is 0, can't move.
// 	// if (pLoco->MissionState == 0)
// 	// {
// 	// 	pLoco->MissionState = 1;
// 	// }
// 	return 0;
// }

ASMJIT_PATCH(0x662CAC, RocketLocomotionClass_Process_Step5_To_Lazy_4, 0x6)
{
	RocketLocomotionClass* pLoco = (RocketLocomotionClass*)(R->ESI() - 4);
	TechnoClass* pRocket = pLoco->LinkedTo;
	if (MissileHoming* homing = GetScript<TechnoExt, MissileHoming>(pRocket))
	{
		if (homing->IsHoming)
		{
			return 0x662A32;
		}
	}
	return 0;
}

ASMJIT_PATCH(0x66304F, RocketLocomotionClass_663030, 0x5)
{
	GET(TechnoClass*, pThis, EDX);

	if (auto pExt = TechnoExt::ExtMap.Find(pThis))
	{
		pExt->_GameObject->Foreach([](Component* c)
			{ if (auto cc = dynamic_cast<ITechnoScript*>(c)) { cc->OnRocketExplosion(); } });
	}

	return 0;
}
#pragma endregion
#endif