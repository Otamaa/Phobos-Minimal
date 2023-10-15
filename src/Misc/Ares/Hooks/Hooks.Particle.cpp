#include <Ext/Particle/Body.h>
#include <Ext/ParticleType/Body.h>
#include <Ext/ParticleSystem/Body.h>
#include <Ext/ParticleSystemType/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/Anim/Body.h>

#include "Header.h"

#include <Notifications.h>
#include <Ext/Rules/Body.h>
#include <SpotlightClass.h>

void NOINLINE ParticleSystemExtData::UpdateLocations()
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

void NOINLINE ParticleSystemExtData::UpdateState()
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

void NOINLINE ParticleSystemExtData::UpdateColor()
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

void NOINLINE ParticleSystemExtData::UpdateSpark()
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

void NOINLINE ParticleSystemExtData::UpdateRailgun()
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

void NOINLINE ParticleSystemExtData::UpdateWindDirection()
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

				if (velCpy.Z < z && smoke.vel.Z >= (z - 260)) {
					smoke.DeleteOnStateLimit = true;
				}
			}
		}
	}
}

void NOINLINE ParticleSystemExtData::UpdateSmoke()
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

	for (int i = int(this->SmokeData.size()) - 1; i > 0; --i) {
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

void NOINLINE ParticleSystemExtData::UpdateInAir_Main(bool allowDraw)
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

void ParicleSystem_Web_AI(ParticleSystemClass* pThis)
{
	for (auto& particle : pThis->Particles)
		particle->BehaviourUpdate();

	for (int i = pThis->Particles.Count - 1; i > 0; --i) {
		auto particle = pThis->Particles[i];

		if (pThis->Particles[i]->hasremaining)
		{
			if (particle->Type->NextParticle != -1)
			{
				const auto pNextType = ParticleTypeClass::Array->Items[particle->Type->NextParticle];
				const auto nCoord = pNextType->NextParticleOffset + particle->Location;
				if (auto particle_ = GameCreate<ParticleClass>(pNextType, nCoord, CoordStruct::Empty, nullptr))
				{
					particle = std::exchange(pThis->Particles[i], particle_);
					particle_->Velocity = particle->Velocity;
					particle_->GasVelocity = particle->GasVelocity;
				}
			}

			particle->UnInit();
		}
		else {
			particle->BehaviourCoordUpdate();
		}
	}
}

void Particle_Web_AI(ParticleClass* pThis)
{
	auto pCell = MapClass::Instance->GetCellAt(pThis->Location);

	if (auto pWarhead = pThis->Type->Warhead) {
		for(auto pCur = pCell->FirstObject; pCur; pCur = pCur->NextObject) {
			if (pCur && pCur->IsAlive && pCur->Health > 0) {
				int damage = pThis->Type->Damage;
				pCur->ReceiveDamage(&damage, 0, pWarhead, nullptr, false, false, nullptr);
			}
		}
	}

	const int Id = pThis->Fetch_ID();
	const int Ecs = LOWORD(pThis->Type->MaxEC) - pThis->RemainingEC + Id;
	const int Ecs_ = LOBYTE(pThis->Type->StateAIAdvance) + (pThis->Fetch_ID() & 1);

	if (!(Ecs % Ecs_))
		++pThis->StartStateAI;

	if (pThis->StartStateAI == pThis->Type->EndStateAI) {
		if (pThis->Type->DeleteOnStateLimit)
			pThis->hasremaining = false;
		else
			pThis->StartStateAI = 0;
	}
}

//DEFINE_HOOK(0x6453D7, ParticleTypeClass_ReadINI_BehavesLike_A, 0x5)
//{
//	LEA_STACK(const char*, pResult, 0x14);
//	R->EBX(ParticleTypeClass::BehavesFromString(pResult));
//	return 0x6453FF;
//}
//
//DEFINE_HOOK(0x644423, ParticleSystemTypeClass_ReadINI_BehavesLike_A, 0x8)
//{
//	LEA_STACK(const char*, pResult, 0x20);
//	R->EAX(ParticleSystemTypeClass::BehavesFromString(pResult));
//	return 0x644461;
//}

//DEFINE_HOOK(0x6458D7, ParticleTypeClass_ReadINI_BehavesLike_B, 0x6)
//{
//	GET(const char*, pResult, EBX);
//
//	for (size_t i = 0; i < ParticleTypeClass::BehavesString.c_size(); ++i)
//	{
//		if (IS_SAME_STR_(pResult, ParticleTypeClass::BehavesString[i]))
//		{
//			switch (i)
//			{
//			case 0:
//				R->EDI(ParticleTypeBehavesLike::Gas);
//				return 0x6458FF;
//			case 1:
//				R->EDI(ParticleTypeBehavesLike::Smoke);
//				return 0x6458FF;
//			case 2:
//				R->EDI(ParticleTypeBehavesLike::Fire);
//				return 0x6458FF;
//			case 3:
//				R->EDI(ParticleTypeBehavesLike::Spark);
//				return 0x6458FF;
//			case 4:
//				R->EDI(ParticleTypeBehavesLike::Railgun);
//				return 0x6458FF;
//			default:
//				break;
//			}
//		}
//	}
//
//	if (IS_SAME_STR_(pResult, "Web"))
//	{
//		R->EDI(ParticleTypeBehavesLike(5)); //result;
//		return 0x6453FF;
//	}
//
//	R->EDI(ParticleTypeBehavesLike::None); //result;
//	return 0x6458FF;
//}

//DEFINE_HOOK(0x644857, ParticleSystemTypeClass_ReadINI_BehavesLike_B, 0x6)
//{
//	GET(const char*, pResult, EBX);
//
//	for (size_t i = 0; i < ParticleSystemTypeClass::BehavesString.c_size(); ++i)
//	{
//		if (IS_SAME_STR_(pResult, ParticleSystemTypeClass::BehavesString[i]))
//		{
//			switch (i)
//			{
//			case 0:
//				R->EDI(ParticleSystemTypeBehavesLike::Smoke);
//				return 0x64487F;
//			case 1:
//				R->EDI(ParticleSystemTypeBehavesLike::Gas);
//				return 0x64487F;
//			case 2:
//				R->EDI(ParticleSystemTypeBehavesLike::Fire);
//				return 0x64487F;
//			case 3:
//				R->EDI(ParticleSystemTypeBehavesLike::Spark);
//				return 0x64487F;
//			case 4:
//				R->EDI(ParticleSystemTypeBehavesLike::Railgun);
//				return 0x64487F;
//			default:
//				break;
//			}
//		}
//	}
//
//	if (IS_SAME_STR_(pResult, "Web"))
//	{
//		R->EDI(ParticleSystemTypeBehavesLike(5)); //result;
//		return 0x64487F;
//	}
//
//	R->EDI(ParticleSystemTypeBehavesLike::None); //result;
//	return 0x64487F;
//}

DEFINE_HOOK(0x62FCF0, ParticleSytemClass_FireDirectioon_AI_DirMult, 0x7)
{
	GET(int, facing, EAX);
	GET(ParticleSystemClass*, pThis, ESI);
	const auto& mult = ParticleSystemTypeExtContainer::Instance.Find(pThis->Type)->FacingMult[facing];
	R->ECX(mult.X);
	R->EAX(mult.Y);
	return 0x62FCFE;
}

#ifndef PARTICLESTUFFSOVERRIDE

//;\Ext\ParticleSystem\Body.cpp
DEFINE_DISABLE_HOOK(0x62DF05, ParticleSystemClass_CTOR_ares)
DEFINE_DISABLE_HOOK(0x62E26B, ParticleSystemClass_DTOR_ares)
DEFINE_DISABLE_HOOK(0x630090, ParticleSystemClass_SaveLoad_Prefix_ares)
DEFINE_DISABLE_HOOK(0x62FF20, ParticleSystemClass_SaveLoad_Prefix_ares)
DEFINE_DISABLE_HOOK(0x630088, ParticleSystemClass_Load_Suffix_ares)
DEFINE_DISABLE_HOOK(0x6300F3, ParticleSystemClass_Save_Suffix_ares)

//;\Ext\ParticleType\Body.cpp
DEFINE_DISABLE_HOOK(0x644DBB, ParticleTypeClass_CTOR_ares)
DEFINE_DISABLE_HOOK(0x645A3B, ParticleTypeClass_SDDTOR_ares)
DEFINE_DISABLE_HOOK(0x6457A0, ParticleTypeClass_SaveLoad_Prefix_ares)
DEFINE_DISABLE_HOOK(0x645660, ParticleTypeClass_SaveLoad_Prefix_ares)
DEFINE_DISABLE_HOOK(0x64578C, ParticleTypeClass_Load_Suffix_ares)
DEFINE_DISABLE_HOOK(0x64580A, ParticleTypeClass_Save_Suffix_ares)
DEFINE_DISABLE_HOOK(0x645405, ParticleTypeClass_LoadFromINI_ares)

DEFINE_DISABLE_HOOK(0x62CDE8, ParticleClass_Update_Fire_ares) //, 5)
DEFINE_DISABLE_HOOK(0x62C2ED, ParticleClass_Update_Gas_ares) //, 6)

DEFINE_OVERRIDE_HOOK(0x72590E, AnnounceInvalidPointer_Particle, 0x9)
{
	GET(AbstractType, nWhat, EBX);

	if (nWhat == AbstractType::Particle)
	{
		GET(ParticleClass*, pThis, ESI);

		if (auto pSys = pThis->ParticleSystem)
		{
			pSys->Particles.Remove(pThis);
		}

		return 0x725C08;
	}

	return nWhat == AbstractType::ParticleSystem ?
		0x725917 : 0x7259DA;
}

//DEFINE_HOOK(0x62EE3F, ParticleClass_SmokeAI_ZeroRadius, 0x6)
//{
//	GET(ParticleTypeClass*, pType, EDX);
//	const auto radius = pType->Radius >> 3;
//
//	if (radius == 0)
//		Debug::FatalError("[%s] Particle with 0 raius , please fix !\n", pType->ID);
//
//	R->EAX(radius);
//	return 0x62EE48;
//}

//DEFINE_OVERRIDE_HOOK(0x62C2C2, ParticleClass_Update_Gas_Damage, 6)
//{
// 	GET(ParticleClass*, pParticle, EBP);
// 	GET(ObjectClass*, pTarget, ESI);
// 	GET(int, nDistance, ECX);
//
// 	if (pTarget->InLimbo)
// 		return 0x62C309;
//
// 	if (auto pTechno = generic_cast<TechnoClass*>(pTarget))
// 	{
// 		if (pTechno->IsSinking || pTechno->IsCrashing || pTechno->TemporalTargetingMe)
// 			return 0x62C309;
//
// 		if (pTechno->WhatAmI() != BuildingClass::AbsID && TechnoExtData::IsChronoDelayDamageImmune(static_cast<FootClass*>(pTechno)))
// 			return 0x62C309;
// 	}
//
// 	auto const& [pAttacker, pOwner] = ParticleExt::GetOwnership(pParticle);
// 	int nDamage = pParticle->Type->Damage;
// 	pTarget->ReceiveDamage(&nDamage, nDistance, pParticle->Type->Warhead, pAttacker, false, false, pOwner);
//
//	return 0x62C309;
//}

void ParticleClass_Gas_Transmography(ObjectClass* pItem, TechnoClass* pAttacker , HouseClass* pOwner , int distance, const CoordStruct& loc, ParticleTypeExtData* pTypeExt, HouseClass* transmoOwner)
{
	int damage = pTypeExt->AttachedToObject->Damage;
	if (pItem->ReceiveDamage(&damage, distance, pTypeExt->AttachedToObject->Warhead, pAttacker, false, false, pOwner) == DamageState::NowDead) {
		if (pTypeExt->TransmogrifyChance >= 0) {

			if (pTypeExt->TransmogrifyOwner != OwnerHouseKind::Neutral)
				transmoOwner = HouseExtData::GetHouseKind(pTypeExt->TransmogrifyOwner, true, nullptr, pOwner, pItem->GetOwningHouse());

			CoordStruct loc_ = loc;
			TechnoExt_ExtData::SpawnVisceroid(loc_, pTypeExt->TransmogrifyType, pTypeExt->TransmogrifyChance, pTypeExt->Transmogrify, transmoOwner);
		}
	}
}

DEFINE_OVERRIDE_HOOK(0x62C23D, ParticleClass_Update_Gas_DamageRange, 6)
{
	GET(ParticleClass*, pThis, EBP);
	auto pTypeExt = ParticleTypeExtContainer::Instance.Find(pThis->Type);

	const auto& [pAttacker, pOwner] = ParticleExtData::GetOwnership(pThis);
	HouseClass* transmoOwner = HouseExtData::FindNeutral();

	if (pTypeExt->DamageRange.Get() <= 0.0)
	{
		for (auto pOccupy = MapClass::Instance->GetCellAt(pThis->Location)->FirstObject; pOccupy; pOccupy = pOccupy->NextObject)
		{
			if (pOccupy && pOccupy->IsAlive && pOccupy->Health > 0)
			{
				 if (auto pTechno = generic_cast<TechnoClass*>(pOccupy))
				 {
				 	if (pTechno->IsSinking || pTechno->IsCrashing || pTechno->TemporalTargetingMe)
				 		continue;

				 	if (pTechno->WhatAmI() != BuildingClass::AbsID && TechnoExtData::IsChronoDelayDamageImmune(static_cast<FootClass*>(pTechno)))
						continue;
				 }

				 auto nX = abs(pThis->Location.X - pOccupy->Location.X);
				 auto nY = abs(pThis->Location.Y - pOccupy->Location.Y);
				 ParticleClass_Gas_Transmography(pOccupy, pAttacker, pOwner, Game::AdjustHeight(nX + nY), pOccupy->Location, pTypeExt, transmoOwner);
			}
		}

	} else {

		const auto pVec = Helpers::Alex::getCellSpreadItems(pThis->Location, std::ceil(pTypeExt->DamageRange.Get()));

		for (const auto pItem : pVec)
		{
			if (pItem->WhatAmI() != BuildingClass::AbsID && TechnoExtData::IsChronoDelayDamageImmune(static_cast<FootClass*>(pItem)))
				continue;

			auto nX = abs(pThis->Location.X - pItem->Location.X);
			auto nY = abs(pThis->Location.Y - pItem->Location.Y);
			ParticleClass_Gas_Transmography(pItem, pAttacker, pOwner, Game::AdjustHeight(nX + nY), pItem->Location, pTypeExt, transmoOwner);
		}
	}

	return 0x62C313;
}

DEFINE_OVERRIDE_HOOK(0x62D015, ParticleClass_Draw_Palette, 6)
{
	GET(ParticleClass*, pThis, EDI);

	ConvertClass* pConvert = FileSystem::ANIM_PAL();
	const auto pTypeExt = ParticleTypeExtContainer::Instance.Find(pThis->Type);
	if (const auto pConvertData = pTypeExt->Palette)
	{
		pConvert = pConvertData->GetConvert<PaletteManager::Mode::Temperate>();
	}

	R->EDX(pConvert);
	return 0x62D01B;
}

DEFINE_DISABLE_HOOK(0x62CDB6, ParticleClass_Update_Fire_ares)

DEFINE_HOOK(0x62CCB8, ParticleClass_Update_Fire, 7)
{
	GET(ParticleClass*, pThis, ESI);

	pThis->RemainingDC = LOWORD(pThis->Type->MaxDC);
	auto const& [pAttacker, pOwner] = ParticleExtData::GetOwnership(pThis);
	const auto pCell = MapClass::Instance->GetCellAt(pThis->Location);
	const auto pTypeExt = ParticleTypeExtContainer::Instance.Find(pThis->Type);

	for (auto pOccupy = pCell->GetContent(pThis->Location.Z); pOccupy; pOccupy = pOccupy->NextObject) {

		if (pOccupy && pOccupy->IsAlive && pOccupy->Health > 0 && !pOccupy->InLimbo)
		{
			if (pThis->ParticleSystem && pAttacker == pThis->ParticleSystem->Owner)
				continue;

			if (auto pTechno = generic_cast<TechnoClass*>(pOccupy))
			{
				if (pTechno->IsSinking || pTechno->IsCrashing || pTechno->TemporalTargetingMe)
					continue;

				if (pTechno->WhatAmI() != BuildingClass::AbsID && TechnoExtData::IsChronoDelayDamageImmune(static_cast<FootClass*>(pTechno)))
					continue;
			}

			int damage = pThis->Type->Damage;
			int length = (int)(pThis->Location.DistanceFrom(pOccupy->GetCoords()) / 10.0);

			pOccupy->ReceiveDamage(&damage, length, pThis->Type->Warhead, pAttacker, false, false, pOwner);
			if (pTypeExt->Fire_DamagingAnim) {
				if (auto pAnimType = MapClass::SelectDamageAnimation(pThis->Type->Damage, pThis->Type->Warhead, pCell->LandType, pThis->Location)) {
					AnimExtData::SetAnimOwnerHouseKind(GameCreate<AnimClass>(pAnimType, pThis->Location),
						pOwner, pOccupy->GetOwningHouse(), pAttacker, false);
				}
			}
		}
	}

	return 0x62CE14;
}

DEFINE_OVERRIDE_HOOK(0x6D9427, TacticalClass_DrawUnits_ParticleSystems, 9)
{
	GET(Layer, layer, EAX);

	if (layer == Layer::Air)
		ParticleSystemExtData::UpdateInAir();

	return layer == Layer::Ground ? 0x6D9430 : 0x6D95A1;
}

DEFINE_OVERRIDE_HOOK(0x62E380, ParticleSystemClass_SpawnParticle, 0xA)
{
	GET(ParticleSystemClass*, pThis, ECX);

	return ParticleSystemExtContainer::Instance.Find(pThis)->What != ParticleSystemExtData::Behave::None
		? 0x62E428 : 0;
}

DEFINE_OVERRIDE_HOOK(0x62E2AD, ParticleSystemClass_Draw, 6)
{
	GET(ParticleSystemClass*, pThis, EDI);
	GET(ParticleSystemTypeClass*, pThisType, EAX);

	if (pThisType->ParticleCap)
	{
		R->ECX(pThis->Particles.Count +
			ParticleSystemExtContainer::Instance.Find(pThis)->OtherParticleData.size());
	}
	else
	{
		R->ECX(0);
	}

	return 0x62E2B3;
}

//DEFINE_HOOK(0x62CE40, ParticleClass_Update_Add, 0x9)
//{
//	GET(ParticleClass*, pThis, ECX);
//
//	if (pThis->Type->BehavesLike == ParticleTypeBehavesLike(5)) {
//		Particle_Web_AI(pThis);
//	}
//
//	return 0x0;
//}

DEFINE_OVERRIDE_HOOK(0x62FD60, ParticleSystemClass_Update, 0x9)
{
	GET(ParticleSystemClass*, pThis, ECX);

	//if (pThis->Type->BehavesLike == ParticleSystemTypeBehavesLike(5)) {
	//	ParicleSystem_Web_AI(pThis);
	//	return 0x0;
	//}

	const bool Handled = ParticleSystemExtContainer::Instance.Find(pThis)->UpdateHandled();

	return Handled ? 0x62FE43 : 0;
}
#endif