#include "NewSWType.h"
#include "SWStateMachine.h"

class SW_LaserStrike : public NewSWType
{
public:
	virtual std::vector<const char*> GetTypeString() const override;
	virtual SuperWeaponFlags Flags(const SWTypeExt::ExtData* pData) const override;

	virtual bool Activate(SuperClass* pThis, const CellStruct& Coords, bool IsPlayer) override;
	virtual void Initialize(SWTypeExt::ExtData* pData) override;
	virtual void LoadFromINI(SWTypeExt::ExtData* pData, CCINIClass* pINI) override;
	virtual bool IsLaunchSite(const SWTypeExt::ExtData* pData, BuildingClass* pBuilding) const override;

	virtual int GetDamage(const SWTypeExt::ExtData* pData) const override;

	using TStateMachine = LaserStrikeStateMachine;

	void newStateMachine(CellStruct XY, SuperClass* pSuper, TechnoClass* pFirer, int maxcount, int deferment , int duration) {
		SWStateMachine::Register(std::make_unique<TStateMachine>(XY, pSuper, pFirer, maxcount, deferment , this , duration));
	}
};