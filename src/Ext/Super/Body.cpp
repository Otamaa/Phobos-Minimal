#include "Body.h"

#include <Ext/Building/Body.h>
#include <Ext/SWType/Body.h>

#include <Misc/Ares/Hooks/Header.h>

#include <Utilities/Macro.h>

#include <Phobos.SaveGame.h>


// This function controls the availability of super weapons. If a you want to
// add to or change the way the game thinks a building provides a super weapon,
// change the lambda UpdateStatus. Available means this super weapon exists at
// all. Setting it to false removes the super weapon. PowerSourced controls
// whether the super weapon charges or can be used.
void SuperExtData::UpdateSuperWeaponStatuses(HouseClass* pHouse)
{
	// look at every sane building this player owns, if it is not defeated already.
	if (!pHouse->Defeated && !pHouse->IsObserver())
	{
		if (pHouse->Supers.Count > 0) {
			pHouse->Supers.for_each([pHouse](SuperClass* pSuper) {
				auto pExt = SuperExtContainer::Instance.Find(pSuper);
				pExt->Statusses.reset();

				//if AlwaysGranted and SWAvaible
				pExt->Statusses.PowerSourced = !pSuper->IsPowered();
				if (pExt->Type->SW_AlwaysGranted && pExt->Type->IsAvailable(pHouse))
				{
					pExt->Statusses.Available = true;
					pExt->Statusses.Charging = true;
					pExt->Statusses.PowerSourced = true;
				}
			});
		}

		pHouse->Buildings.for_each([=](BuildingClass* pBld) {
			if (pBld->IsAlive && !pBld->InLimbo)
			{
				bool PowerChecked = false;
				bool HasPower = false;

				// check for upgrades. upgrades can give super weapons, too.
				for (const auto type : pBld->GetTypes())
				{
					if (auto pUpgradeExt = BuildingTypeExtContainer::Instance.TryFind(const_cast<BuildingTypeClass*>(type)))
					{
						for (auto i = 0; i < pUpgradeExt->GetSuperWeaponCount(); ++i)
						{
							const auto idxSW = pUpgradeExt->GetSuperWeaponIndex(i);

							if (idxSW >= 0)
							{
								const auto pSuperExt = SuperExtContainer::Instance.Find(pHouse->Supers[idxSW]);
								auto& status = pSuperExt->Statusses;

								if (!status.Charging)
								{
									if (pSuperExt->Type->IsAvailable(pHouse))
									{
										status.Available = true;

										if (!PowerChecked)
										{
											HasPower = pBld->HasPower
												&& !pBld->IsUnderEMP()
												&& (TechnoExtContainer::Instance.Find(pBld)->Is_Operated || TechnoExt_ExtData::IsOperated(pBld));

											PowerChecked = true;
										}

										if (!status.Charging && HasPower)
										{
											status.PowerSourced = true;

											if (!pBld->IsBeingWarpedOut()
												&& (pBld->CurrentMission != Mission::Construction)
												&& (pBld->CurrentMission != Mission::Selling)
												&& (pBld->QueuedMission != Mission::Construction)
												&& (pBld->QueuedMission != Mission::Selling))
											{
												status.Charging = true;
											}
										}
									}
								}
							}
						}
					}
				}
			}
		});

		// kill off super weapons that are disallowed and
		// factor in the player's power status
		const bool hasPower = pHouse->HasFullPower();
		const bool isCampaign = SessionClass::Instance->GameMode == GameMode::Campaign;
		const bool bIsSWShellEnabled = Unsorted::SWAllowed || isCampaign;

		if (!hasPower || !bIsSWShellEnabled) {
			pHouse->Supers.for_each([&](SuperClass* pSuper) {

				const auto pExt = SuperExtContainer::Instance.Find(pSuper);
				auto& nStatus = pExt->Statusses;

				// turn off super weapons that are disallowed.
				if (!bIsSWShellEnabled && pSuper->Type->DisableableFromShell)
				{
					nStatus.Available = false;
				}

				// if the house is generally on low power,
				// powered super weapons aren't powered
				if (!hasPower && pSuper->IsPowered())
				{
					nStatus.PowerSourced &= hasPower;
				}
			});
		}
	}
}


// =============================
// load / save

template <typename T>
void SuperExtData::Serialize(T& Stm) {

	Stm
		.Process(this->Name)
		.Process(this->Type, true)
		.Process(this->Temp_CellStruct)
		.Process(this->Temp_IsPlayer)
		.Process(this->CameoFirstClickDone)
		.Process(this->FirstClickAutoFireDone)
		//.Process(this->Firer)
		.Process(this->Statusses)

		.Process(this->MusicTimer)
		.Process(this->MusicActive)
		;
}

// =============================
// container
SuperExtContainer SuperExtContainer::Instance;

bool SuperExtContainer::LoadAll(const json& root)
{
	this->Clear();

	if (root.contains(SuperExtContainer::ClassName))
	{
		auto& container = root[SuperExtContainer::ClassName];

		for (auto& entry : container[SuperExtData::ClassName])
		{
			uint32_t oldPtr = 0;
			if (!ExtensionSaveJson::ReadHex(entry, "OldPtr", oldPtr))
				return false;

			size_t dataSize = entry["datasize"].get<size_t>();
			std::string encoded = entry["data"].get<std::string>();
			auto buffer = this->AllocateNoInit();

			PhobosByteStream loader(dataSize);
			loader.data = std::move(Base64Handler::decodeBase64(encoded, dataSize));
			PhobosStreamReader reader(loader);

			PHOBOS_SWIZZLE_REGISTER_POINTER(oldPtr, buffer, SuperExtData::ClassName);

			buffer->LoadFromStream(reader);

			if (!reader.ExpectEndOfBlock())
				return false;
		}

		return true;
	}

	return false;

}

