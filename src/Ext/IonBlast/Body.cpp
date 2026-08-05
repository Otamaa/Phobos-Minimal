#include "Body.h"

#include <Surface.h>

#include <Ext/WarheadType/Body.h>
#include <Ext/Surface/Body.h>
#include <Ext/Tactical/Body.h>
#include <Ext/Infantry/Body.h>

#include <Utilities/Patch.h>
#include <Utilities/Macro.h>

#include <RectangleStruct.h>

#include <Misc/DamageArea.h>

void __fastcall FakeIonBlastClass::DestroySurfaces() {
	for (size_t i = 0; i < IonBlastClass_Surfaces.size(); ++i) {
		CallDTOR(std::exchange(IonBlastClass_Surfaces[i], nullptr));
	}
}

void __fastcall FakeIonBlastClass::InitOneTime()
{
	// 0x53D339: test IonBlastClass_inited / 0x53D344: jnz loc_53D545
	if (IonBlastClass_inited())
		return;

	// ── Initialise radial bounds (0x53D34A–0x53D37E) ─────────────────────────
	double upperBound = 0.0;
	double lowerBound = -57.0;

	int frameIndex = 0; // var_2C+4  (0x53D36E: mov [var_2C+4], 0)

	for (auto& pSurfaces : IonBlastClass_Surfaces)
	{
		// ── Allocate BSurface (0x53D384–0x53D3CB) ────────────────────────────
		pSurfaces = GameCreate<BSurface>(512, 256, 1, 0);

		// ── Fill surface to 0xFF  — vtable call [edx+18h] (0x53D3D7) ─────────
		pSurfaces->Fill(-1); // SUSPECT: confirm vtable slot

		// ── Lock surface — vtable call [edx+5Ch] (0x53D3E2) ─────────────────
		// 0x53D3DC: push 0 / 0x53D3DF: push 0 → Lock(0, 0)
		// 0x53D3E5: pLocked = eax
		// 0x53D3EC: add edi, 0x10100  — offset to centre pixel
		//   0x10100 bytes = row 128 × 512-byte stride + col 128  (byte-addressed)
		//   IDA shows +0x8080 because it casts to __int16*; raw byte offset is 0x10100.
		// VERIFY: vtable offset 0x5C = Lock on XSurface/BSurface in YRpp
		uint8_t* pLocked = (uint8_t*)pSurfaces->Lock(0, 0);
		uint8_t* pCentre = pLocked + 0x8080; // centre of surface (row 128, col 128)

		// ── Outer loop: row = 127 down to 0  (EBX) ───────────────────────────
		for (int row = 127; row >= 0; --row)
		{
			// 0x53D3F9: imul ebp,ebx / 0x53D3FC: shl ebp,2  →  cached4RowSq = 4*row*row
			const int cached4RowSq = 4 * row * row; // EBP / var_14

			// ── Inner loop: col = 255 down to 0  (ESI) ───────────────────────
			for (int col = 255; col >= 0; --col)
			{
				// 0x53D408: imul eax,esi / 0x53D40B: add eax,ebp → col*col + 4*row*row
				// 0x53D411: fild / 0x53D415: fstp [n] / 0x53D418: call FastMath::Sqrt
				const double radius = Math::sqrt(static_cast<double>(col * col + cached4RowSq));

				// 0x53D421: fcomp var_8 (lowerBound) / test ah,1 / jnz skip
				// Skips if radius < lowerBound (FPU C2 set = unordered or less-than)
				if (radius < lowerBound)
					continue;

				// 0x53D437: fcomp var_10 (upperBound) / test ah,41h / jz skip
				// Skips if radius > upperBound
				if (radius > upperBound)
					continue;

				// ── Compute pixel brightness (0x53D442–0x53D490) ─────────────
				//
				// 0x53D442: fild [frameIndex]
				// 0x53D44B: fmul dbl_A9FFA8 (7.1125 = kRingStep)
				// 0x53D451: fsubr [radius]          → radius - frameIndex*kRingStep
				// 0x53D455: fadd  dbl_7EC0A0 (38.0) → + kSinOffset    VERIFY
				// 0x53D45B: fmul  dbl_7EC030 (0.11) → * kSinScale     VERIFY
				// 0x53D464: call  FastMath::Sin
				// 0x53D469: fmul  dbl_7EC098 (3.5)  → * kSinAmplitude VERIFY
				// 0x53D472: fadd  dbl_7E3D90 (3.0)  → + kSinBias      VERIFY
				// 0x53D478: fld   [radius]
				// 0x53D47C: fdiv  dbl_7EC038 (51.0) → / kRadialDivisor VERIFY
				// 0x53D482: fadd  float_1_0  (1.0)  → + kDivisorBias   VERIFY
				// 0x53D488: fdivp st(1),st           → numerator / denominator
				// 0x53D48A: fadd  dbl0_5     (0.5)  → + kPixelBias     VERIFY
				// 0x53D490: call  __ftol             → truncate to int (Westwood ftol)
				//
				// Note: EBP XOR'd to 0 at 0x53D449 — IonBlastData_53D960 always called with a1=0.
				const double sinArg = (radius - static_cast<double>(frameIndex) * 7.1125 + 38.0) * 0.11;
				const double numerator = Math::sin(sinArg) * 3.5 + 3.0;
				const double denom = radius / 51.0 + 1.0;
				const double brightness = numerator / denom + 0.5;

				// 0x53D490: __ftol → (int)brightness  (Westwood truncate-toward-zero)
				const int brightnessInt = static_cast<int>(brightness);

				// 0x53D495: push eax (=brightnessInt) / 0x53D496: push ebp (=0)
				// 0x53D497: call IonBlastData_53D960(a1=0, a2=brightnessInt)
				const uint8_t pixel = static_cast<uint8_t>(IonBlastData_53D960(0, brightnessInt));

				// ── Four quadrant pixel writes (0x53D49C–0x53D4BB) ───────────
				//
				// 0x53D49E: ecx = row << 9  (row * 512, byte stride)
				// Q1: pCentre[ row*512 + col]  — 0x53D4A8: mov [edx+edi], al
				// Q2: pCentre[ row*512 - col]  — 0x53D4AF: mov [edx+edi], al (edx=ecx-esi)
				// Q3: pCentre[-row*512 + col]  — 0x53D4B6: mov [esi+edx], al (edx=edi-ecx)
				// Q4: pCentre[-row*512 - col]  — 0x53D4BB: mov [edx], al    (edx=edx-esi)
				const int rowOffset = row * 512; // ecx = row << 9
				pCentre[rowOffset + col] = pixel; // Q1
				pCentre[rowOffset - col] = pixel; // Q2
				pCentre[-rowOffset + col] = pixel; // Q3
				pCentre[-rowOffset - col] = pixel; // Q4
			}
		}

		// ── Per-frame bound updates (0x53D4CB–0x53D51B) ──────────────────────
		//
		// Upper bound:
		//   0x53D4CB: fld num_256 (256.0)
		//   0x53D4D1: fsub dbl_A9FF80 (7.1125)  →  256.0 - kRingStep
		//   0x53D4D7: fcomp var_10 (upperBound)
		//   0x53D4E0: jnz skip  (skip if 256.0-kRingStep <= upperBound)
		//   0x53D4E2: fld dbl_A9FF80 / fadd var_10 / fstp var_10
		if ((256.0 - 7.1125) > upperBound)
			upperBound += 7.1125;

		// Lower bound:
		//   0x53D4F0: fld dbl_A9FF80 / fmul dbl_7E5190 (1.2) / fadd var_8 / fst var_8
		//   0x53D504: fcomp var_10  →  if lowerBound > upperBound → lowerBound = upperBound
		lowerBound = 7.1125 * 1.2 + lowerBound;
		if (lowerBound > upperBound)
			lowerBound = upperBound;

		// ── Advance and check termination (0x53D51F–0x53D538) ────────────────
		//
		// 0x53D527: add [var_24], 4 / 0x53D52A: inc esi (frameIndex)
		// 0x53D52B: cmp [var_24], offset dbl_AA0108 (0xAA0108)
		// 0x53D538: jl → loc_53D380
		++frameIndex;
	}

	// 0x53D53E: mov IonBlastClass_inited, 1
	IonBlastClass_inited = true;
}

