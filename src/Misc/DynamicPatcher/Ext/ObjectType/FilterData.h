#pragma once

#include <algorithm>
#include <string>
#include <vector>
#include <set>

#include <GeneralStructures.h>
#include <HouseClass.h>
#include <ObjectClass.h>
#include <TechnoClass.h>
#include <BulletClass.h>

#include <Misc/DynamicPatcher/Common/INI/INIConfig.h>

#include <Misc/DynamicPatcher/Ext/Helper/CastEx.h>
#include <Misc/DynamicPatcher/Ext/Helper/MathEx.h>
#include <Misc/DynamicPatcher/Ext/Helper/Status.h>
#include <Misc/DynamicPatcher/Ext/Helper/StringEx.h>

class FilterData : public INIConfig
{
public:
	std::vector<std::string> AffectTypes{};
	std::vector<std::string> NotAffectTypes{};

	bool AffectTechno = true;
	bool AffectBuilding = true;
	bool AffectInfantry = true;
	bool AffectUnit = true;
	bool AffectAircraft = true;

	bool AffectBullet = true;
	bool AffectMissile = true;
	bool AffectTorpedo = true;
	bool AffectCannon = true;
	bool AffectBomb = true;

	bool AffectStand = false;
	bool AffectSelf = false;
	bool AffectInAir = true;
	std::vector<std::string> NotAffectMarks{};
	std::vector<std::string> OnlyAffectMarks{};

	bool AffectsOwner = true;
	bool AffectsAllies = true;
	bool AffectsEnemies = true;
	bool AffectsCivilian = true;

	virtual void Read(INI_EX& reader) override
	{
		Read(reader, "");
	}

	virtual void Read(INI_EX& reader, std::string title)
	{
		reader.ParseList(AffectTypes, Type->ID, (title + "AffectTypes").c_str());
		KratosCRT::ClearIfGetNone(AffectTypes);
		reader.ParseList(NotAffectTypes, Type->ID, (title + "NotAffectTypes").c_str());
		ClearIfGetNone(NotAffectTypes);

		reader.ReadBool(Type->ID, (title + "AffectTechno").c_str() ,&AffectTechno);
		reader.ReadBool(Type->ID, (title + "AffectBuilding").c_str(), &AffectBuilding);
		reader.ReadBool(Type->ID, (title + "AffectInfantry").c_str(), &AffectInfantry);
		reader.ReadBool(Type->ID, (title + "AffectUnit").c_str(), &AffectUnit);
		reader.ReadBool(Type->ID, (title + "AffectAircraft").c_str(), &AffectAircraft);

		if (!AffectBuilding && !AffectInfantry && !AffectUnit && !AffectAircraft) {
			AffectTechno = false;
		}

		reader.ReadBool(Type->ID, (title + "AffectBullet").c_str(), &AffectBullet);
		reader.ReadBool(Type->ID, (title + "AffectMissile").c_str(), &AffectMissile);
		reader.ReadBool(Type->ID, (title + "AffectTorpedo").c_str(), &AffectTorpedo);
		reader.ReadBool(Type->ID, (title + "AffectCannon").c_str(), &AffectCannon);
		reader.ReadBool(Type->ID, (title + "AffectBomb").c_str(), &AffectBomb);

		if (!AffectMissile && !AffectCannon && !AffectBomb)
		{
			AffectBullet = false;
		}

		reader.ReadBool(Type->ID, (title + "AffectStand").c_str(), &AffectStand);
		reader.ReadBool(Type->ID, (title + "AffectSelf").c_str(), &AffectSelf);
		reader.ReadBool(Type->ID, (title + "AffectInAir").c_str(), &AffectInAir);

		reader.ParseList(NotAffectMarks, Type->ID, (title + "NotAffectMarks").c_str());
		ClearIfGetNone(NotAffectMarks);
		reader.ParseList(OnlyAffectMarks, Type->ID, (title + "OnlyAffectMarks").c_str());
		ClearIfGetNone(OnlyAffectMarks);

		bool affectsAllies = true;
		if (reader.ReadBool(Type->ID, (title + "AffectsAllies").c_str(), &affectsAllies)){
			AffectsAllies = affectsAllies;
			AffectsOwner = affectsAllies;
		}

		bool affectsOwner = true;
		if (reader.ReadBool(Type->ID, (title + "AffectsAllies").c_str(), &affectsAllies)){
			AffectsOwner = affectsOwner;
		}

		reader.ReadBool(Type->ID, (title + "AffectsEnemies").c_str(), &AffectsEnemies);
		reader.ReadBool(Type->ID, (title + "AffectsCivilian").c_str(), &AffectsCivilian);
	}

