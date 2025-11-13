#include "Body.h"

#include <Utilities/Macro.h>

#include <Ext/TechnoType/Body.h>
#include <Ext/WarheadType/Body.h>

WeaponStruct* FakeInfantryClass::_GetDeployWeapon()
{
	int deployFireWeapon = this->Type->DeployFireWeapon;
	int weaponIndex = deployFireWeapon == -1 ? this->SelectWeapon(this->Target) : deployFireWeapon;
	return this->GetWeapon(weaponIndex);
}

#include <Ext/Anim/Body.h>

// Handle infantry that survives damage
void HandleInfantryDamaged(FakeInfantryClass* pThis, TechnoClass* source, HouseClass* sourceHouse) {

	if (!pThis->Owner->IsControlledByHuman()
		&& pThis->Type->Engineer
		&& (pThis->GetCurrentMission() == Mission::Guard
		|| pThis->GetCurrentMission() == Mission::Area_Guard))
	{
		pThis->QueueMission(Mission::Hunt, false);
	}

	CoordStruct target_ = source ? source->Location : CoordStruct::Empty;
	pThis->Scatter(target_, false, false);

	if (source && pThis->PanicDurationLeft < 100)
	{

		if (pThis->Type->Fraidycat)
		{
			pThis->PanicDurationLeft = 300;
		}
		else if (!pThis->Type->Fearless && !pThis->HasAbility(AbilityType::Fearless))
		{
			int PanixMax = 100;
			if (pThis->Type->Doggie && pThis->IsRedHP())
			{
				PanixMax = RulesExtData::Instance()->DoggiePanicMax;
			}

			pThis->PanicDurationLeft = PanixMax;
		}
	}
	else if (!pThis->Type->Fearless && !pThis->HasAbility(AbilityType::Fearless))
	{

		int morefear = 50;
		auto _HPPercent = pThis->GetHealthPercentage();

		if (_HPPercent > RulesClass::Instance->ConditionRed)
		{
			morefear = 25;
		}

		if (_HPPercent > RulesClass::Instance->ConditionYellow)
		{
			morefear /= 2;
		}

		pThis->PanicDurationLeft = MinImpl(300, pThis->PanicDurationLeft + morefear);
	}
}

void FinalizeInfantryDeath(FakeInfantryClass* pThis, TechnoClass* pKiller) {
	if (pThis->Type->Crashable && pThis->Crash(pKiller))
		return

		pThis->UnInit();
}

