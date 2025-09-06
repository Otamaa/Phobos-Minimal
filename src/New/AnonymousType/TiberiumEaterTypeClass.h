#pragma once

#include <Utilities/Constructs.h>
#include <Utilities/Enum.h>
#include <Utilities/Template.h>

class TiberiumEaterTypeClass
{
public:
	Valueable<int> TransDelay { -1 };
	Valueable<float>  CashMultiplier { 1.0 };
	Valueable<int> AmountPerCell { 0 };
	std::vector<Point2D> Cells { std::vector<Point2D>(1) };
	Valueable<bool> Display { true };
	Valueable<AffectedHouse> DisplayToHouse { AffectedHouse::All };
	Valueable<Point2D> DisplayOffset { Point2D::Empty };
	ValueableVector<AnimTypeClass*> Anims {};
	std::vector<NullableVector<AnimTypeClass*>> Anims_Tiberiums {};
	Valueable<bool> AnimMove { true };

public:

	void LoadFromINI(CCINIClass* pINI, const char* pSection);
	bool Load(PhobosStreamReader& stm, bool registerForChange);
	bool Save(PhobosStreamWriter& stm) const;

private:

	template <typename T>
	bool Serialize(T& stm);
};