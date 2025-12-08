#include "Body.h"

#include <Ext/ParticleSystemType/Body.h>
#include <Ext/ParticleType/Body.h>
#include <Ext/Particle/Body.h>

#include <Ext/Rules/Body.h>
#include <Utilities/Macro.h>

#include <ParticleTypeClass.h>
#include <ParticleClass.h>
#include <SpotlightClass.h>
#include <GameOptionsClass.h>
#include <TacticalClass.h>

ParticleSystemExtData::ParticleSystemExtData(ParticleSystemClass* pObj) : ObjectExtData(pObj),
What(Behave::None),
HeldType(nullptr),
OtherParticleData(),
SmokeData(),
AlphaIsLightFlash(true)
{
	this->Name = pObj->Type->ID;
	this->AbsType = ParticleSystemClass::AbsID;

	auto pType = pObj->Type;
	{
		if (!ParticleSystemTypeExtContainer::Instance.Find(pType)->ApplyOptimization || (size_t)pType->HoldsWhat >= ParticleTypeClass::Array->size())
			return;

		this->HeldType = ParticleTypeClass::Array->Items[pType->HoldsWhat];
		if (!this->HeldType->UseLineTrail && !this->HeldType->AlphaImage)
		{
			auto bIsZero = (int)this->HeldType->BehavesLike;
			auto nBehave = (int)pType->BehavesLike;
			if (bIsZero <= 1)
				bIsZero = bIsZero == 0;
			if (nBehave == bIsZero)
			{
				if (!nBehave)
				{
					this->What = Behave::Smoke;
					return;
				}
				auto v11 = nBehave - 3;
				if (!v11)                       // 0
				{
					this->What = Behave::Spark;
					return;
				}
				if (v11 == 1)                   // // 1
				{
					this->What = Behave::Railgun;
					return;
				}
			}
			//else
			//{
			//	if (this->HeldType->ColorList.Count < 3) {
			//		return;
			//	}
			//
			//	switch (pType->BehavesLike)
			//	{
			//	case ParticleSystemTypeBehavesLike::Smoke:
			//		this->What = Behave::Smoke;
			//		return;
			//	case ParticleSystemTypeBehavesLike::Spark:
			//		this->What = Behave::Spark;
			//		return;
			//	case ParticleSystemTypeBehavesLike::Railgun:
			//		this->What = Behave::Railgun;
			//		return;
			//	default:
			//		break;
			//	}
			//}
		}
	}
}

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
	if (!this->HeldType || this->HeldType->ColorList.Count < 2)
	{
		return;  // Safety: Need valid particle type with at least 2 colors
	}
	const int maxColorIndex = this->HeldType->ColorList.Count - 2;

	for (auto& particle : this->OtherParticleData)
	{
		const double newFactor =
			ScenarioClass::Instance->Random.RandomDouble() * 0.05 +
			this->HeldType->ColorSpeed +
			particle.ColorFactor;

		if (newFactor <= 1.0)
		{
			particle.ColorFactor = static_cast<float>(newFactor);
		}
		else if (particle.C < maxColorIndex)
		{
			particle.ColorFactor = 0.0f;
			particle.C++;
		}
		else
		{
			particle.ColorFactor = 1.0f;  // Clamp at final color
		}
	}
}

