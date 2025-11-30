#include "LaserStrike.h"

SuperWeaponFlags SW_LaserStrike::Flags(const SWTypeExtData* pData) const
{
	return SuperWeaponFlags::NoEvent | SuperWeaponFlags::NoMessage;
}

bool SW_LaserStrike::Activate(SuperClass* pThis, const CellStruct& Coords, bool IsPlayer)
{
	if (pThis->IsCharged) {
		const auto pData = SWTypeExtContainer::Instance.Find(pThis->Type);
		this->newStateMachine(Coords, pThis, this->GetFirer(pThis , Coords, false),
			ScenarioClass::Instance->Random.RandomRanged(pData->LaserStrikeMin, pData->LaserStrikeMax) ,
			pData->SW_Deferment ,
			pData->LaserStrikeDuration
		);
	}

	return true;
}

void SW_LaserStrike::Initialize(SWTypeExtData* pData)
{
	pData->This()->Action = Action(PhobosNewActionType::SuperWeaponAllowed);
	pData->SW_AITargetingMode = SuperWeaponAITargetingMode::Nuke;
}

void SW_LaserStrike::LoadFromINI(SWTypeExtData* pData, CCINIClass* pINI)
{
	const char* section = pData->This()->ID;

	INI_EX exINI(pINI);
	pData->LaserStrikeDuration.Read(exINI, section, "LaserStrike.Duration");
	pData->LaserStrikeRadius.Read(exINI, section, "LaserStrike.Radius");
	pData->LaserStrikeMax.Read(exINI, section, "LaserStrike.Max");
	pData->LaserStrikeMin.Read(exINI, section, "LaserStrike.Min");
	pData->LaserStrikeMaxRadius.Read(exINI, section, "LaserStrike.MaxRadius");
	pData->LaserStrikeMinRadius.Read(exINI, section, "LaserStrike.MinRadius");
	pData->LaserStrikeRadiusReduce.Read(exINI, section, "LaserStrike.RadiusReduce");
	pData->LaserStrikeRadiusReduceAcceleration.Read(exINI, section, "LaserStrike.RadiusReduceAccel");
	pData->LaserStrikeRadiusReduceMax.Read(exINI, section, "LaserStrike.RadiusReduceMax");
	pData->LaserStrikeRadiusReduceMin.Read(exINI, section, "LaserStrike.RadiusReduceMin");
	pData->LaserStrikeROF.Read(exINI, section, "LaserStrike.ROF");
	pData->LaserStrikeScatter_Max.Read(exINI, section, "LaserStrike.ScatterMax");
	pData->LaserStrikeScatter_Min.Read(exINI, section, "LaserStrike.ScatterMin");
	pData->LaserStrikeScatter_Max_IncreaseMax.Read(exINI, section, "LaserStrike.ScatterMaxIncreaseMax");
	pData->LaserStrikeScatter_Max_IncreaseMin.Read(exINI, section, "LaserStrike.ScatterMaxIncreaseMin");
	pData->LaserStrikeScatter_Max_Increase.Read(exINI, section, "LaserStrike.ScatterMaxIncrease");
	pData->LaserStrikeScatter_Min_IncreaseMax.Read(exINI, section, "LaserStrike.ScatterMinIncreaseMax");
	pData->LaserStrikeScatter_Min_IncreaseMin.Read(exINI, section, "LaserStrike.ScatterMinIncreaseMin");
	pData->LaserStrikeScatter_Min_Increase.Read(exINI, section, "LaserStrike.ScatterMinIncrease");
	pData->LaserStrikeLines.Read(exINI, section, "LaserStrike.LineCount");
	pData->LaserStrikeAngle.Read(exINI, section, "LaserStrike.Angle");
	pData->LaserStrikeAngleAcceleration.Read(exINI, section, "LaserStrike.AngleAccel");
	pData->LaserStrikeAngleMax.Read(exINI, section, "LaserStrike.AngleMax");
	pData->LaserStrikeAngleMin.Read(exINI, section, "LaserStrike.AngleMin");
	pData->LaserStrikeZeroRadius_Weapon.Read(exINI, section, "LaserStrike.ZeroRadiusWeapon" , true);
	pData->LaserStrikeInnerColor.Read(exINI, section, "LaserStrike.InnerColor");
	pData->LaserStrikeOuterColor.Read(exINI, section, "LaserStrike.OuterColor");
	pData->LaserStrikeOuterSpread.Read(exINI, section, "LaserStrike.OuterSpread");
	pData->LaserStrikeLaserDuration.Read(exINI, section, "LaserStrike.LaserDuration");
	pData->LaserStrikeLaserHeight.Read(exINI, section, "LaserStrike.LaserHeight");
	pData->LaserStrikeThickness.Read(exINI, section, "LaserStrike.LaserThickness");
	pData->LaserStrikeRate.Read(exINI, section, "LaserStrike.Rate");
}

