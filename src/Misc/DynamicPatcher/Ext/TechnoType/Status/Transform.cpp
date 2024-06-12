#include "../TechnoStatus.h"

#include <Locomotor/Cast.h>

#include <Misc/DynamicPatcher/Ext/Helper/Physics.h>

void TechnoStatus::OnUpdate_Transform()
{
	if (!_transformLocked)
	{
		// 执行变形逻辑
		if (Transform->IsAlive())
		{
			if (!_hasBeenChanged || _changeToType != Transform->Data.TransformToType)
			{
				_changeToType = Transform->Data.TransformToType;
				TechnoTypeClass* pTargetType = nullptr;
				if (IsNotNone(_changeToType) && (pTargetType = TechnoTypeClass::Find(_changeToType.c_str())) != nullptr)
				{
					_hasBeenChanged = ChangeTechnoTypeTo(pTargetType);
				}
				else
				{
					Transform->End();
				}
			}
		}
		else if (_hasBeenChanged)
		{
			// 还原
			_hasBeenChanged = false;
			ChangeTechnoTypeTo(pSourceType);
		}
		// 单位类型发生了改变，发出通知
		if (pTechno->GetTechnoType() != pTargetType)
		{
			pTargetType = pTechno->GetTechnoType();
			// 通过GameObject发出通知
			_gameObject->ExtChanged = true;

			// 在天上，但不会飞
			if (pTechno->IsInAir() && !pTargetType->ConsideredAircraft)
			{
				FallingDown(pTechno, 0, false);
			}
		}
	}
}

void TechnoStatus::OnReceiveDamageDestroy_Transform()
{
	if (_hasBeenChanged)
	{
		// 死亡时强制还原
		// Logger.Log("强制还原类型 {0}, ConverTypeStatus = {1}", OwnerObject, ConvertTypeStatus);
		ChangeTechnoTypeTo(pSourceType);
		_hasBeenChanged = false;
		_transformLocked = true;
	}
}

bool TechnoStatus::ChangeTechnoTypeTo(TechnoTypeClass* pNewType)
{
	AbstractType absType = GetAbsType();
	TechnoTypeClass** pType = nullptr;
	switch (absType)
	{
	case AbstractType::Infantry:
		pType = reinterpret_cast<TechnoTypeClass**>(&(static_cast<InfantryClass*>(pTechno)->Type));
		break;
	case AbstractType::Unit:
		pType = reinterpret_cast<TechnoTypeClass**>(&(static_cast<UnitClass*>(pTechno)->Type));
		break;
	case AbstractType::Aircraft:
		pType = reinterpret_cast<TechnoTypeClass**>(&(static_cast<AircraftClass*>(pTechno)->Type));
		break;
	default:
		Debug::Log("Warning: %s is not FootClass, conversion not allowed\n", pNewType->get_ID());
		return false;
	}
	FootClass* pFoot = static_cast<FootClass*>(pTechno);

	// Detach CLEG targeting
	auto tempUsing = pFoot->TemporalImUsing;
	if (tempUsing && tempUsing->Target)
		tempUsing->Detach();

	HouseClass* const pHouse = pFoot->Owner;

	// Remove tracking of old techno
	if (!pFoot->InLimbo)
	{
		pHouse->RegisterLoss(pFoot, false);
	}
	pHouse->RemoveTracking(pFoot);

	int oldHealth = pFoot->Health;

	// Generic type-conversion
	TechnoTypeClass* prevType = *pType;
	*pType = pNewType;

	// Readjust health according to percentage
	pFoot->SetHealthPercentage((double)(oldHealth) / (double)prevType->Strength);
	pFoot->EstimatedHealth = pFoot->Health;

	// Add tracking of new techno
	pHouse->AddTracking(pFoot);
	if (!pFoot->InLimbo)
	{
		pHouse->RegisterGain(pFoot, false);
	}
	pHouse->RecheckTechTree = true;

	// Adjust ammo
	int ammoLeft = pFoot->Ammo;
	ammoLeft = MinImpl(ammoLeft, pNewType->Ammo);
	pFoot->Ammo = ammoLeft;
	if (ammoLeft < 0 || ammoLeft >= pNewType->Ammo)
	{
		pFoot->ReloadTimer.Stop();
	}
	else
	{
		int reloadLeft = pFoot->ReloadTimer.GetTimeLeft();
		int reloadPrev = 0;
		int reloadNew = 0;
		if (ammoLeft == 0)
		{
			reloadPrev = prevType->EmptyReload;
			reloadNew = pNewType->EmptyReload;
		}
		else if (pFoot->Ammo)
		{
			reloadPrev = prevType->Reload;
			reloadNew = pNewType->Reload;
		}
		int pass = reloadPrev - reloadLeft;
		if (pass <= 0 || pass >= reloadNew)
		{
			pFoot->ReloadTimer.Stop();
		}
		else
		{
			int reload = reloadNew - pass;
			pFoot->ReloadTimer.Start(reload);
		}
	}

	// Adjust ROT
	DirStruct bodyFacing = pFoot->PrimaryFacing.Current();
	if (absType == AbstractType::Aircraft)
	{
		bodyFacing = pFoot->SecondaryFacing.Current();
		pFoot->PrimaryFacing.Set_Current(bodyFacing);
		pFoot->SecondaryFacing.Set_ROT(pNewType->ROT);
	}
	else
	{
		pFoot->PrimaryFacing.Set_ROT(pNewType->ROT);
	}

	// Locomotor change, referenced from Ares 0.A's abduction code, not sure if correct, untested
	CLSID nowLocoID;
	ILocomotion* prevLoco = pFoot->Locomotor.GetInterfacePtr();
	if (JumpjetLocomotionClass* prevJJ = locomotion_cast<JumpjetLocomotionClass*>(prevLoco))
	{
		bodyFacing = prevJJ->Facing.Current();
		pFoot->PrimaryFacing.Set_Current(bodyFacing);
		pFoot->SecondaryFacing.Set_Current(bodyFacing);
	}
	CoordStruct dest = prevLoco->Destination();
	if (dest.IsEmpty())
	{
		dest = pFoot->GetCoords();
	}
	const auto& toLoco = pNewType->Locomotor;
	if ((SUCCEEDED(static_cast<LocomotionClass*>(prevLoco)->GetClassID(&nowLocoID)) && nowLocoID != toLoco))
	{
		// because we are throwing away the locomotor in a split second, piggybacking
		// has to be stopped. otherwise the object might remain in a weird state.
		while (LocomotionClass::End_Piggyback(pFoot->Locomotor));
		// throw away the current locomotor and instantiate
		// a new one of the default type for this unit.
		if (auto newLoco = LocomotionClass::CreateInstance(toLoco))
		{
			newLoco->Link_To_Object(pFoot);
			pFoot->Locomotor = std::move(newLoco);
			pFoot->Locomotor->Move_To(dest);
			if (JumpjetLocomotionClass* newJJ = locomotion_cast<JumpjetLocomotionClass*>(pFoot->Locomotor))
			{
				newJJ->Facing.Set_Current(bodyFacing);
			}
		}
	}
	return true;
}