void ParticleSystemExtData::UpdateSpark()
{
	auto pOwner = this->This();
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
			//auto pHeld = this->HeldType;
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
						data->Colors = ColorStruct::Interpolate(&this->HeldType->StartColor1, &this->HeldType->StartColor2, random->RandomDouble());
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

void  ParticleSystemExtData::UpdateRailgun()
{
	auto pThis = this->This();

	if (!pThis->TimeToDie && !this->OtherParticleData.size())
	{
		CoordStruct targetCoords = pThis->TargetCoords;
		CoordStruct currentCoords = pThis->Location;

		CoordStruct DifferenceCoords = targetCoords - currentCoords;
		int differeceCoordsLXYZLength = (int)Math::sqrt(double((DifferenceCoords.X * DifferenceCoords.X) + (DifferenceCoords.Y * DifferenceCoords.Y) + (DifferenceCoords.Z * DifferenceCoords.Z)));
		int differeceCoordsLXYLength = (int)Math::sqrt(double((DifferenceCoords.X * DifferenceCoords.X) + (DifferenceCoords.Y * DifferenceCoords.Y)));
		int Difference_Z = DifferenceCoords.Z;
		int Difference_X = DifferenceCoords.X;

		if (Difference_Z >= differeceCoordsLXYZLength)
		{
			Difference_Z = differeceCoordsLXYZLength;
		}

		if (Difference_Z <= -differeceCoordsLXYZLength)
		{
			Difference_Z = -differeceCoordsLXYZLength;
		}

		if (Difference_X >= differeceCoordsLXYLength)
		{
			Difference_X = differeceCoordsLXYLength;
		}

		if (Difference_X <= -differeceCoordsLXYLength)
		{
			Difference_X = -differeceCoordsLXYLength;
		}

		const auto ParticlePerCoords = differeceCoordsLXYZLength * pThis->Type->ParticlesPerCoord;
		Matrix3D mtx = Matrix3D::GetIdentity();
		const auto acos = Math::acos((double)Difference_X / (double)differeceCoordsLXYLength);
		mtx.PreRotateZ(float(acos < 0 ? -acos : acos));
		const auto theta = Math::asin((double)Difference_Z / (double)differeceCoordsLXYLength);
		mtx.PreRotateX(float(theta));

		auto pHeldType = this->HeldType;

		this->OtherParticleData.resize((int)ParticlePerCoords);

		double var = 0.0;

		for (int i = 0; i < (int)ParticlePerCoords; ++i)
		{
			const float curVal = float((double)i / (float)ParticlePerCoords);
			const double radians = curVal * differeceCoordsLXYZLength * pThis->Type->SpiralDeltaPerCoord;

			Vector3D<float> first_ {
				0 , Math::cos(radians) ,Math::sin(radians)
			};

			Vector3D<float> mtx_mult {};
			Matrix3D::MatrixMultiply(&mtx_mult, &mtx, &first_);

			const Vector3D<float> PositionPerturbation_ {
				float(ScenarioClass::Instance->Random.RandomDoubleCentered() * pThis->Type->PositionPerturbationCoefficient
				+ (first_.X * pThis->Type->SpiralRadius))

				,float(ScenarioClass::Instance->Random.RandomDoubleCentered() * pThis->Type->PositionPerturbationCoefficient
				+ (first_.Y * pThis->Type->SpiralRadius))

				,float(ScenarioClass::Instance->Random.RandomDoubleCentered() * pThis->Type->PositionPerturbationCoefficient
				+ (first_.Z * pThis->Type->SpiralRadius))
			};

			auto CoordSturct_Lerp = [](const CoordStruct* CurrentCoord, const CoordStruct* TargetCoord, float factor)
				{
					auto t_neg = 1.0 - factor;
					return CoordStruct {
						(int)((double)TargetCoord->X * factor + (double)CurrentCoord->X * t_neg)
						,(int)((double)TargetCoord->Y * factor + (double)CurrentCoord->Y * t_neg)
						,(int)((double)TargetCoord->Z * factor + (double)CurrentCoord->Z * t_neg)
					};
				};

			//lerp result stored as Vel
			CoordStruct lerp = CoordSturct_Lerp(&pThis->Location, &pThis->TargetCoords, curVal);
			lerp += CoordStruct { (int)PositionPerturbation_.X , (int)PositionPerturbation_.Y , (int)PositionPerturbation_.Z };

			auto Data = &this->OtherParticleData[i]; // .emplace_back();
			Vector3D<float> MovementPerturbation {
				float(ScenarioClass::Instance->Random.RandomDoubleCentered() * pThis->Type->MovementPerturbationCoefficient)
				, float(ScenarioClass::Instance->Random.RandomDoubleCentered() * pThis->Type->MovementPerturbationCoefficient)
				, float(ScenarioClass::Instance->Random.RandomDoubleCentered() * pThis->Type->MovementPerturbationCoefficient)
			};

			mtx_mult += MovementPerturbation;

			const auto sparkVelLength = Math::sqrt(double((mtx_mult.X * mtx_mult.X) + (mtx_mult.Y * mtx_mult.Y) + (mtx_mult.Z * mtx_mult.Z)));

			if (sparkVelLength != 0.0)
			{

				const auto float_sparkVelLength = (float)sparkVelLength;
				mtx_mult.X = mtx_mult.X / float_sparkVelLength;
				mtx_mult.Y = mtx_mult.Y / float_sparkVelLength;
				mtx_mult.Z = mtx_mult.Z / float_sparkVelLength;
			}

			Data->velB = mtx_mult;

			Data->vel.X = (float)lerp.X;
			Data->vel.Y = (float)lerp.Y;
			Data->vel.Z = (float)lerp.Z;

			auto rand = (ScenarioClass::Instance->Random.RandomDouble() + var - 0.5) * pThis->Type->VelocityPerturbationCoefficient;
			if (pThis->Type->VelocityPerturbationCoefficient <= rand)
			{
				rand = pThis->Type->VelocityPerturbationCoefficient;
			}

			if (rand <= -pThis->Type->MovementPerturbationCoefficient)
			{
				rand = -pThis->Type->MovementPerturbationCoefficient;
			}

			var = rand;
			Data->A = float(var + pHeldType->Velocity);
			Data->RemainingEC = LOWORD(pHeldType->MaxEC) + ScenarioClass::Instance->Random.RandomFromMax(9);
		}

		if (pThis->Type->Laser)
		{
			GameCreate<LaserDrawClass>(
				pThis->Location,
				pThis->TargetCoords,
				0, 1,
				pThis->Type->LaserColor,
				ColorStruct::Empty,
				ColorStruct::Empty,
				10, false, true, 0.5f, 0.1f);
		}

		pThis->TimeToDie = true;
	}

	// this one updating the pisitional value on each particle
	// this was executed by `ParticleClass_AI` on vanilla
	for (auto& movement : this->OtherParticleData)
	{
		const auto state = movement.A;
		movement.A = float(ScenarioClass::Instance->Random.RandomDoubleCentered() * 0.1 + state);
		const bool IsECdone = --movement.RemainingEC <= 0;
		const auto copy_velB = movement.velB;
		const auto copy_vel = movement.vel;

		movement.vel.X = copy_velB.X * state + copy_vel.X;
		movement.vel.Y = copy_velB.Y * state + copy_vel.Y;
		movement.vel.Z = copy_velB.Z * state + copy_vel.Z;

		if (IsECdone)
			movement.Empty = true;
	}

	// the game use reverse of this
	// update both pisition and the color then finally do the state updates
	this->UpdateState();
	this->UpdateColor();
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
		//auto x_ = ParticleClass::SmokeWind_X.begin() + WindDir;
		//auto y_ = ParticleClass::SmokeWind_Y.begin() + WindDir;
		for (auto& smoke : this->SmokeData)
		{
			CoordStruct velCpy = smoke.vel;

			if (smoke.LinkedParticleType->WindEffect > 0)
			{
				const auto& [dirX, dirY] = ParticleTypeExtContainer::Instance.Find(smoke.LinkedParticleType)->WindMult[WindDir];
				smoke.vel.X += int(smoke.velB.X + smoke.LinkedParticleType->WindEffect * dirX);
				smoke.vel.Y += int(smoke.velB.Y + smoke.LinkedParticleType->WindEffect * dirY);
				smoke.vel.Z += int(smoke.velB.Z);
			}
			else
			{ // if no wind effect
				smoke.vel.X += int(smoke.velB.X);
				smoke.vel.Y += int(smoke.velB.Y);
				smoke.vel.Z += int(smoke.velB.Z);
			}

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
	auto const pOwnerObj = this->This();
	auto const pOwnerObjType = pOwnerObj->Type;
	auto const pOwnerObj_Owner = pOwnerObj->Owner;

	ObjectClass* Owner_obj = flag_cast_to<ObjectClass*>(pOwnerObj_Owner);

	if (Owner_obj)
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
		if (auto pNext = ParticleTypeClass::Array->get_or_default(curData->LinkedParticleType->NextParticle))
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


	//updating the current particle ?
	if (!pOwnerObj->TimeToDie && pOwnerObj->IsAlive)
	{
		auto pOwnerFoot = flag_cast_to<FootClass*>(pOwnerObj_Owner);

		if (!(Unsorted::CurrentFrame() % (int)pOwnerObj->SpawnFrames) && (!pOwnerFoot || pOwnerFoot->TubeIndex < 0))
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

void ParticleSystemExtData::UpdateInAir_Main(bool allowDraw)
{
	const auto pHeldType = this->HeldType;
	if (!pHeldType)
	{
		return;
	}

	ColorStruct* color = pHeldType->ColorList.Items;
	const int colorCount = pHeldType->ColorList.Count;

	auto& rect = DSurface::ViewBounds;
	for (auto& movement : this->OtherParticleData)
	{
		CoordStruct Coord {
			static_cast<int>(movement.vel.X),
			static_cast<int>(movement.vel.Y),
			static_cast<int>(movement.vel.Z)
		};

		if (allowDraw || !MapClass::Instance->IsLocationShrouded(Coord))
		{
			Point2D outClient = TacticalClass::Instance->CoordsToClient(Coord);

			const auto y_copy = rect->Y + outClient.Y;
			outClient.Y += rect->Y;

			// Viewport culling - check if particle is on screen
			if (outClient.X < rect->X || outClient.X >= rect->X + rect->Width ||
				y_copy < rect->Y || y_copy >= rect->Y + rect->Height)  // FIXED: rect->Y
			{
				continue;  // Off-screen, skip rendering
			}

			/* Original
			if (outClient.X >= rect->X
				&& outClient.X < rect->X + rect->Width
				&& y_copy >= rect->Y
				&& y_copy < rect->X + rect->Height
				)
				*/
			{
				uintptr_t buff = *reinterpret_cast<uint16_t*>(ABuffer::Instance->GetBuffer(outClient.X, y_copy - ABuffer::Instance->Area.Y));

				if (buff == 0)
				{
					continue;
				}


				{
					int ZBuff = *reinterpret_cast<uint16_t*>((ZBuffer::Instance->GetBuffer(outClient.X, outClient.Y - ZBuffer::Instance->Area.Y)));
					int Zadjust = Game::AdjustHeight(Coord.Z);

					uint16_t zTest = static_cast<uint16_t>(
						LOWORD(ZBuffer::Instance->Area.Y) +
						LOWORD(ZBuffer::Instance->MaxValue) -
						LOWORD(outClient.Y)
					) - Zadjust - 50;

					if (zTest < ZBuff)
					{
						int idx = 0;
						ColorStruct* selected = &movement.Colors;

						// SAFETY: Bounds check before array access
						if (movement.C > 0 && movement.C < colorCount)
						{
							idx = movement.C;
							selected = &color[movement.C];
						}

						// SAFETY: Ensure idx+1 is in bounds
						if (idx + 1 >= colorCount)
						{
							idx = colorCount - 2;  // Clamp to last valid pair
						}

						// Interpolate between two colors based on ColorFactor
						ColorStruct finalColor = ColorStruct::Interpolate(
							&color[idx + 1],      // Target color
							selected,             // Current color
							static_cast<double>(movement.ColorFactor)  // Blend factor (0.0 to 1.0)
						);

						uint32_t pixelColor;

						if (buff >= 127u)
						{
							// Full brightness - use color directly
							pixelColor = DSurface::RGBA_To_Pixel(finalColor.R, finalColor.G, finalColor.B);
						}
						else
						{
							// Dim the color based on alpha buffer value
							// buff is 0-127, so this darkens the color proportionally
							uint32_t data_r = (buff * finalColor.R) >> 7;  // Divide by 128
							uint32_t data_g = (buff * finalColor.G) >> 7;
							uint32_t data_b = (buff * finalColor.B) >> 7;
							pixelColor = DSurface::RGBA_To_Pixel(data_r, data_g, data_b);
						}

						DSurface::Temp->Put_Pixel(outClient, pixelColor);
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
			Point2D outClient = TacticalClass::Instance->CoordsToClient(draw.vel);
			DWORD drawingFlag = 0x2E00;
			outClient.Y += rect->Y;
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
			if (auto pManager = ParticleTypeExtContainer::Instance.Find(draw.LinkedParticleType)->Palette.GetConvert())
				pal = pManager;

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
			auto pExt = ParticleSystemExtContainer::Instance.Find(pSys);

			if (!pExt)
				Debug::FatalError("ParticleSystem without Ext[%x]", pSys);

			pExt->UpdateInAir_Main(StopDrawing);
		}
	}
}

//DEFINE_JUMP(LJMP, 0x62ED53, 0x62ED61);

template <typename T>
void ParticleSystemExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->What)
		.Process(this->OtherParticleData)
		.Process(this->SmokeData)
		.Process(this->HeldType)
		.Process(this->AlphaIsLightFlash)
		;
}

// =============================
// container
ParticleSystemExtContainer ParticleSystemExtContainer::Instance;
std::vector<ParticleSystemExtData*> Container<ParticleSystemExtData>::Array;

void Container<ParticleSystemExtData>::Clear()
{
	Array.clear();
}

bool ParticleSystemExtContainer::LoadGlobals(PhobosStreamReader& Stm)
{
	return LoadGlobalArrayData(Stm);
}

bool ParticleSystemExtContainer::SaveGlobals(PhobosStreamWriter& Stm)
{
	return SaveGlobalArrayData(Stm);
}

// =============================
// container hooks

#include <Ext/Anim/Body.h>

ASMJIT_PATCH(0x62DF05, ParticleSystemClass_CTOR, 0x5)
{
	GET(ParticleSystemClass*, pItem, ESI);
	ParticleSystemExtContainer::Instance.Allocate(pItem);
	return 0;
}

ASMJIT_PATCH(0x62E26B, ParticleSystemClass_DTOR, 0x6)
{
	GET(ParticleSystemClass* const, pItem, ESI);

	if (pItem->Owner && pItem->Owner->WhatAmI() == AnimClass::AbsID)
	{
		for (AnimClass* anim : AnimExtContainer::AnimsWithAttachedParticles)
		{
			auto pAnimExt = AnimExtContainer::Instance.Find(anim);

			if (pAnimExt->AttachedSystem == pItem)
			{
				pAnimExt->AttachedSystem.detachptr();
			}
		}
	}

	ParticleSystemExtContainer::Instance.Remove(pItem);
	return 0;
}

HRESULT __stdcall FakeParticleSystemClass::_Load(IStream* pStm)
{
	HRESULT hr = this->ParticleSystemClass::Load(pStm);
	if (SUCCEEDED(hr))
		hr = ParticleSystemExtContainer::Instance.LoadKey(this, pStm);

	return hr;
}

HRESULT __stdcall FakeParticleSystemClass::_Save(IStream* pStm, BOOL clearDirty)
{
	HRESULT hr = this->ParticleSystemClass::Save(pStm, clearDirty);
	if (SUCCEEDED(hr))
		hr = ParticleSystemExtContainer::Instance.SaveKey(this, pStm);

	return hr;
}

// DEFINE_FUNCTION_JUMP(VTABLE, 0x7EFBB0, FakeParticleSystemClass::_Load)
// DEFINE_FUNCTION_JUMP(VTABLE, 0x7EFBB4, FakeParticleSystemClass::_Save)

// Vanilla won't swizzle owner house of Particle System when loading, which was fine before
// But now it might trigger a crash since DamageAllies/Enemies/OwnerMultiplier will check its house
// Fix it at here for now. If we extend ParticleSystemClass in the future this should be moved to there

ASMJIT_PATCH(0x62FFBB, ParticleSystemClass_Load_OwnerHouse, 0x8)
{

	GET(ParticleSystemClass*, pThis, EDI);

	SWIZZLE(pThis->OwnerHouse);

	return 0;
}

Vector3D<float> GetRandomPerturbation(float coefficient)
{
	const float rand1 = (float)ScenarioClass::Instance->Random.RandomDoubleCentered();
	const float rand2 = (float)ScenarioClass::Instance->Random.RandomDoubleCentered();
	const float rand3 = (float)ScenarioClass::Instance->Random.RandomDoubleCentered();

	return {
		rand1 * coefficient,
		rand2 * coefficient,
		rand3 * coefficient
	};
}

Matrix3D CreateRotationMatrix(const Vector3D<float>& trajectory, float distance)
{
	// Calculate angles for rotation
	const float horizontalDist = Math::sqrt(trajectory.X * trajectory.X +
												 trajectory.Y * trajectory.Y);

	// Clamp values for arc functions
	float clampedZ = std::clamp(trajectory.Z, -distance, distance);
	float clampedX = std::clamp(trajectory.X, -horizontalDist, horizontalDist);

	const float pitchAngle = (float)Math::asin(clampedZ / distance);
	float yawAngle = (float)Math::acos(clampedX / horizontalDist);

	// Adjust yaw based on Y direction
	if (trajectory.Y < 0.0f)
	{
		yawAngle = -yawAngle;
	}

	// Build rotation matrix
	Matrix3D matrix {};
	matrix.MakeIdentity();
	matrix.PreRotateZ(yawAngle);
	matrix.PreRotateX(pitchAngle);
	return matrix;
}

void FakeParticleSystemClass::__AI()
{
	auto pTypeExt = this->_GetTypeExtData();
	auto pExt = this->_GetExtData();
	bool wasHandled = false;

	if (pTypeExt->ApplyOptimization)
	{
		switch (pExt->What)
		{
		case ParticleSystemExtData::Behave::Spark:
			wasHandled = this->Type->BehavesLike == ParticleSystemTypeBehavesLike::Spark;
			pExt->UpdateSpark();
			break;
		case ParticleSystemExtData::Behave::Railgun:
			wasHandled = this->Type->BehavesLike == ParticleSystemTypeBehavesLike::Railgun;
			pExt->UpdateRailgun();
			break;
		case ParticleSystemExtData::Behave::Smoke:
			wasHandled = this->Type->BehavesLike == ParticleSystemTypeBehavesLike::Smoke;
			pExt->UpdateSmoke();
			break;
		default:
			break;
		}
	}

	if (!wasHandled)
	{
		switch (this->Type->BehavesLike)
		{
		case ParticleSystemTypeBehavesLike::Smoke:
			this->__Smoke_AI();
			break;
		case ParticleSystemTypeBehavesLike::Fire:
			this->__Fire_AI();
			break;
		case ParticleSystemTypeBehavesLike::Railgun:
			this->__Railgun_AI();
			break;
		case ParticleSystemTypeBehavesLike::Gas:
			this->__Gas_AI();
			break;
		case ParticleSystemTypeBehavesLike::Spark:
			this->__Spark_AI();
			break;
		case ParticleSystemTypeBehavesLike(5):
			this->__Web_AI();
			break;
		default:
			break;
		}
	}

	if (this->Lifetime-- == 1)
		this->TimeToDie = true;

	if (this->IsAlive
		&& this->TimeToDie
		&& !this->Particles.Count
		&& pExt->OtherParticleData.empty()
		&& pExt->SmokeData.empty())
	{
		this->Limbo();
		this->IsAlive = false;
		AbstractClass::Array2->push_back(this);
	}
}

ParticleClass* FakeParticleSystemClass::CreateHoldsWhatParticle(const CoordStruct& position, const CoordStruct& target)
{
	if (this->Type->HoldsWhat > -1)
	{
		auto pParticle = GameCreate<ParticleClass>(ParticleTypeClass::Array->Items[Type->HoldsWhat], position, target, this);
		this->Particles.push_back(pParticle);
		return pParticle;
	}

	return nullptr;
}

template<auto Func>
void FakeParticleSystemClass::ProcessParticleLifecycle()
{
	for (int i = this->Particles.Count - 1; i >= 0; --i)
	{
		ParticleClass* particle = this->Particles[i];

		if (!particle->hasremaining)
		{
			// Particle still alive - update coordinates
			((FakeParticleClass*)this->Particles[i])->__Coord_AI();
			continue;
		}

		// Particle is dead - spawn children if configured
		if (particle->Type->NextParticle != -1)
		{
			(this->*Func)(particle);
		}

		// Remove dead particle
		particle->UnInit();
	}
}

void FakeParticleSystemClass::__Web_AI()
{
	this->UpdateAllParticlesBehind();
	this->ProcessParticleLifecycle<&FakeParticleSystemClass::TransitionToNextParticle>();
}

int GetRandomOffset(int range)
{
	const int random = (ScenarioClass::Instance->Random.Random()) % range;
	return (random > 0) ? (range + random) : (random - range);
}

void FakeParticleSystemClass::UpdateAllParticlesFront()
{
	for (int i = 0; i < this->Particles.Count; ++i)
	{
		((FakeParticleClass*)this->Particles[i])->__AI();
	}
}

void FakeParticleSystemClass::UpdateAllParticlesBehind()
{
	for (int i = this->Particles.Count - 1; i >= 0; --i)
	{
		((FakeParticleClass*)this->Particles[i])->__AI();
	}
}

void FakeParticleSystemClass::UpdateAndCoordAllParticles()
{
	for (int i = this->Particles.Count - 1; i >= 0; --i)
	{
		((FakeParticleClass*)this->Particles[i])->__AI();
		((FakeParticleClass*)this->Particles[i])->__Coord_AI();
	}
}

void FakeParticleSystemClass::RemoveDeadParticles()
{
	for (int i = this->Particles.Count - 1; i >= 0; --i)
	{

		ParticleClass* particle = this->Particles[i];

		if (particle->hasremaining)
		{
			particle->UnInit();
		}
	}
}

void FakeParticleSystemClass::SpawnChildParticles(ParticleClass* parent)
{
	const ParticleTypeClass* parentType = parent->Type;
	ParticleTypeClass* childType = ParticleTypeClass::Array->Items[parentType->NextParticle];

	// Calculate random offset based on parent particle radius
	const int radiusDiv8 = parentType->Radius >> 3;
	const int offsetX = GetRandomOffset(radiusDiv8);
	const int offsetY = GetRandomOffset(radiusDiv8);

	// Spawn two child particles on either side
	this->SpawnChildParticle(parent, childType, offsetX, offsetY);
	this->SpawnChildParticle(parent, childType, -offsetX, -offsetY);
}

Vector3D<float> FakeParticleSystemClass::CalculateTrajectory() const
{
	return {
		static_cast<float>(this->TargetCoords.X - this->Location.X),
		static_cast<float>(this->TargetCoords.Y - this->Location.Y),
		static_cast<float>(this->TargetCoords.Z - this->Location.Z)
	};
}

#pragma region Spark

void FakeParticleSystemClass::ProcessSparkSpawning()
{
	// Check if we should spawn this frame
	if (!this->ShouldSpawnThisFrame())
	{
		return;
	}

	// Calculate particle count
	const int particleCount = this->CalculateParticleCount();

	// Spawn burst of sparks
	for (int i = 0; i < particleCount; ++i)
	{
		this->SpawnSparkParticle();
	}

	// Create spotlight if conditions met
	this->CreateSpotlightIfNeeded();
}

void FakeParticleSystemClass::SpawnSparkParticle()
{
	const CoordStruct position = this->Location;

	if (auto particle = this->CreateHoldsWhatParticle(position, position))
	{
		this->SetupRandomVelocity(particle);
	}
}

int FakeParticleSystemClass::CalculateParticleCount() const
{
	const int halfCap = this->Type->ParticleCap / 2;
	return halfCap + (Math::abs(ScenarioClass::Instance->Random.Random()) % halfCap);
}

bool FakeParticleSystemClass::ShouldSpawnThisFrame() const
{
	// Always spawn on first frame
	if (this->SparkSpawnFrames == 1)
	{
		return true;
	}

	// Random chance based on percentage
	const float random = (float)ScenarioClass::Instance->Random.RandomDouble();
	return random <= this->Type->SpawnSparkPercentage;
}

void FakeParticleSystemClass::UpdateSpotlight()
{
	// Random spotlight radius variation
	const float random = (float)ScenarioClass::Instance->Random.RandomDouble();

	if (random < 0.3f)
	{
		// Shrink
		this->SpotlightRadius = std::max(17, this->SpotlightRadius - 3);
	}
	else if (random < 0.6f)
	{
		// Grow
		this->SpotlightRadius = std::min(41, this->SpotlightRadius + 3);
	}
}

void FakeParticleSystemClass::CreateSpotlightIfNeeded()
{
	// Only create on high detail
	if (GameOptionsClass::Instance->DetailLevel != 2)
	{
		return;
	}

	// Only on first frame with valid light size
	if (this->SparkSpawnFrames != this->Type->SparkSpawnFrames)
	{
		return;
	}

	if (this->Type->LightSize <= 0)
	{
		return;
	}

	if (this->Type->OneFrameLight)
	{
		return;
	}

	GameCreate<SpotlightClass>(this->Location, this->Type->LightSize);
}

void FakeParticleSystemClass::SetupRandomVelocity(ParticleClass* particle)
{
	const ParticleTypeClass* type = particle->Type;

	// Generate random velocity components
	Vector3D<float> velocity {
		static_cast<float>(ScenarioClass::Instance->Random.Random() % type->XVelocity),
		static_cast<float>(ScenarioClass::Instance->Random.Random() % type->YVelocity),
		static_cast<float>(type->MinZVelocity +
						  Math::abs(ScenarioClass::Instance->Random.Random()) % type->ZVelocityRange)
	};

	// Get magnitude before direction modification
	const float oldmagnitude = (float)velocity.Length();

	// Add directional bias
	if (this->unknown_bool_F9)
	{
		// Generate fresh random direction per particle
		velocity.X += static_cast<float>(ScenarioClass::Instance->Random.Random() % type->XVelocity);
		velocity.Y += static_cast<float>(ScenarioClass::Instance->Random.Random() % type->YVelocity);
		velocity.Z += static_cast<float>(type->MinZVelocity +
											  Math::abs(ScenarioClass::Instance->Random.Random()) % type->ZVelocityRange);
	}
	else
	{
		// Use spawn direction from class
		velocity += this->Type->SpawnDirection;
	}

	// Normalize and apply magnitude
	const float newmagnitude = (float)velocity.Length();

	if (newmagnitude > 0.0f)
	{
		velocity /= newmagnitude;
		velocity *= oldmagnitude;
	}

	particle->Spark10C = velocity;
}

void FakeParticleSystemClass::__Spark_AI()
{
	// Spawn spark burst if still active
	if (this->SparkSpawnFrames > 0)
	{
		this->ProcessSparkSpawning();
		this->UpdateSpotlight();
		--this->SparkSpawnFrames;

		if (this->SparkSpawnFrames <= 0)
		{
			this->TimeToDie = 1;
		}
	}

	// Update all particles
	this->UpdateAllParticlesFront();

	// Remove dead particles
	this->RemoveDeadParticles();
}

#pragma endregion

#pragma region Gas

void  FakeParticleSystemClass::TransitionToNextParticle(ParticleClass* oldParticle)
{
	const ParticleTypeClass* parentType = oldParticle->Type;
	ParticleTypeClass* childType = ParticleTypeClass::Array->Items[parentType->NextParticle];

	// Calculate spawn position with offset
	CoordStruct spawnPos = oldParticle->Location + parentType->NextParticleOffset;

	// Create new particle
	auto newParticle = GameCreate<ParticleClass>(childType, spawnPos, CoordStruct::Empty, this);

	// Add to vector
	this->Particles.push_back(newParticle);

	// Inherit properties from old particle
	newParticle->Velocity = oldParticle->Velocity;
	newParticle->GasVelocity = oldParticle->GasVelocity;

	// Inherit properties from old particle
}

void FakeParticleSystemClass::__Gas_AI()
{
	// Update all particles
	this->UpdateAllParticlesBehind();

	// Handle particle lifecycle with transitions
	this->ProcessParticleLifecycle<&FakeParticleSystemClass::TransitionToNextParticle>();
}

#pragma endregion

#pragma region Smoke

void FakeParticleSystemClass::SpawnChildParticle(ParticleClass* parent, ParticleTypeClass* childType,
						int offsetX, int offsetY)
{
	const CoordStruct spawnPos {
		parent->Location.X + offsetX,
		parent->Location.Y + offsetY,
		parent->Location.Z
	};

	auto child = GameCreate<ParticleClass>(childType, spawnPos, CoordStruct::Empty, this);

	// Add to vector
	this->Particles.push_back(child);

	// Inherit properties from parent
	child->Velocity = parent->Velocity;

	// Add random translucency variation
	const int translucencyBonus = ((ScenarioClass::Instance->Random.Random()) % 6 != 0) ? 25 : 0;
	child->Translucency = parent->Translucency + translucencyBonus;
}

void FakeParticleSystemClass::SpawnSmokeParticles()
{
	// Check spawn timing
	if ((Unsorted::CurrentFrame % (int)this->SpawnFrames) != 0)
	{
		return;
	}

	// Check if owner prevents spawning
	if (auto pFoot = flag_cast_to<FootClass*>(this->Owner))
	{
		if (pFoot->TubeIndex != 0)
		{
			return; // Special condition prevents smoke
		}
	}

	// Calculate random spawn position
	const int spawnRadius = this->Type->SpawnRadius + 1;
	const int randomX = ScenarioClass::Instance->Random.Random() % spawnRadius;
	const int randomY = ScenarioClass::Instance->Random.Random() % spawnRadius;

	const CoordStruct spawnPos {
		this->Location.X + randomX,
		this->Location.Y + randomY,
		this->Location.Z + 10
	};

	// Create particle

	if (ParticleClass* particle = this->CreateHoldsWhatParticle(spawnPos, this->TargetCoords))
	{

		// Adjust properties based on spawn rate
		if (this->SpawnFrames > this->Type->SpawnTranslucencyCutoff)
		{
			particle->Translucency += 25;
		}

		// Adjust velocity based on spawn delay
		const float velocityReduction = (this->SpawnFrames - this->Type->SpawnFrames) * 0.35f;
		particle->Velocity = std::max(2.0f, particle->Velocity - velocityReduction);
	}
}

void FakeParticleSystemClass::UpdateSmokeAttachedPosition()
{
	AbstractClass* owner = this->Owner;
	if (!owner)
	{
		return;
	}

	// Only update for active non-building owners
	if (!flag_cast_to<ObjectClass*, false>(owner))
	{
		return;
	}

	if (owner->WhatAmI() == BuildingClass::AbsID)
	{
		return;
	}

	// Get owner's center position
	CoordStruct ownerCenter = owner->GetCoords();

	// Apply spawn offset
	const CoordStruct newPos {
		ownerCenter.X + this->SpawnDistanceToOwner.X,
		ownerCenter.Y + this->SpawnDistanceToOwner.Y,
		ownerCenter.Z + this->SpawnDistanceToOwner.Z
	};

	this->SetLocation(newPos);
}

void FakeParticleSystemClass::UpdateSpawnTiming()
{
	// Gradually slow down spawn rate
	this->SpawnFrames += this->Type->Slowdown;

	// Mark for death when spawn rate exceeds cutoff
	if (this->SpawnFrames > this->Type->SpawnCutoff)
	{
		this->TimeToDie = 1;
	}
}

void FakeParticleSystemClass::__Smoke_AI()
{
	// Update position if attached to owner
	this->UpdateSmokeAttachedPosition();

	// Update all particles
	this->UpdateAllParticlesFront();

	// Handle particle transitions and removal
	this->ProcessParticleLifecycle<&FakeParticleSystemClass::SpawnChildParticles>();

	// Spawn new smoke particles if active
	if (!this->TimeToDie && this->IsAlive)
	{
		this->SpawnSmokeParticles();
	}

	// Update spawn timing
	this->UpdateSpawnTiming();
}
#pragma endregion

#pragma region Fire

void FakeParticleSystemClass::__Fire_AI()
{
	// Update existing particles
	this->UpdateAndCoordAllParticles();

	// Remove dead particles
	this->RemoveDeadParticles();

	// Update system position if attached to unit
	const bool positionUpdated = this->UpdateAttachedPosition();

	// Spawn new particles if system is active
	if (!this->TimeToDie)
	{
		this->SpawnFireParticles(positionUpdated);
	}
}

bool FakeParticleSystemClass::UpdateAttachedPosition()
{
	TechnoClass* owner = flag_cast_to<TechnoClass*>(this->Owner);

	// Remove system if owner is gone
	if (!owner || !owner->IsAlive)
	{
		this->UnInit();
		return false;
	}

	// Update position if owner is rotating and has target
	if (owner->Target && owner->PrimaryFacing.Is_Rotating())
	{
		this->UpdatePositionFromOwner(owner);
		return true;
	}

	return false;
}

void FakeParticleSystemClass::UpdatePositionFromOwner(TechnoClass* owner)
{
	// Get target position
	CoordStruct targetPos = owner->Target->GetCoords();

	// Calculate direction vector
	const CoordStruct ownerPos = owner->Location;
	const Vector3D<float>  direction {
		static_cast<float>(targetPos.X - ownerPos.X),
		static_cast<float>(targetPos.Y - ownerPos.Y),
		static_cast<float>(targetPos.Z - ownerPos.Z)
	};

	const float distance = (float)direction.Length();

	// Get current facing
	DirStruct facing = owner->PrimaryFacing.Current();

	// Calculate offset based on facing
	const double facingAngle = (facing.Raw - Math::BINARY_ANGLE_MASK) * Math::DIRECTION_FIXED_MAGIC;
	const float offsetX = Math::cos(facingAngle) * distance + ownerPos.X;
	const float offsetY = (float)ownerPos.Y - Math::sin(facingAngle) * distance;

	// Update target and system position
	CoordStruct newPos { (int)offsetX, (int)offsetY, ownerPos.Z };
	this->TargetCoords = newPos;
	CoordStruct flh;
	owner->GetFLH(&flh, 0, CoordStruct::Empty);
	this->SetLocation(flh);
}

#include <WaveClass.h>

void FakeParticleSystemClass::SpawnFireParticles(bool forceSpawn)
{
	if (!this->IsAlive)
	{
		return;
	}

	// Check spawn timing
	const bool shouldSpawn = (Unsorted::CurrentFrame % this->Type->SpawnFrames) == 0 ||
		((Unsorted::CurrentFrame % 3) == 0 && forceSpawn);

	if (!shouldSpawn)
	{
		return;
	}

	// Calculate spawn parameters
	const float distanceToTarget = float((this->Location - this->TargetCoords).Length());
	const int waveIntensity = (distanceToTarget >= 200.0f) ? 1 : 3;

	// Calculate wave offset
	const int wavePhase = Unsorted::CurrentFrame % 500;
	const int waveOffset = int(int(12 * WaveClass::SonicBeamSineTable[wavePhase]) / (waveIntensity * 3.0f));

	// Get direction to target
	Point2D sourcePos { this->Location.X, this->Location.Y };
	Point2D targetPos { this->TargetCoords.X, this->TargetCoords.Y };
	const int direction = Game::Point2DToDir8(&sourcePos, &targetPos);

	//static constexpr Point2D Directions8[8] = {
	//	{  1,  0 }, // East
	//	{  1,  1 }, // South-East
	//	{  0,  1 }, // South
	//	{ -1,  1 }, // South-West
	//	{ -1,  0 }, // West
	//	{ -1, -1 }, // North-West
	//	{  0, -1 }, // North
	//	{  1, -1 }  // North-East
	//	};

	auto& dir = this->_GetTypeExtData()->FacingMult[direction];

	// Calculate spawn position with wave
	CoordStruct spawnPos {
		this->TargetCoords.X + waveOffset * dir.X,
		this->TargetCoords.Y + waveOffset * dir.Y,
		this->TargetCoords.Z
	};

	// Spawn particle
	CoordStruct sourceCoord {
		this->Location.X,
		this->Location.Y,
		this->Location.Z + 1
	};

	this->SpawnHeldParticleRandom(&sourceCoord, &spawnPos, 4);
}

#pragma endregion

#pragma region Railgun

void FakeParticleSystemClass::__Railgun_AI()
{
	// Initialize spiral on first frame
	if (!this->TimeToDie && !this->Particles.Count)
	{
		this->CreateSpiralTrail();
		this->CreateLaserBeam();
		this->TimeToDie = 1;
	}

	// Update all particles
	this->UpdateAllParticlesFront();

	// Remove dead particles
	this->RemoveDeadParticles();
}

void FakeParticleSystemClass::SpawnSpiralParticle(int index, int totalCount, float distance, const Matrix3D& rotationMatrix)
{
	const float progress = static_cast<float>(index) / static_cast<float>(totalCount);

	// Calculate spiral position
	const Vector3D<float> spiralOffset = this->CalculateSpiralOffset(progress, distance, rotationMatrix);

	// Calculate spawn position along path
	const CoordStruct spawnPos = this->CalculateSpawnPosition(progress, spiralOffset);

	// Create particle
	if (ParticleClass* particle = this->CreateHoldsWhatParticle(spawnPos, spawnPos))
	{
		// Setup particle direction and velocity
		this->SetupParticleVelocity(particle, spiralOffset, progress);
	}
}

Vector3D<float> FakeParticleSystemClass::CalculateSpiralOffset(float progress, float distance, const Matrix3D& rotationMatrix)
{
	// Calculate spiral angle
	const float spiralAngle = float(distance * progress * this->Type->SpiralDeltaPerCoord);

	// Create spiral circle point
	const Vector3D<float> circlePoint {
		0.0f,
		(float)Math::cos(spiralAngle),
		(float)Math::sin(spiralAngle)
	};

	// Rotate to align with trajectory and Scale by spiral radius
	return  rotationMatrix.RotateVector(circlePoint) * this->Type->SpiralRadius;
}

CoordStruct FakeParticleSystemClass::CalculateSpawnPosition(float progress, const Vector3D<float>& spiralOffset)
{
	// Add random position perturbation
	const Vector3D<float>  perturbation = GetRandomPerturbation(
		(float)this->Type->PositionPerturbationCoefficient
	);

	// Interpolate along path
	Vector3D<float> startPt {
		(float)this->Location.X,
		(float)this->Location.Y,
		(float)this->Location.Z
	};

	Vector3D<float> targetPt {
		(float)this->TargetCoords.X,
		(float)this->TargetCoords.Y,
		(float)this->TargetCoords.Z
	};

	const Vector3D<float> result = startPt.Lerp(targetPt, progress) + spiralOffset + perturbation;
	// Add spiral offset and perturbation
	return {
		static_cast<int>(result.X),
		static_cast<int>(result.Y),
		static_cast<int>(result.Z)
	};
}

void FakeParticleSystemClass::SetupParticleVelocity(ParticleClass* particle, const Vector3D<float>& direction,
							   float progress)
{
	// Copy direction and add movement perturbation
	Vector3D<float> velocity = direction;
	const Vector3D<float> movementPert = GetRandomPerturbation(
		(float)this->Type->MovementPerturbationCoefficient
	);

	velocity.X += movementPert.X;
	velocity.Y += movementPert.Y;
	velocity.Z += movementPert.Z;

	// Normalize direction

	const float length = (float)velocity.Length();

	if (length > 0.0f)
	{
		velocity.X /= length;
		velocity.Y /= length;
		velocity.Z /= length;
	}

	particle->Spark10C = velocity;

	// Calculate velocity magnitude with perturbation
	const float baseVelocity = particle->Type->Velocity;
	const float velocityPert = this->CalculateVelocityPerturbation(progress);
	particle->Velocity = baseVelocity + velocityPert;
}

float FakeParticleSystemClass::CalculateVelocityPerturbation(float progress) const
{
	const float random = (float)ScenarioClass::Instance->Random.RandomDoubleCentered();
	const float perturbation = (float)((random + progress) * (this->Type->VelocityPerturbationCoefficient * 0.5f));

	// Clamp to coefficient range
	const float minPert = (float)-this->Type->MovementPerturbationCoefficient;
	const float maxPert = (float)this->Type->VelocityPerturbationCoefficient;

	return std::clamp(perturbation, minPert, maxPert);
}

void FakeParticleSystemClass::CreateSpiralTrail()
{
	// Calculate trajectory vector
	const Vector3D<float> trajectory = this->CalculateTrajectory();
	const float distance = (float)trajectory.Length();
	const int particleCount = static_cast<int>(distance * this->Type->ParticlesPerCoord);

	// Create rotation matrix for spiral
	Matrix3D rotationMatrix = CreateRotationMatrix(trajectory, distance);

	// Spawn particles along spiral path
	for (int i = 0; i < particleCount; ++i)
	{
		this->SpawnSpiralParticle(i, particleCount, distance, rotationMatrix);
	}
}

void FakeParticleSystemClass::CreateLaserBeam()
{
	if (!this->Type->Laser)
	{
		return;
	}

	ColorStruct outerColor { 0, 0, 0 };
	ColorStruct blankColor { 0, 0, 0 };

	GameCreate<LaserDrawClass>(
		this->Location,
		this->TargetCoords,
		0,
		1,
		this->Type->LaserColor,
		outerColor,
		blankColor,
		10,
		0,
		1,
		0.0f,
		0.5f
	);
}

#pragma endregion


DEFINE_FUNCTION_JUMP(VTABLE, 0x7EFBF8, FakeParticleSystemClass::__AI)