	constexpr bool CanAffectHouse(HouseClass* pHouse, HouseClass* pTargetHouse)
	{
		return !pHouse || !pTargetHouse || (pTargetHouse == pHouse ? AffectsOwner : (IsCivilian(pTargetHouse) ? AffectsCivilian : pTargetHouse->IsAlliedWith(pHouse) ? AffectsAllies : AffectsEnemies));
	}

	constexpr bool CanAffectType(const char* ID)
	{
		if (!NotAffectTypes.empty() && std::find(NotAffectTypes.begin(), NotAffectTypes.end(), ID) != NotAffectTypes.end())
		{
			return false;
		}
		bool can = AffectTypes.empty();
		if (!can)
		{
			can = std::find(AffectTypes.begin(), AffectTypes.end(), ID) != AffectTypes.end();
		}
		return can;
	}

	constexpr bool CanAffectType(AbstractType absType)
	{
		switch (absType)
		{
		case AbstractType::Building:
			return AffectBuilding;
		case AbstractType::Infantry:
			return AffectInfantry;
		case AbstractType::Unit:
			return AffectUnit;
		case AbstractType::Aircraft:
			return AffectAircraft;
		}
		return false;
	}

	constexpr bool CanAffectType(BulletType bulletType, bool isLevel)
	{
		switch (bulletType)
		{
		case BulletType::INVISO:
			return true;
		case BulletType::ARCING:
			return AffectCannon;
		case BulletType::BOMB:
			return AffectBomb;
		case BulletType::ROCKET:
		case BulletType::MISSILE:
			// 导弹和直线导弹都算Missile
			if (isLevel)
			{
				return AffectTorpedo;
			}
			return AffectMissile;
		}
		return false;
	}

	constexpr bool CanAffectType(BulletClass* pBullet)
	{
		return CanAffectType(pBullet->Type->ID) && CanAffectType(WhatAmI(pBullet), pBullet->Type->Level);
	}

	constexpr bool CanAffectType(TechnoClass* pTechno)
	{
		return CanAffectType(pTechno->GetTechnoType()->ID) && CanAffectType(pTechno->WhatAmI());
	}

	bool CanAffectType(ObjectClass* pObject)
	{
		TechnoClass* pTechno = nullptr;
		BulletClass* pBullet = nullptr;
		if (CastToTechno(pObject, pTechno))
		{
			return CanAffectType(pTechno);
		}
		else if (CastToBullet(pObject, pBullet))
		{
			return CanAffectType(pBullet);
		}
		return false;
	}

	constexpr bool HasMarks()
	{
		return !OnlyAffectMarks.empty() || !NotAffectMarks.empty();
	}

	constexpr bool OnMark(std::vector<std::string> marks)
	{
		bool hasWhiteList = !OnlyAffectMarks.empty();
		bool hasBlackList = !NotAffectMarks.empty();
		return (!hasWhiteList || (!marks.empty() && CheckOnMarks(OnlyAffectMarks, marks)))
				&& (!hasBlackList || (marks.empty() || !CheckOnMarks(NotAffectMarks, marks)));
	}

#pragma region save/load
	template <typename T>
	bool Serialize(T& stream)
	{
		return stream
			.Process(this->AffectTypes)
			.Process(this->NotAffectTypes)

			.Process(this->AffectTechno)
			.Process(this->AffectBuilding)
			.Process(this->AffectInfantry)
			.Process(this->AffectUnit)
			.Process(this->AffectAircraft)

			.Process(this->AffectBullet)
			.Process(this->AffectMissile)
			.Process(this->AffectTorpedo)
			.Process(this->AffectCannon)
			.Process(this->AffectBomb)

			.Process(this->AffectStand)
			.Process(this->AffectSelf)
			.Process(this->AffectInAir)
			.Process(this->NotAffectMarks)
			.Process(this->OnlyAffectMarks)

			.Process(this->AffectsOwner)
			.Process(this->AffectsAllies)
			.Process(this->AffectsEnemies)
			.Process(this->AffectsCivilian)
			.Success();
	};

	virtual bool Load(PhobosStreamReader& stream, bool registerForChange) override
	{
		INIConfig::Load(stream, registerForChange);
		return this->Serialize(stream);
	}
	virtual bool Save(PhobosStreamWriter& stream) const override
	{
		INIConfig::Save(stream);
		return const_cast<FilterData*>(this)->Serialize(stream);
	}
#pragma endregion
};
