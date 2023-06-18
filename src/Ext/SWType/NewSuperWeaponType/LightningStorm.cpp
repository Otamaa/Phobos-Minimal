#include "LightningStorm.h"

SuperClass* SW_LightningStorm::CurrentLightningStorm = nullptr;

bool SW_LightningStorm::HandleThisType(SuperWeaponType type) const
{
	return (type == SuperWeaponType::LightningStorm);
}

SuperWeaponFlags SW_LightningStorm::Flags() const
{
	return SuperWeaponFlags::NoMessage | SuperWeaponFlags::NoEvent;
}

bool SW_LightningStorm::Activate(SuperClass* pThis, const CellStruct& Coords, bool IsPlayer)
{
	if (pThis->IsCharged)
	{
		// the only thing we do differently is to remember which
		// SW has been fired here. all needed changes are done
		// by hooks.
		if (SWTypeExt::ExtData* pData = SWTypeExt::ExtMap.Find(pThis->Type))
		{
			auto duration = pData->Weather_Duration.Get(RulesClass::Instance->LightningStormDuration);
			auto deferment = pData->SW_Deferment.Get(RulesClass::Instance->LightningDeferment);
			CurrentLightningStorm = pThis;
			LightningStorm::Start(duration, deferment, Coords, pThis->Owner);

			return true;
		}
	}
	return false;
}

bool SW_LightningStorm::AbortFire(SuperClass* pSW, bool IsPlayer)
{
	SWTypeExt::ExtData* pData = SWTypeExt::ExtMap.Find(pSW->Type);

	// only one Lightning Storm allowed
	if (LightningStorm::Active || LightningStorm::HasDeferment())
	{
		if (IsPlayer)
		{
			pData->PrintMessage(pData->Message_Abort, pSW->Owner);
		}
		return true;
	}
	return false;
}

void SW_LightningStorm::Initialize(SWTypeExt::ExtData* pData)
{ 
	// Defaults to Lightning Storm values
	pData->Weather_DebrisMin = 2;
	pData->Weather_DebrisMax = 4;
	pData->Weather_IgnoreLightningRod = false;
	pData->Weather_ScatterCount = 1;

	pData->Weather_RadarOutageAffects = AffectedHouse::Enemies;

	pData->EVA_Detected = VoxClass::FindIndexById(GameStrings::EVA_WeatherDeviceReady);
	pData->EVA_Ready = VoxClass::FindIndexById(GameStrings::EVA_LightningStormReady);
	pData->EVA_Activated = VoxClass::FindIndexById(GameStrings::EVA_LightningStormCreated);

	pData->Message_Launch = GameStrings::TXT_LIGHTNING_STORM_APPROACHING();
	pData->Message_Activate = GameStrings::TXT_LIGHTNING_STORM();
	pData->Message_Abort = GameStrings::LightningStormActive_msg();

	pData->SW_AITargetingMode = SuperWeaponAITargetingMode::LightningStorm;
	pData->CursorType = int(MouseCursorType::LightningStorm);

}

void SW_LightningStorm::LoadFromINI(SWTypeExt::ExtData* pData, CCINIClass* pINI)
{ 
	const char* section = pData->get_ID();

	INI_EX exINI(pINI);
	pData->Weather_Duration.Read(exINI, section, "Lightning.Duration");
	pData->Weather_RadarOutage.Read(exINI, section, "Lightning.RadarOutage");
	pData->Weather_HitDelay.Read(exINI, section, "Lightning.HitDelay");
	pData->Weather_ScatterDelay.Read(exINI, section, "Lightning.ScatterDelay");
	pData->Weather_ScatterCount.Read(exINI, section, "Lightning.ScatterCount");
	pData->Weather_Separation.Read(exINI, section, "Lightning.Separation");
	pData->Weather_PrintText.Read(exINI, section, "Lightning.PrintText");
	pData->Weather_IgnoreLightningRod.Read(exINI, section, "Lightning.IgnoreLightningRod");
	pData->Weather_DebrisMin.Read(exINI, section, "Lightning.DebrisMin");
	pData->Weather_DebrisMax.Read(exINI, section, "Lightning.DebrisMax");
	pData->Weather_CloudHeight.Read(exINI, section, "Lightning.CloudHeight");
	pData->Weather_BoltExplosion.Read(exINI, section, "Lightning.BoltExplosion");
	pData->Weather_RadarOutageAffects.Read(exINI, section, "Lightning.RadarOutageAffects");
	pData->Weather_Clouds.Read(exINI, section, "Lightning.Clouds");
	pData->Weather_Bolts.Read(exINI, section, "Lightning.Bolts");
	pData->Weather_Debris.Read(exINI, section, "Lightning.Debris");
	pData->Weather_Sounds.Read(exINI, section, "Lightning.Sounds");
}

WarheadTypeClass* SW_LightningStorm::GetWarhead(const SWTypeExt::ExtData* pData) const
{
	return pData->SW_Warhead.Get(RulesClass::Instance->LightningWarhead);
}

int SW_LightningStorm::GetDamage(const SWTypeExt::ExtData* pData) const
{
	return pData->SW_Damage.Get(RulesClass::Instance->LightningDamage);
}

SWRange SW_LightningStorm::GetRange(const SWTypeExt::ExtData* pData) const
{
	return pData->SW_Range->empty() ? SWRange(RulesClass::Instance->LightningCellSpread) : pData->SW_Range;
}