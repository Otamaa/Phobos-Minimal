//TODO:

template<size_t idx>
static void* AresExtMap_Find(void* const key)
{
	return AresData::AresThiscall<AresData::FunctionIndices::ExtMapFindID, void*, DWORD, void*>()(AresData::AresStaticInstanceFinal[idx], key);
}

#include <Ext/ParticleSystem/Body.h>
#include <Notifications.h>
#include <Ext/Rules/Body.h>
#include <SpotlightClass.h>

struct ParticleExt_
{
	enum class Behave : int
	{
		None = 0,
		One = 1,
		Two = 2,
		Three = 3
	};

	struct ExtData
	{
		ParticleSystemClass* OwnerObject;
		InitState Init;
		Behave What;
		ParticleTypeClass* HeldType;

		struct Movement
		{
			Vector3D<float> vel;
			Vector3D<float> velB;
			DWORD A;
			float ColorFactor;
			int C; //counter for the color change update
			DWORD DecreasedSomething;
			BYTE Empty; //state counter
			ColorStruct Colors;
		};
		static_assert(sizeof(Movement) == 0x2C, "Invalid Size");
		std::vector<Movement> OtherParticleData;

		struct Draw
		{
			CoordStruct vel;
			Vector3D<float> velB;
			DWORD A;
			int ImageFrame;
			DWORD C;
			ParticleTypeClass* LinkedParticleType;
			BYTE Transparancy;
			BYTE DeleteOnStateLimit; //state counter
			BYTE byte30;
			BYTE byte31;
		};
		static_assert(sizeof(Draw) == 0x2C, "Invalid Size");
		std::vector<Draw> SmokeData;


		void NOINLINE Alloc()
		{

			if (auto pType = OwnerObject->Type)
			{
				if (pType->HoldsWhat >= 0)
				{
					HeldType = ParticleTypeClass::Array->Items[pType->HoldsWhat];

					if (HeldType->UseLineTrail && !HeldType->AlphaImage)
					{
						int Held_behaves = (int)HeldType->BehavesLike;
						int This_behaves = (int)pType->BehavesLike;

						if (Held_behaves <= 1)
							Held_behaves = Held_behaves == 0;

						if (This_behaves == Held_behaves)
						{
							if (This_behaves == 0)
							{
								this->What = Behave::Three;
								return;
							}

							const auto This_behaves_negThree = This_behaves - 3;
							if (!This_behaves_negThree)
							{
								this->What = Behave::One;
								return;
							}

							if (This_behaves_negThree == 1)
							{
								this->What = Behave::Two;
								return;
							}
						}
					}
				}
			}
		}