void ProcessStandardDeathType(FakeInfantryClass* pThis, WarheadTypeClass* warhead,
	TechnoClass* source, HouseClass* sourceHouse, bool isCyborgDeath , InfDeath infDeath) {
	bool Succeeded = false;
	auto pWarheadExt = WarheadTypeExtContainer::Instance.Find(warhead);

	if(pThis->Type->NotHuman){
		if (auto pDeathAnim = pWarheadExt->NotHuman_DeathAnim.Get(nullptr))
		{
			auto pAnim = GameCreate<AnimClass>(pDeathAnim, pThis->Location);
			auto pInvoker = source ? source->GetOwningHouse() : nullptr;
			AnimExtData::SetAnimOwnerHouseKind(pAnim, pInvoker, pThis->GetOwningHouse(), source, true, true);
			pAnim->ZAdjust = pThis->GetZAdjustment();
			Succeeded = true;
		}
		else
		{
			auto const& whSequence = pWarheadExt->NotHuman_DeathSequence;
			// Die1-Die5 sequences are offset by 10
			COMPILETIMEEVAL auto Die = [](int x) { return x + 10; };

			int resultSequence = Die(1);

			if (!whSequence.isset()
				&& TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType())->NotHuman_RandomDeathSequence.Get())
			{
				resultSequence = ScenarioClass::Instance->Random.RandomRanged(Die(1), Die(5));
			}
			else if (whSequence.isset())
			{
				resultSequence = std::clamp(Die(Math::abs(whSequence.Get())), Die(1), Die(5));
			}

			pThis->_GetExtData()->IsUsingDeathSequence = true;

			//BugFix : when the sequence not declared , it keep the infantry alive ! , wtf WW ?!
			Succeeded = pThis->PlayAnim(static_cast<DoType>(resultSequence), true);
		}

		if (Succeeded)
		{
			if (infDeath == InfDeath::Virus)
			{
				auto pAnim = GameCreate<AnimClass>(RulesClass::Instance->InfantryVirus, pThis->Location, 0, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200, 0, 0);
				auto pInvoker = source
					? source->Owner
					: sourceHouse;

				AnimExtData::SetAnimOwnerHouseKind(pAnim, pInvoker, pThis->Owner, source, true, false);

				if (ParticleSystemClass::Array->valid_index(RulesClass::Instance->InfantryVirus->SpawnsParticle))
				{
					auto pParticleType = ParticleTypeClass::Array->Items[RulesClass::Instance->InfantryVirus->SpawnsParticle];
					ParticleSystemClass::Instance->SpawnParticle(pParticleType, &pThis->Location);
				}
			}

			if (!isCyborgDeath) {
				return; // Don't finalize - cyborg stays as debris
			}
		}
		else //infdeath fail
		{
			pThis->UnInit();
			return;
		}
	} else {
		AnimTypeClass* pTypeAnim = pWarheadExt->InfDeathAnim;
		for (auto begin = pWarheadExt->InfDeathAnims.begin(); begin != pWarheadExt->InfDeathAnims.end(); ++begin)
		{
			if (begin->first == pThis->Type)
			{
				pTypeAnim = begin->second;
				break;
			}
		}

		if (pTypeAnim)
		{
			auto pAnim = GameCreate<AnimClass>(pTypeAnim, pThis->Location);
			HouseClass* const Invoker = (source)
				? source->Owner
				: sourceHouse
				;

			AnimExtData::SetAnimOwnerHouseKind(pAnim, Invoker, pThis->Owner, source, false, true);
		}
		else
		{
			AnimClass* pAnim = nullptr;
			switch (infDeath)
			{
			case InfDeath::Die1:
				if (pThis->PlayAnim(DoType::Die1, true, false))
				{
					if (!isCyborgDeath) {
						return;
					}
				}

				break;
			case InfDeath::Die2:
				if (pThis->PlayAnim(DoType::Die2, true, false))
				{
					if (!isCyborgDeath) {
						return;
					}
				}

				break;
			case InfDeath::Explode:
				pAnim = GameCreate<AnimClass>(RulesClass::Instance->InfantryExplode, pThis->Location, 0, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200, 0, 0);
				break;
			case InfDeath::Flames:
				pAnim = GameCreate<AnimClass>(RulesClass::Instance->FlamingInfantry, pThis->Location, 0, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200, 0, 0);
				break;
			case InfDeath::Electro:
			{
				AnimTypeClass* El = RulesExtData::Instance()->ElectricDeath;

				if (!El)
				{
					El = AnimTypeClass::Array->Items[0];
				}

				pAnim = GameCreate<AnimClass>(El, pThis->Location, 0, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200, 0, 0);

			}
			break;
			case InfDeath::HeadPop:
				pAnim = GameCreate<AnimClass>(RulesClass::Instance->InfantryHeadPop, pThis->Location, 0, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200, 0, 0);
				break;
			case InfDeath::Nuked:
				pAnim = GameCreate<AnimClass>(RulesClass::Instance->InfantryNuked, pThis->Location, 0, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200, 0, 0);
				break;
			case InfDeath::Virus:
			{
				pAnim = GameCreate<AnimClass>(RulesClass::Instance->InfantryVirus, pThis->Location, 0, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200, 0, 0);
			}
			break;
			case InfDeath::Mutate:
			{
				auto curLoc = pThis->Location;
				pThis->UnmarkAllOccupationBits(curLoc);
				auto pCell = MapClass::Instance->GetCellAt(CellClass::Coord2Cell(curLoc));
				const bool Hasbuilding = pCell->GetBuilding();

				if (GroundType::Get(pCell->LandType)->Cost[0] == 0.0 && !pThis->OnBridge)
				{
					bool fail = false;
					if (!pThis->PlayAnim(DoType::Die2, true, false)) {
						pThis->UnInit();//fail
						fail = true;
					}

					if (!isCyborgDeath || fail) {
						return;
					}
					else break;
				}

				CoordStruct closest{};
				pCell->FindInfantrySubposition(&closest, curLoc, false, false, false);

				if (!closest.IsValid())
				{
					bool fail = false;
					if (!pThis->PlayAnim(DoType::Die2, true, false)) {
						pThis->UnInit();//fail
						fail = true;
					}

					if (!isCyborgDeath || fail) {
						return;
					}
					else break;
				}

				if (Hasbuilding)
				{
					bool fail = false;

					if (!pThis->PlayAnim(DoType::Die2, true, false)) {
						pThis->UnInit();//fail
						fail = true;
					}

					if (!isCyborgDeath || fail) {
						return;
					}
					else break;

				}

				pThis->MarkAllOccupationBits(curLoc);
				auto pAnim_Mutate = GameCreate<AnimClass>(RulesClass::Instance->InfantryMutate, pThis->Location, 0, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200, 0, 0);

				auto pInvoker = source
					? source->Owner
					: sourceHouse;

				AnimExtData::SetAnimOwnerHouseKind(pAnim_Mutate, pInvoker, pThis->Owner, source, true, false);

				pAnim_Mutate->MarkAllOccupationBits(pThis->Location);
			}
			break;
			case InfDeath::Brute:
				pAnim = GameCreate<AnimClass>(RulesClass::Instance->InfantryBrute, pThis->Location, 0, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200, 0, 0);
				break;
			default:
				break;
			}

			if (pAnim)
			{
				auto pInvoker = source
					? source->Owner
					: sourceHouse;

				const auto& [bChanged, result] = AnimExtData::SetAnimOwnerHouseKind(pAnim, pInvoker, pThis->Owner, source, true, false);

				if (infDeath == InfDeath::Mutate && bChanged && result != OwnerHouseKind::Default)
				{
					pAnim->LightConvert = nullptr;
				}
			}
		}

	}


	FinalizeInfantryDeath(pThis, source);
}

