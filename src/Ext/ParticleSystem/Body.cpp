#include "Body.h"

#include <Ext/ParticleSystemType/Body.h>
#include <Ext/ParticleType/Body.h>

#include <Ext/Rules/Body.h>
#include <Utilities/Macro.h>

#include <ParticleTypeClass.h>
#include <ParticleClass.h>
#include <SpotlightClass.h>
#include <GameOptionsClass.h>
#include <TacticalClass.h>

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

#ifdef azadasd
void ParticleSystemExtData::UpdateRailgun()
{
	auto pOwnerObj = this->AttachedToObject;
	auto pOwnerObjType = pOwnerObj->Type;

	if (!pOwnerObj->TimeToDie && this->OtherParticleData.empty())
	{
		const auto nParticleLoc = pOwnerObj->Location;
		const auto nTargetLoc = pOwnerObj->TargetCoords;
		const auto nDifferenct = (nParticleLoc - nTargetLoc);

		pOwnerObj->TimeToDie = true;
		auto nDifferenceLength = (int)nDifferenct.Length();
		auto nMaxXY = (int)(Point2D { nDifferenct.X , nDifferenct.Y }.Length());
		auto nMagNeg = -nDifferenceLength;
		auto nMagCopy = nDifferenceLength >= nDifferenct.Z ?
			nDifferenct.Z : nDifferenceLength;

		if (nMagCopy < nMagNeg)
			nMagCopy = nMagNeg;

		auto const nMaxXYNeg = -(double)nMaxXY;

		auto nMaxXYCopy = (double)nMaxXY >= ((double)nDifferenct.X) ?
			((double)nDifferenct.X) : (double)nDifferenceLength;

		if (nMaxXYCopy < nMaxXYNeg)
			nMaxXYCopy = nMaxXYNeg;

		auto nASin = Math::asin(double(nMagCopy / nDifferenceLength));
		auto nACos = Math::acos(nMaxXYCopy / nMaxXY);
		Matrix3D mtx {};
		mtx.MakeIdentity();
		mtx.RotateZ(nDifferenct.Y > 0 ? nACos : -nACos);
		mtx.RotateX(nASin);

		auto pHeldType = this->HeldType;
		auto nSpinDelta = pOwnerObjType->SpiralDeltaPerCoord;
		auto nVel = pHeldType->Velocity;
		auto nSpiralRadius = pOwnerObjType->SpiralRadius;
		auto nPositionPerturbationCoefficient = pOwnerObjType->PositionPerturbationCoefficient;
		auto nMovementPerturbationCoefficient = pOwnerObjType->MovementPerturbationCoefficient;
		auto nVelocityPerturbationCoefficient = pOwnerObjType->VelocityPerturbationCoefficient;
		int nDecidedsize = (int)(pOwnerObjType->ParticlesPerCoord * nDifferenceLength);
		this->OtherParticleData.resize(nDecidedsize);

		auto nMovementPerturbationCoefficientneg = -nMovementPerturbationCoefficient;
		double nVal = 0.0;

		for (int i = 0; i < nDecidedsize; ++i)
		{
			const double v91 = double((float)i / (double)nDecidedsize);
			const auto radians = v91 * nDifferenceLength * nSpinDelta;

			Vector3D<float> nDummy {
				0.0f,
				Math::cos(radians) ,
				Math::sin(radians)
			};

			Vector3D<float> nResult = Matrix3D::MatrixMultiply(mtx, nDummy);

			//============== LerpCoords
			const auto  val__ = 1.0 - v91;

			CoordStruct nDummy_d {
			int((nTargetLoc.X * val__)
				+ (nParticleLoc.X * v91)
				+ (ScenarioClass::Instance->Random.RandomDouble_Closest() * nPositionPerturbationCoefficient + nResult.X * nSpiralRadius))
			,
			int((val__ * nTargetLoc.Y)
				+ (v91 * nParticleLoc.Y)
				+ (nSpiralRadius * nResult.Y + nPositionPerturbationCoefficient * ScenarioClass::Instance->Random.RandomDouble_Closest()))
			,
			int((nParticleLoc.Z * v91)
				+ (nTargetLoc.Z * val__)
				+ (ScenarioClass::Instance->Random.RandomDouble_Closest() * nPositionPerturbationCoefficient + nResult.Z * nSpiralRadius))
			};
			//=====================

			//============= MovementPerturbationCoeff
			Vector3D<float> nMovementDummy {
				float((ScenarioClass::Instance->Random.RandomDouble_Closest() * nMovementPerturbationCoefficient) + nResult.X),
				float((ScenarioClass::Instance->Random.RandomDouble_Closest() * nMovementPerturbationCoefficient) + nResult.Y),
				float((ScenarioClass::Instance->Random.RandomDouble_Closest() * nMovementPerturbationCoefficient) + nResult.Z)
			};
			//=============

			const auto nMag = nMovementDummy.Length();

			if (nMag != 0.0)
			{
				const auto idkHere = (float)(1.0 / nMag);
				nMovementDummy.X = nMovementDummy.X * idkHere;
				nMovementDummy.Y = nMovementDummy.Y * idkHere;
				nMovementDummy.Z = idkHere * nMovementDummy.Z;
			}

			const auto nRand_Double7 = ScenarioClass::Instance->Random.RandomDouble_Closest() + nVal;

			auto nVelocityPerturbationCoefficient_copy = nVelocityPerturbationCoefficient;

			nVal = 0.5 * (nRand_Double7 * nVelocityPerturbationCoefficient);

			if (nVal <= nVelocityPerturbationCoefficient)
				nVelocityPerturbationCoefficient_copy = 0.5 * nRand_Double7;
			else
				nVal = nVelocityPerturbationCoefficient_copy;

			if (nMovementPerturbationCoefficientneg > nVelocityPerturbationCoefficient_copy)
				nVal = nMovementPerturbationCoefficientneg;

			auto Data = &this->OtherParticleData[i]; // .emplace_back();

			Data->velB = nMovementDummy; // storing MovementPerturbationCoeff vector

			// this one use for coordinate storage that already lerp with the cur pos and target pos
			Data->vel.X = nDummy_d.X;
			Data->vel.Y = nDummy_d.Y;
			Data->vel.Z = nDummy_d.Z;
			//

			Data->A = float(nVel + nVal); // velocity multiplier
			// particle life times
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
#endif

void  ParticleSystemExtData::UpdateRailgun()
{
	auto pThis = this->AttachedToObject;

	if (!pThis->TimeToDie && !this->OtherParticleData.size())
	{
		CoordStruct targetCoords = pThis->TargetCoords;
		CoordStruct currentCoords = pThis->Location;

		CoordStruct DifferenceCoords = targetCoords - currentCoords;
		int differeceCoordsLXYZLength = (int)std::sqrt(double((DifferenceCoords.X * DifferenceCoords.X) + (DifferenceCoords.Y * DifferenceCoords.Y) + (DifferenceCoords.Z * DifferenceCoords.Z)));
		int differeceCoordsLXYLength = (int)std::sqrt(double((DifferenceCoords.X * DifferenceCoords.X) + (DifferenceCoords.Y * DifferenceCoords.Y)));
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
			Matrix3D::MatrixMultiply(&mtx_mult  , &mtx, &first_);

			const Vector3D<float> PositionPerturbation_ {
				float(ScenarioClass::Instance->Random.RandomDouble_Closest() * pThis->Type->PositionPerturbationCoefficient
				+ (first_.X * pThis->Type->SpiralRadius))

				,float(ScenarioClass::Instance->Random.RandomDouble_Closest() * pThis->Type->PositionPerturbationCoefficient
				+ (first_.Y * pThis->Type->SpiralRadius))

				,float(ScenarioClass::Instance->Random.RandomDouble_Closest() * pThis->Type->PositionPerturbationCoefficient
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
				float(ScenarioClass::Instance->Random.RandomDouble_Closest() * pThis->Type->MovementPerturbationCoefficient)
				, float(ScenarioClass::Instance->Random.RandomDouble_Closest() * pThis->Type->MovementPerturbationCoefficient)
				, float(ScenarioClass::Instance->Random.RandomDouble_Closest() * pThis->Type->MovementPerturbationCoefficient)
			};

			mtx_mult += MovementPerturbation;

			const auto sparkVelLength = std::sqrt(double((mtx_mult.X * mtx_mult.X) + (mtx_mult.Y * mtx_mult.Y) + (mtx_mult.Z * mtx_mult.Z)));

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
		movement.A = float(ScenarioClass::Instance->Random.RandomDouble_Closest() * 0.1 + state);
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

	FootClass* Owner_obj = flag_cast_to<FootClass*>(pOwnerObj_Owner);

	if (Owner_obj) {
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

#ifdef zaawdawd
void Railgun_AI_Vanilla_Test(ParticleSystemClass* pThis)
{

	if (!pThis->TimeToDie && !pThis->Particles.Count)
	{
		CoordStruct targetCoords = pThis->TargetCoords;
		CoordStruct currentCoords = pThis->Location;

		CoordStruct DifferenceCoords = targetCoords - currentCoords;
		int differeceCoordsLXYZLength = (int)std::sqrt(double((DifferenceCoords.X * DifferenceCoords.X) + (DifferenceCoords.Y * DifferenceCoords.Y) + (DifferenceCoords.Z * DifferenceCoords.Z)));
		int differeceCoordsLXYLength = (int)std::sqrt(double((DifferenceCoords.X * DifferenceCoords.X) + (DifferenceCoords.Y * DifferenceCoords.Y)));
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
		Matrix3D mtx {};
		mtx.MakeIdentity();
		const auto acos = Math::acos((double)Difference_X / (double)differeceCoordsLXYLength);
		mtx.PreRotateZ(acos < 0 ? -acos : acos);
		const auto theta = Math::asin((double)Difference_Z / (double)differeceCoordsLXYLength);
		mtx.PreRotateX(theta);

		double var = 0.0;
		for (int i = 0; i < (int)ParticlePerCoords; ++i)
		{
			float curVal = float((double)i / (float)ParticlePerCoords);

			Vector3D<float> first_ {
				0 , Math::cos((double)curVal) ,Math::sin((double)curVal)
			};

			Vector3D<float> mtx_mult = Matrix3D::MatrixMultiply(&mtx, first_);

			const Vector3D<float> PositionPerturbation_ {
				float(ScenarioClass::Instance->Random.RandomDouble_Closest() * pThis->Type->PositionPerturbationCoefficient
				+ (first_.X * pThis->Type->SpiralRadius))

				,float(ScenarioClass::Instance->Random.RandomDouble_Closest() * pThis->Type->PositionPerturbationCoefficient
				+ (first_.Y * pThis->Type->SpiralRadius))

				,float(ScenarioClass::Instance->Random.RandomDouble_Closest() * pThis->Type->PositionPerturbationCoefficient
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

			pThis->Particles.AddItem(GameCreate<ParticleClass>(ParticleTypeClass::Array->Items[pThis->Type->HoldsWhat], &lerp, &lerp, pThis));
			auto partilce = *pThis->Particles.back();
			Vector3D<float>* vel = &partilce->Spark10C;
			partilce->Spark10C = mtx_mult;
			Vector3D<float> MovementPerturbation {
				float(ScenarioClass::Instance->Random.RandomDouble_Closest() * pThis->Type->MovementPerturbationCoefficient)
				, float(ScenarioClass::Instance->Random.RandomDouble_Closest() * pThis->Type->MovementPerturbationCoefficient)
				, float(ScenarioClass::Instance->Random.RandomDouble_Closest() * pThis->Type->MovementPerturbationCoefficient)
			};
			partilce->Spark10C += MovementPerturbation;

			const auto sparkVelLength = std::sqrt(double((partilce->Spark10C.X * partilce->Spark10C.X) + (partilce->Spark10C.Y * partilce->Spark10C.Y) + (partilce->Spark10C.Z * partilce->Spark10C.Z)));

			Vector3D<float> last = partilce->Spark10C;
			if (sparkVelLength != 0.0)
			{
				last.X = vel->X / sparkVelLength;
				last.Y = vel->Y / sparkVelLength;
				last.Z = vel->Z / sparkVelLength;
			}
			partilce->Spark10C = last; //stored as VelB

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
			//storead as A
			partilce->Velocity = float(var + partilce->Type->Velocity);
		}

		if (pThis->Type->Laser)
		{
			GameCreate<LaserDrawClass>(
				pThis->Location,
				pThis->TargetCoords,
				0, 1u,
				pThis->Type->LaserColor,
				ColorStruct::Empty,
				ColorStruct::Empty,
				10, false, true, 0.5f, 0.1f);
		}

		pThis->TimeToDie = true;
	}

	for (int i = 0; i < pThis->Particles.Count; ++i)
	{
		pThis->Particles[i]->BehaviourUpdate();
	}

	for (int a = pThis->Particles.Count - 1; a >= 0; --a)
	{
		if (pThis->Particles[a]->hasremaining)
			pThis->Particles[a]->UnInit();
	}
}
#endif

//DEFINE_JUMP(LJMP, 0x62ED53, 0x62ED61);

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


	//	switch (this->AttachedToObject->Type->BehavesLike)
	//	{
	//	case ParticleSystemTypeBehavesLike::Smoke:
	//	{
	//		const auto pOwner = this->AttachedToObject;

	//		pOwner->Smoke_AI();
	//		/*
	//		if (auto pAttachedOwner = generic_cast<FootClass*>(pOwner->Owner))
	//		{
	//			auto pAttachedOwner_loc = pAttachedOwner->GetCoords();
	//			pAttachedOwner_loc += pOwner->SpawnDistanceToOwner;
	//			pOwner->SetLocation(pAttachedOwner_loc);
	//		}

	//		pOwner->Particles.for_each([](ParticleClass* pPart) {
	//			pPart->BehaviourUpdate();
	//		});

	//		pOwner->Particles.for_each([pOwner](ParticleClass* pPart) {

	//			if(pPart->hasremaining && pPart->Type->NextParticle == -1){
	//				pPart->UnInit();
	//				return;
	//			} else if (!pPart->hasremaining){
	//				pPart->BehaviourUpdate();
	//				return;
	//			}

	//			if ( pPart->Type->NextParticle != -1) {
	//				auto pPartLoc = pPart->Location;
	//				auto rad = pPart->Type->Radius >> 3;
	//				auto _xAdd = ScenarioClass::Instance->Random.Random() % rad;
	//				rad = _xAdd > 0 ? rad + _xAdd : _xAdd - rad;
	//				pPartLoc.X += rad;
	//				rad = pPart->Type->Radius >> 3;
	//				auto _yAdd = ScenarioClass::Instance->Random.Random() % rad;
	//				rad = _yAdd > 0 ? rad + _yAdd : _yAdd - rad;
	//				pPartLoc.Y += rad;
	//				auto pNext = ParticleTypeClass::Array->Items[pPart->Type->NextParticle];
	//				auto pPart_nn = GameCreate<ParticleClass>(pNext, pPartLoc, CoordStruct::Empty, pOwner);
	//				pOwner->Particles.AddItem(pPart_nn);
	//				pPart_nn->Velocity = pPart->Velocity;
	//				auto v22 = ScenarioClass::Instance->Random.Random() % 6;
	//				pPart_nn->Translucency = pPart->Translucency + (v22 != 0 ? 0x19 : 0);

	//			} else {

	//				auto pPart_nn = GameCreate<ParticleClass>(pPart->Type, pPart->Location, CoordStruct::Empty, pOwner);
	//				pOwner->Particles.AddItem(pPart_nn);
	//				pPart_nn->Velocity = pPart->Velocity;
	//				auto v22 = ScenarioClass::Instance->Random.Random() % 6;
	//				pPart_nn->Translucency = pPart->Translucency + (v22 != 0 ? 0x19 : 0);
	//				pPart->UnInit();
	//			}
	//		});

	//		if (!pOwner->TimeToDie && pOwner->IsAlive && !(Unsorted::CurrentFrame() % (int)pOwner->SpawnFrames))
	//		{

	//		}
	//		*/
	//		pOwner->Lifetime--;
	//		if (pOwner->Lifetime == 0)
	//			pOwner->UnInit();

	//		if (pOwner->IsAlive
	//			&& pOwner->TimeToDie
	//			&& !pOwner->Particles.Count)
	//		{
	//			pOwner->Limbo();
	//			pOwner->IsAlive = false;
	//			AbstractClass::Array2->AddItem(pOwner);
	//		}

	//		return false;
	//	}

	//	case ParticleSystemTypeBehavesLike::Fire:
	//	{
	//		this->AttachedToObject->Fire_AI();

	//		const auto pOwner = this->AttachedToObject;

	//		pOwner->Lifetime--;
	//		if (pOwner->Lifetime == 0)
	//			pOwner->UnInit();

	//		if (pOwner->IsAlive
	//			&& pOwner->TimeToDie
	//			&& !pOwner->Particles.Count)
	//		{
	//			pOwner->Limbo();
	//			pOwner->IsAlive = false;
	//			AbstractClass::Array2->AddItem(pOwner);
	//		}

	//		return false;
	//	}

	//	case ParticleSystemTypeBehavesLike::Gas:
	//	{
	//		this->AttachedToObject->Gas_AI();

	//		const auto pOwner = this->AttachedToObject;

	//		pOwner->Lifetime--;
	//		if (pOwner->Lifetime == 0)
	//			pOwner->UnInit();

	//		if (pOwner->IsAlive
	//			&& pOwner->TimeToDie
	//			&& !pOwner->Particles.Count)
	//		{
	//			pOwner->Limbo();
	//			pOwner->IsAlive = false;
	//			AbstractClass::Array2->AddItem(pOwner);
	//		}

	//		return false;
	//	}

	//	case ParticleSystemTypeBehavesLike::Railgun:
	//	{
	//		this->AttachedToObject->Railgun_AI();

	//		const auto pOwner = this->AttachedToObject;

	//		pOwner->Lifetime--;
	//		if (pOwner->Lifetime == 0)
	//			pOwner->UnInit();

	//		if (pOwner->IsAlive
	//			&& pOwner->TimeToDie
	//			&& !pOwner->Particles.Count)
	//		{
	//			pOwner->Limbo();
	//			pOwner->IsAlive = false;
	//			AbstractClass::Array2->AddItem(pOwner);
	//		}

	//		return false;
	//	}
	//	default:
			return false;
	//	}
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
			Point2D outClient = TacticalClass::Instance->CoordsToClient(Coord);

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
						//ColorStruct emp { 255 , 0 , 0 };

						if ((uint16_t)buff >= 127u)
						{
							const auto put_Color = DSurface::RGBA_To_Pixel(emp.R, emp.G, emp.B);
							DSurface::Temp->Put_Pixel(outClient, put_Color);
						}
						else
						{
							uintptr_t data_r = (buff * emp.R) >> 7;
							uintptr_t data_g = (buff * emp.G) >> 7;
							uintptr_t data_b = (buff * emp.B) >> 7;
							const auto put_Color = DSurface::RGBA_To_Pixel(data_r, data_g, data_b);
							DSurface::Temp->Put_Pixel(outClient, put_Color);
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
			Point2D outClient = TacticalClass::Instance->CoordsToClient(draw.vel);
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
				pal = pManager->GetOrDefaultConvert<PaletteManager::Mode::Temperate>(pal);

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
				if (!nBehave) {
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

template <typename T>
void ParticleSystemExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->Initialized)
		.Process(this->What)
		.Process(this->OtherParticleData)
		.Process(this->SmokeData)
		.Process(this->HeldType, true)
		.Process(this->AlphaIsLightFlash)
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

#include <Misc/Hooks.Otamaa.h>

HRESULT __stdcall FakeParticleSystemClass::_Load(IStream* pStm)
{

	ParticleSystemExtContainer::Instance.PrepareStream(this, pStm);
	HRESULT res = this->ParticleSystemClass::Load(pStm);

	if (SUCCEEDED(res))
		ParticleSystemExtContainer::Instance.LoadStatic();

	return res;
}

HRESULT __stdcall FakeParticleSystemClass::_Save(IStream* pStm, bool clearDirty)
{

	ParticleSystemExtContainer::Instance.PrepareStream(this, pStm);
	HRESULT res = this->ParticleSystemClass::Save(pStm, clearDirty);

	if (SUCCEEDED(res))
		ParticleSystemExtContainer::Instance.SaveStatic();

	return res;
}

DEFINE_JUMP(VTABLE, 0x7EFBB0, MiscTools::to_DWORD(&FakeParticleSystemClass::_Load))
DEFINE_JUMP(VTABLE, 0x7EFBB4, MiscTools::to_DWORD(&FakeParticleSystemClass::_Save))