		void NOINLINE UpdateLocations()
		{
			auto const Gravity = RulesClass::Instance->Gravity;

			for (auto& Data : this->OtherParticleData)
			{
				if (--Data.DecreasedSomething <= 0)
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

		void NOINLINE UpdateState()
		{

			auto iter = std::remove_if(this->OtherParticleData.begin(), this->OtherParticleData.end(), [](const auto& data)
 {
	 return data.Empty;
			});
			this->OtherParticleData.erase(iter);

			auto iterDraw = std::remove_if(this->SmokeData.begin(), this->SmokeData.end(), [](const auto& data)
 {
	 return data.DeleteOnStateLimit;
			});

			this->SmokeData.erase(iterDraw);
		}

		void NOINLINE  UpdateColor()
		{

			const auto pHeldType = this->HeldType;

			for (auto& Data : this->OtherParticleData)
			{
				const auto v6 = ScenarioClass::Instance->Random.RandomDouble() + pHeldType->ColorSpeed + Data.ColorFactor;
				double v7 = 1.0;

				if (v6 <= 1.0)
				{
					v7 = v6;
				}
				else
				{
					if (Data.C < (pHeldType->ColorList.Count - 2))
					{
						v7 = 0.0;
						Data.C++;
					}
				}

				Data.ColorFactor = (float)v7;
			}
		}

		void NOINLINE  Update_Behave_One()
		{
			const auto pOwner = this->OwnerObject;
			const auto SparkSpawnFrames = pOwner->SparkSpawnFrames;
			const auto SparkSpawnNeg1 = SparkSpawnFrames - 1;

			if ((SparkSpawnNeg1 - 1) >= 0)
			{
				pOwner->SparkSpawnFrames = SparkSpawnNeg1;
				if ((SparkSpawnNeg1 - 1) <= 0)
					pOwner->TimeToDie = true;

				auto random = &ScenarioClass::Instance->Random;

				const auto first_randDouble = random->RandomDouble();

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

				const auto pType = pOwner->Type;
				if (!SparkSpawnNeg1 || pType->SpawnSparkPercentage > random->RandomDouble())
				{
					int cap = 0;
					const auto pHeld = this->HeldType;
					if (pType->ParticleCap >= 0)
						cap = pType->ParticleCap;

					const auto halfcap = cap / 2;
					const auto randomHalfCap = random->RandomFromMax(halfcap);
					const auto resize = halfcap + randomHalfCap;

					this->OtherParticleData.reserve(resize);

					for (int i = resize; i > 0; --i)
					{
						auto const nVelX = random->RandomFromMax(pHeld->XVelocity);
						auto const nVelY = random->RandomFromMax(pHeld->YVelocity);
						auto const nVelZ = pHeld->MinZVelocity + random->RandomFromMax(pHeld->ZVelocityRange);
						Vector3D<float> nVels { (float)nVelX  , (float)nVelY ,(float)nVelZ };
						auto const nVelsMag = nVels.Magnitude();
						auto const nSpawnDir = pType->SpawnDirection + nVels;
						auto const nSpawnDirMag = nVels.Magnitude();
						Vector3D<float> nVelsC {};

						if (nSpawnDirMag != 0.0)
						{
							const auto idkHere = (float)(1.0 / nSpawnDirMag);
							nVelsC.X = nSpawnDir.X * idkHere;
							nVelsC.Y = nSpawnDir.Y * idkHere;
							nVelsC.Z = idkHere * nSpawnDir.Z;
						}

						auto Data = &this->OtherParticleData.emplace_back();
						Data->velB = (nVelsC * nVelsMag);
						Data->vel.X = (float)pOwner->Location.X;
						Data->vel.Y = (float)pOwner->Location.Y;
						Data->vel.Z = (float)pOwner->Location.Z;
						Data->A = 0.0f;
						Data->ColorFactor = 0.0f;
						Data->C = 0.0f;
						Data->Empty = false;
						Data->DecreasedSomething = pHeld->MaxEC + random->RandomFromMax(pHeld->MaxEC);

						if (pHeld->StartColor1 && pHeld->StartColor2)
						{
							auto const nFact = (float)random->RandomDouble();
							Data->Colors.Lerp(pHeld->StartColor1, pHeld->StartColor2, nFact);
						}
						else
						{
							if (pHeld->ColorList.Count)
							{
								Data->Colors.R = pHeld->ColorList[0]->Red;
								Data->Colors.G = pHeld->ColorList[0]->Green;
								Data->Colors.B = pHeld->ColorList[0]->Blue;
							}
						}
					}

					if (GameOptionsClass::Instance->GameSpeed == 2)
					{
						if (pOwner->SparkSpawnFrames == pType->SparkSpawnFrames &&
							!pType->OneFrameLight && pType->LightSize > 0
							)
						{
							GameCreate<SpotlightClass>(pOwner->Location, pType->LightSize);
						}
					}
				}
			}

			this->UpdateLocations();
			this->UpdateState();
			this->UpdateColor();
		}

		void NOINLINE  Update_Behave_Two()
		{
			auto const pOwnerObj = this->OwnerObject;
			auto const pOwnerObjType = pOwnerObj->Type;
			if (!pOwnerObj->TimeToDie && this->OtherParticleData.begin() == this->OtherParticleData.end())
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
				const auto pHeldType = this->HeldType;
				const auto nSpinDelta = pOwnerObjType->SpiralDeltaPerCoord;
				const auto nVel = pHeldType->Velocity;
				const auto nSpiralRadius = pOwnerObjType->SpiralRadius;
				const auto nPositionPerturbationCoefficient = pOwnerObjType->PositionPerturbationCoefficient;
				const auto nMovementPerturbationCoefficient = pOwnerObjType->MovementPerturbationCoefficient;
				const auto nVelocityPerturbationCoefficient = pOwnerObjType->VelocityPerturbationCoefficient;
				const size_t nDecidedsize = (size_t)(pOwnerObjType->ParticlesPerCoord * nMagSquared);
				this->OtherParticleData.resize(nDecidedsize);

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
						nMovementDummy.X = int((nRand_Double4 * nMovementPerturbationCoefficient) + nResult.X);
						nMovementDummy.Y = int((nRand_Double5 * nMovementPerturbationCoefficient) + nResult.Y);
						nMovementDummy.Z = int((nRand_Double6 * nMovementPerturbationCoefficient) + nResult.Z);
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

						auto Data = &this->OtherParticleData.emplace_back();
						Data->velB = nVelsC;
						Data->vel = nDummy_d;
						Data->A = nVel + nVal;
						Data->ColorFactor = 0.0f;
						Data->C = 0.0f;
						Data->Empty = false;
						Data->DecreasedSomething = pHeldType->MaxEC + ScenarioClass::Instance->Random.RandomFromMax(9);

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
		}

		void NOINLINE UpdateWindDirection()
		{
			if ((Unsorted::CurrentFrame() & 1) != 0)
			{
				auto rand = &ScenarioClass::Instance->Random;

				for (auto& smoke : this->SmokeData)
				{
					if (!rand->RandomFromMax(3))
					{
						Point2D velXY { smoke.vel.X , smoke.vel.Y };
						auto randDir = rand->RandomRanged(-1, 1);

						if (rand->RandomBool())
							velXY.X += randDir;
						else
							velXY.Y += randDir;

						velXY.X = std::clamp(velXY.X, -5, 5);
						velXY.Y = std::clamp(velXY.Y, -5, 5);
					}
				}

				for (auto& smoke : this->SmokeData)
				{
					auto pLink = smoke.LinkedParticleType;
					if (smoke.ImageFrame < pLink->EndStateAI)
					{
						if (!((smoke.A + pLink->MaxEC - smoke.C) % (smoke.A % 2 + pLink->StateAIAdvance)
							|| (smoke.ImageFrame = smoke.ImageFrame + 1, smoke.ImageFrame + 1 < pLink->EndStateAI)))
						{
							if (pLink->DeleteOnStateLimit)
								smoke.DeleteOnStateLimit = 1;
						}
					}

					if (smoke.velB.Z > 3.0)
						smoke.velB.Z = int((float)smoke.velB.Z - pLink->Deacc);

					if (--smoke.C <= 0)
						smoke.DeleteOnStateLimit = 1;
				}

				auto WindDir = RulesClass::Instance->WindDirection;
				auto dirX = ParticleClass::GasWind_X[WindDir];
				auto dirY = ParticleClass::GasWind_Y[WindDir];

				for (auto& smoke : this->SmokeData)
				{
					const auto LinkedWind = smoke.LinkedParticleType->WindEffect;
					CoordStruct velCpy = smoke.vel;
					smoke.vel.X += smoke.velB.X + LinkedWind * dirX;
					smoke.vel.Y += smoke.velB.Y + LinkedWind * dirY;
					smoke.vel.Z += smoke.velB.Z;

					const auto pCell = MapClass::Instance->GetCellAt(velCpy);

					if (pCell->ContainsBridge()) {
						int z = pCell->GetFloorHeight({ velCpy.X , velCpy.Y});

						if (velCpy.Z < z) {
							z -= 260;
							if (z >= velCpy.Z) {
								smoke.DeleteOnStateLimit = true;
							}
						}
					}
				}
			}
		}

		void NOINLINE  Update_Behave_Three()
		{
			//this one waay to complex to digest atm
			//too much code about moving  the vectors
			//possible that probably sorting stuffs
			//or maybe empalace/ensert/etc ,//

			auto const pOwnerObj = this->OwnerObject;
			auto const pOwnerObjType = pOwnerObj->Type;
			auto const pOwnerObj_Owner = pOwnerObj->Owner;

			if (pOwnerObj_Owner && pOwnerObj_Owner->AbstractFlags & AbstractFlags::Techno)
			{
				auto coords = pOwnerObj_Owner->GetCoords();
				CoordStruct SpawnDistance = coords + pOwnerObj->SpawnDistanceToOwner;
				pOwnerObj->SetLocation(SpawnDistance);
			}

			this->UpdateWindDirection();
			auto rand = &ScenarioClass::Instance->Random;
			//back cannot be used ,..
			size_t distance = std::distance(std::prev(this->SmokeData.end()) , this->SmokeData.begin());

			if (distance)
			{
				for (size_t i = distance; i > 0; --i)
				{

				}
			}
		}

		bool NOINLINE  UpdateHandled()
		{
			switch (this->What)
			{
			case Behave::One:
				this->Update_Behave_One();
				break;
			case Behave::Two:
				this->Update_Behave_Two();
				break;
			case Behave::Three:
				this->Update_Behave_Three();
				break;
			default:
				return false;
			}

			const auto pOwner = this->OwnerObject;

			if (pOwner->Lifetime-- == -1)
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

		//the color adjusting is not right it seems
		//need investigation
		void NOINLINE  UpdateInAir_Main(bool stopDrawing)
		{
			const auto pHeldType = this->HeldType;
			ColorStruct** color = pHeldType ? (ColorStruct**)pHeldType->ColorList.Items : nullptr;
			auto& rect = DSurface::ViewBounds;

			for (auto& movement : this->OtherParticleData)
			{
				CoordStruct Coord = { (int)movement.vel.X ,(int)movement.vel.Y ,(int)movement.vel.Z };

				if (!stopDrawing || !MapClass::Instance->IsWithinUsableArea(Coord))
				{
					Point2D outClient;
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
							int Zadjust = TacticalClass::AdjustForZ(Coord.Z);

							if ((uint16_t)(LOWORD(ZBuffer::Instance->Area.Y) + LOWORD(ZBuffer::Instance->MaxValue) - LOWORD(outClient.Y)) - Zadjust - 50 < ZBuff)
							{
								//TODO :
								int idx = 0;
								ColorStruct* selected = &movement.Colors;

								if (movement.C)
								{
									//this were lea , so wtf ?
									idx = 3 * movement.C;
									selected = *(color + (3 * movement.C));
								}

								ColorStruct emp {};
								emp.Lerp(*(color + idx + 3), selected, movement.ColorFactor);

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
				const auto linked = draw.LinkedParticleType;
				if (const auto image = linked->GetImage())
				{
					const auto offs = -15 - TacticalClass::AdjustForZ(draw.vel.Z);
					Point2D outClient;
					TacticalClass::Instance->CoordsToClient(&draw.vel, &outClient);
					DWORD drawingFlag = 0x2E00;
					outClient.Y += rect->X;
					if (GameOptionsClass::Instance->GameSpeed == 2)
					{
						int trans = draw.Transparancy;
						if (trans == 25)
						{
							drawingFlag = 0x2E02;
						}
						else if (trans == 50)
						{
							drawingFlag = 0x2E04;
						}
						else if (trans >= 75u)
						{
							drawingFlag = 0x2E06;
						}

						auto pal = FileSystem::ANIM_PAL();
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
		}
	};

	static void NOINLINE  UpdateInAir()
	{
		if (ParticleSystemClass::Array->Count && GameOptionsClass::Instance->GameSpeed && RulesExt::DetailsCurrentlyEnabled())
		{
			bool StopDrawing = false;
			if (Unsorted::ArmageddonMode() || !Game::hInstance() || ((ScenarioClass::Instance->SpecialFlags.RawFlags + 1) & 16) == 0)
				StopDrawing = true;

			for (auto pSys : *ParticleSystemClass::Array)
			{
				ParticleExt_::Find(pSys)->UpdateInAir_Main(StopDrawing);
			}
		}
	}

	static std::unordered_map<ParticleSystemClass*, std::unique_ptr<ExtData>> Map;

	static ExtData* Find(ParticleSystemClass* pFind)
	{
		auto iter = Map.find(pFind);
		return iter != Map.end() ? iter->second.get() : nullptr;
	}
};

std::unordered_map<ParticleSystemClass*, std::unique_ptr<ParticleExt_::ExtData>> ParticleExt_::Map;

DEFINE_OVERRIDE_HOOK(0x6D9427, TacticalClass_DrawUnits_ParticleSystems, 9)
{
	GET(Layer, layer, EAX);

	if (layer == Layer::Air)
		ParticleExt_::UpdateInAir();

	return layer == Layer::Ground ? 0x6D9430 : 0x6D95A1;
}

DEFINE_OVERRIDE_HOOK(0x62E380, ParticleSystemClass_SpawnParticle, 0xA)
{
	GET(ParticleSystemClass*, pThis, ECX);

	return ParticleExt_::Find(pThis)->What != ParticleExt_::Behave::None
		? 0x62E428 : 0;
}

DEFINE_OVERRIDE_HOOK(0x62E2AD, ParticleSystemClass_Draw, 6)
{
	GET(ParticleSystemClass*, pThis, EDI);
	GET(ParticleSystemTypeClass*, pThisType, EAX);

	if (pThisType->ParticleCap)
	{
		const auto pExt = ParticleExt_::Find(pThis);
		R->ECX(pThis->Particles.Count + pExt->OtherParticleData.size());
	}
	else
	{
		R->ECX(0);
	}

	return 0x62E2B3;
}

DEFINE_OVERRIDE_HOOK(0x62FD60, ParticleSystemClass_Update, 9)
{
	GET(ParticleSystemClass*, pThis, ECX);

	const bool Handled = ParticleExt_::Find(pThis)->UpdateHandled();

	return Handled ? 0x62FE43 : 0;
}
/*
; \Ext\ParticleSystem\Body.cpp
62DF05 = ParticleSystemClass_CTOR, 5
62E26B = ParticleSystemClass_DTOR, 6
630090 = ParticleSystemClass_SaveLoad_Prefix, 5
62FF20 = ParticleSystemClass_SaveLoad_Prefix, 7
630088 = ParticleSystemClass_Load_Suffix, 5
6300F3 = ParticleSystemClass_Save_Suffix, 6
; \Ext\ParticleSystem\Hooks.cpp


*/


//DEFINE_HOOK(0x62FD60, ParticleSystemClass_Update, 9)
//{
//	GET(ParticleSystemClass*, pThis, ECX);
//	const auto pThisExt = (AresParticleExtData*)AresExtMap_Find<0>(pThis);
//	if(!pThisExt->DataA.empty() && !pThisExt->DataB.empty())
//	Debug::Log("ParticeSystem [%s] With ExtPtr [%x - offs %x] ! \n", pThis->get_ID(), pThisExt);
//	//return HandleParticleSys(pThis) ? 0x62FE43 : 0;
//	return 0x0;
//}