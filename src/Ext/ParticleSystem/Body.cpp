#include "Body.h"

#include <Ext/ParticleSystemType/Body.h>
#include <Ext/ParticleType/Body.h>

#include <Ext/Rules/Body.h>

#include <ParticleTypeClass.h>
#include <ParticleClass.h>
#include <SpotlightClass.h>

void ParticleSystemExtData::UpdateLocations()
{
	const auto gravity = (float)RulesClass::Instance->Gravity;
	for (auto& Data : this->OtherParticleData)
	{
		if (--Data.RemainingEC <= 0)
		{
			Data.Empty = true;
			continue;
		}

		Data.velB.Z -= gravity;

		Data.vel.X += Data.velB.X;
		Data.vel.Y += Data.velB.Y;

		CoordStruct nCoordBeforeZ { (int)Data.vel.X , (int)Data.vel.Y , (int)Data.vel.Z };
		Data.vel.Z = (Data.velB.Z - gravity) + Data.vel.Z;

		CoordStruct nVelBCoord = { (int)Data.velB.X , (int)Data.velB.Y , (int)Data.velB.Z };
		CoordStruct nCoord { (int)Data.vel.X , (int)Data.vel.Y , (int)Data.vel.Z };

		auto const pCell = MapClass::Instance->GetCellAt(nCoord);
		const auto nZOffs = pCell->GetFloorHeight({ nCoord.X , nCoord.Y });

		if (nCoord.Z < nZOffs)
		{
			Data.Empty = true;
			continue;
		}

		if (pCell->ContainsBridge() || MapClass::Instance->GetCellAt(nCoordBeforeZ)->ContainsBridge())
		{
			const auto nZOffsWithBridge = nZOffs + Unsorted::BridgeHeight;
			if (nCoord.Z >= nZOffsWithBridge)
			{

				if (nCoordBeforeZ.Z < nZOffsWithBridge)
					Data.Empty = true;

				continue;
			}
			else if (nCoordBeforeZ.Z >= nZOffsWithBridge)
			{
				Data.Empty = true;
				continue;
			}
		}

		if ((nCoord.Z - 150) < nZOffs)
		{
			auto const pBuilding = pCell->GetBuilding();

			if (!pBuilding)
			{
				if (pCell->ConnectsToOverlay())
				{
					Data.Empty = true;
				}

			}
			else
				if (!pBuilding->Type->LaserFence || pBuilding->LaserFenceFrame < 8u)
				{
					if (!pBuilding->CanDeploySlashUnload())
						Data.Empty = true;
				}
		}
	}
}

void ParticleSystemExtData::UpdateState()
{
	this->OtherParticleData.remove_all_if([](const auto& data)
 {
	 return data.Empty;
	});

	this->SmokeData.remove_all_if([](const auto& data)
 {
	 return data.DeleteOnStateLimit;
	});

}

void ParticleSystemExtData::UpdateColor()
{
	const auto pHeldType = this->HeldType;
	const auto colorCounts = pHeldType->ColorList.Count - 2;

	for (auto& Data : this->OtherParticleData)
	{
		const double v6 = ScenarioClass::Instance->Random.RandomDouble() * 0.05 +
			pHeldType->ColorSpeed + Data.ColorFactor;

		double v7 = 1.0;

		if (v6 <= 1.0)
		{
			v7 = v6;
		}
		else
		{
			if (Data.C < colorCounts)
			{
				v7 = 0.0;
				Data.C++;
			}
		}

		Data.ColorFactor = (float)v7;
	}
}