void ProcessInfantryDeathAnimation(FakeInfantryClass* pThis, WarheadTypeClass* warhead,
	TechnoClass* source, HouseClass* sourceHouse, bool isCyborgDeath) {

	CoordStruct location = pThis->Location;

	// Check if near ground and in water
	if (pThis->GetHeight() <= 10)
	{
		CellClass* cell = MapClass::Instance->GetCellAt(location);

		if (cell->LandType == LandType::Water) // Water terrain
		{
			if (pThis->IsABomb)
			{
				// Water splash effects
				GameCreate<AnimClass>(RulesClass::Instance->Wake, pThis->Location, 0, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200, 0, 0);

				auto splash_loc = pThis->Location + CoordStruct{ 0, 0, 3 };
				GameCreate<AnimClass>(RulesClass::Instance->SplashList[0], splash_loc, 0, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200, 0, 0);


				FinalizeInfantryDeath(pThis , source);
				return;
			}
		}
	}

	// Cyborg prone death
	if (pThis->Type->Cyborg && pThis->Crawling)
	{
		GameCreate<AnimClass>(RulesClass::Instance->InfantryExplode, pThis->Location, 0, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200, 0, 0);
		FinalizeInfantryDeath(pThis, source);
		return;
	}

	// Jump jet units explode
	if (pThis->Type->JumpJet)
	{
		GameCreate<AnimClass>(RulesClass::Instance->InfantryExplode, pThis->Location, 0, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200, 0, 0);
		FinalizeInfantryDeath(pThis, source);
		return;
	}

	auto infDeath = warhead->InfDeath;

	if (pThis->SequenceAnim == DoType::Paradrop)
	{
		if (infDeath == InfDeath::Virus)
		{
			auto pAnim = GameCreate<AnimClass>(RulesClass::Instance->InfantryVirus, pThis->Location, 0, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200, 0, 0);
			auto pInvoker = source
				? source->Owner
				: sourceHouse;

			AnimExtData::SetAnimOwnerHouseKind(pAnim, pInvoker, pThis->Owner, source, true, false);

			if (ParticleSystemClass::Array->valid_index(RulesClass::Instance->InfantryVirus->SpawnsParticle))
			{
				auto pParticleType = ParticleTypeClass::Array->Items[RulesClass::Instance->InfantryVirus->SpawnsParticle];
				ParticleSystemClass::Instance->SpawnParticle(pParticleType, &pThis->Location);
			}
		}

		infDeath = InfDeath::Explode;
	}

	if (auto pAttackerBld = cast_to<BuildingClass*>(source))
	{
		if (pAttackerBld->Type->LaserFence)
		{
			infDeath = InfDeath::Electro;
		}
	}

	// Custom death animations from InfantryType
	if (pThis->Type->DeathAnims.Count > 0)
	{
		int infDeathInt = (int)infDeath;

		if (infDeathInt < 0
			|| infDeathInt >= pThis->Type->DeathAnims.Count
			|| !pThis->Type->DeathAnims[infDeathInt])
		{
			infDeathInt = 0;
		}

		if (auto pDeathAnim = pThis->Type->DeathAnims[infDeathInt])
		{
			auto pAnim = GameCreate<AnimClass>(pDeathAnim, pThis->Location, 0, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200, 0, 0);

			HouseClass* Invoker = (source)
				? source->Owner
				: sourceHouse
				;

			AnimExtData::SetAnimOwnerHouseKind(pAnim, Invoker, pThis->Owner, source, false, true);

		}

		FinalizeInfantryDeath(pThis, source);
		return;
	}

	// Process standard death types
	ProcessStandardDeathType(pThis, warhead, source, sourceHouse, isCyborgDeath, infDeath);
}

