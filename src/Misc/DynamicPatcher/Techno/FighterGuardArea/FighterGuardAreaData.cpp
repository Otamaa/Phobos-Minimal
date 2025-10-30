#include "FighterGuardAreaData.h"


#ifdef _aaaaaaa 
void FighterAreaGuardData::Read(INI_EX& parser, const char* pSection, TechnoTypeClass* pType)
{
	this->AreaGuard.Read(parser, pSection ,"Fighter.AreaGuard");
	this->AutoGuard.Read(parser, pSection ,"Fighter.AutoGuard");
	this->DefaultToGuard.Read(parser, pSection ,"Fighter.DefaultToGuard");

	this->Enable = AreaGuard || AutoGuard || DefaultToGuard;

	this->GuardRange.Read(parser, pSection ,"Fighter.GuardRange");
	this->AutoFire.Read(parser, pSection ,"Fighter.AutoFire");
	this->MaxAmmo.Read(parser, pSection ,"Fighter.Ammo");
	this->MaxAmmo.Read(parser, pSection ,"Fighter.HoldAmmo");
	this->GuardRadius.Read(parser, pSection ,"Fighter.GuardRadius");
	this->FindRangeAroundSelf.Read(parser, pSection ,"Fighter.FindRangeAroundSelf");
	this->ChaseRange.Read(parser, pSection ,"Fighter.ChaseRange");
	this->Clockwise.Read(parser, pSection ,"Fighter.Clockwise");
	this->Randomwise.Read(parser, pSection ,"Fighter.Randomwise");
}

#endif