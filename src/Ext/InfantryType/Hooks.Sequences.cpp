#include "Body.h"

#include <Ext/Infantry/Body.h>

// replacing reference of 7EAF7C
// Replacing DoControls* with own
// replace the name 0x8255C8u

#ifndef AdditionalSequence
static COMPILETIMEEVAL const char* Sequences_ident[] = {
		"Ready",
		"Guard",
		"Prone",
		"Walk",
		"FireUp",
		"Down",
		"Crawl",
		"Up",
		"FireProne",
		"Idle1",
		"Idle2",
		"Die1",
		"Die2",
		"Die3",
		"Die4",
		"Die5",
		"Tread",
		"Swim",
		"WetIdle1",
		"WetIdle2",
		"WetDie1",
		"WetDie2",
		"WetAttack",
		"Hover",
		"Fly",
		"Tumble",
		"FireFly",
		"Deploy",
		"Deployed",
		"DeployedFire",
		"DeployedIdle",
		"Undeploy",
		"Cheer",
		"Paradrop",
		"AirDeathStart",
		"AirDeathFalling",
		"AirDeathFinish",
		"Panic",
		"Shovel",
		"Carry",
		"SecondaryFire",
		"SecondaryProne",
		"SecondaryFireFly",
		"SecondaryWetAttack"
};

static COMPILETIMEEVAL std::array<DoStruct, std::size(Sequences_ident)> Sequences_Master = { {
	{1, 0, 0, 0},
	{1, 0, 0, 0},
	{1, 0, 0, 6},
	{1, 1, 1, 3},
	{1, 0, 0, 1},
	{0, 1, 0, 1},
	{1, 1, 1, 1},
	{0, 0, 0, 1},
	{1, 0, 0, 1},
	{1, 0, 0, 3},
	{1, 0, 0, 3},
	{0, 0, 0, 1},
	{0, 0, 0, 1},
	{0, 0, 0, 1},
	{0, 0, 0, 1},
	{0, 0, 0, 1},
	{1, 0, 0, 3},
	{1, 1, 1, 1},
	{1, 0, 0, 3},
	{1, 0, 0, 3},
	{0, 0, 0, 1},
	{0, 0, 0, 1},
	{1, 0, 0, 1},
	{1, 1, 0, 2},
	{1, 1, 0, 1},
	{1, 1, 0, 1},
	{1, 1, 0, 1},
	{0, 0, 0, 1},
	{1, 0, 0, 1},
	{1, 0, 0, 1},
	{1, 0, 0, 1},
	{0, 0, 0, 1},
	{0, 0, 0, 3},
	{1, 0, 0, 1},
	{0, 0, 0, 3},
	{0, 0, 0, 1},
	{0, 0, 0, 3},
	{1, 1, 1, 4},
	{1, 0, 0, 6},
	{1, 1, 1, 3},
	{1, 0, 0, 1},
	{1, 0, 0, 1},
	//{16, 0, 0, 0}, //leftover ?
	{1, 1, 0, 1}, //SecondaryFireFly
	{1, 1, 0, 1}, //SecondaryWetAttack
}
};

struct NewDoType
{
	COMPILETIMEEVAL void FORCEDINLINE Initialize()
	{
		for (auto at = this->begin(); at != this->end(); ++at)
		{
			at->StartFrame = 0;
			at->CountFrames = 0;
			at->FacingMultiplier = 0;
			at->Facing = DoTypeFacing::None;
			at->SoundCount = 0;
			at->SoundData[0].Index = -1;
			at->SoundData[0].StartFrame = 0;
			at->SoundData[1].Index = -1;
			at->SoundData[1].StartFrame = 0;
		}
	}

	COMPILETIMEEVAL FORCEDINLINE static const char* GetSequenceName(DoType sequence)
	{
		return Sequences_ident[(int)sequence];
	}

	COMPILETIMEEVAL FORCEDINLINE DoInfoStruct GetSequence(DoType sequence) const
	{
		return this->Data[(int)sequence];
	}

	COMPILETIMEEVAL FORCEDINLINE DoInfoStruct& GetSequence(DoType sequence)
	{
		return this->Data[(int)sequence];
	}