void ParticleSystemExtData::UpdateSpark()
{
	auto pOwner = this->AttachedToObject;
	auto SparkSpawnFrames = pOwner->SparkSpawnFrames;
	auto SparkSpawnNeg1 = SparkSpawnFrames - 1;

	if (SparkSpawnNeg1 >= 0)
	{
		pOwner->SparkSpawnFrames = SparkSpawnNeg1;
		if (pOwner->SparkSpawnFrames <= 0)
			pOwner->TimeToDie = true;

		auto random = &ScenarioClass::Instance->Random;

		auto first_randDouble = random->RandomDouble();

		if (first_randDouble < 0.3)
		{
			int spotRadius_dec = pOwner->SpotlightRadius - 3;
			if (spotRadius_dec < 17)
				spotRadius_dec = 17;

			pOwner->SpotlightRadius = spotRadius_dec;
		}
		else
			if (first_randDouble < 0.6)
			{
				int spotRadius_inc = pOwner->SpotlightRadius + 3;
				if (spotRadius_inc > 41)
					spotRadius_inc = 41;

				pOwner->SpotlightRadius = spotRadius_inc;
			}

		auto pType = pOwner->Type;
		if (!pOwner->SparkSpawnFrames || pType->SpawnSparkPercentage > random->RandomDouble())
		{
			int cap = 0;
			auto pHeld = this->HeldType;
			if (pType->ParticleCap >= 0)
				cap = pType->ParticleCap;

			auto halfcap = cap / 2;
			auto randomHalfCap = random->RandomFromMax(halfcap);
			auto resize = halfcap + randomHalfCap;

			this->OtherParticleData.reserve(resize + this->OtherParticleData.size());

			for (int i = resize; i > 0; --i)
			{
				auto nVelX = random->RandomFromMax(this->HeldType->XVelocity);
				auto nVelY = random->RandomFromMax(this->HeldType->YVelocity);
				auto nVelZ = this->HeldType->MinZVelocity + random->RandomFromMax(this->HeldType->ZVelocityRange);
				Vector3D<float> nVels { (float)nVelX  , (float)nVelY ,(float)nVelZ };
				auto nVelsMag = nVels.Length();
				auto nSpawnDir = pOwner->Type->SpawnDirection + nVels;
				auto nSpawnDirMag = nSpawnDir.Length();

				auto data = &this->OtherParticleData.emplace_back();

				if (nSpawnDirMag != 0.0)
				{
					const auto idkHere = (float)(1.0 / nSpawnDirMag);
					data->velB.X = float((nSpawnDir.X * idkHere));
					data->velB.Y = float((nSpawnDir.Y * idkHere));
					data->velB.Z = float((idkHere * nSpawnDir.Z));
				}

				data->velB.X = float(data->velB.X * nVelsMag);
				data->velB.Y = float(data->velB.Y * nVelsMag);
				data->velB.Z = float(data->velB.Z * nVelsMag);
				data->vel.X = float(pOwner->Location.X);
				data->vel.Y = float(pOwner->Location.Y);
				data->vel.Z = float(pOwner->Location.Z);
				data->RemainingEC = LOWORD(this->HeldType->MaxEC) + random->RandomFromMax(this->HeldType->MaxEC);

				if (this->HeldType->ColorList.Count)
				{
					if (this->HeldType->StartColor1 && this->HeldType->StartColor2)
					{
						data->Colors = ColorStruct::Interpolate(this->HeldType->StartColor1, this->HeldType->StartColor2, random->RandomDouble());
					}
					else
					{
						data->Colors = *this->HeldType->ColorList.Items;
					}
				}
			}
		}

		if (GameOptionsClass::Instance->DetailLevel == 2)
		{
			if (pOwner->SparkSpawnFrames == pType->SparkSpawnFrames &&
				!pType->OneFrameLight && pType->LightSize > 0
				)
			{
				GameCreate<SpotlightClass>(pOwner->Location, pType->LightSize);
			}
		}
	}

	this->UpdateLocations();
	this->UpdateState();
	this->UpdateColor();
}

