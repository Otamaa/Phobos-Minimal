#include "Body.h"
#include <Ext/Rules/Body.h>

#include <Utilities/Macro.h>

#include <ParticleSystemClass.h>

#include <Phobos.SaveGame.h>

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
	const char* pID = Name.data();

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

bool ParticleSystemTypeExtContainer::LoadAll(const json& root)
{
	this->Clear();

	if (root.contains(ParticleSystemTypeExtContainer::ClassName))
	{
		auto& container = root[ParticleSystemTypeExtContainer::ClassName];

		for (auto& entry : container[ParticleSystemTypeExtData::ClassName])
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

			PHOBOS_SWIZZLE_REGISTER_POINTER(oldPtr, buffer, ParticleSystemTypeExtData::ClassName);

			buffer->LoadFromStream(reader);

			if (!reader.ExpectEndOfBlock())
				return false;
		}

		return true;
	}

	return false;

}

bool ParticleSystemTypeExtContainer::SaveAll(json& root)
{
	auto& first_layer = root[ParticleSystemTypeExtContainer::ClassName];

	json _extRoot = json::array();
	for (auto& _extData : ParticleSystemTypeExtContainer::Array)
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

	first_layer[ParticleSystemTypeExtData::ClassName] = std::move(_extRoot);

	return true;
}

void ParticleSystemTypeExtContainer::LoadFromINI(ext_t::base_type* key, CCINIClass* pINI, bool parseFailAddr)
{
	if (auto ptr = this->Find(key))
	{
		if (!pINI)
		{
			return;
		}

		//load anywhere other than rules
		ptr->LoadFromINI(pINI, parseFailAddr);
		//this function can be called again multiple time but without need to re-init the data
		ptr->SetInitState(InitState::Ruled);
	}

}

void ParticleSystemTypeExtContainer::WriteToINI(ext_t::base_type* key, CCINIClass* pINI)
{

	if (auto ptr = this->TryFind(key))
	{
		if (!pINI)
		{
			return;
		}

		ptr->WriteToINI(pINI);
	}
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