#include <SlaveManagerClass.h>

void HandleInfantryDeath(FakeInfantryClass* pThis, WarheadTypeClass* warhead,
	TechnoClass* source, HouseClass* sourceHouse, bool forced) {

	// Handle slave manager cleanup
	if (auto pEnslave = pThis->SlaveOwner)
	{
		if (auto pManager = pEnslave->SlaveManager)
		{
			pManager->Killed(pThis);
		}
	}

	// Handle gunner transport

	auto MyTransport = pThis->Transporter;

	if (MyTransport
		&& MyTransport->WhatAmI() == UnitClass::AbsID
		&& pThis->Type->Gunner)
	{
		static_cast<UnitClass*>(MyTransport)->RemovePassenger(pThis);
	}

	// Death announcement and mission cleanup
	pThis->Destroyed(source);
	pThis->StopMoving();
	pThis->Stun();
	pThis->QueueMission(Mission::None, false);
	pThis->QueueMission(Mission::Guard, false);
	pThis->NextMission();
	pThis->KillPassengers(source);

	if (!pThis->_GetExtData()->GarrisonedIn) {

		// Check if special cyborg death
		bool isCyborgDeath = false;
		if (forced && pThis->Type->Cyborg) {
			isCyborgDeath = true;

			// Cyborg bomb explosion
			if (pThis->IsABomb) {
				GameCreate<AnimClass>(RulesClass::Instance->InfantryExplode, pThis->Location, 0, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200, 0, 0);
			}
		}

		// Handle death animations based on location and type
		ProcessInfantryDeathAnimation(pThis, warhead, source, sourceHouse, isCyborgDeath);
	}
}

DamageState FakeInfantryClass::_Take_Damage(int* damage, int distance, WarheadTypeClass* warhead, TechnoClass* source, bool ignoreDefenses, bool PreventsPassengerEscape, HouseClass* sourceHouse) {

	if (!warhead)
		return DamageState::Unaffected;

	auto pWarheadExt = WarheadTypeExtContainer::Instance.Find(warhead);

	// Apply prone damage modifier
	if (this->IsDeployed() && ignoreDefenses) {
		*damage = static_cast<int>(*damage * pWarheadExt->DeployedDamage.Get(this));
	}
	else if (this->Crawling && *damage > 0 && !ignoreDefenses) {
		*damage = (int)MaxImpl(((double)*damage * warhead->ProneDamage), 1);
	}


	// Airborne units immune to certain death types (electrocution)
	if (warhead->InfDeath == InfDeath::Mutate && this->GetHeight() > 0) {
		*damage = 0;
	}

	// Call parent damage handler
	DamageState result = this->FootClass::ReceiveDamage(
		damage,
		distance,
		warhead,
		source,
		ignoreDefenses,
		ignoreDefenses,
		sourceHouse
	);

	if (result == DamageState::PostMortem || result == DamageState::Unaffected)
		return result;

	if (result == DamageState::NowDead) {
		HandleInfantryDeath(this, warhead, source, sourceHouse, ignoreDefenses);
		return result;
	}

	HandleInfantryDamaged(this, source, sourceHouse);
	return result;
}

