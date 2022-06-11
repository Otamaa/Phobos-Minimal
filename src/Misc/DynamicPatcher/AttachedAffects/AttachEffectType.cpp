#include "AttachEffectType.h"

Enumerable<AttachEffectType>::container_t Enumerable<AttachEffectType>::Array;

const char* Enumerable<AttachEffectType>::GetMainSection()
{
	return "AETypes";
}

void AttachEffectType::LoadFromINI(CCINIClass* pINI)
{
	const char* pSection = this->Name;
	INI_EX exINI(pINI);

	#pragma region CommonAE

	if (exINI.ReadString(pSection, "AffectTypes"))
	{
		char* context = nullptr;
		for (char* cur = strtok_s(exINI.value(),
			Phobos::readDelims, &context);
			cur;
			cur = strtok_s(nullptr, Phobos::readDelims,
			&context)) {
			AffectTypes.push_back(cur);
		}
	}

	if (exINI.ReadString(pSection, "NotAffectTypes"))
	{
		char* context = nullptr;
		for (char* cur = strtok_s(exINI.value(),
			Phobos::readDelims, &context);
			cur;
			cur = strtok_s(nullptr, Phobos::readDelims,
			&context)) {
			NotAffectTypes.push_back(cur);
		}
	}

	Duration.Read(exINI, pSection, "Duration");
	if (Duration > 0)
		HoldDuration = false;
	HoldDuration.Read(exINI, pSection, "HoldDuration");
	ResetDurationOnReapply.Read(exINI, pSection, "ResetDurationOnReapply");
	Delay.Read(exINI, pSection, "Delay");

	Nullable<Point2D> Dummy_RandomDelay;
	Dummy_RandomDelay.Read(exINI, pSection, "RandomDelay");

	if (Dummy_RandomDelay.isset() && (abs(Dummy_RandomDelay.Get().X) > 0 || abs(Dummy_RandomDelay.Get().Y) > 0))
	{
		RandomDelay = true;
		MinDelay = abs(Dummy_RandomDelay.Get().X);
		MaxDelay = abs(Dummy_RandomDelay.Get().Y);

		if (MinDelay > MaxDelay)
			std::swap(MinDelay, MaxDelay);
	}

	InitialDelay.Read(exINI, pSection, "InitialDelay");

	Nullable<Point2D> Dummy_InitialRandomDelay;
	Dummy_InitialRandomDelay.Read(exINI, pSection, "InitialRandomDelay");

	if (Dummy_InitialRandomDelay.isset() && (abs(Dummy_InitialRandomDelay.Get().X) > 0 || abs(Dummy_InitialRandomDelay.Get().Y) > 0))
	{
		InitialRandomDelay = true;
		InitialMinDelay = abs(Dummy_InitialRandomDelay.Get().X);
		InitialMaxDelay = abs(Dummy_InitialRandomDelay.Get().Y);

		if (InitialMinDelay > InitialMaxDelay)
			std::swap(InitialMinDelay, InitialMaxDelay);
	}

	DiscardOnEntry.Read(exINI, pSection, "DiscardOnEntry");
	PenetratesIronCurtain.Read(exINI, pSection, "PenetratesIronCurtain");
	FromTransporter.Read(exINI, pSection, "FromTransporter");
	OwnerTarget.Read(exINI, pSection, "OwnerTarget");

	if (exINI.ReadString(pSection, "Cumulative"))
	{
		if (IS_SAME_STR_("Yes", exINI.value()))
			Cumulative = CumulativeMode::YES;
		else if (IS_SAME_STR_("No", exINI.value()))
			Cumulative = CumulativeMode::NO;
		else if (IS_SAME_STR_("Attacker", exINI.value()))
			Cumulative = CumulativeMode::ATTACKER;
	}

	Group.Read(exINI, pSection, "Group");
	OverrideSameGroup.Read(exINI, pSection, "OverrideSameGroup");
	Next.Read(pINI, pSection, "Next", "<none>");

	// 赋予出厂单位时只赋予一次
	AttachOnceInTechnoType.Read(exINI, pSection, "AttachOnceInTechnoType");
	// 赋予对象过滤
	AttachWithDamage.Read(exINI, pSection, "AttachWithDamage");
	AffectBullet.Read(exINI, pSection, "AffectBullet");
	OnlyAffectBullet.Read(exINI, pSection, "OnlyAffectBullet");
	AffectMissile.Read(exINI, pSection, "AffectMissile");
	AffectTorpedo.Read(exINI, pSection, "AffectTorpedo");
	AffectCannon.Read(exINI, pSection, "AffectCannon");
	#pragma endregion

	#pragma region Anim
	AnimationTypeData.IdleAnim.Read(exINI, pSection, "Animation");
	AnimationTypeData.ActiveAnim.Read(exINI, pSection, "ActiveAnim");
	AnimationTypeData.HitAnim.Read(exINI, pSection, "HitAnim");
	AnimationTypeData.DoneAnim.Read(exINI, pSection, "DoneAnim");
	#pragma endregion

	#pragma region Stat
	AttachStatusTypeData.FirepowerMultiplier.Read(exINI, pSection, "Status.FirepowerMultiplier");
	AttachStatusTypeData.ArmorMultiplier.Read(exINI, pSection, "Status.ArmorMultiplier");
	AttachStatusTypeData.SpeedMultiplier.Read(exINI, pSection, "Status.SpeedMultiplier");
	AttachStatusTypeData.ROFMultiplier.Read(exINI, pSection, "Status.ROFMultiplier");
	AttachStatusTypeData.Cloakable.Read(exINI, pSection, "Status.Cloakable");
	AttachStatusTypeData.ForceDecloak.Read(exINI, pSection, "Status.ForceDecloak");
	#pragma endregion

	#pragma region Stand
	StandTypeData.Type.Read(exINI.GetINI(), pSection, "Stand.Type",NONE_STR);
	StandTypeData.Offset.Read(exINI, pSection, "Stand.Offset");
	StandTypeData.Direction.Read(exINI, pSection, "Stand.Direction");
	StandTypeData.Direction = StandTypeData.Direction.Get() * 2;
	StandTypeData.LockDirection.Read(exINI, pSection, "Stand.LockDirection");
	StandTypeData.IsOnTurret.Read(exINI, pSection, "Stand.IsOnTurret");
	StandTypeData.IsOnWorld.Read(exINI, pSection, "Stand.IsOnWorld");
	StandTypeData.DrawLayer.Read(exINI, pSection, "Stand.DrawLayer");
	StandTypeData.ZOffset.Read(exINI, pSection, "Stand.ZOffset");
	StandTypeData.Powered.Read(exINI, pSection, "Stand.SameHouse");
	StandTypeData.SameHouse.Read(exINI, pSection, "Stand.SameTarget");
	StandTypeData. SameTarget.Read(exINI, pSection, "Stand.SameLoseTarget");
	StandTypeData. SameLoseTarget.Read(exINI, pSection, "Stand.ForceAttackMaster");
	StandTypeData. ForceAttackMaster.Read(exINI, pSection, "Stand.MobileFire");
	StandTypeData. MobileFire.Read(exINI, pSection, "Stand.Powered");
	StandTypeData. Explodes.Read(exINI, pSection, "Stand.Explodes");
	StandTypeData. ExplodesWithMaster.Read(exINI, pSection, "Stand.ExplodesWithMaster");
	StandTypeData. RemoveAtSinking.Read(exINI, pSection, "Stand.RemoveAtSinking");
	StandTypeData. PromoteFormMaster.Read(exINI, pSection, "Stand.PromoteFormMaster");
	double experienceToMaster = 0.0;
	StandTypeData.ExperienceToMaster.Read(exINI, pSection, "Stand.ExperienceToMaster");
	if (StandTypeData.ExperienceToMaster > 1.0)
	{
		experienceToMaster = 1.0;
	}
	else if (StandTypeData.ExperienceToMaster < 0.0)
	{
		experienceToMaster = 0.0;
	}
	StandTypeData.ExperienceToMaster = experienceToMaster;

	StandTypeData. VirtualUnit.Read(exINI, pSection, "Stand.VirtualUnit");
	StandTypeData. SameTilter.Read(exINI, pSection, "Stand.SameTilter");
	StandTypeData. IsTrain.Read(exINI, pSection, "Stand.IsTrain");
	StandTypeData. CabinHead.Read(exINI, pSection, "Stand.IsCabinHead");
	StandTypeData.CabinGroup.Read(exINI, pSection, "Stand.CabinGroup");
	#pragma endregion
	/*#pragma region AutoWeapon
	AutoWeaponTypeData.WeaponIndex.Read(exINI, pSection, "AutoWeapon.WeaponIndex");
	AutoWeaponTypeData.WeaponIndex.Read(exINI, pSection, "AutoWeapon.EliteWeaponIndex");

	if (!AutoWeaponTypeData.EliteWeaponIndex.isset())
		AutoWeaponTypeData.EliteWeaponIndex = AutoWeaponTypeData.WeaponIndex.Get();

	AutoWeaponTypeData.WeaponTypes.Read(exINI, pSection, "AutoWeapon.Types");
	AutoWeaponTypeData.EliteWeaponTypes.Read(exINI, pSection, "AutoWeapon.EliteTypes");

	if (AutoWeaponTypeData.EliteWeaponTypes.empty() && !AutoWeaponTypeData.WeaponTypes.empty())
	{
		for (auto const& data : AutoWeaponTypeData.WeaponTypes)
			AutoWeaponTypeData.EliteWeaponTypes.push_back(data);

		AutoWeaponTypeData.EliteWeaponTypes.SetHasValue(true);
	}

	AutoWeaponTypeData.RandomTypesNum.Read(exINI, pSection, "AutoWeapon.RandomTypesNum");
	AutoWeaponTypeData.EliteRandomTypesNum.Read(exINI, pSection, "AutoWeapon.EliteRandomTypesNum");

	if (!AutoWeaponTypeData.EliteRandomTypesNum.isset())
		AutoWeaponTypeData.EliteRandomTypesNum = AutoWeaponTypeData.RandomTypesNum.Get();

	AutoWeaponTypeData.FireOnce.Read(exINI, pSection, "AutoWeapon.FireOnce");
	AutoWeaponTypeData.FireFLH.Read(exINI, pSection, "AutoWeapon.FireFLH");
	AutoWeaponTypeData.EliteFireFLH.Read(exINI, pSection, "AutoWeapon.EliteFireFLH");

	if(!AutoWeaponTypeData.EliteFireFLH.isset())
		AutoWeaponTypeData.EliteFireFLH  = AutoWeaponTypeData.FireFLH.Get();

	AutoWeaponTypeData.TargetFLH.Read(exINI, pSection, "AutoWeapon.TargetFLH");
	AutoWeaponTypeData.EliteTargetFLH.Read(exINI, pSection, "AutoWeapon.EliteTargetFLH");

	if(!AutoWeaponTypeData.EliteTargetFLH.isset())
		AutoWeaponTypeData.EliteTargetFLH  = AutoWeaponTypeData.TargetFLH.Get();

	AutoWeaponTypeData.TargetFLH.Read(exINI, pSection, "AutoWeapon.TargetFLH");
	AutoWeaponTypeData.EliteTargetFLH.Read(exINI, pSection, "AutoWeapon.EliteTargetFLH");

	AutoWeaponTypeData.MoveTo.Read(exINI, pSection, "AutoWeapon.MoveTo");
	AutoWeaponTypeData.EliteMoveTo.Read(exINI, pSection, "AutoWeapon.EliteMoveTo");

	if (AutoWeaponTypeData.MoveTo.isset())
	{
		if(!AutoWeaponTypeData.EliteMoveTo.isset())
		AutoWeaponTypeData.EliteMoveTo = AutoWeaponTypeData.MoveTo.Get();
		AutoWeaponTypeData.TargetFLH = AutoWeaponTypeData.FireFLH.Get() + AutoWeaponTypeData.MoveTo.Get();
	}

	if(AutoWeaponTypeData.EliteMoveTo.isset())
		AutoWeaponTypeData.EliteTargetFLH = AutoWeaponTypeData.EliteFireFLH.Get() + AutoWeaponTypeData.EliteMoveTo.Get();

	AutoWeaponTypeData.FireToTarget.Read(exINI, pSection, "AutoWeapon.FireToTarget");
	AutoWeaponTypeData.IsOnTurret.Read(exINI, pSection, "AutoWeapon.IsOnTurret");

	AutoWeaponTypeData.IsOnWorld.Read(exINI, pSection, "AutoWeapon.IsOnWorld");
	AutoWeaponTypeData.IsAttackerMark.Read(exINI, pSection, "AutoWeapon.IsAttackerMark");
	AutoWeaponTypeData.ReceiverAttack.Read(exINI, pSection, "AutoWeapon.ReceiverAttack");
	AutoWeaponTypeData.ReceiverOwnBullet = AutoWeaponTypeData.ReceiverAttack;
	AutoWeaponTypeData.ReceiverOwnBullet.Read(exINI, pSection, "AutoWeapon.ReceiverOwnBullet");
	#pragma endregion

	#pragma region Blackhole
	BlackHoleTypeData.CommonData.Read(exINI, pSection, "BlackHole.");
	BlackHoleTypeData.Range.Read(exINI, pSection, "BlackHole.Range");
	BlackHoleTypeData.EliteRange.Read(exINI, pSection, "BlackHole.EliteRange");

	if(!BlackHoleTypeData.EliteRange.isset())
		BlackHoleTypeData.EliteRange = BlackHoleTypeData.Range.Get();

	BlackHoleTypeData.Rate.Read(exINI, pSection, "BlackHole.Rate");
	BlackHoleTypeData.EliteRate.Read(exINI, pSection, "BlackHole.EliteRate");

	if(!BlackHoleTypeData.EliteRate.isset())
	BlackHoleTypeData.EliteRate = BlackHoleTypeData.Rate.Get();

	BlackHoleTypeData.AffectTypes.Read(exINI, pSection, "BlackHole.AffectTypes");
	BlackHoleTypeData.NotAffectTypes.Read(exINI, pSection, "BlackHole.NotAffectTypes");
	BlackHoleTypeData.AffectTechno.Read(exINI, pSection, "BlackHole.AffectTechno");
	BlackHoleTypeData.OnlyAffectTechno.Read(exINI, pSection, "BlackHole.OnlyAffectTechno");
	BlackHoleTypeData.AffectMissile.Read(exINI, pSection, "BlackHole.AffectMissile");
	BlackHoleTypeData.AffectTorpedo.Read(exINI, pSection, "BlackHole.AffectTorpedo");
	BlackHoleTypeData.AffectCannon.Read(exINI, pSection, "BlackHole.AffectCannon");
	BlackHoleTypeData.AffectsOwner.Read(exINI, pSection, "BlackHole.AffectsOwner");
	BlackHoleTypeData.AffectsAllies.Read(exINI, pSection, "BlackHole.AffectsAllies");
	BlackHoleTypeData.AffectsEnemies.Read(exINI, pSection, "BlackHole.AffectsEnemies");
	#pragma endregion

	#pragma region DestroySelf
	DestroySelfTypeData.CommonData.Read(exINI, pSection, "DestroySelf.");
	DestroySelfTypeData.Delay.Read(exINI, pSection, "DestroySelf.Delay");
	DestroySelfTypeData.Enable = DestroySelfTypeData.Delay > 0;
	DestroySelfTypeData.Peaceful.Read(exINI, pSection, "DestroySelf.Peaceful");
	#pragma endregion

	#pragma region DisableWeapon
	DisableWeaponTypeData.CommonData.Read(exINI, pSection, "Weapon.");
	DisableWeaponTypeData.WeaponDisable.Read(exINI, pSection, "Weapon.Disable");
	#pragma endregion

	#pragma region LauchSw
	FireSuperTypeData.CommonData.Read(exINI, pSection,"FireSuperWeapon.");
	FireSuperTypeData.Data.Read(exINI, pSection);
	FireSuperTypeData.EliteData.Read(exINI, pSection, true);
	#pragma endregion

	#pragma region GiftBox
	GiftBoxTypeData.CommonData.Read(exINI, pSection, "GiftBox.");
	GiftBoxTypeData.Gifts.Read(exINI, pSection, "GiftBox.Types");
	if (!GiftBoxTypeData.Gifts.empty())
	{
		GiftBoxTypeData.Enable = true;
		auto const nBaseSize = (int)GiftBoxTypeData.Gifts.size();
		GiftBoxTypeData.Nums.Clear();
		GiftBoxTypeData.Nums.Reserve(nBaseSize);
		GiftBoxTypeData.Nums.Count = nBaseSize;
		auto const pNumKey = "GiftBox.Nums";

		for (auto& nSpawnMult : GiftBoxTypeData.Nums)
			nSpawnMult = 1;

		if (exINI.ReadString(pSection, pNumKey))
		{
			int nCount = 0;
			char* context = nullptr;
			for (char* cur = strtok_s(exINI.value(), Phobos::readDelims, &context); cur; cur = strtok_s(nullptr, Phobos::readDelims, &context))
			{
				if (Phobos::Config::MoreDetailSLDebugLog)
					Debug::Log("Parsing %d Size of [%s]%s=%s idx[%d] \n", nBaseSize, pSection, pNumKey, cur, nCount);

				int buffer = 1;
				if (Parser<int>::TryParse(cur, &buffer))
					GiftBoxTypeData.Nums[nCount] = buffer;

				if (++nCount >= nBaseSize)
					break;
			}
		}

		GiftBoxTypeData.RandomWeights.Read(exINI, pSection, "GiftBox.RandomWeights");
		GiftBoxTypeData.Chances.Read(exINI, pSection, "GiftBox.Chances");
		GiftBoxTypeData.Remove.Read(exINI, pSection, "GiftBox.Remove");
		GiftBoxTypeData.Destroy.Read(exINI, pSection, "GiftBox.Explodes");
		GiftBoxTypeData.Delay.Read(exINI, pSection, "GiftBox.Delay");
		GiftBoxTypeData.RandomDelay.Read(exINI, pSection, "GiftBox.RandomDelay");

		if (GiftBoxTypeData.RandomDelay.isset() &&
			(abs(GiftBoxTypeData.RandomDelay.Get().X) > 0 || abs(GiftBoxTypeData.RandomDelay.Get().Y) > 0))
		{
			int DelayMin = abs(GiftBoxTypeData.RandomDelay.Get().X);
			int DelayMax = abs(GiftBoxTypeData.RandomDelay.Get().Y);

			if (DelayMin > DelayMax)
				std::swap(DelayMin, DelayMax);

			GiftBoxTypeData.RandomDelay = Point2D(DelayMin, DelayMax);
		}

		GiftBoxTypeData.RandomRange.Read(exINI, pSection, "GiftBox.RandomRange");
		GiftBoxTypeData.EmptyCell.Read(exINI, pSection, "GiftBox.RandomToEmptyCell");
		GiftBoxTypeData.RandomType.Read(exINI, pSection, "GiftBox.RandomType");
		GiftBoxTypeData.OpenWhenDestoryed.Read(exINI, pSection, "GiftBox.OpenWhenDestoryed");

	}
	#pragma endregion

	//OverrideWeapon
	*/

}

void AttachEffectType::LoadFromStream(PhobosStreamReader& Stm) { Debug::Log("Loading Element From AttachEffectType ! \n"); this->Serialize(Stm); }
void AttachEffectType::SaveToStream(PhobosStreamWriter& Stm) { this->Serialize(Stm); }