int SW_LaserStrike::GetDamage(const SWTypeExtData* pData) const
{
	return pData->SW_Damage.Get(pData->This()->WeaponType->Damage);
}

bool SW_LaserStrike::IsLaunchSite(const SWTypeExtData* pData, BuildingClass* pBuilding) const
{
	if (!this->IsLaunchsiteAlive(pBuilding))
		return false;

	if (!pData->SW_Lauchsites.empty() && pData->SW_Lauchsites.Contains(pBuilding->Type))
		return true;

	return this->IsSWTypeAttachedToThis(pData, pBuilding);
}

void LaserStrikeStateMachine::Update()
{
	auto pData = this->GetTypeExtData();

	if (!this->AlreadyActivated)
	{
		pData->PrintMessage(pData->Message_Launch, this->Super->Owner);
		this->AlreadyActivated = true;
	}

	// waiting. lurking in the shadows.
	if (this->Deferment > 0)
	{
		if (--this->Deferment)
		{
			return;
		}
	}

	if (pData->LaserStrikeRadius >= 0)
	{
		// activation stuff
		pData->PrintMessage(pData->Message_Activate, Super->GetOwningHouse());

		auto const sound = pData->SW_ActivationSound.Get(-1);
		if (sound != -1)
		{
			VocClass::PlayGlobal(sound, Panning::Center, 1.0);
		}

		if (!(this->LaserStrikeRate > 0))
		{
			if (this->LaserStrikesetRadius)
			{
				this->LaserStrikeRadius = pData->LaserStrikeRadius;
				this->LaserStrikeRadiusReduce = pData->LaserStrikeRadiusReduce;
				this->LaserStrikeAngle = pData->LaserStrikeAngle;
				this->LaserStrikeScatter_Max = pData->LaserStrikeScatter_Max;
				this->LaserStrikeScatter_Min = pData->LaserStrikeScatter_Min;
				this->LaserStrikesetRadius = false;
			}

			CoordStruct center = CellClass::Cell2Coord(this->Coords);
			center.Z = MapClass::Instance->GetCellFloorHeight(center);

			if (this->LaserStrikeRadius >= 0 && !this->LaserStrikeStop)
			{
				int angleDelta = 360 / pData->LaserStrikeLines;

				for (int angle = this->LaserStrikeStartAngle;
					angle < this->LaserStrikeStartAngle + 360;
					angle += angleDelta)
				{
					int ScatterX = ScenarioClass::Instance->Random.RandomRanged(
						this->LaserStrikeScatter_Min, this->LaserStrikeScatter_Max);
					int ScatterY = ScenarioClass::Instance->Random.RandomRanged(
						this->LaserStrikeScatter_Min, this->LaserStrikeScatter_Max);

					if (ScenarioClass::Instance->Random.RandomFromMax(1))
						ScatterX = -ScatterX;

					if (ScenarioClass::Instance->Random.RandomFromMax(1))
						ScatterY = -ScatterY;

					CoordStruct pos
					{
						center.X + static_cast<int>(
							this->LaserStrikeRadius * std::cos(angle * Math::GAME_PI / 180)) + ScatterX,
						center.Y + static_cast<int>(
							this->LaserStrikeRadius * std::sin(angle * Math::GAME_PI / 180)) + ScatterY,
						0
					};

					CoordStruct posAir = pos + CoordStruct { 0, 0, pData->LaserStrikeLaserHeight };
					auto pCell = MapClass::Instance->TryGetCellAt(pos);

					if (pCell)
						pos.Z = pCell->GetCoordsWithBridge().Z;
					else
						pos.Z = MapClass::Instance->GetCellFloorHeight(pos);

					LaserDrawClass* pLaser = GameCreate<LaserDrawClass>
						(
							posAir,
							pos,
							pData->LaserStrikeInnerColor,
							pData->LaserStrikeOuterColor,
							pData->LaserStrikeOuterSpread,
							pData->LaserStrikeLaserDuration
						);

					pLaser->Thickness = pData->LaserStrikeThickness; // only respected if IsHouseColor
					pLaser->IsHouseColor = true;

					if (this->LaserStrikeROF <= 0)
					{
						if (auto pWeapon = pData->This()->WeaponType)
							WeaponTypeExtData::DetonateAt4(pWeapon, pos, Firer, pData->GetNewSWType()->GetDamage(pData), false, Super->Owner);
					}
				}

				if (this->LaserStrikeROF > 0)
					this->LaserStrikeROF--;
				else
					this->LaserStrikeROF = pData->LaserStrikeROF;

				if (this->LaserStrikeRadiusReduce <= pData->LaserStrikeRadiusReduceMax
					&& this->LaserStrikeRadiusReduce >= pData->LaserStrikeRadiusReduceMin)
					this->LaserStrikeRadiusReduce += pData->LaserStrikeRadiusReduceAcceleration;

				if (this->LaserStrikeAngle <= pData->LaserStrikeAngleMax
					&& this->LaserStrikeAngle >= pData->LaserStrikeAngleMin)
					this->LaserStrikeAngle += pData->LaserStrikeAngleAcceleration;

				if (this->LaserStrikeScatter_Max <= pData->LaserStrikeScatter_Max_IncreaseMax
					&& this->LaserStrikeScatter_Max >= pData->LaserStrikeScatter_Max_IncreaseMin)
					this->LaserStrikeScatter_Max += pData->LaserStrikeScatter_Max_Increase;

				if (this->LaserStrikeScatter_Min <= pData->LaserStrikeScatter_Min_IncreaseMax
					&& this->LaserStrikeScatter_Min >= pData->LaserStrikeScatter_Min_IncreaseMin)
					this->LaserStrikeScatter_Min += pData->LaserStrikeScatter_Min_Increase;

				this->LaserStrikeRadius -= LaserStrikeRadiusReduce;
				this->LaserStrikeStartAngle -= LaserStrikeAngle;

				if (pData->LaserStrikeMaxRadius >= 0)
				{
					if (this->LaserStrikeRadius > pData->LaserStrikeMaxRadius)
						this->LaserStrikeStop = true;
				}

				if (pData->LaserStrikeMinRadius >= 0)
				{
					if (this->LaserStrikeRadius < pData->LaserStrikeMinRadius)
						this->LaserStrikeStop = true;
				}
			}
			else
			{
				this->LaserStrikesetRadius = true;
				this->LaserStrikeStop = false;
				this->LaserStrikeRate = pData->LaserStrikeRate;

				if (pData->SW_RadarEvent)
				{
					RadarEventClass::Create(
						RadarEventType::SuperweaponActivated, this->Coords);
				}

				if (pData->LaserStrikeZeroRadius_Weapon)
					WeaponTypeExtData::DetonateAt4(pData->LaserStrikeZeroRadius_Weapon, center, Firer, pData->LaserStrikeZeroRadius_Weapon->Damage, false, Super->Owner);

				if (this->MaxCount > 0){
					this->MaxCountCounter--;
					if (this->MaxCountCounter <= 0){
						this->Clock.Stop();
						return;
					}
				}
			}
		}
		else
		{
			this->LaserStrikeRate--;
		}
	}
}

