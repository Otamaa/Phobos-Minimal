#include "Body.h"

#include <Utilities/Macro.h>

#include <Phobos.SaveGame.h>

void ReadWinDirMult(std::array<Point2D, (size_t)FacingType::Count>& arr, INI_EX& exINI, const char* pID, const int* beginX , const int* beginY) {
	for (size_t i = 0; i < arr.size(); ++i) {
		if(!detail::read(arr[i], exINI, pID, (std::string("WindDirectionMult") + std::to_string(i)).c_str())) {
			arr[i].X = *(beginX + i);
			arr[i].Y = *(beginY + i);
		}
	}
}

bool ParticleTypeExtData::LoadFromINI(CCINIClass* pINI, bool parseFailAddr)
{
	if (!this->ObjectTypeExtData::LoadFromINI(pINI, parseFailAddr) || parseFailAddr)
		return false;

	auto pThis = this->This();
	const char* pID = pThis->ID;

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

			this->Gas_DriftSpeedX->Max = Max_driftX;
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

	this->Palette.Read(exINI, pID, "Palette");
	this->DamageRange.Read(exINI, pID, "DamageRange");

	///if (IS_SAME_STR_(pID, "SuperNapalmCloudPart"))
	//	Debug::LogInfo("AlphaImageNAme [%s] ", this->This()->AlphaImageFile);

	if (pThis->StateAIAdvance == 0 && pThis->StartStateAI < pThis->EndStateAI) {
		Debug::RegisterParserError();
		Debug::LogInfo("[Developer warning] [{}] has StateAIAdvance=0 in conjunction with StartStateAI value less than EndStateAI. StateAIAdvance set to 1 to prevent crashes from occuring.",
			pID);
		pThis->StateAIAdvance = 1;
	}

	return true;
}

// =============================
// load / save
template <typename T>
void ParticleTypeExtData::Serialize(T& Stm)
{
	Stm
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
		.Process(this->Trails)
		;


}

// =============================
// container
ParticleTypeExtContainer ParticleTypeExtContainer::Instance;

bool ParticleTypeExtContainer::LoadAll(const json& root)
{
	this->Clear();

	if (root.contains(ParticleTypeExtContainer::ClassName))
	{
		auto& container = root[ParticleTypeExtContainer::ClassName];

		for (auto& entry : container[ParticleTypeExtData::ClassName])
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

			PHOBOS_SWIZZLE_REGISTER_POINTER(oldPtr, buffer, ParticleTypeExtData::ClassName);

			buffer->LoadFromStream(reader);

			if (!reader.ExpectEndOfBlock())
				return false;
		}

		return true;
	}

	return false;

}

bool ParticleTypeExtContainer::SaveAll(json& root)
{
	auto& first_layer = root[ParticleTypeExtContainer::ClassName];

	json _extRoot = json::array();
	for (auto& _extData : ParticleTypeExtContainer::Array)
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

	first_layer[ParticleTypeExtData::ClassName] = std::move(_extRoot);

	return true;
}

void ParticleTypeExtContainer::LoadFromINI(ext_t::base_type* key, CCINIClass* pINI, bool parseFailAddr)
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

void ParticleTypeExtContainer::WriteToINI(ext_t::base_type* key, CCINIClass* pINI)
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

ASMJIT_PATCH(0x644DBB, ParticleTypeClass_CTOR, 0x5)
{
	GET(ParticleTypeClass*, pItem, ESI);
	ParticleTypeExtContainer::Instance.Allocate(pItem);
	return 0;
}

ASMJIT_PATCH(0x645A42, ParticleTypeClass_SDDTOR, 0xA)
{
	GET(ParticleTypeClass*, pItem, ESI);
	ParticleTypeExtContainer::Instance.Remove(pItem);

	return 0;
}

bool FakeParticleTypeClass::_ReadFromINI(CCINIClass* pINI)
{
	bool status = this->ParticleTypeClass::LoadFromINI(pINI);
	ParticleTypeExtContainer::Instance.LoadFromINI(this, pINI, !status);
	return status;
}

DEFINE_FUNCTION_JUMP(VTABLE, 0x7F01EC, FakeParticleTypeClass::_ReadFromINI)
