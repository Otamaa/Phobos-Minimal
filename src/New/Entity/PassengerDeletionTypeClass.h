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

public:

	TechnoTypeClass* OwnerType { nullptr };

	bool Enabled { false };
	Valueable<int> Rate { 0 };
	Valueable<bool> Rate_SizeMultiply { true };
	Valueable<bool> Rate_AffectedByVeterancy;
	Valueable<bool> UseCostAsRate {};
	Valueable<double> CostMultiplier { 1.0 };
	Nullable<int> CostRateCap {};
	Valueable<AffectedHouse> AllowedHouses { AffectedHouse::All };
	Valueable<bool> DontScore {};
	Valueable<bool> Soylent {};
	Valueable<double> SoylentMultiplier { 1.0 };
	Valueable<AffectedHouse> SoylentAllowedHouses { AffectedHouse::Enemies };
	Valueable<bool> DisplaySoylent {};
	Valueable<AffectedHouse> DisplaySoylentToHouses { AffectedHouse::All };
	Valueable<Point2D> DisplaySoylentOffset {};
	NullableIdx<VocClass> ReportSound {};
	Nullable<AnimTypeClass*> Anim {};
	Valueable<bool> UnderEMP {};

	void LoadFromINI(CCINIClass* pINI, const char* pSection);
	bool Load(PhobosStreamReader& stm, bool registerForChange);
	bool Save(PhobosStreamWriter& stm) const;

	static std::pair<bool, bool> CanParse(INI_EX exINI, const char* pSection);

private:

	template <typename T>
	bool Serialize(T& stm);
};
