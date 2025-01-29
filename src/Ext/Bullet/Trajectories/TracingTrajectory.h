#include "PhobosTrajectory.h"

enum class TraceTargetMode
{
	Connection = 0,
	Global = 1,
	Body = 2,
	Turret = 3,
	RotateCW = 4,
	RotateCCW = 5,
};

class TracingTrajectoryType final : public PhobosTrajectoryType
{
public:

	TracingTrajectoryType(TrajectoryFlag variant) : PhobosTrajectoryType { variant }
	{ }

	TracingTrajectoryType() : PhobosTrajectoryType { TrajectoryFlag::Tracing }
	{ }

	virtual ~TracingTrajectoryType() = default;
	virtual void InvalidatePointer(AbstractClass* ptr, bool bRemoved) override { };
	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;
	virtual bool Save(PhobosStreamWriter& Stm) const override;

	virtual bool Read(CCINIClass* const pINI, const char* pSection) override;

	Valueable<TraceTargetMode> TraceMode { TraceTargetMode::Connection };
	Valueable<int> TheDuration { 0 };
	Valueable<int> TolerantTime { -1 };
	Valueable<int> ROT { -1 };
	Valueable<bool> BulletSpin {};
	Valueable<bool> PeacefullyVanish {};
	Valueable<bool> TraceTheTarget { true };
	Valueable<bool> CreateAtTarget {};
	Valueable<CoordStruct> CreateCoord {};
	Valueable<CoordStruct> OffsetCoord {};
	Valueable<CoordStruct> WeaponCoord {};
	ValueableVector<WeaponTypeClass*> Weapons {};
	ValueableVector<int> WeaponCount {};
	ValueableVector<int> WeaponDelay {};
	Valueable<int> WeaponInitialDelay {};
	Valueable<int> WeaponCycle { -1 };
	Valueable<bool> WeaponCheck {};
	Valueable<bool> Synchronize { true };
	Valueable<bool> SuicideAboveRange {};
	Valueable<bool> SuicideIfNoWeapon {};

private:
	template <typename T>
	bool Serialize(T& Stm);
};

class TracingTrajectory final : public PhobosTrajectory
{
public:

	TracingTrajectory() : PhobosTrajectory { TrajectoryFlag::Tracing } { }
	TracingTrajectory(BulletClass* pBullet, PhobosTrajectoryType* pType) :
		PhobosTrajectory { TrajectoryFlag::Tracing , pBullet,  pType }
	{ }
	virtual ~TracingTrajectory() = default;

	virtual void InvalidatePointer(AbstractClass* ptr, bool bRemoved) override { }
	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;
	virtual bool Save(PhobosStreamWriter& Stm) const override;

	virtual TracingTrajectoryType* GetTrajectoryType() const { return reinterpret_cast<TracingTrajectoryType*>(PhobosTrajectory::GetTrajectoryType()); }

	virtual void OnUnlimbo(CoordStruct* pCoord, VelocityClass* pVelocity) override;
	virtual bool OnAI() override;
	virtual void OnAIPreDetonate() override;
	virtual void OnAIVelocity(VelocityClass* pSpeed, VelocityClass* pPosition) override;
	virtual TrajectoryCheckReturnType OnAITargetCoordCheck(CoordStruct& coords) override;
	virtual TrajectoryCheckReturnType OnAITechnoCheck(TechnoClass* pTechno) override;

	int WeaponIndex {};
	int WeaponCount {};
	int WeaponCycle {};
	CDTimerClass ExistTimer {};
	CDTimerClass WeaponTimer {};
	CDTimerClass TolerantTimer {};
	bool TechnoInTransport {};
	bool NotMainWeapon {};
	CoordStruct FLHCoord {};
	CoordStruct BuildingCoord {};
	double FirepowerMult { 1.0 };

private:
	template <typename T>
	bool Serialize(T& Stm);

	void GetTechnoFLHCoord(TechnoClass* pTechno);
	void SetSourceLocation();
	inline void InitializeDuration(int duration);
	inline bool InvalidFireCondition(TechnoClass* pTechno);
	bool BulletDetonatePreCheck();
	void ChangeFacing();
	bool CheckFireFacing();
	VelocityClass ChangeVelocity();
	AbstractClass* GetBulletTarget(TechnoClass* pTechno, HouseClass* pOwner, WeaponTypeClass* pWeapon);
	CoordStruct GetWeaponFireCoord(TechnoClass* pTechno);
	bool PrepareTracingWeapon();
	void CreateTracingBullets(WeaponTypeClass* pWeapon);
};