#include "Body.h"
#include <Ext/Rules/Body.h>

#include <Utilities/Macro.h>

#include <ParticleSystemClass.h>

static void ReadFacingDirMult(std::array<Point2D, (size_t)FacingType::Count>& arr, INI_EX& exINI, const char* pID, const int* beginX, const int* beginY)
{
	for (size_t i = 0; i < arr.size(); ++i)
	{
		if (!detail::read(arr[i], exINI, pID, (std::string("FacingDirectionMult") + std::to_string(i)).c_str()))
		{
			arr[i].X = *(beginX + i);
			arr[i].Y = *(beginY + i);
		}
	}
}

bool ParticleSystemTypeExtData::LoadFromINI(CCINIClass* pINI, bool parseFailAddr)
{
	if (!this->ObjectTypeExtData::LoadFromINI(pINI, parseFailAddr))
		return false;

	auto pThis = This();
	const char* pID = Name();

	if (parseFailAddr)
		return false;

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
		if (pThis->ParticleCap < 2 && !this->ApplyOptimization)
		{
			Debug::LogInfo("ParticleSystem[{}] BehavesLike=Spark ParticleCap need to be more than 1 , fixing", pID);
			pThis->ParticleCap = 2;
		}
		break;
	default:
		break;
	}

	this->AdjustTargetCoordsOnRotation.Read(exINI, pID, "AdjustTargetCoordsOnRotation");

	if (pThis->LightSize > 94)
		Debug::LogInfo("ParticleSystem[{}] with LightSize > 94 value [{}]", pID, pThis->LightSize);

	return true;
}

// =============================
// load / save
template <typename T>
void ParticleSystemTypeExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->ApplyOptimization)
		.Process(this->FacingMult)
		.Process(this->AdjustTargetCoordsOnRotation)
		;
}

// =============================
// container
ParticleSystemTypeExtContainer ParticleSystemTypeExtContainer::Instance;
std::vector<ParticleSystemTypeExtData*> Container<ParticleSystemTypeExtData>::Array;

bool ParticleSystemTypeExtContainer::LoadGlobals(PhobosStreamReader& Stm)
{
	Clear();

	size_t Count = 0;
	if (!Stm.Load(Count))
		return false;

	Array.reserve(Count);

	for (size_t i = 0; i < Count; ++i)
	{

		void* oldPtr = nullptr;

		if (!Stm.Load(oldPtr))
			return false;

		auto newPtr = new ParticleSystemTypeExtData(nullptr, noinit_t());
		PHOBOS_SWIZZLE_REGISTER_POINTER((long)oldPtr, newPtr, "ParticleSystemTypeExtData")
		ExtensionSwizzleManager::RegisterExtensionPointer(oldPtr, newPtr);
		newPtr->LoadFromStream(Stm);
		Array.push_back(newPtr);
	}

	return true;
}

bool ParticleSystemTypeExtContainer::SaveGlobals(PhobosStreamWriter& Stm)
{
	Stm.Save(Array.size());

	for (auto& item : Array)
	{
		// write old pointer and name, then delegate
		Stm.Save(item);
		item->SaveToStream(Stm);
	}

	return true;
}

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

bool FakeParticleSystemTypeClass::_ReadFromINI(CCINIClass* pINI)
{
	bool status = this->ParticleSystemTypeClass::LoadFromINI(pINI);
	ParticleSystemTypeExtContainer::Instance.LoadFromINI(this, pINI, !status);
	return status;
}

DEFINE_FUNCTION_JUMP(VTABLE, 0x7F010C, FakeParticleSystemTypeClass::_ReadFromINI)

HRESULT __stdcall FakeParticleSystemTypeClass::_Load(IStream* pStm)
{
	auto hr = this->ParticleSystemTypeClass::Load(pStm);

	if (SUCCEEDED(hr))
	{
		hr = ParticleSystemTypeExtContainer::Instance.ReadDataFromTheByteStream(this,
			ParticleSystemTypeExtContainer::Instance.AllocateNoInit(this), pStm);
	}

	return hr;
}

HRESULT __stdcall FakeParticleSystemTypeClass::_Save(IStream* pStm, BOOL clearDirty)
{
	auto hr = this->ParticleSystemTypeClass::Save(pStm, clearDirty);

	if (SUCCEEDED(hr))
	{
		hr = ParticleSystemTypeExtContainer::Instance.WriteDataToTheByteStream(this, pStm);
	}

	return hr;
}

//DEFINE_FUNCTION_JUMP(VTABLE, 0x7F00BC, FakeParticleSystemTypeClass::_Load)
//DEFINE_FUNCTION_JUMP(VTABLE, 0x7F00C0, FakeParticleSystemTypeClass::_Save)