void ParticleSystemExtData::UpdateRailgun()
{
	auto pOwnerObj = this->AttachedToObject;
	auto pOwnerObjType = pOwnerObj->Type;

	if (!pOwnerObj->TimeToDie && this->OtherParticleData.empty())
	{
		pOwnerObj->TimeToDie = true;
		auto nDifferenct = (pOwnerObj->Location - pOwnerObj->TargetCoords);
		auto nMagSquared = nDifferenct.Length();
		auto nMaxXY = CoordStruct { nDifferenct.X , nDifferenct.Y , 0 }.Length();
		auto nMagNeg = -nMagSquared;
		auto nMagCopy = nMagSquared >= ((double)nDifferenct.Z) ?
			((double)nDifferenct.Z) : nMagSquared;

		if (nMagCopy < nMagNeg)
			nMagCopy = nMagNeg;

		auto const nMaxXYNeg = -nMaxXY;

		auto nMaxXYCopy = nMaxXY >= ((double)nDifferenct.X) ?
			((double)nDifferenct.X) : nMagSquared;

		if (nMaxXYCopy < nMaxXYNeg)
			nMaxXYCopy = nMaxXYNeg;

		auto nSin = (float)Math::sin(nMagCopy / nMagSquared);
		auto nCos = (float)Math::cos(nMaxXYCopy / nMaxXY) * ((((nDifferenct.Y - nDifferenct.Y) >> 0x1F) & 0xFFFFFFFE) + 1);
		Matrix3D mtx {};
		mtx.MakeIdentity();
		mtx.RotateZ(nCos);
		mtx.RotateX(nSin);

		auto pHeldType = this->HeldType;
		auto nSpinDelta = pOwnerObjType->SpiralDeltaPerCoord;
		auto nVel = pHeldType->Velocity;
		auto nSpiralRadius = pOwnerObjType->SpiralRadius;
		auto nPositionPerturbationCoefficient = pOwnerObjType->PositionPerturbationCoefficient;
		auto nMovementPerturbationCoefficient = pOwnerObjType->MovementPerturbationCoefficient;
		auto nVelocityPerturbationCoefficient = pOwnerObjType->VelocityPerturbationCoefficient;
		size_t nDecidedsize = (size_t)(pOwnerObjType->ParticlesPerCoord * nMagSquared);
		this->OtherParticleData.resize(nDecidedsize);

		if (nDecidedsize > 0)
		{
			double nVal = 0.0;
			for (size_t i = 0; i < nDecidedsize; ++i)
			{
				auto nMovementPerturbationCoefficientneg = -nMovementPerturbationCoefficient;

				auto v91 = i / nDecidedsize;
				auto radians = v91 * nMagSquared * nSpinDelta;

				Vector3D<float> nDummy {
					0.0f,
					Math::cos((float)radians) ,
					nDummy.Z = Math::sin((float)radians)
				};

				auto nResult = Matrix3D::MatrixMultiply(mtx, nDummy);

				Vector3D<float> nDummy_d {
				float((pOwnerObj->TargetCoords.X * (1.0 - v91))
					+ (pOwnerObj->Location.X * v91)
					+ (ScenarioClass::Instance->Random.RandomDouble() * nPositionPerturbationCoefficient + nResult.X * nSpiralRadius))
				,
				float(((1.0 - v91) * pOwnerObj->TargetCoords.Y)
					+ float(v91 * pOwnerObj->Location.Y)
					+ (nSpiralRadius * nResult.Y + nPositionPerturbationCoefficient * ScenarioClass::Instance->Random.RandomDouble()))
				,
				float((pOwnerObj->Location.Z * v91)
					+ float(pOwnerObj->TargetCoords.Z * (1.0 - v91))
					+ (ScenarioClass::Instance->Random.RandomDouble() * nPositionPerturbationCoefficient + nResult.Z * nSpiralRadius))
				};

				CoordStruct nMovementDummy {
					int((ScenarioClass::Instance->Random.RandomDouble() * nMovementPerturbationCoefficient) + nResult.X),
					int((ScenarioClass::Instance->Random.RandomDouble() * nMovementPerturbationCoefficient) + nResult.Y),
					int((ScenarioClass::Instance->Random.RandomDouble() * nMovementPerturbationCoefficient) + nResult.Z)
				};

				const auto nMag = nMovementDummy.Length();
				Vector3D<float> nVelsC {};

				if (nMag != 0.0)
				{
					const auto idkHere = (float)(1.0 / nMag);
					nVelsC.X = nMovementDummy.X * idkHere;
					nVelsC.Y = nMovementDummy.Y * idkHere;
					nVelsC.Z = idkHere * nMovementDummy.Z;
				}

				const auto nRand_Double7 = ScenarioClass::Instance->Random.RandomDouble();

				auto nVelocityPerturbationCoefficient_copy = nVelocityPerturbationCoefficient;

				nVal = 0.5 * (nRand_Double7 + nVal) * nVelocityPerturbationCoefficient;
				if (nVal < nVelocityPerturbationCoefficient)
					nVelocityPerturbationCoefficient_copy = 0.5 * nRand_Double7;

				if (nMovementPerturbationCoefficientneg > nVelocityPerturbationCoefficient_copy)
					nVal = nMovementPerturbationCoefficientneg;

				auto Data = &this->OtherParticleData.emplace_back();
				Data->velB = nVelsC;
				Data->vel = nDummy_d;
				Data->A = float(nVel + nVal);
				Data->RemainingEC = LOWORD(pHeldType->MaxEC) + ScenarioClass::Instance->Random.RandomFromMax(9);

				if (pHeldType->ColorList.Count)
				{
					if (pHeldType->StartColor1 && pHeldType->StartColor2)
					{
						Data->Colors.Interpolate(pHeldType->StartColor1, pHeldType->StartColor2, ScenarioClass::Instance->Random.RandomDouble());
					}
					else
					{
						Data->Colors = *pHeldType->ColorList.Items;
					}
				}
			}

			if (pOwnerObjType->Laser)
			{
				GameCreate<LaserDrawClass>(
					pOwnerObj->Location,
					pOwnerObj->TargetCoords,
					0, 1u,
					pOwnerObjType->LaserColor,
					ColorStruct::Empty,
					ColorStruct::Empty,
					10, false, true, 0.5f, 0.1f);
			}
		}
	}
}