bool LaserStrikeStateMachine::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	return SWStateMachine::Load(Stm, RegisterForChange)
		&& Stm
		.Process(Firer, RegisterForChange)
		.Process(LaserStrikesetRadius)
		.Process(LaserStrikeRadius)
		.Process(LaserStrikeStartAngle)
		.Process(LaserStrikeStop)
		.Process(LaserStrikeRate)
		.Process(LaserStrikeROF)
		.Process(LaserStrikeRadiusReduce)
		.Process(LaserStrikeAngle)
		.Process(LaserStrikeScatter_Max)
		.Process(LaserStrikeScatter_Min)
		.Process(LaserStrikeDuration)
		.Process(AlreadyActivated)
		.Process(Deferment)
		.Process(MaxCount)
		.Process(MaxCountCounter)
		.Success();
}

bool LaserStrikeStateMachine::Save(PhobosStreamWriter& Stm) const
{
	return SWStateMachine::Save(Stm)
		&& Stm
		.Process(Firer)
		.Process(LaserStrikesetRadius)
		.Process(LaserStrikeRadius)
		.Process(LaserStrikeStartAngle)
		.Process(LaserStrikeStop)
		.Process(LaserStrikeRate)
		.Process(LaserStrikeROF)
		.Process(LaserStrikeRadiusReduce)
		.Process(LaserStrikeAngle)
		.Process(LaserStrikeScatter_Max)
		.Process(LaserStrikeScatter_Min)
		.Process(LaserStrikeDuration)
		.Process(AlreadyActivated)
		.Process(Deferment)
		.Process(MaxCount)
		.Process(MaxCountCounter)
		.Success();
}

void LaserStrikeStateMachine::InvalidatePointer(AbstractClass* ptr, bool remove)
{
	AnnounceInvalidPointer(Firer, ptr, remove);
}
