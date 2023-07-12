#include <Ext/ParticleSystem/Body.h>

#include <ScenarioClass.h>
#include <Notifications.h>
#include <Utilities/Constructs.h>
#include <GameOptionsClass.h>
#include <SpotlightClass.h>
#include <LaserDrawClass.h>

// have an idea how it works little bit 
// need more stuffs to finish this !
// 
struct ParticleSystemExt_ExtData
{
	struct __declspec(align(4)) ParticleSysExt_someDatas
	{
		Vector3D<float> vel;
		Vector3D<float> velB;
		DWORD A;
		float B;
		int C;
		int MaxEC;
		BYTE Empty;
		ColorStruct Colors;
	};


	std::vector<ParticleSysExt_someDatas> PreCalculatedParticlesData;
	std::vector<ParticleSysExt_someDatas> SomeArray_b;
	ParticleSystemClass* OwnerObject;
	ParticleTypeClass* AdditionalHeldType;
	int Behave;

	static void HandleSpark(ParticleSystemExt::ExtData* pThis)
	{
		auto const pOwnerObj = pThis->OwnerObject();
		auto pSparkFramesHere = pOwnerObj->SparkSpawnFrames;

		if (pSparkFramesHere-- > 0)
		{
			pOwnerObj->SparkSpawnFrames = pSparkFramesHere;

			if (pSparkFramesHere-- <= 0)
				pOwnerObj->TimeToDie = 1;

			auto& nRand = ScenarioClass::Instance->Random;

			auto const nRandDoubleHere = nRand.RandomDouble();
			if (nRandDoubleHere < 0.3)
			{
				auto nSpotRadius = pOwnerObj->SpotlightRadius - 3;
				if (nSpotRadius < 17)
					nSpotRadius = 17;

				pOwnerObj->SpotlightRadius = nSpotRadius;
			}
			else if (nRandDoubleHere < 0.6)
			{
				auto nSpotRadius = pOwnerObj->SpotlightRadius + 3;
				if (nSpotRadius > 41)
					nSpotRadius = 41;

				pOwnerObj->SpotlightRadius = nSpotRadius;
			}

			auto const pOwnerObjType = pOwnerObj->Type;
			auto const nRandDoubleHere_2 = nRand.RandomDouble();
			if (!pOwnerObj->SparkSpawnFrames || pOwnerObjType->SpawnSparkPercentage > nRandDoubleHere_2)
			{
				int nCap = 0;
				auto pAdd = pThis->AdditionalHeldType;
				if (pOwnerObjType->ParticleCap >= 0)
					nCap = pOwnerObjType->ParticleCap;

				auto const nHalfCap = nCap / 2;
				auto const nRandCap = nRand.RandomFromMax(nHalfCap);
				const auto nSize = (size_t)(nHalfCap + nRandCap);
				pThis->PreCalculatedParticlesData.resize(nSize);

				for (size_t i = nSize; i > 0; --i)
				{
					auto const nVelX = nRand.RandomFromMax(pAdd->XVelocity);
					auto const nVelY = nRand.RandomFromMax(pAdd->YVelocity);
					auto const nVelZ = pAdd->MinZVelocity + nRand.RandomFromMax(pAdd->ZVelocityRange);
					Vector3D<float> nVels { (float)nVelX  , (float)nVelY ,(float)nVelZ };
					auto const nVelsMag = nVels.Magnitude();
					auto const nSpawnDir = pOwnerObjType->SpawnDirection + nVels;
					auto const nSpawnDirMag = nVels.Magnitude();
					Vector3D<float> nVelsC {};

					if (nSpawnDirMag != 0.0)
					{
						const auto idkHere = (float)(1.0 / nSpawnDirMag);
						nVelsC.X = nSpawnDir.X * idkHere;
						nVelsC.Y = nSpawnDir.Y * idkHere;
						nVelsC.Z = idkHere * nSpawnDir.Z;
					}

					auto Data = &pThis->PreCalculatedParticlesData[i];
					if (Data == pThis->PreCalculatedParticlesData.end()._Ptr) {
						pThis->PreCalculatedParticlesData.emplace_back();
						Data = &pThis->PreCalculatedParticlesData.back();
					}

					Data->velB = (nVelsC * nVelsMag);
					Data->vel.X = (float)pOwnerObj->Location.X;
					Data->vel.Y = (float)pOwnerObj->Location.Y;
					Data->vel.Z = (float)pOwnerObj->Location.Z;
					Data->A = 0.0f;
					Data->B = 0.0f;
					Data->C = 0.0f;
					Data->Empty = false;
					Data->MaxEC = pAdd->MaxEC + nRand.RandomFromMax(pAdd->MaxEC);

					if (pAdd->StartColor1 && pAdd->StartColor2)
					{
						auto const nFact = (float)nRand.RandomDouble();
						Data->Colors.Lerp(pAdd->StartColor1, pAdd->StartColor2, nFact);
					}
					else
					{
						if (pAdd->ColorList.Count)
						{
							Data->Colors.R = pAdd->ColorList[0]->Red;
							Data->Colors.G = pAdd->ColorList[0]->Green;
							Data->Colors.B = pAdd->ColorList[0]->Blue;
						}
					}
				}

				if (GameOptionsClass::Instance->GameSpeed == 2)
				{
					if (pOwnerObj->SparkSpawnFrames == pOwnerObjType->SparkSpawnFrames &&
						!pOwnerObjType->OneFrameLight && pOwnerObjType->LightSize > 0
						)
					{
						GameCreate<SpotlightClass>(pOwnerObj->Location, pOwnerObjType->LightSize);
					}
				}
			}
		}

		UpdateLocation(pThis);
		UpdateState(pThis);
		UpdateColor(pThis);
	}