void ParticleSystemExtData::UpdateWindDirection()
{
	if ((Unsorted::CurrentFrame() & 1) != 0)
	{
		auto rand = &ScenarioClass::Instance->Random;

		for (auto& smoke : this->SmokeData)
		{
			if (!rand->RandomFromMax(3))
			{
				Point2D velBXY { (int)smoke.velB.X , (int)smoke.velB.Y };
				auto randDir = rand->RandomRanged(-1, 1);

				if (rand->RandomBool())
					velBXY.X += randDir;
				else
					velBXY.Y += randDir;

				if (velBXY.X >= -5)
				{
					if (velBXY.X > 5)
						velBXY.X = 5;
				}
				else
				{
					velBXY.X = -5;
				}

				smoke.velB.X = (float)velBXY.X;

				if (velBXY.Y >= -5)
				{
					if (velBXY.Y > 5)
						velBXY.Y = 5;
				}
				else
				{
					velBXY.Y = -5;
				}

				smoke.velB.Y = (float)velBXY.Y;
			}
		}

		for (auto& smoke : this->SmokeData)
		{
			auto pLink = smoke.LinkedParticleType;
			if (smoke.ImageFrame < pLink->EndStateAI)
			{
				if (!((smoke.StateAdvance + pLink->MaxEC - smoke.RemainingEC) % (smoke.StateAdvance % 2 + pLink->StateAIAdvance)
					|| (smoke.ImageFrame = smoke.ImageFrame + 1, smoke.ImageFrame + 1 < pLink->EndStateAI)))
				{
					if (pLink->DeleteOnStateLimit)
						smoke.DeleteOnStateLimit = 1;
				}
			}

			if (smoke.velB.Z > 3.0)
				smoke.velB.Z = smoke.velB.Z - pLink->Deacc;

			if (--smoke.RemainingEC <= 0)
				smoke.DeleteOnStateLimit = 1;
		}

		auto WindDir = RulesClass::Instance->WindDirection;

		for (auto& smoke : this->SmokeData)
		{
			const auto& [dirX, dirY] = ParticleTypeExtContainer::Instance.Find(smoke.LinkedParticleType)->WindMult[WindDir];
			CoordStruct velCpy = smoke.vel;
			smoke.vel.X += int(smoke.velB.X + smoke.LinkedParticleType->WindEffect * dirX);
			smoke.vel.Y += int(smoke.velB.Y + smoke.LinkedParticleType->WindEffect * dirY);
			smoke.vel.Z += int(smoke.velB.Z);

			const auto pCell = MapClass::Instance->GetCellAt(velCpy);

			if (pCell->ContainsBridge())
			{
				int z = pCell->GetFloorHeight({ velCpy.X , velCpy.Y }) + Unsorted::BridgeHeight;

				if (velCpy.Z < z && smoke.vel.Z >= (z - 260))
				{
					smoke.DeleteOnStateLimit = true;
				}
			}
		}
	}
}

