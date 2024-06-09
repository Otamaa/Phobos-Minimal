#include "Body.h"

void ParticleTypeExtData::Initialize() {
	LaserTrail_Types.reserve(2);
}

void ReadWinDirMult(std::array<Point2D, (size_t)FacingType::Count>& arr, INI_EX& exINI, const char* pID, const int* beginX , const int* beginY) {
	for (size_t i = 0; i < arr.size(); ++i) {
		if(!detail::read(arr[i], exINI, pID, (std::string("WindDirectionMult") + std::to_string(i)).c_str())) {
			arr[i].X = *(beginX + i);
			arr[i].Y = *(beginY + i);
		}
	}
}

void ParticleTypeExtData::LoadFromINIFile(CCINIClass* pINI, bool parseFailAddr)
{
	auto pThis = this->AttachedToObject;
	const char* pID = pThis->ID;

	if (parseFailAddr)
		return;

	INI_EX exINI(pINI);

	switch (pThis->BehavesLike)
	{
	case ParticleTypeBehavesLike::Smoke: {
		/*
			WindFacingMult Smoke[at 0 - value(x:0, y : -2)]
			WindFacingMult Smoke[at 1 - value(x:2, y : -2)]
			WindFacingMult Smoke[at 2 - value(x:2, y : 0)]
			WindFacingMult Smoke[at 3 - value(x:1, y : 2)]
			WindFacingMult Smoke[at 4 - value(x:0, y : 2)]
			WindFacingMult Smoke[at 5 - value(x:-2, y : 2)]
			WindFacingMult Smoke[at 6 - value(x:-2, y : 0)]
			WindFacingMult Smoke[at 7 - value(x:-2, y : -2)]
		*/
		this->DeleteWhenReachWater.Read(exINI, pID, "Smoke.DeleteWhenReachWater");
		ReadWinDirMult(this->WindMult, exINI , pID, ParticleClass::SmokeWind_X.begin(), ParticleClass::SmokeWind_Y.begin());
	}
	break;
	case ParticleTypeBehavesLike::Fire: {
	//	this->ExpireAfterDamaging.Read(exINI, pID, "Fire.ExpireAfterDamaging");
		this->Fire_DamagingAnim.Read(exINI, pID, "Fire.DamagingAnim");
		this->ReadjustZ.Read(exINI, pID , "ReadjustZCoord");
	}
	break;
	case ParticleTypeBehavesLike::Railgun: {
		this->ReadjustZ.Read(exINI, pID , "ReadjustZCoord");
		break;
	}
	case ParticleTypeBehavesLike::Gas: {
		/*
			[at 0 - value(x:0, y : -2)]
			[at 1 - value(x:2, y : -2)]
			[at 2 - value(x:2, y : 0)]
			[at 3 - value(x:2, y : 2)]
			[at 4 - value(x:0, y : 2)]
			[at 5 - value(x:-2, y : 2)]
			[at 6 - value(x:-2, y : 0)]
			[at 7 - value(x:-2, y : -2)]
		*/

		ReadWinDirMult(this->WindMult, exINI, pID, ParticleClass::GasWind_X.begin(), ParticleClass::GasWind_Y.begin());

		if(Phobos::Otamaa::CompatibilityMode){
			int Max_driftX = 2;
			detail::read(Max_driftX, exINI, pID, "Gas.MaxDriftSpeed");

			this->Gas_DriftSpeedX.GetEx()->X = Max_driftX;
		}

		this->Gas_DriftSpeedX.Read(exINI, pID, "Gas.MaxDriftSpeedX");
		this->Gas_DriftSpeedY.Read(exINI, pID, "Gas.MaxDriftSpeedY");

		this->Transmogrify.Read(exINI, pID, "Gas.Transmogrify");
		this->TransmogrifyChance.Read(exINI, pID, "Gas.TransmogrifyChance");
		this->TransmogrifyType.Read(exINI, pID, "Gas.TransmogrifyType", true);
		this->TransmogrifyOwner.Read(exINI, pID, "Gas.TransmogrifyOwner");
	}break;
	default:
		break;
	}

	this->LaserTrail_Types.Read(exINI, pID, "LaserTrail.Types");

	this->Trails.Read(exINI, pID, false);

	this->Palette.Read(exINI, pID, "CustomPalette");
	this->DamageRange.Read(exINI, pID, "DamageRange");

	///if (IS_SAME_STR_(pID, "SuperNapalmCloudPart"))
	//	Debug::Log("AlphaImageNAme [%s] \n", this->AttachedToObject->AlphaImageFile);
}

// =============================
// load / save
template <typename T>
void ParticleTypeExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->Initialized)
		.Process(this->LaserTrail_Types)
		.Process(this->ReadjustZ)
		.Process(this->Palette)
		.Process(this->DamageRange)
		.Process(this->DeleteWhenReachWater)
		.Process(this->WindMult)
		.Process(this->Gas_DriftSpeedX)
		.Process(this->Gas_DriftSpeedY)
		.Process(this->Transmogrify)
		.Process(this->TransmogrifyChance)
		.Process(this->TransmogrifyType)
		.Process(this->TransmogrifyOwner)
		.Process(this->Fire_DamagingAnim)
		;

	this->Trails.Serialize(Stm);
}

// =============================
// container
ParticleTypeExtContainer ParticleTypeExtContainer::Instance;

// =============================
// container hooks

DEFINE_HOOK(0x644DBB, ParticleTypeClass_CTOR, 0x5)
{
	GET(ParticleTypeClass*, pItem, ESI);
	ParticleTypeExtContainer::Instance.Allocate(pItem);
	return 0;
}

DEFINE_HOOK(0x645A42, ParticleTypeClass_SDDTOR, 0xA)
{
	GET(ParticleTypeClass*, pItem, ESI);
	ParticleTypeExtContainer::Instance.Remove(pItem);

	return 0;
}

DEFINE_HOOK_AGAIN(0x6457A0, ParticleTypeClass_SaveLoad_Prefix, 0x5)
DEFINE_HOOK(0x645660, ParticleTypeClass_SaveLoad_Prefix, 0x7)
{
	GET_STACK(ParticleTypeClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	ParticleTypeExtContainer::Instance.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK(0x64578C, ParticleTypeClass_Load_Suffix, 0x5)
{
	ParticleTypeExtContainer::Instance.LoadStatic();
	return 0;
}

DEFINE_HOOK(0x64580A, ParticleTypeClass_Save_Suffix, 0x7)
{
	ParticleTypeExtContainer::Instance.SaveStatic();
	return 0;
}

DEFINE_HOOK_AGAIN(0x645414 , ParticleTypeClass_LoadFromINI, 0x5)
DEFINE_HOOK(0x645405, ParticleTypeClass_LoadFromINI, 0x5)
{
	GET(ParticleTypeClass*, pItem, ESI);
	GET_STACK(CCINIClass*, pINI, STACK_OFFS(0xDC , -0x4));

	ParticleTypeExtContainer::Instance.LoadFromINI(pItem, pINI , R->Origin() == 0x645414);
	return 0;
}