	static void UpdateLocation(ParticleSystemExt::ExtData* pThis)
	{
		auto const Gravity = RulesClass::Instance->Gravity;

		for (auto& Data : pThis->PreCalculatedParticlesData)
		{
			if (--Data.MaxEC <= 0)
			{
				Data.Empty = true;
				continue;
			}

			Data.velB.Z = Data.velB.Z - Gravity;
			Data.vel = Data.vel + Data.velB;
			CoordStruct nVelBCoord = { (int)Data.velB.X , (int)Data.velB.Y , (int)Data.velB.Z };
			CoordStruct nCoord { (int)Data.vel.X , (int)Data.vel.Y , (int)Data.vel.Z };
			auto const pCell = MapClass::Instance->GetCellAt(nCoord);
			const auto nZOffs = pCell->GetFloorHeight({ nCoord.X , nCoord.Y });

			if (nCoord.Z < nZOffs)
				break;

			auto const pVelBCoordCell = MapClass::Instance->GetCellAt(nVelBCoord);
			if (pCell->ContainsBridge() || pVelBCoordCell->ContainsBridge())
			{
				const auto nZOffsWithBridge = nZOffs + CellClass::BridgeHeight;
				if (nCoord.Z >= nZOffsWithBridge && nVelBCoord.Z < nZOffsWithBridge)
					break;
				else if (nVelBCoord.Z >= nZOffsWithBridge)
					break;
			}

			if (nCoord.Z - 150 < nZOffs)
			{
				auto const pBuilding = pCell->GetBuilding();

				if (pBuilding)
				{
					if (pBuilding->Type->LaserFence || pBuilding->LaserFenceFrame < 8)
					{
						if (!pBuilding->CanDeploySlashUnload())
							Data.Empty = true;

						continue;
					}
				}

				if (pCell->ConnectsToOverlay())
					Data.Empty = true;

				continue;
			}

			Data.Empty = true;
		}
	}

	static void UpdateState(ParticleSystemExt::ExtData* pThis)
	{
		auto v1 = pThis->PreCalculatedParticlesData.end();
		auto v2 = pThis->PreCalculatedParticlesData.begin();

		if (v2 != v1)
		{
			do
			{
				if (v2->Empty)
					break;

				++v2;
			}
			while (v2 != v1);

			if (v2 != v1)
			{
				for (auto i = v2 + 1; i != v1; ++i)
				{
					if (!i->Empty)
					{
						std::memcpy(&(*(v2++)), &(*i), sizeof(ParticleSysExt_someDatas));
					}
				}
			}
		}

		if (v2 != pThis->PreCalculatedParticlesData.end())
			pThis->PreCalculatedParticlesData.end() = v2;

		// moving empty state back ?
		auto v5 = pThis->SomeArray_b.end();
		auto v6 = pThis->SomeArray_b.begin();

		if (v6 != v5)
		{
			do
			{
				if ((*v6).Empty)
					break;

				++v6;
			}
			while (v6 != v5);
			if (v6 != v5)
			{
				for (auto j = v6 + 1; j != v5; ++j)
				{
					if (!(*j).Empty)
					{
						std::memcpy(&(*(v6++)), &(*j), sizeof(ParticleSysExt_someDatas));
					}
				}
			}
		}

		if (v6 != pThis->SomeArray_b.end())
			pThis->SomeArray_b.end() = v6;
	}