void ParticleSystemExtData::UpdateSmoke()
{
	auto const pOwnerObj = this->AttachedToObject;
	auto const pOwnerObjType = pOwnerObj->Type;
	auto const pOwnerObj_Owner = pOwnerObj->Owner;

	if (pOwnerObj_Owner && pOwnerObj_Owner->AbstractFlags & AbstractFlags::Object)
	{
		auto coords = pOwnerObj_Owner->GetCoords();
		CoordStruct SpawnDistance = coords + pOwnerObj->SpawnDistanceToOwner;
		pOwnerObj->SetLocation(SpawnDistance);
	}

	this->UpdateWindDirection();
	auto rand = &ScenarioClass::Instance->Random;

	//if current state still going on
	//update it for the next particle

	for (int i = int(this->SmokeData.size()) - 1; i > 0; --i)
	{
		auto curData = &this->SmokeData[i];

		if (!curData->DeleteOnStateLimit)
			continue;

		//reuse the data for the next particle ?
		if (auto pNext = ParticleTypeClass::Array->GetItemOrDefault(curData->LinkedParticleType->NextParticle))
		{
			auto range = pNext->Radius << 8;
			auto movement = &this->SmokeData.emplace_back();

			movement->vel.X = curData->vel.X + pNext->NextParticleOffset.X + rand->RandomRanged(-range, range);
			movement->vel.Y = curData->vel.Y + pNext->NextParticleOffset.Y + rand->RandomRanged(-range, range);
			movement->vel.Z = curData->vel.Z + pNext->NextParticleOffset.Z;
			movement->RemainingEC = LOWORD(pNext->MaxEC) + rand->RandomFromMax(pNext->MaxEC);
			movement->Translucency = LOBYTE((!rand->RandomFromMax(5) ? 25 : 0) + curData->Translucency);
			movement->LinkedParticleType = pNext;
			movement->ImageFrame = pNext->StartStateAI;
		}
	}

	this->UpdateState();
	FootClass* Owner_obj = generic_cast<FootClass*>(pOwnerObj_Owner);

	//updating the current particle ?
	if (!pOwnerObj->TimeToDie && pOwnerObj->IsAlive)
	{
		if (!(Unsorted::CurrentFrame() % (int)pOwnerObj->SpawnFrames) && (!Owner_obj || Owner_obj->TubeIndex < 0))
		{
			if (auto pParticle = this->HeldType)
			{
				auto range = pOwnerObjType->SpawnRadius + 1;
				auto rand_x = rand->RandomRanged(-range, range);
				auto rand_y = rand->RandomRanged(-range, range);
				auto Loc = CoordStruct { rand_x , rand_y  , 10 } + pOwnerObj->Location;
				auto movement = &this->SmokeData.emplace_back();
				movement->vel = Loc;
				auto v47 = MapClass::Instance->GetCellFloorHeight(Loc);
				if (v47 < Loc.Z)
					v47 = Loc.Z;

				movement->vel.Z = v47;
				movement->Translucency = LOBYTE(pParticle->Translucency);

				const auto frames = pOwnerObj->SpawnFrames;
				if (frames > (double)pOwnerObjType->SpawnTranslucencyCutoff)
				{
					movement->Translucency = LOBYTE(pParticle->Translucency) + 25u;
				}

				const auto vel_ = pParticle->Velocity - (frames - (float)pOwnerObjType->SpawnFrames) * 0.35;
				movement->velB.Z = float(vel_ < 2.0 ? 2.0 : vel_);
				movement->StateAdvance = this->SmokeData.size() + pOwnerObj->Fetch_ID();
				movement->RemainingEC = LOWORD(pParticle->MaxEC) + rand->RandomFromMax(pParticle->MaxEC);
				movement->ImageFrame = pParticle->StartStateAI;
				movement->LinkedParticleType = pParticle;
				movement->DeleteOnStateLimit = false;
			}
		}
	}

	auto spawn__ = pOwnerObjType->Slowdown + pOwnerObj->SpawnFrames;
	pOwnerObj->SpawnFrames = spawn__;

	if (spawn__ > (double)pOwnerObjType->SpawnCutoff)
		pOwnerObj->TimeToDie = true;
}

