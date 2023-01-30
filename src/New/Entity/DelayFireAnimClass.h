//#pragma once
//
//#include <GeneralStructures.h>
//#include <SpecificStructures.h>



// TODO : CompleteThis
//class TechnoClass;
//class WarheadTypeClass;
//class TechnoTypeClass;
//class DelayFireAnimClass final
//{
//public:
//	DelayFireAnimClass();
//	DelayFireAnimClass(TechnoClass* pTechno, bool isAttached);
//	DelayFireAnimClass(TechnoClass* pTechno) : DelayFireAnimClass(pTechno, false) { };
//	virtual ~DelayFireAnimClass() { KillAnim(); };
//
//	void OnInit() { }
//	void OnUnInit() { }
//	void OnDetonate(CoordStruct* location) { }
//	void OnPut(CoordStruct pCoord, short faceDirValue8) { }
//	void OnRemove();
//	void OnReceiveDamage(args_ReceiveDamage* args);
//	void OnFire(AbstractClass* pTarget, int weaponIndex) { }
//
//	void OnSelect(bool& selectable) { }
//	void OnGuardCommand() { }
//	void OnStopCommand() { }
//	void OnDeploy() { }
//
//	bool CanBeTargeted(WeaponTypeClass* pWeapon) const;
//	bool CanBePenetrated(WarheadTypeClass* pWarhead) const;
//
//	void OnTemporalUpdate(TemporalClass* pTemporal);
//	void OnUpdate();
//
//	void KillAnim();
//
//	static void SyncShieldToAnother(TechnoClass* pFrom, TechnoClass* pTo);
//
//	virtual void InvalidatePointer(void* ptr, bool bDetach);
//	bool Load(PhobosStreamReader& Stm, bool RegisterForChange);
//	bool Save(PhobosStreamWriter& Stm) const;
//
//private:
//	template <typename T>
//	bool Serialize(T& Stm);
//
//	void UpdateType();
//
//	void CreateAnim();
//	void UpdateIdleAnim();
//	AnimTypeClass* GetIdleAnimType() const;
//
//	void CloakCheck();
//	void OnlineCheck();
//	void TemporalCheck();
//	bool ConvertCheck();
//
//	/// Properties ///
//	TechnoClass* Techno;
//	TechnoTypeClass* CurTechnoType;
//	TimerStruct TimerDelay;
//	AnimClass* IdleAnim;
//	bool Cloak;
//	bool Online;
//	bool Temporal;
//	bool Available;
//	bool Attached;
//	bool AreAnimsHidden;
//	int LastBreakFrame;
//};