int FakeInfantryClass::_SelectWeaponAgainst(AbstractClass* pTarget)
{
	auto pThisType = this->Type;
	int wp = -1;
	if (!pThisType->DeployFire || pThisType->DeployFireWeapon == -1)
	{
		return this->TechnoClass::SelectWeapon(pTarget);
	}

	if ((pThisType->IsGattling || TechnoTypeExtContainer::Instance.Find(pThisType)->MultiWeapon.Get()) && !this->IsDeployed())
	{
		return pThisType->DeployFireWeapon;
	}

	if (!this->InOpenToppedTransport || pThisType->OpenTransportWeapon == -1)
	{
		wp = 0;
	}
	else
	{
		wp = pThisType->OpenTransportWeapon;
	}

	return wp;
}

// =============================
// load / save

template <typename T>
void InfantryExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->IsUsingDeathSequence)
		.Process(this->CurrentDoType)
		.Process(this->SkipTargetChangeResetSequence)
		.Process(this->GarrisonedIn)
		;
}

// =============================
// container

InfantryExtContainer InfantryExtContainer::Instance;
std::vector<InfantryExtData*> Container<InfantryExtData>::Array;

void Container<InfantryExtData>::Clear()
{
	Array.clear();
}

bool InfantryExtContainer::LoadGlobals(PhobosStreamReader& Stm)
{
	return LoadGlobalArrayData(Stm);
}

bool InfantryExtContainer::SaveGlobals(PhobosStreamWriter& Stm)
{
	return SaveGlobalArrayData(Stm);
}

// =============================
// container hooks

//has NoInit constructor
ASMJIT_PATCH(0x517AEB, InfantryClass_CTOR, 0x5)
{
	GET(InfantryClass*, pItem, ESI);

	if(pItem->Type)
		InfantryExtContainer::Instance.Allocate(pItem);

	return 0;
}

ASMJIT_PATCH(0x517F83, InfantryClass_DTOR, 0x6)
{
	GET(InfantryClass* const, pItem, ESI);
	InfantryExtContainer::Instance.Remove(pItem);
	return 0;
}

void FakeInfantryClass::_Detach(AbstractClass* target, bool all)
{
	InfantryExtContainer::Instance.InvalidatePointerFor(this, target, all);
	this->InfantryClass::PointerExpired(target, all);
}

DEFINE_FUNCTION_JUMP(VTABLE, 0x7EB080, FakeInfantryClass::_Detach)

HRESULT __stdcall FakeInfantryClass::_Load(IStream* pStm)
{
	HRESULT hr = this->InfantryClass::Load(pStm);
	if (SUCCEEDED(hr))
		hr = InfantryExtContainer::Instance.LoadKey(this, pStm);

	return hr;
}

HRESULT __stdcall FakeInfantryClass::_Save(IStream* pStm, BOOL clearDirty)
{
	HRESULT hr = this->InfantryClass::Save(pStm, clearDirty);
	if (SUCCEEDED(hr))
		hr = InfantryExtContainer::Instance.SaveKey(this, pStm);

	return hr;
}

// DEFINE_FUNCTION_JUMP(VTABLE, 0x7EB06C, FakeInfantryClass::_Load)
// DEFINE_FUNCTION_JUMP(VTABLE, 0x7EB070, FakeInfantryClass::_Save)

ASMJIT_PATCH(0x51A002, InfantryClass_UpdatePosition_InfiltrateBuilding, 0x6)
{
	GET(InfantryClass*, pThis, ESI);
	GET(BuildingClass*, pBuilding, EDI);

	if (const auto pTag = pBuilding->AttachedTag)
		pTag->RaiseEvent(TriggerEvent::SpiedBy, pThis, CellStruct::Empty);

	if (const auto pTag = pBuilding->AttachedTag)
		pTag->RaiseEvent(TriggerEvent::SpyAsHouse, pThis, CellStruct::Empty);

	if (const auto pTag = pBuilding->AttachedTag)
		pTag->RaiseEvent(TriggerEvent::SpyAsInfantry, pThis, CellStruct::Empty);

	return 0;
}