void FakeIonBlastClass::_AI()
{
	const auto pData = WarheadTypeExtData::IonBlastExt.get_or_default(this);
	const int Ripple_Radius = pData && pData->Ripple_Radius.isset() ? MinImpl((int)IonBlastClass_Surfaces.Size, pData->Ripple_Radius.Fetch() + 1) : IonBlastClass_Surfaces.Size;
	
	//79 or IonBlastClass_Surfaces.Size -1 i suppose
	//do not raise the lifetime above the ripple radius, otherwise it will crash when trying to access the surface array
	if (this->Lifetime >= (Ripple_Radius - 1)) {
		GameDelete<true, false>(this);
		return;
	}

	{
		const auto screenPos = TacticalClass::Instance->CoordsToClient(this->Location);

		if (!this->DisableIonBeam && this->Lifetime == 0)
		{
			CoordStruct spawnCoord = this->Location;
			spawnCoord.Z += 5;
			const auto Rules = RulesClass::Instance();

			auto* mapCell = MapClass::Instance->GetCellAt(this->Location);
			const bool isWater = mapCell->LandType == LandType::Water;

			if (const auto animId = isWater ? Rules->SplashList[Rules->SplashList.Count - 1] :
				pData ? pData->Ion_Blast.Get(Rules->IonBlast) : Rules->IonBlast)
			{
				GameCreate<AnimClass>(animId, spawnCoord);
			}

			if (const auto pBeam = pData ? pData->Ion_Beam.Get(Rules->IonBeam) : Rules->IonBeam)
			{
				GameCreate<AnimClass>(pBeam, spawnCoord);
			}

			if (const auto pWH = pData ? pData->Ion_WH.Get(Rules->IonCannonWarhead) : Rules->IonCannonWarhead)
			{

				const int nDamage = pData ? pData->Ion_Damage.Get(Rules->IonCannonDamage) : Rules->IonCannonDamage;

				if (mapCell->ContainsBridge())
				{
					CoordStruct target = this->Location;
					target.Z += CellClass::BridgeHeight;
					DamageArea::Apply(&target, nDamage, nullptr, pWH, true, nullptr);
				}

				DamageArea::Apply(&this->Location, nDamage, nullptr, pWH, true, nullptr);
				MapClass::FlashbangWarheadAt(nDamage, pWH, this->Location, false, SpotlightFlags::None);
			}
		}

		if (!pData || pData->Ion_Rocking)
		{
			int16_t centerX = static_cast<int16_t>(this->Location.X / 256);
			int16_t centerY = static_cast<int16_t>(this->Location.Y / 256);

			for (int16_t dy = -3; dy <= 3; ++dy)
			{
				for (int16_t dx = -3; dx <= 3; ++dx)
				{
					CellStruct cell { static_cast<int16_t>(centerX + dx), static_cast<int16_t>(centerY + dy) };
					auto* mapCell = MapClass::Instance->GetCellAt(cell);

					for (ObjectClass* pObj = mapCell->FirstObject; pObj != nullptr; pObj = pObj->NextObject)
					{
						if (!pObj->IsAlive)
							continue;

						if (pObj->WhatAmI() == InfantryClass::AbsID || pObj->WhatAmI() == UnitClass::AbsID) {

							auto unit = (FootClass*)pObj;

							if (unit->IsSinking)
								continue;

							CoordStruct unitCoord = unit->Location;
							Point2D unitScreen = TacticalClass::Instance->CoordsToClient(unitCoord);

							int dxPix = unitScreen.X - screenPos.X;
							int dyPix = unitScreen.Y - screenPos.Y;
							int dist = static_cast<int>(Math::sqrt(dxPix * dxPix + dyPix * dyPix)) + 8;

							if (dist < 256)
							{
								Surface* surf = IonBlastClass_Surfaces[this->Lifetime];
								char* locked = static_cast<char*>(surf->Lock(dist + 0x100, 128));
								if (*locked > 0)
								{
									unit->SetSpeedPercentage(0.0f);
									unit->height_subtract_6B4 = 2 * IonBlastData_53D8E0(*locked).Y;
								}

								auto vox = unit->GetTechnoType()->MainVoxel.VXL;

								if (vox && !vox->LoadFailed && *locked >= 0)
								{
									float deltax = static_cast<float>(this->Location.X - unit->Location.X);
									float deltay = static_cast<float>(this->Location.Y - unit->Location.Y);
									float deltaz = static_cast<float>(this->Location.Z - unit->Location.Z);
									const float len = Math::sqrt(deltax * deltax + deltay * deltay + deltaz * deltaz);

									if (Math::abs(len) > 0.00002f)
									{
										deltax /= len;
										deltay /= len;
										deltaz /= len;

										const auto& facing_ = unit->PrimaryFacing;
										const auto facing_Current = facing_.Current();

										const float facingAngle = (facing_Current.Raw - Math::BINARY_ANGLE_MASK) * -0.0000958767f;
										const float sinA = Math::sin((double)facingAngle);
										const float cosA = Math::cos((double)facingAngle);

										const float ux = deltax * cosA + deltay * sinA;
										const float uz = deltax * sinA - deltay * cosA;
										const float uy = deltaz;

										float proj = Math::sqrt(ux * ux + uz * uz + uy * uy);
										const float align = cosA * ux - sinA * proj;

										if (Math::abs(align - deltax) > 0.0002f || Math::abs(cosA * proj + sinA * ux - deltay) > 0.0002f)
										{
											proj = -proj;
										}

										const float blastDist = len + 51.0f;
										const float blastOffset = (Math::sin(double(len - static_cast<float>(this->Lifetime) * 7.1125f + 38.0f) * 0.11f) * 3.5f + 3.0f) * 51.0f;
										const float blastFactor = Math::cos(double(len - static_cast<float>(this->Lifetime) * 7.1125f + 38.0f) * 0.11f);
										const float curve = (blastFactor * 0.11f * 51.0f * 3.5f * blastDist - blastOffset) / (blastDist * blastDist);

										unit->AngleRotatedSideways = float(proj * curve * Math::GAME_TWOPI);
										unit->AngleRotatedForwards = float(-ux * curve * Math::GAME_TWOPI);
									}
								}
							}
						}
					}
				}
			}
		}

		++this->Lifetime;
	} 
}

