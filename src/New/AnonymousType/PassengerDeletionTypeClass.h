#pragma once

#include <Utilities/Constructs.h>
#include <Utilities/Enum.h>
#include <Utilities/Template.h>

#include <Point2D.h>

class TechnoTypeClass;
class AnimTypeClass;
class PassengerDeletionTypeClass
{
public:

	PassengerDeletionTypeClass() = default;

	PassengerDeletionTypeClass(TechnoTypeClass* pOwnerType);

	PassengerDeletionTypeClass(const PassengerDeletionTypeClass& other) = default;
	PassengerDeletionTypeClass& operator=(const PassengerDeletionTypeClass& other) = default;

	~PassengerDeletionTypeClass() noexcept = default;

	TechnoTypeClass* OwnerType;

	bool Enabled;
	Valueable<int> Rate;
	Valueable<bool> Rate_SizeMultiply;
	Valueable<bool> Rate_AffectedByVeterancy;
	Valueable<bool> UseCostAsRate;
	Valueable<double> CostMultiplier;
	Nullable<int> CostRateCap;
	Valueable<AffectedHouse> AllowedHouses;
	Valueable<bool> DontScore;
	Valueable<bool> Soylent;
	Valueable<double> SoylentMultiplier;
	Valueable<AffectedHouse> SoylentAllowedHouses;
	Valueable<bool> DisplaySoylent;
	Valueable<AffectedHouse> DisplaySoylentToHouses;
	Valueable<Point2D> DisplaySoylentOffset;
	NullableIdx<VocClass> ReportSound;
	Nullable<AnimTypeClass*> Anim;

	void LoadFromINI(CCINIClass* pINI, const char* pSection);
	bool Load(PhobosStreamReader& stm, bool registerForChange);
	bool Save(PhobosStreamWriter& stm) const;

	static std::pair<bool, bool> CanParse(INI_EX exINI, const char* pSection);

private:

	template <typename T>
	bool Serialize(T& stm);
};
