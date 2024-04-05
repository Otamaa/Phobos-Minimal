#include "Body.h"

void BulletExtData::AddGlobalScripts(std::list<std::string>& globalScripts, IExtData* ext)
{
	// Base Component
	//globalScripts.push_back(BulletStatus::ScriptName);
	//globalScripts.push_back(AttachEffect::ScriptName);
	//globalScripts.push_back(BulletTrail::ScriptName);
}

BulletExtDataContainer ExtMap {};
std::vector<BulletClass*> BulletExtData::TargetHasDecoyBullets {};