	static void UpdateColor(ParticleSystemExt::ExtData* pThis)
	{
		const auto pHeldType = pThis->AdditionalHeldType;
		const auto v5 = pHeldType->ColorList.Count - 2;

		for (auto& Data : pThis->PreCalculatedParticlesData)
		{
			auto i = pHeldType->ColorSpeed;
			const auto nRandDouble = ScenarioClass::Instance->Random.RandomDouble();
			const auto v6 = nRandDouble + i + Data.B;
			double v7 = 1.0;

			if (v6 <= 1.0)
			{
				v7 = v6;
			}
			else
			{
				auto result = Data.C;
				if (result < v5)
				{
					result = (result + 1);
					v7 = 0.0;
					Data.C = result;
				}
			}

			Data.B = (float)v7;
		}
	}

	static void HandleCaseTwo(ParticleSystemExt::ExtData* pThis)
	{
		auto const pOwnerObj = pThis->OwnerObject();
		auto const pOwnerObjType = pOwnerObj->Type;
		if (!pOwnerObj->TimeToDie && pThis->PreCalculatedParticlesData.begin() == pThis->PreCalculatedParticlesData.end())
		{
			pOwnerObj->TimeToDie = true;
			auto const nDifferenct = (pOwnerObj->Location - pOwnerObj->TargetCoords);
			auto const nMagSquared = nDifferenct.MagnitudeSquared();
			auto const nMaxXY = CoordStruct { nDifferenct.X , nDifferenct.Y , 0 }.MagnitudeSquared();
			auto const nMagNeg = -nMagSquared;
			auto nMagCopy = nMagSquared >= ((double)nDifferenct.Z) ?
				((double)nDifferenct.Z) : nMagSquared;

			if (nMagCopy < nMagNeg)
				nMagCopy = nMagNeg;

			auto const nMaxXYNeg = -nMaxXY;

			auto nMaxXYCopy = nMaxXY >= ((double)nDifferenct.X) ?
				((double)nDifferenct.X) : nMagSquared;

			if (nMaxXYCopy < nMaxXYNeg)
				nMaxXYCopy = nMaxXYNeg;

			const auto nSin = (float)Math::sin(nMagCopy / nMagSquared);
			const auto nCos = (float)Math::cos(nMaxXYCopy / nMaxXY) * ((((nDifferenct.Y - nDifferenct.Y) >> 0x1F) & 0xFFFFFFFE) + 1);
			Matrix3D mtx {};
			mtx.MakeIdentity();
			mtx.RotateZ(nCos);
			mtx.RotateX(nSin);
			const auto pHeldType = pThis->AdditionalHeldType;
			const auto nSpinDelta = pOwnerObjType->SpiralDeltaPerCoord;
			const auto nVel = pHeldType->Velocity;
			const auto nSpiralRadius = pOwnerObjType->SpiralRadius;
			const auto nPositionPerturbationCoefficient = pOwnerObjType->PositionPerturbationCoefficient;
			const auto nMovementPerturbationCoefficient = pOwnerObjType->MovementPerturbationCoefficient;
			const auto nVelocityPerturbationCoefficient = pOwnerObjType->VelocityPerturbationCoefficient;
			const size_t nDecidedsize = (size_t)(pOwnerObjType->ParticlesPerCoord * nMagSquared);
			pThis->PreCalculatedParticlesData.resize(nDecidedsize);

			if (nDecidedsize > 0)
			{
				double nVal = 0.0;
				for (size_t i = 0; i < nDecidedsize; ++i)
				{
					auto nMovementPerturbationCoefficientneg = -nMovementPerturbationCoefficient;
					Vector3D<float> nDummy {};
					const auto v91 = i / nDecidedsize;
					const auto radians = v91 * nMagSquared * nSpinDelta;
					nDummy.X = 0.0f;
					nDummy.Y = Math::cos((float)radians);
					nDummy.Z = Math::sin((float)radians);
					const auto nResult = Matrix3D::MatrixMultiply(mtx, nDummy);
					const auto nRand_Double1 = ScenarioClass::Instance->Random.RandomDouble();
					const auto nRand_Double2 = ScenarioClass::Instance->Random.RandomDouble();
					const auto nRand_Double3 = ScenarioClass::Instance->Random.RandomDouble();
					const auto v91conv = 1.0 - v91;

					Vector3D<float> nDummy_d {};
					nDummy_d.X = (pOwnerObj->TargetCoords.X * v91conv)
						+ (pOwnerObj->Location.X * v91)
						+ (nRand_Double1 * nPositionPerturbationCoefficient + nResult.X * nSpiralRadius);
					nDummy_d.Y = (v91conv * pOwnerObj->TargetCoords.Y)
						+ (v91 * pOwnerObj->Location.Y)
						+ (nSpiralRadius * nResult.Y + nPositionPerturbationCoefficient * nRand_Double2);
					nDummy_d.Z = (pOwnerObj->Location.Z * v91)
						+ (pOwnerObj->TargetCoords.Z * v91conv)
						+ (nRand_Double3 * nPositionPerturbationCoefficient + nResult.Z * nSpiralRadius);

					const auto nRand_Double4 = ScenarioClass::Instance->Random.RandomDouble();
					const auto nRand_Double5 = ScenarioClass::Instance->Random.RandomDouble();
					const auto nRand_Double6 = ScenarioClass::Instance->Random.RandomDouble();
					CoordStruct nMovementDummy {};
					nMovementDummy.X = (nRand_Double4 * nMovementPerturbationCoefficient) + nResult.X;
					nMovementDummy.Y = (nRand_Double5 * nMovementPerturbationCoefficient) + nResult.Y;
					nMovementDummy.Z = (nRand_Double6 * nMovementPerturbationCoefficient) + nResult.Z;
					const auto nMag = nMovementDummy.MagnitudeSquared();
					Vector3D<float> nVelsC {};

					if (nMag != 0.0)
					{
						const auto idkHere = (float)(1.0 / nMag);
						nVelsC.X = nMovementDummy.X * idkHere;
						nVelsC.Y = nMovementDummy.Y * idkHere;
						nVelsC.Z = idkHere * nMovementDummy.Z;
					}

					const auto nRand_Double7 = ScenarioClass::Instance->Random.RandomDouble();
					const auto nvalhere = (nRand_Double7 + nVal) * nVelocityPerturbationCoefficient;
					auto nVelocityPerturbationCoefficient_copy = nVelocityPerturbationCoefficient;
					nVal = 0.5 * nvalhere;
					if (nVal < nVelocityPerturbationCoefficient)
						nVelocityPerturbationCoefficient_copy = 0.5 * nRand_Double7;

					if (nMovementPerturbationCoefficientneg > nVelocityPerturbationCoefficient_copy)
						nVal = nMovementPerturbationCoefficientneg;

					auto Data = &pThis->PreCalculatedParticlesData[i];
					Data->velB = nVelsC;
					Data->vel = nDummy_d;
					Data->A = nVel + nVal;
					Data->B = 0.0f;
					Data->C = 0.0f;
					Data->Empty = false;
					Data->MaxEC = pHeldType->MaxEC + ScenarioClass::Instance->Random.RandomFromMax(9);

					if (pHeldType->StartColor1 && pHeldType->StartColor2)
					{
						auto const nFact = (float)ScenarioClass::Instance->Random.RandomDouble();
						Data->Colors.Lerp(pHeldType->StartColor1, pHeldType->StartColor2, nFact);
					}
					else
					{
						if (pHeldType->ColorList.Count)
						{
							Data->Colors.R = pHeldType->ColorList[0]->Red;
							Data->Colors.G = pHeldType->ColorList[0]->Green;
							Data->Colors.B = pHeldType->ColorList[0]->Blue;
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

		for (auto& v46 : pThis->PreCalculatedParticlesData)
		{
			const auto coeff = v46.A;
			v46.A = ScenarioClass::Instance->Random.RandomDouble() + coeff;
			v46.vel.X += (v46.velB.X * coeff);
			v46.vel.Y += (v46.velB.Y * coeff);
			v46.vel.Z += (v46.velB.Z * coeff);

			if (--v46.MaxEC <= 0)
				v46.Empty = 1;
		}

		UpdateState(pThis);
		UpdateColor(pThis);
	}

	static void HandleCaseThree(ParticleSystemExt::ExtData* pThis)
	{

	}
};

bool IsHandled(ParticleSystemClass* pThis)
{
	auto const pThisExt = ParticleSystemExt::ExtMap.Find(pThis);
	switch (pThisExt->Behave)
	{
	case BehavesLike::Spark: //1
		ParticleSystemExt_ExtData::HandleSpark(pThisExt);
		break;
	case BehavesLike::Fire: //2
		ParticleSystemExt_ExtData::HandleCaseTwo(pThisExt);
		break;
		//case 3:
		//	pThis->HandleCaseThree();
		//	break;
	default:
		return false;
	}

	ParticleSystemClass* v3 = pThisExt->OwnerObject();
	if (v3->Lifetime-- == 1)     // Lifetime 
		v3->TimeToDie = 1;

	if (v3->IsAlive
	  && v3->TimeToDie
	  && !v3->Particles.Count
	  && pThisExt->PreCalculatedParticlesData.begin() == pThisExt->PreCalculatedParticlesData.end()
	  && pThisExt->SomeArray_b.begin() == pThisExt->SomeArray_b.end())
	{
		v3->Limbo();
		v3->IsAlive = 0;
		AbstractClass::Array2->AddItem(v3);
	}

	return true;
}

DEFINE_HOOK(0x62FD60, ParticleSystemClass_Update, 0x9)
{
	GET(ParticleSystemClass*, pThis, ECX);
	return IsHandled(pThis) ? 0x62FE43 : 0;
}

