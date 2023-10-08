#include "PrismForwardingData.h"

signed int PrismForwardingData::GetUnusedWeaponSlot(BuildingTypeClass* pThis, bool elite)
{
	for (auto idxWeapon = 2u; idxWeapon < 13u; ++idxWeapon)
	{ //13-18 is AlternateFLH0-4
		auto Weapon = (elite ? pThis->GetEliteWeapon(idxWeapon) : pThis->GetWeapon(idxWeapon))->WeaponType;

		if (!Weapon) {
			return static_cast<int>(idxWeapon);
		}
	}
	return -1;
}

void PrismForwardingData::Initialize(BuildingTypeClass* pThis)
{
	this->Enabled = EnabledState::No;
	if (pThis == RulesClass::Instance->PrismType)
	{
		this->Enabled = EnabledState::Yes;
	}
	this->Targets.push_back(pThis);
}

void PrismForwardingData::LoadFromINIFile(BuildingTypeClass* pThis, CCINIClass* pINI)
{
	const char* pID = pThis->ID;
	if (pINI->ReadString(pID, "PrismForwarding", "", Phobos::readBuffer) > 0)
	{
		if ((IS_SAME_STR_(Phobos::readBuffer, "yes"))
			|| (IS_SAME_STR_(Phobos::readBuffer, "true")))
		{
			this->Enabled = EnabledState::Yes;
		}
		else if (IS_SAME_STR_(Phobos::readBuffer, "forward"))
		{
			this->Enabled = EnabledState::Forward;
		}
		else if (IS_SAME_STR_(Phobos::readBuffer, "attack"))
		{
			this->Enabled = EnabledState::Attack;
		}
		else if ((IS_SAME_STR_(Phobos::readBuffer, "no"))
			|| (IS_SAME_STR_(Phobos::readBuffer, "false")))
		{
			this->Enabled = EnabledState::No;
		}
	}

	if (this->Enabled != EnabledState::No)
	{
		INI_EX exINI(pINI);

		this->Targets.Read(exINI, pID, "PrismForwarding.Targets");
		this->MaxFeeds.Read(exINI, pID, "PrismForwarding.MaxFeeds");
		this->MaxChainLength.Read(exINI, pID, "PrismForwarding.MaxChainLength");
		this->MaxNetworkSize.Read(exINI, pID, "PrismForwarding.MaxNetworkSize");
		this->SupportModifier.Read(exINI, pID, "PrismForwarding.SupportModifier");
		this->DamageAdd.Read(exINI, pID, "PrismForwarding.DamageAdd");
		this->ToAllies.Read(exINI, pID, "PrismForwarding.ToAllies");
		this->MyHeight.Read(exINI, pID, "PrismForwarding.MyHeight");
		this->BreakSupport.Read(exINI, pID, "PrismForwarding.BreakSupport");
		this->Intensity.Read(exINI, pID, "PrismForwarding.Intensity");
		this->ChargeDelay.Read(exINI, pID, "PrismForwarding.ChargeDelay");

		if (this->ChargeDelay < 1)
		{
			Debug::Log("[Developer Error] %s has an invalid PrismForwarding.ChargeDelay (%d), overriding to 1.\n", pThis->ID, this->ChargeDelay.Get());
			this->ChargeDelay = 1;
		}

		auto SuperWH = RulesClass::Instance->C4Warhead;
		if (!SuperWH)
		{
			SuperWH = WarheadTypeClass::Find("Super");
		}

		auto const ReadSupportWeapon = [=, &exINI]
		(Valueable<int>& pSetting, const char* const pKey, bool const elite)
			{
				if (exINI.ReadString(pID, pKey) > 0)
				{
					if (auto const pWeapon = WeaponTypeClass::FindOrAllocate(exINI.value()))
					{
						auto const idxWeapon = pSetting != -1
							? pSetting : this->GetUnusedWeaponSlot(pThis, elite);
						if (idxWeapon == -1)
						{
							Debug::FatalErrorAndExit(
								"BuildingType [%s] is a Prism Tower however there "
								"are no free\nweapon slots to assign the %ssupport "
								"weapon to.", pThis->ID, elite ? "elite " : "");
						}

						pSetting = idxWeapon;

						if (!pWeapon->Warhead)
						{
							pWeapon->Warhead = SuperWH;
						}
						pWeapon->NeverUse = true; //the modder shouldn't be expected to have to set this

						auto Weapon = (elite ? pThis->GetEliteWeapon(idxWeapon) : pThis->GetWeapon(idxWeapon));
						Weapon->WeaponType = pWeapon;

						//now get the FLH
						auto supportFLH = &pThis->Weapon[13 + elite].FLH; //AlternateFLH0 or 1
						if (*supportFLH == CoordStruct::Empty)
						{
							//assuming that, for Prism Towers, this means the FLH was not set.
							supportFLH = &(elite ? pThis->GetEliteWeapon(0) : pThis->GetWeapon(0))->FLH; //[Elite]Primary
						}
						Weapon->FLH = *supportFLH;
					}
				}
			};

		ReadSupportWeapon(this->SupportWeaponIndex, "PrismForwarding.SupportWeapon", false);
		ReadSupportWeapon(this->EliteSupportWeaponIndex, "PrismForwarding.EliteSupportWeapon", true);
	}
}

bool PrismForwardingData::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	return Stm
		.Process(this->Enabled)
		.Process(this->Targets, RegisterForChange)
		.Process(this->MaxFeeds)
		.Process(this->MaxChainLength)
		.Process(this->MaxNetworkSize)
		.Process(this->SupportModifier)
		.Process(this->DamageAdd)
		.Process(this->MyHeight)
		.Process(this->Intensity)
		.Process(this->ChargeDelay)
		.Process(this->ToAllies)
		.Process(this->BreakSupport)
		.Process(this->SupportWeaponIndex)
		.Process(this->EliteSupportWeaponIndex)
		.Success() && Stm.RegisterChange(this)
		;
}

bool PrismForwardingData::Save(PhobosStreamWriter& Stm) const
{
	return Stm
		.Process(this->Enabled)
		.Process(this->Targets)
		.Process(this->MaxFeeds)
		.Process(this->MaxChainLength)
		.Process(this->MaxNetworkSize)
		.Process(this->SupportModifier)
		.Process(this->DamageAdd)
		.Process(this->MyHeight)
		.Process(this->Intensity)
		.Process(this->ChargeDelay)
		.Process(this->ToAllies)
		.Process(this->BreakSupport)
		.Process(this->SupportWeaponIndex)
		.Process(this->EliteSupportWeaponIndex)
		.Success() && Stm.RegisterChange(this)
		;
}