bool SuperExtContainer::SaveAll(json& root)
{
	auto& first_layer = root[SuperExtContainer::ClassName];

	json _extRoot = json::array();
	for (auto& _extData : SuperExtContainer::Array)
	{
		PhobosByteStream saver(sizeof(*_extData));
		PhobosStreamWriter writer(saver);

		_extData->SaveToStream(writer);

		json entry;
		ExtensionSaveJson::WriteHex(entry, "OldPtr", (uint32_t)_extData);
		entry["datasize"] = saver.data.size();
		entry["data"] = Base64Handler::encodeBase64(saver.data);
		_extRoot.push_back(std::move(entry));
	}

	first_layer[SuperExtData::ClassName] = std::move(_extRoot);

	return true;
}

// .cpp file
SuperExtData::SuperExtData(SuperClass* pObj) : AbstractExtended(pObj)
	// Large aggregates
	, Name()
	, Statusses()
	, MusicTimer()
	// Pointer
	, Type(nullptr)
	// CellStruct
	, Temp_CellStruct()
	// bools
	, Temp_IsPlayer(false)
	, CameoFirstClickDone(false)
	, FirstClickAutoFireDone(false)
	, MusicActive(false)
{
	this->Type = SWTypeExtContainer::Instance.Find(pObj->Type);
	this->Name = pObj->Type->ID;
	this->AbsType = SuperClass::AbsID;
}
// =============================
// container hooks

ASMJIT_PATCH(0x6CB10E, SuperClass_CTOR, 0x7)
{
	GET(SuperClass*, pItem, ESI);

	SuperExtContainer::Instance.Allocate(pItem);
	return 0;
}

ASMJIT_PATCH(0x6CB1BD, SuperClass_SDDTOR, 0x7)
{
	GET(SuperClass*, pItem, ESI);
	SuperExtContainer::Instance.Remove(pItem);
	return 0;
}

// ASMJIT_PATCH(0x6CE001 , SuperClass_Detach , 0x5)
// {
// 	GET(SuperClass*, pThis, ESI);
// 	GET(void*, target, EAX);
// 	GET_STACK(bool, all, STACK_OFFS(0x4, -0x8));
//
// 	SuperExtContainer::Instance.InvalidatePointerFor(pThis , target , all);
//
// 	return target == pThis->Type ? 0x6CE006 : 0x6CE009;
// }

//void __fastcall SuperClass_Detach_Wrapper(SuperClass* pThis ,DWORD , AbstractClass* target , bool all)\
//{
//	SuperExtContainer::Instance.InvalidatePointerFor(pThis , target , all);
//	pThis->SuperClass::PointerExpired(target , all);
//}
//DEFINE_FUNCTION_JUMP(VTABLE, 0x7F4010, GET_OFFSET(SuperClass_Detach_Wrapper))

int FakeSuperClass::_GetAnimStage()
{
	if (!this->Granted) {
		return 0;
	}

	auto pType = this->Type;

	// Get effective recharge time
	int customRechargeTime = this->CustomChargeTime;
	int rechargeTime = (customRechargeTime == -1) ? pType->RechargeTime : customRechargeTime;

	// Calculate remaining delay time
	int delayTime = this->RechargeTimer.TimeLeft;
	int started = this->RechargeTimer.StartTime;

	if (started != -1) {
		int elapsed = Unsorted::CurrentFrame - started;
		if (elapsed >= delayTime) {
			delayTime = 0;
		} else {
			delayTime -= elapsed;
		}
	}

	// Calculate progress ratio
	double progress = 0.0;

	if (pType->UseChargeDrain) {
		if (this->ChargeDrainState == ChargeDrainState::Draining) {
			// Draining state - reverse progress
			// [HOOK APPLIED: 0x6CBF5B - SuperClass_GetCameoChargeStage_ChargeDrainRatio]
			int rechargeTime1 = rechargeTime;
			int rechargeTime2 = (customRechargeTime == -1) ? pType->RechargeTime : customRechargeTime;
			int timeLeft = delayTime;

			// Use per-SW charge-to-drain ratio instead of Rule->ChargeToDrainRatio
			auto pTypeExt = SWTypeExtContainer::Instance.Find(pType);
			const double ratio = pTypeExt->GetChargeToDrainRatio();

			if (Math::abs(rechargeTime2 * ratio) > 0.001) {
				progress = 1.0 - (rechargeTime1 * ratio - timeLeft) / (rechargeTime2 * ratio);
			} else {
				progress = 0.0;
			}
		} else {
			// Charging state
			int divisor = (customRechargeTime == -1) ? pType->RechargeTime : customRechargeTime;
			progress = (double)(rechargeTime - delayTime) / divisor;
		}
	}
	else
	{
		// Non-ChargeDrain mode
		if (this->IsCharged) {
			return 54;
		}

		// Normal charging
		int divisor = (customRechargeTime == -1) ? pType->RechargeTime : customRechargeTime;
		progress = (double)(rechargeTime - delayTime) / divisor;
	}

	// Convert to stage
	// [HOOK APPLIED: 0x6CC053 - SuperClass_GetCameoChargeStage_FixFullyCharged]
	int stage = (int)(progress * 54.0);

	// Clamp to 0-54 (original clamped to 53, hook fixes to 54)
	if (stage > 54) {
		stage = 54;
	}

	return stage;
}
DEFINE_FUNCTION_JUMP(LJMP, 0x6CBEE0, FakeSuperClass::_GetAnimStage)
DEFINE_FUNCTION_JUMP(CALL, 0x6CBE7E, FakeSuperClass::_GetAnimStage)
DEFINE_FUNCTION_JUMP(CALL, 0x6CBE8A, FakeSuperClass::_GetAnimStage)