void __fastcall FakeIonBlastClass::_DrawAll()
{
	if (DSurface::Temp->Get_Pitch() != IonBlastPitch())
	{

		IonBlastPitch = DSurface::Temp->Get_Pitch();
		ionblast_A9FAE8[0] = 0;

		for (int i = 1; i < (int)ionblast_A9FAE8.size(); ++i)
		{
			Point2D data = IonBlastData_53D8E0(i);
			ionblast_A9FAE8[i] = data.X + IonBlastPitch() * data.Y;
		}
	}

	for (int i = IonBlastClass::Array->Count - 1; i >= 0; --i)
	{
		static_cast<FakeIonBlastClass*>(IonBlastClass::Array->Items[i])->_Draw();
	}
}

#pragma optimize("", off )
void FakeIonBlastClass::_Draw()
{
	static COMPILETIMEEVAL int IonBlastSurfaceWidth = 512;
	static COMPILETIMEEVAL int IonBlastSurfaceHeight = 256;
	static COMPILETIMEEVAL int IonBlastSurfaceCount = 80;

	if (!FakeRulesClass::DetailsCurrentlyEnabled()) // 0x53D580 : Options.DetailLevel == 2
		return;

	// EXTENSION: Fog of War culling. Not present in vanilla.
	if (ScenarioClass::Instance->SpecialFlags.StructEd.FogOfWar)
	{
		if (MapClass::Instance->IsLocationFogged(this->Location))
			return;
	}

	auto [screenPos, isInViewport] = TacticalClass::Instance->GetCoordsToClientSituation(this->Location);

	if (!isInViewport) // 0x53D5A8
		return;

	// EXTENSION: vanilla indexes IonBlastClass_Surfaces[state] unguarded at 0x53D5DE.
	// An out-of-range Lifetime hands a garbage Surface* to the lock helper and faults
	// there rather than here. Cheap guard, no behavioural change for valid states.
	if (this->Lifetime < 0 || this->Lifetime >= IonBlastSurfaceCount)
		return;

	DSurface* targetSurface = DSurface::Temp();
	DSurface* sourceSurface = static_cast<DSurface*>(IonBlastClass_Surfaces[this->Lifetime]);

	if (!targetSurface || !sourceSurface)
		return;

	RectangleStruct viewportRect {
		.X = DSurface::ViewBounds->X,
		.Y = DSurface::ViewBounds->Y,
		.Width = DSurface::ViewBounds->Width,
		.Height = DSurface::ViewBounds->Height - 7
	};

	RectangleStruct destRect {
		.X = screenPos.X - 256,
		.Y = screenPos.Y - 128,
		.Width = IonBlastSurfaceWidth,
		.Height = IonBlastSurfaceHeight
	};

	RectangleStruct srcRect {
		.X = 0, .Y = 0,
		.Width = IonBlastSurfaceWidth, .Height = IonBlastSurfaceHeight
	};

	// srcSubRect is the OUT parameter: lockregion narrows it to the clipped span.
	RectangleStruct srcSubRect {
		.X = 0, .Y = 0,
		.Width = IonBlastSurfaceWidth, .Height = IonBlastSurfaceHeight
	};

	bool    regionClipped = false;
	int32_t destBufferOffset = 0;
	int32_t srcBufferOffset = 0;

	// VERIFY: vanilla passes these two as (__int16*) but the callee stores full pointers.
	// The int16_t* cast is preserved only to match the vanilla prototype; the storage is
	// int32_t so nothing is truncated. Consider retyping the helper to void**.
	if (!Blit_helper_lockregion(
		targetSurface,
		&viewportRect,
		&destRect,
		sourceSurface,
		&srcRect,
		&srcSubRect,
		&regionClipped,
		reinterpret_cast<int16_t*>(&destBufferOffset),
		reinterpret_cast<int16_t*>(&srcBufferOffset)))
	{
		return;
	}

	// destRect has been clipped in place by the helper - everything below must use the
	// post-clip X/Y (matches vanilla, which re-reads a3 after the call at 0x53D6B1).
	uint16_t* destRow = reinterpret_cast<uint16_t*>(destBufferOffset);
	const int8_t* srcRow = reinterpret_cast<const int8_t*>(srcBufferOffset);

	const int pitch = targetSurface->Get_Pitch();  // 0x53D5F5, vftable +0x74
	const int surfaceWidth = targetSurface->Get_Width();  // 0x53D6D2, vftable +0x7C
	const int zBufferWidth = ZBuffer::Instance->Width;

	// BUGFIX 4: all of this is 16-bit unsigned in vanilla (si). Doing it in uint16_t
	// reproduces both the truncation and the wrap-on-decrement at 0x53D765.
	const uint16_t zRef = static_cast<uint16_t>(
		static_cast<uint16_t>(ZBuffer::Instance->MaxValue)
		- static_cast<uint16_t>(Game::AdjustHeight(this->Location.Z)));

	uint16_t zThreshold = static_cast<uint16_t>(zRef - static_cast<uint16_t>(destRect.Y) - 3);

	int16_t* zBufferRow = reinterpret_cast<int16_t*>(ZBuffer::Instance->GetBuffer(0, destRect.Y));

	// 0x53D6E5 : if the worst-case linear walk would run past the Z buffer, fall back to
	// the per-pixel GetBuffer() path. Note the comparison is >=, i.e. "unsafe" -> slow path.
	const uintptr_t zBufferEndProbe = reinterpret_cast<uintptr_t>(
		&zBufferRow[surfaceWidth + (srcSubRect.Height + 1) * zBufferWidth]);

	if (zBufferEndProbe >= reinterpret_cast<uintptr_t>(ZBuffer::Instance->BufferTail)) {
		// ---- Conservative path (0x53D794): resolve every Z sample individually ----
		for (int row = 0; row < srcSubRect.Height; ++row) {
			const int8_t* srcPixels = srcRow;

			for (int col = 0; col < srcSubRect.Width; ++col) {
				// BUGFIX 3: signed load. Values >= 0x80 are negative and must be skipped.
				const int pixel = *srcPixels++;

				if (pixel <= 0)
					continue;

				const uint16_t zValue = *reinterpret_cast<uint16_t*>(
					ZBuffer::Instance->GetBuffer(destRect.X + col, destRect.Y + row));

				if (zValue > zThreshold)
				{
					// BUGFIX 2: offset is relative to the CURRENT column, not the row base.
					// pixel is guaranteed 1..127 by the signed test above, so the 289-entry
					// table is never over-indexed (vanilla has no bounds check either).
					destRow[col] = destRow[col + ionblast_A9FAE8[pixel]];
				}
			}

			destRow = reinterpret_cast<uint16_t*>(reinterpret_cast<uint8_t*>(destRow) + pitch);
			srcRow += IonBlastSurfaceWidth; // BUGFIX 1: flat 512-BYTE row stride (0x53D80C)
			--zThreshold;
		}
	} else {
		// ---- Fast path (0x53D6F8): the Z row pointer walks linearly ----
		const uint16_t* zRow = reinterpret_cast<const uint16_t*>(&zBufferRow[destRect.X]);

		for (int row = 0; row < srcSubRect.Height; ++row) {
			const int8_t* srcPixels = srcRow;

			for (int col = 0; col < srcSubRect.Width; ++col) {
				const int pixel = *srcPixels++; // BUGFIX 3

				if (pixel > 0 && zRow[col] > zThreshold)
					destRow[col] = destRow[col + ionblast_A9FAE8[pixel]]; // BUGFIX 2
			}

			destRow = reinterpret_cast<uint16_t*>(reinterpret_cast<uint8_t*>(destRow) + pitch);
			zRow += zBufferWidth;         // 0x53D780 : net advance is one full Z row
			srcRow += IonBlastSurfaceWidth; // BUGFIX 1 (0x53D75F)
			--zThreshold;
		}
	}

	targetSurface->Unlock();
	sourceSurface->Unlock();
}
#pragma optimize("", on )

