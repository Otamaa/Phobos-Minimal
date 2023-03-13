#include <Ext/ParticleSystem/Body.h>

#include <ScenarioClass.h>
#include <Notifications.h>
#include <Utilities/Constructs.h>
#include <GameOptionsClass.h>
#include <SpotlightClass.h>

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
				if (pOwnerObjType->ParticleCap)
					nCap = pOwnerObjType->ParticleCap;

				if (!nCap)
					return;

				auto const nHalfCap = nCap / 2;
				auto const nRandCap = nRand.RandomFromMax(nHalfCap);

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

				ParticleDatas* Data = nullptr;

				if ((size_t)(nHalfCap + nRandCap) > pThis->PreCalculatedParticlesData.size())
				{
					pThis->PreCalculatedParticlesData.emplace_back();
					Data = &pThis->PreCalculatedParticlesData.back();
				}
				else
				{
					Data = &pThis->PreCalculatedParticlesData[nHalfCap + nRandCap];
				}

				Data->velB = (nVelsC * nVelsMag);
				Data->vel.X = (float)pOwnerObj->Location.X;
				Data->vel.Y = (float)pOwnerObj->Location.Y;
				Data->vel.Z = (float)pOwnerObj->Location.Z;
				Data->A = 0;
				Data->B = 0.0f;
				Data->C = 0;
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
		auto const Gravity = RulesGlobal->Gravity;

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
			auto const pCell = Map[nCoord];
			const auto nZOffs = pCell->GetFloorHeight({ nCoord.X , nCoord.Y });

			if (nCoord.Z < nZOffs)
				break;

			auto const pVelBCoordCell = Map[nVelBCoord];
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

	void HandleCaseTwo()
	{
		auto const pOwnerObj = this->OwnerObject;
		auto const pOwnerObjType = pOwnerObj->Type;
		if (!pOwnerObj->TimeToDie && this->PreCalculatedParticlesData.begin() == this->PreCalculatedParticlesData.end())
		{
			pOwnerObj->TimeToDie = true;
			auto const nDistance = pOwnerObj->Location.DistanceFrom(pOwnerObj->TargetCoords);
		}
	}

	void HandleCaseThree()
	{

	}
};

bool IsHandled(ParticleSystemClass* pThis)
{
	auto const pThisExt = ParticleSystemExt::ExtMap.Find(pThis);
	switch (pThisExt->Behave)
	{
	case BehavesLike::Spark:
		ParticleSystemExt_ExtData::HandleSpark(pThisExt);
		break;
	//case 2:
	//	pThis->HandleCaseTwo();
	//	break;
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

