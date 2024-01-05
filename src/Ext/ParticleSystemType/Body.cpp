#include "Body.h"

void ReadFacingDirMult(std::array<Point2D, (size_t)FacingType::Count>& arr, INI_EX& exINI, const char* pID, const int* beginX, const int* beginY)
{
	char buff_wind[0x25];

	for (size_t i = 0; i < arr.size(); ++i) {
		IMPL_SNPRNINTF(buff_wind, sizeof(buff_wind) - 1, "FacingDirectionMult%d", i);

		if(!detail::read(arr[i], exINI, pID, buff_wind)) {
			arr[i].X = *(beginX + i);
			arr[i].Y = *(beginY + i);
		}
	}
}

void ParticleSystemTypeExtData::LoadFromINIFile(CCINIClass* pINI, bool parseFailAddr)
{
	auto pThis = this->AttachedToObject;
	const char* pID = pThis->ID;

	if (parseFailAddr)
		return;

	INI_EX exINI(pINI);
	switch (pThis->BehavesLike)
	{
	case ParticleSystemTypeBehavesLike::Fire:
	{
		ReadFacingDirMult(this->FacingMult, exINI, pID, ParticleSystemClass::FireWind_X.begin(), ParticleSystemClass::FireWind_Y.begin());
	}
	break;
	case ParticleSystemTypeBehavesLike::Spark:
		if (pThis->ParticleCap < 2){
			Debug::Log("ParticleSystem[%s] BehavesLike=Spark ParticleCap need to be more than 1 , fixing\n", pID);
			pThis->ParticleCap = 2;
		}
		break;
	default:
		break;
	}

	this->ApplyOptimization.Read(exINI, pID, "ApplyOptimization");
	this->AdjustTargetCoordsOnRotation.Read(exINI, pID, "AdjustTargetCoordsOnRotation");

	if (pThis->LightSize > 94)
		Debug::Log("ParticleSystem[%s] with LightSize > 94 value [%d]\n", pID , pThis->LightSize);

}

// =============================
// load / save
template <typename T>
void ParticleSystemTypeExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->Initialized)
		.Process(this->ApplyOptimization)
		.Process(this->FacingMult)
		.Process(this->AdjustTargetCoordsOnRotation)
		;
}

// =============================
// container
ParticleSystemTypeExtContainer ParticleSystemTypeExtContainer::Instance;

// =============================
// container hooks

DEFINE_HOOK(0x644217, ParticleSystemTypeClass_CTOR, 0x5)
{
	GET(ParticleSystemTypeClass*, pItem, ESI);
	ParticleSystemTypeExtContainer::Instance.Allocate(pItem);
	return 0;
}

DEFINE_HOOK(0x644276, ParticleSystemTypeClass_SDDTOR, 0x6)
{
	GET(ParticleSystemTypeClass*, pItem, ESI);
	ParticleSystemTypeExtContainer::Instance.Remove(pItem);

	return 0;
}

DEFINE_HOOK_AGAIN(0x644830, ParticleSystemTypeClass_SaveLoad_Prefix, 0x8)
DEFINE_HOOK(0x6447E0, ParticleSystemTypeClass_SaveLoad_Prefix, 0x5)
{
	GET_STACK(ParticleSystemTypeClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	ParticleSystemTypeExtContainer::Instance.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK(0x64481F, ParticleSystemTypeClass_Load_Suffix, 0x6)
{
	ParticleSystemTypeExtContainer::Instance.LoadStatic();
	return 0;
}

DEFINE_HOOK(0x644844, ParticleSystemTypeClass_Save_Suffix, 0x5)
{
	ParticleSystemTypeExtContainer::Instance.SaveStatic();
	return 0;
}

DEFINE_HOOK_AGAIN(0x644620, ParticleSystemTypeClass_LoadFromINI, 0x5)
DEFINE_HOOK(0x644617, ParticleSystemTypeClass_LoadFromINI, 0x5)
{
	GET(ParticleSystemTypeClass*, pItem, ESI);
	GET(CCINIClass*, pINI, EBX);

	ParticleSystemTypeExtContainer::Instance.LoadFromINI(pItem, pINI , R->Origin() == 0x644620);
	return 0;
}