ASMJIT_PATCH(0x53CB91, IonBlastClass_DTOR, 6)
{
	GET(IonBlastClass*, IB, ECX);
	WarheadTypeExtData::IonBlastExt.erase(IB);
	return 0;
}

DEFINE_FUNCTION_JUMP(CALL, 0x531758, FakeIonBlastClass::InitOneTime)
DEFINE_FUNCTION_JUMP(CALL, 0x6BE3CE, FakeIonBlastClass::DestroySurfaces)
DEFINE_FUNCTION_JUMP(CALL, 0x53D326, FakeIonBlastClass::_AI)
DEFINE_FUNCTION_JUMP(CALL, 0x6D4656, FakeIonBlastClass::_DrawAll)
DEFINE_FUNCTION_JUMP(LJMP, 0x53D850, FakeIonBlastClass::_DrawAll)

bool FakeIonBlastClass::SaveGlobals(PhobosStreamWriter& Stm)
{
	//save it as int instead of size_t
	int Count = (int)IonBlastClass::Array->Count;

	if (!Stm.Save(Count))
		return false;

	for (int i = 0; i < Count; ++i)
	{
		Debug::Log("Saving IonBlastClass [Item(%d) - %x] to stream\n", i, (long)(&IonBlastClass::Array->Items[i]));
		FakeIonBlastClass* pBlast = (FakeIonBlastClass*)IonBlastClass::Array->Items[i];

		if (!Stm.Save(pBlast))
			return false;

		if (!pBlast->Save(Stm))
			return false;
	}

	return true;
}

void FakeIonBlastClass::Clear()
{
	for (auto& Ion : *IonBlastClass::Array) {
		GameDelete<true,false>(Ion);
	}
}

bool FakeIonBlastClass::LoadGlobals(PhobosStreamReader& Stm)
{
	int Count = 0;

	if (Stm.Load(Count))
	{
		if (Count > 0)
		{
			for (int i = 0; i < Count; ++i)
			{
				auto pNew = GameCreate<IonBlastClass>();
				IonBlastClass::Array->push_back(pNew);
				FakeIonBlastClass* pBlast = (FakeIonBlastClass*)pNew;

				if (!Stm.RegisterChange(pNew)){
					Debug::FatalError("Failed to RegisterChange IonBlastClass Item [%d]\n", i);
					return false;
				}


				if (!pBlast->Load(Stm)) {
					Debug::FatalError("Failed to Load IonBlastClass Item [%d]\n", i);
					return false;
				}
			}
		}

		return true;
	}

	return false;
}