	COMPILETIMEEVAL FORCEDINLINE static DoStruct* GetSequenceData(DoType sequence)
	{
		return const_cast<DoStruct*>(&Sequences_Master[(int)sequence]);
	}

	COMPILETIMEEVAL FORCEDINLINE  DoInfoStruct* begin() { return std::begin(Data); }
	COMPILETIMEEVAL FORCEDINLINE  DoInfoStruct* end() { return std::end(Data); }

	DoInfoStruct Data[std::size(Sequences_ident)];
};

DEFINE_HOOK(0x523876, InfantryTypeClass_CTOR_Initialize, 6)
{
	GET(InfantryTypeClass*, pItem, ESI);

	pItem->ArrayIndex = R->ECX<int>();
	pItem->OccupyWeapon.FLH.X = 0;
	pItem->OccupyWeapon.FLH.Y = 0;
	pItem->OccupyWeapon.FLH.Z = 0;
	pItem->OccupyWeapon.WeaponType = 0;
	pItem->OccupyWeapon.BarrelLength = 0;
	pItem->OccupyWeapon.BarrelThickness = 0;
	pItem->OccupyWeapon.TurretLocked = 0;
	pItem->EliteOccupyWeapon.WeaponType = 0;
	pItem->EliteOccupyWeapon.BarrelLength = 0;
	pItem->EliteOccupyWeapon.FLH.X = 0;
	pItem->EliteOccupyWeapon.FLH.Y = 0;
	pItem->EliteOccupyWeapon.FLH.Z = 0;
	pItem->EliteOccupyWeapon.BarrelThickness = 0;
	pItem->EliteOccupyWeapon.TurretLocked = 0;
	pItem->RotCount = 8;
	pItem->RadarVisible = 0;
	pItem->Crushable = 1;
	pItem->Repairable = 0;
	pItem->Crewed = 0;
	pItem->ImmuneToPsionics = 0;
	pItem->ImmuneToPsionicWeapons = 0;
	pItem->ImmuneToPoison = 0;
	pItem->Parasiteable = 1;
	pItem->Organic = 1;
	pItem->ConsideredAircraft = 0;
	pItem->Bunkerable = 0;

	pItem->Sequence = (DoControls*)GameCreate<NewDoType>();
	pItem->Sequence->Initialize();

	if (auto pExt = InfantryTypeExtContainer::Instance.Allocate(pItem))
		pExt->Type = TechnoTypeExtContainer::Instance.Find(pItem);

	return 0x523970;
}

DEFINE_HOOK(0x520820, InfantryClass_FiringAI_SecondaryFireFly, 0x5)
{
	GET(InfantryClass*, pThis, EBP);
	GET_STACK(int, weaponIdx, 0x34 - 0x24);

	DoType result = weaponIdx == 0 ? DoType::FireFly :
		DoType(42);

	if (!((NewDoType*)pThis->Type->Sequence)->GetSequence(result).CountFrames)
		return 0;

	pThis->PlayAnim(result);
	return 0x520831;
}

// Fixes SecondaryFire / SecondaryProne sequences not remapping to WetAttack in water.
// Ideally there would be WetAttackSecondary but adding new sequences would be a big undertaking.
// Also adds a toggle for not using water sequences at all - Starkku
// Also add new sequence - Otamaa
DEFINE_HOOK(0x51D7E0, InfantryClass_DoAction_SecondaryWetAttack, 0x5)
{
	GET(FakeInfantryClass*, pThis, ESI);
	GET(DoType, type, EDI);

	enum
	{
		Continue = 0x0,
		SkipWaterSequences = 0x51D842,
		UseSwim = 0x51D83D,
		UseWetAttack = 0x51D82F,
		ApplySequence = 0x51D842
	};

	if (pThis->_GetTypeExtData()->OnlyUseLandSequences)
	{
		R->EBP(false);
		return SkipWaterSequences;
	}

	if (type == DoType::Walk || type == DoType::Crawl) // Restore overridden instructions.
	{
		R->EBP(false);
		return UseSwim;
	}

	if (type == DoType::SecondaryFire || type == DoType::SecondaryProne)
	{
		R->EBP(false);

		if (!((NewDoType*)pThis->Type->Sequence)->GetSequence(DoType(43)).CountFrames)
		{
			return UseWetAttack;
		}

		R->EDI(DoType(43));
		return ApplySequence;
	}

	return Continue;
}