bool ParticleSystemExtData::UpdateHandled()
{
	switch (this->What)
	{
	case Behave::Spark:
		this->UpdateSpark();
		break;
	case Behave::Railgun:
		this->UpdateRailgun();
		break;
	case Behave::Smoke:
		this->UpdateSmoke();
		break;
	default:
		return false;
	}

	const auto pOwner = this->AttachedToObject;

	if (pOwner->Lifetime-- == 1)
		pOwner->TimeToDie = true;

	if (pOwner->IsAlive
		&& pOwner->TimeToDie
		&& !pOwner->Particles.Count
		&& this->OtherParticleData.empty()
		&& this->SmokeData.empty())
	{
		pOwner->Limbo();
		pOwner->IsAlive = false;
		AbstractClass::Array2->AddItem(pOwner);
	}

	return true;
}

void ParticleSystemExtData::UpdateInAir_Main(bool allowDraw)
{
	const auto pHeldType = this->HeldType;
	ColorStruct* color = pHeldType ? pHeldType->ColorList.Items : nullptr;
	auto& rect = DSurface::ViewBounds;

	for (auto& movement : this->OtherParticleData)
	{
		CoordStruct Coord = { (int)movement.vel.X ,(int)movement.vel.Y ,(int)movement.vel.Z };

		if (allowDraw || !MapClass::Instance->IsLocationShrouded(Coord))
		{
			Point2D outClient {};
			TacticalClass::Instance->CoordsToClient(&Coord, &outClient);

			const auto y_copy = rect->Y + outClient.Y;
			outClient.Y += rect->Y;
			if (outClient.X >= rect->X
				&& outClient.X < rect->X + rect->Width
				&& y_copy >= rect->Y
				&& y_copy < rect->X + rect->Height
				)
			{
				uintptr_t buff = *reinterpret_cast<uint16_t*>(ABuffer::Instance->GetBuffer(outClient.X, y_copy - ABuffer::Instance->Area.Y));

				if ((uint16_t)buff)
				{
					int ZBuff = *reinterpret_cast<uint16_t*>((ZBuffer::Instance->GetBuffer(outClient.X, outClient.Y - ZBuffer::Instance->Area.Y)));
					int Zadjust = Game::AdjustHeight(Coord.Z);

					if ((uint16_t)(LOWORD(ZBuffer::Instance->Area.Y) + LOWORD(ZBuffer::Instance->MaxValue) - LOWORD(outClient.Y)) - Zadjust - 50 < ZBuff)
					{

						int idx = 0;
						ColorStruct* selected = &movement.Colors;

						if (movement.C)
						{
							idx = movement.C;
							selected = &color[movement.C];
						}

						ColorStruct emp = ColorStruct::Interpolate(&(color[idx + 1]), selected, (double)movement.ColorFactor);

						if ((uint16_t)buff >= 127u)
						{
							DSurface::Temp->Put_Pixel(outClient, DSurface::RGBA_To_Pixel(emp.R, emp.G, emp.B));
						}
						else
						{
							uint16_t buff__ = (uint16_t)buff;
							auto data_r = buff__ * uintptr_t(emp.R >> 7);
							auto data_g = buff__ * uintptr_t(emp.G >> 7);
							auto data_b = buff__ * uintptr_t(emp.B >> 7);
							DSurface::Temp->Put_Pixel(outClient, DSurface::RGBA_To_Pixel(data_r, data_g, data_b));
						}
					}
				}
			}

		}
	}

	for (auto& draw : this->SmokeData)
	{
		if (const auto image = draw.LinkedParticleType->GetImage())
		{
			const auto offs = -15 - Game::AdjustHeight(draw.vel.Z);
			Point2D outClient;
			TacticalClass::Instance->CoordsToClient(&draw.vel, &outClient);
			DWORD drawingFlag = 0x2E00;
			outClient.Y += rect->X;
			if (GameOptionsClass::Instance->DetailLevel == 2)
			{
				int trans = draw.Translucency;
				if (trans == 25u)
				{
					drawingFlag = 0x2E02;
				}
				else if (trans == 50u)
				{
					drawingFlag = 0x2E04;
				}
				else if (trans >= 75u)
				{
					drawingFlag = 0x2E06;
				}
			}

			ConvertClass* pal = FileSystem::ANIM_PAL();
			if (auto pManager = ParticleTypeExtContainer::Instance.Find(draw.LinkedParticleType)->Palette)
				pal = pManager->GetConvert<PaletteManager::Mode::Temperate>();

			DSurface::Temp->DrawSHP(
				pal,
				image,
				draw.ImageFrame,
				&outClient,
				&rect,
				(BlitterFlags)drawingFlag,
				0,
				offs,
				2,
				1000,
				0,
				0,
				0,
				0,
				0
			);
		}
	}
}

