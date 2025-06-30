#include "Body.h"
#include <Ext/Rules/Body.h>

#include <ParticleSystemClass.h>

static void ReadFacingDirMult(std::array<Point2D, (size_t)FacingType::Count>& arr, INI_EX& exINI, const char* pID, const int* beginX, const int* beginY)
{
	for (size_t i = 0; i < arr.size(); ++i) {
		if(!detail::read(arr[i], exINI, pID, (std::string("FacingDirectionMult") + std::to_string(i)).c_str())) {
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
	this->ApplyOptimization.Read(exINI, pID, "ApplyOptimization");

	switch (pThis->BehavesLike)
	{
	case ParticleSystemTypeBehavesLike::Fire:
	{
		ReadFacingDirMult(this->FacingMult, exINI, pID, ParticleSystemClass::FireWind_X.begin(), ParticleSystemClass::FireWind_Y.begin());
	}
	break;
	case ParticleSystemTypeBehavesLike::Spark:
		//these bug only happen on vanilla particle drawings
		if (pThis->ParticleCap < 2 && !this->ApplyOptimization){
			Debug::LogInfo("ParticleSystem[{}] BehavesLike=Spark ParticleCap need to be more than 1 , fixing", pID);
			pThis->ParticleCap = 2;
		}
		break;
	default:
		break;
	}

	this->AdjustTargetCoordsOnRotation.Read(exINI, pID, "AdjustTargetCoordsOnRotation");

	if (pThis->LightSize > 94)
		Debug::LogInfo("ParticleSystem[{}] with LightSize > 94 value [{}]", pID , pThis->LightSize);

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

ASMJIT_PATCH(0x644217, ParticleSystemTypeClass_CTOR, 0x5)
{
	GET(ParticleSystemTypeClass*, pItem, ESI);
	ParticleSystemTypeExtContainer::Instance.Allocate(pItem);
	return 0;
}

ASMJIT_PATCH(0x644276, ParticleSystemTypeClass_SDDTOR, 0x6)
{
	GET(ParticleSystemTypeClass*, pItem, ESI);
	ParticleSystemTypeExtContainer::Instance.Remove(pItem);

	return 0;
}
#include <Misc/Hooks.Otamaa.h>

HRESULT __stdcall FakeParticleSystemTypeClass::_Load(IStream* pStm)
{

	ParticleSystemTypeExtContainer::Instance.PrepareStream(this, pStm);
	HRESULT res = this->ParticleSystemTypeClass::Load(pStm);

	if (SUCCEEDED(res))
		ParticleSystemTypeExtContainer::Instance.LoadStatic();

	return res;
}

HRESULT __stdcall FakeParticleSystemTypeClass::_Save(IStream* pStm, bool clearDirty)
{

	ParticleSystemTypeExtContainer::Instance.PrepareStream(this, pStm);
	HRESULT res = this->ParticleSystemTypeClass::Save(pStm, clearDirty);

	if (SUCCEEDED(res))
		ParticleSystemTypeExtContainer::Instance.SaveStatic();

	return res;
}

DEFINE_FUNCTION_JUMP(VTABLE, 0x7F00BC, FakeParticleSystemTypeClass::_Load)
DEFINE_FUNCTION_JUMP(VTABLE, 0x7F00C0, FakeParticleSystemTypeClass::_Save)

ASMJIT_PATCH(0x644617, ParticleSystemTypeClass_LoadFromINI, 0x5)
{
	GET(ParticleSystemTypeClass*, pItem, ESI);
	GET(CCINIClass*, pINI, EBX);

	ParticleSystemTypeExtContainer::Instance.LoadFromINI(pItem, pINI , R->Origin() == 0x644620);
	return 0;
}ASMJIT_PATCH_AGAIN(0x644620, ParticleSystemTypeClass_LoadFromINI, 0x5)