//the fuck is this ,...
//DEFINE_PATCH_TYPED(BYTE, 0x51DAA9, 6u);
//DEFINE_PATCH_TYPED(BYTE, 0x51DAAE, 3u);

#pragma region ReplaceMasterControl

DEFINE_HOOK_AGAIN(0x51D1BA, InfantryClass_ReplaceMasterControl_Interrupt, 0x7)//Scatter
DEFINE_HOOK_AGAIN(0x51D925, InfantryClass_ReplaceMasterControl_Interrupt, 0x7)//DoType
DEFINE_HOOK(0x51C9E4, InfantryClass_ReplaceMasterControl_Interrupt, 0x7) //FireError
{
	GET(DoType, type, EAX);
	R->CL(NewDoType::GetSequenceData(type)->Interrupt);
	return R->Origin() + 0x7;
}

DEFINE_HOOK(0x521BFC, InfantryClass_ReadyToCommerce_ReplaceMasterControl, 0x7)
{
	GET(DoType, type, ESI);
	R->AL(NewDoType::GetSequenceData(type)->Interrupt);
	return 0x521C03;
}

#include <GameOptionsClass.h>

DEFINE_HOOK(0x51D9CF, InfantryClass_DoType_ReplaceMasterControl_Rates, 0x9)
{
	GET(DoType, todo, EDI);
	GET(FakeInfantryClass*, pThis, ESI);

	const bool normalize =
		todo == DoType::Idle1
		|| todo == DoType::Idle2
		|| todo == DoType::WetIdle1
		|| todo == DoType::WetIdle2
		|| todo == DoType::Hover
		|| todo == DoType::Cheer;

	const auto pExt = pThis->_GetTypeExtData();
	pThis->SequenceAnim = todo; //oof;

	if (pExt->AllSequnceEqualRates || !normalize) {
		pThis->Animation.Start(pExt->SquenceRates[(int)todo]);
	} else {
		pThis->Animation.Start(GameOptionsClass::Instance->GetAnimSpeed(pExt->SquenceRates[(int)todo]));
	}

	return 0x51DA4A;
}

//DEFINE_HOOK_AGAIN(0x51DA27, InfantryClass_DoType_ReplaceMasterControl_Rates, 0x7)
//DEFINE_HOOK(0x51D9FA, InfantryClass_DoType_ReplaceMasterControl_Rates, 0x7)
//{
//	GET(FakeInfantryClass*, pThis, ESI);
//	GET(int, _doType, EDI);
//
//	R->AL(pThis->_GetTypeExtData()->SquenceRates[_doType]);
//	return R->Origin() + 0x7;
//}

#pragma endregion

#pragma region S/L
DEFINE_HOOK(0x524B10, InfantryTypeClass_Load_DoControls, 0x5)
{
	GET(InfantryTypeClass*, pThis, EDI);
	GET(IStream*, pStream, ESI);

	pThis->Sequence = (DoControls*)GameCreate<NewDoType>();
	pStream->Read(pThis->Sequence, sizeof(NewDoType), nullptr);
	return 0x524B31;
}

DEFINE_HOOK(0x524C3C, InfantryTypeClass_Save_DoControls, 0x6)
{
	GET(InfantryTypeClass*, pThis, EDI);
	GET(IStream*, pStream, ESI);

	pStream->Write((void*)pThis->Sequence, sizeof(NewDoType), nullptr);
	return 0x524C50;
}
#pragma endregion