void ParticleSystemExtData::UpdateInAir()
{
	if (ParticleSystemClass::Array->Count && GameOptionsClass::Instance->DetailLevel && RulesExtData::DetailsCurrentlyEnabled())
	{
		bool StopDrawing = false;
		if (Unsorted::ArmageddonMode() || !Game::hInstance() || ((ScenarioClass::Instance->SpecialFlags.RawFlags + 1) & 16) == 0)
			StopDrawing = true;

		for (auto pSys : *ParticleSystemClass::Array)
		{
			ParticleSystemExtContainer::Instance.Find(pSys)->UpdateInAir_Main(StopDrawing);
		}
	}
}

void ParticleSystemExtData::InitializeConstant()
{
	if (auto pType = this->AttachedToObject->Type)
	{
		if (!ParticleSystemTypeExtContainer::Instance.Find(pType)->ApplyOptimization || (size_t)pType->HoldsWhat >= ParticleTypeClass::Array->size())
			return ;

		this->HeldType = ParticleTypeClass::Array->GetItem(pType->HoldsWhat);

		if (!this->HeldType->UseLineTrail && !this->HeldType->AlphaImage) {

			auto bIsZero = (int)this->HeldType->BehavesLike;
			auto nBehave = (int)pType->BehavesLike;

			if (bIsZero <= 1)
				bIsZero = bIsZero == 0;

			if (nBehave == bIsZero) {

				if (nBehave == 0) {
					this->What = Behave::Smoke;
					return;
				}

				auto v11 = nBehave - 3;

				if (!v11) {
					this->What = Behave::Spark;
					return;
				}

				if (v11 == 1) {
					this->What = Behave::Railgun;
					return;
				}
			}
		}
	}
}

template <typename T>
void ParticleSystemExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->Initialized)
		.Process(this->What)
		.Process(this->OtherParticleData)
		.Process(this->SmokeData)
		.Process(this->HeldType)
		;
}

// =============================
// container
ParticleSystemExtContainer ParticleSystemExtContainer::Instance;

// =============================
// container hooks

DEFINE_HOOK(0x62DF05, ParticleSystemClass_CTOR, 0x5)
{
	GET(ParticleSystemClass*, pItem, ESI);
	ParticleSystemExtContainer::Instance.Allocate(pItem);
	return 0;
}

DEFINE_HOOK(0x62E26B, ParticleSystemClass_DTOR, 0x6)
{
	GET(ParticleSystemClass* const, pItem, ESI);
	ParticleSystemExtContainer::Instance.Remove(pItem);
	return 0;
}

DEFINE_HOOK_AGAIN(0x62FF20, ParticleSystemClass_SaveLoad_Prefix, 0x7)
DEFINE_HOOK(0x630090, ParticleSystemClass_SaveLoad_Prefix, 0x5)
{
	GET_STACK(ParticleSystemClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	ParticleSystemExtContainer::Instance.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK(0x630088, ParticleSystemClass_Load_Suffix, 0x5)
{
	ParticleSystemExtContainer::Instance.LoadStatic();
	return 0;
}

DEFINE_HOOK(0x6300F3, ParticleSystemClass_Save_Suffix, 0x6)
{
	ParticleSystemExtContainer::Instance.SaveStatic();
	return 0x0;
}