#pragma region ReadSequence
static void ReadSequence(DoControls* pDoInfo, FakeInfantryTypeClass* pInf, CCINIClass* pINI)
{
	INI_EX IniEX(pINI);

	char section[0x100] = {};
	if (pINI->GetString(pInf->ImageFile, "Sequence", section) > 0)
	{
		pInf->_GetExtData()->SquenceRates.resize(std::size(Sequences_ident));

		for (size_t i = 0; i < std::size(Sequences_ident); ++i)
		{
			char sequenceData[0x100] = {};
			if (pINI->GetString(section, Sequences_ident[i], sequenceData) > 0)
			{
				auto& data = pDoInfo->Data[i];
				const std::string basename = Sequences_ident[i];

				pInf->_GetExtData()->SquenceRates[i] = pINI->ReadInteger(section, (basename + ".Rate").c_str(), Sequences_Master[i].Rate);

				char bufferFacing[4];
				if (sscanf(sequenceData, "%d,%d,%d,%s",
					&data.StartFrame,
					&data.CountFrames,
					&data.FacingMultiplier,
					bufferFacing
				) > 3)
				{
					for (size_t a = 0; a < EnumFunctions::FacingType_to_strings.size(); ++a)
					{
						if (IS_SAME_STR_(EnumFunctions::FacingType_to_strings[a], bufferFacing))
						{
							data.Facing = DoTypeFacing(a);
						}
					}
				}

				char bufferSounds[0x100] = {};
				if (pINI->GetString(section, (basename + "Sounds").c_str(), bufferSounds) > 0)
				{
					auto v7 = strtok(bufferSounds, " ,\t");
					while (v7)
					{
						auto v8 = atoi(v7);
						auto v9 = strtok(0, " ,\t");
						if (!v9)
						{
							break;
						}

						data.SoundCount = v8;

						auto v10 = VocClass::FindIndexById(v9);
						v7 = strtok(0, " ,\t");
						if (v10 != -1)
						{
							for (auto at = data.SoundData;
								at != std::end(data.SoundData);
								++at)
							{
								at->Index = v10;
							}
						}
					}
				}
			}
		}
	}
}

DEFINE_HOOK(0x523D00, InfantryTypeClass_ReadSequence, 0x6)
{
	GET(FakeInfantryTypeClass*, pThis, ECX);
	ReadSequence(pThis->Sequence, pThis, &CCINIClass::INI_Art());
	return 0x524096;
}
#pragma endregion

#else
DEFINE_HOOK(0x523932, InfantryTypeClass_CTOR_Initialize, 8)
{
	GET(InfantryTypeClass*, pItem, ESI)
		pItem->Sequence->Initialize();
	return 0x523970;
}
#endif

DEFINE_HOOK(0x520BE5, InfantryClass_UpdateSequence_DeadBodies, 0x6)
{
	enum { SkipGameCode = 0x520CA9 };

	GET(InfantryTypeClass*, pType, ECX);

	AnimTypeClass* pAnimType = nullptr;

	if (pType->DeadBodies.Count)
		pAnimType = pType->DeadBodies[ScenarioClass::Instance->Random.RandomFromMax(pType->DeadBodies.Count - 1)];
	else if (!pType->NotHuman && RulesClass::Instance->DeadBodies.Count)
		pAnimType = RulesClass::Instance->DeadBodies[ScenarioClass::Instance->Random.RandomFromMax(RulesClass::Instance->DeadBodies.Count - 1)];

	if (pAnimType)
	{
		GET(InfantryClass*, pThis, ESI);

		const auto pAnim = GameCreate<AnimClass>(pAnimType, pThis->GetCoords());
		{
			const auto pOwner = pThis->Owner;
			pAnim->Owner = pOwner;
			//pAnim->LightConvert = ColorScheme::Array->GetItem(pOwner->ColorSchemeIndex)->LightConvert;
		}
	}

	return SkipGameCode;
}

DEFINE_HOOK(0x520E75, InfantryClass_SequenceAI_Sounds, 0x6)
{
	GET(InfantryClass*, pThis, ESI);

	const int doType = (int)pThis->SequenceAnim;

	// out of bound read fix
	if (doType == -1)
		return 0x520EF4;

	//const auto pSeq = &pThis->Type->Sequence->Data[doType];
	//
	//for (int i = 0; i < pSeq->SoundCount; ++i) {
	//	for (auto at = pSeq->SoundData; at != (&pSeq->SoundData[2]); ++at) {
	//		if (pThis->Animation.HasChanged && at->Index != -1) {
	//			const int count = pSeq->CountFrames < 1 ? 1 : pSeq->CountFrames;
	//			if (pThis->Animation.Value % count == at->StartFrame) {
	//				VoxClass::PlayAtPos(at->Index, &pThis->Location);
	//			}
	//		}
	//	}
	//}
	//
	//return 0x520EF4;
	return 0x0;
}