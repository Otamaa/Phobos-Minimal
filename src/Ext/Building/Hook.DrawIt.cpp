#include "Body.h"

#include <Ext/BuildingType/Body.h>

#include <Utilities/Macro.h>
#include <Utilities/Patch.h>

#include <TacticalClass.h>

// ─── Main function ────────────────────────────────────────────────────────────
void  FakeBuildingClass::_Draw_It(
    Point2D* pPixel,    // arg0
	RectangleStruct* pBound)    // a6
{
	BuildingTypeClass* const type = this->Type;
	auto* pTypeExt = this->_GetTypeExtData();
	auto* pExt = this->_GetExtData();

    {
     
		if (pTypeExt->IsHideDuringSpecialAnim &&
			(this->Anims[(int)BuildingAnimSlot::Special] ||
				this->Anims[(int)BuildingAnimSlot::SpecialTwo] ||
				this->Anims[(int)BuildingAnimSlot::SpecialThree]))
			return;

        if (pExt->LimboID >= 0)
            return;

		if (type->InvisibleInGame)
			return;
    }

    CellStruct cellIdx = this->GetMapCoords();
    SHPStruct* shape = this->GetImage();

    if (!shape)
        return;

	auto curMission = this->GetCurrentMission();

	if (this->BState == BStateType::Construction && curMission == Mission::Selling) {
		for (auto& anim : this->Anims) {
			if (anim) {
				anim->Invisible = true;
			}
		}
	}

    int   zAdjust = type->NormalZAdjust;
    SHPStruct* depthShape = FileSystem::BUILDINGZ_SHA();
    bool  openRoof = false;

	if (curMission == Mission::Unload){
		if (TechnoClass* contactUnit = this->GetRadioContact()) {
			TechnoTypeClass* unitType = GET_TECHNOTYPE(contactUnit);
			if (unitType->JumpJet || unitType->BalloonHover) {
				openRoof = true;
			}
        }
    }

	int tintColor = 0;

	//at building center cell
	CoordStruct center = this->GetCoords();
	const CellStruct centerCell = {
		static_cast<short>(center.X >> 8),
		static_cast<short>(center.Y >> 8)
	};

	auto pCenterCell = MapClass::Instance->GetCellAt(centerCell);

	if (!pCenterCell->IsShrouded()) {
		tintColor = TechnoExtData::ApplyTintColor(this, true, true, false);
		TechnoExtData::ApplyCustomTint(this, &tintColor, nullptr);
	}

	auto pBuildingCell = MapClass::Instance->GetCellAt(cellIdx);
	auto& door = this->UnloadTimer;

	if (curMission == Mission::Open && (door.IsOpening() || door.IsClosing() || door.IsOpen()))
	{
		// Calculate gate frame based on door state
		int gateFrame = (int)(door.GetCompletePercent()
					* this->Type->GateStages);

		if (door.IsClosing()) {
			gateFrame = this->Type->GateStages - gateFrame;
		}

		if (door.IsClosed()) {
			gateFrame = 0;
		}

		if (door.IsOpen()) {
			gateFrame = this->Type->GateStages - 1;
		}

		// Clamp frame to valid range
		gateFrame = MaxImpl(0, MinImpl(gateFrame, this->Type->GateStages - 1));

		// Add damage frame offset if building is damaged
		if (this->GetHealthPercentage() <= RulesClass::Instance->ConditionYellow) {
			gateFrame += this->Type->GateStages + 1;
		}

		ZGradient zgrad = ZGradient::Ground;

		if (gateFrame < this->Type->GateStages / 2 || pTypeExt->IsBarGate) {
			zgrad = ZGradient::Deg90;
		}

		const int lightLevel = pBuildingCell->Color1.Red;
		const int depthAdjust = Game::AdjustHeight(this->GetZ());

		this->Draw_Object(shape,
			gateFrame,
			pPixel,
			pBound,
			DirType::North,  // rotation
			256,  // scale
			zAdjust - depthAdjust,  // height adjust
			zgrad,  // ZGradient
			1,  // useZBuffer
			lightLevel,
			tintColor,
			0, 0, 0, 0, BlitterFlags::None  // z-shape params
		);

		return;
	}

	if (curMission == Mission::Unload) {
		if (openRoof && type->RoofDeployingAnim) {
			if (this->Type->RoofDeployingAnim) {
				shape = this->Type->RoofDeployingAnim;
				zAdjust = -40;
			}
		} else {
			if (this->Type->DeployingAnim) {
				shape = this->Type->DeployingAnim;
				zAdjust = -20;
			}
		}
	}

	int zShapeX = 198;
	int zShapeY = 446;

    {
        const bool applyZShape = (curMission != Mission::Construction && curMission != Mission::Selling) || pTypeExt->ZShapePointMove_OnBuildup;

        if (applyZShape)
        {
            zShapeX += type->ZShapePointMove.X;
            zShapeY += type->ZShapePointMove.Y;
        }
    }

    // Compute screen-space depth shadow placement from building tile dimensions
    //         The cell-coord pair from Center_Coord / 256 is passed as input.
    {
        // Reinterpret cell position as the XY input expected by the screen helper
        Point2D screenInput = {
            static_cast<int>(static_cast<short>(center.X >> 8)),
            static_cast<int>(static_cast<short>(center.Y >> 8))
        };

        const int buildH = this->Type->GetFoundationWidth();
        const int buildW = this->Type->GetFoundationHeight(0);

		Point2D screenSize {
			(buildW << 8) - 256 , (buildH << 8) - 256
		};
        
		Point2D screenOffset = TacticalClass::Instance->
			AdjustForZShapeMove(screenSize.X, screenSize.Y);

        zShapeX -= screenOffset.X;
        zShapeY -= screenOffset.Y;

        // Wide buildings (>= 8 tiles) don't use the depth shape
        if (buildW >= 8)
            depthShape = nullptr;
    }

	const int shapeNum = this->GetShapeNumber();

    // Only draws if the clipping rect has positive height (visible on screen).
    if (pBound->Height > 0)
    {
		const int lightLevel = (int16)this->Type->ExtraLight + pBuildingCell->Color1.Red;
		const int depthAdjust = Game::AdjustHeight(this->GetZ());

		if (!pTypeExt->Firestorm_Wall)
		{
			// Frame selection: min(Shape_Number(), frameCount/2)
			// Shape frame count is stored as a signed word at shape+6.
			const int frameCount = shape->Frames;

			const int displayFrame = (shapeNum >= frameCount / 2)
				? (frameCount / 2)
				: shapeNum;

			this->Draw_Object(
				shape, displayFrame,
				pPixel, pBound,
				DirType::North,
				256,
				zAdjust - depthAdjust,
				ZGradient::Deg90,
				1,
				lightLevel, tintColor,
				depthShape,
				0,
				zShapeX,
				zShapeY,
				BlitterFlags::None);
		}
		else
		{
			this->Draw_Object(shape,
				shapeNum,
				pPixel,
				pBound,
				DirType::North,  // rotation
				256,  // scale
				-1 - depthAdjust,  // height adjust
				ZGradient::Ground,  // ZGradient
				1,  // useZBuffer
				lightLevel,
				tintColor,
				depthShape,
				0,
				zShapeX,
				zShapeY,
				BlitterFlags::None  // flags
			);
		}
    }

	if (type->BibShape &&
		(this->BState != BStateType::Construction || pTypeExt->ZShapePointMove_OnBuildup)		
		)
    {
        if (this->ActuallyPlacedOnMap)
        {
            // Bib uses drawflag1=0 (no depth sort), NOT 2 like the main shape.
            // Height = -1 - Adjust_For_Height (note: more negative than under-door).
			const int lightLevel = (int)(this->Type->ExtraLight + pBuildingCell->Color1.Red);
			const int heightZ = -1 - Game::AdjustHeight(this->GetZ());

            this->Draw_Object(
                type->BibShape, shapeNum,
                pPixel, pBound,
				DirType::North,
				256,
				heightZ,
				ZGradient::Ground,
                1,
				lightLevel, tintColor,
                nullptr, 0, 0, 0, BlitterFlags::None);
        }
    }

    if (curMission != Mission::Unload)
        return;

	if (const auto RoofAnim = openRoof ? this->Type->UnderRoofDoorAnim : this->Type->UnderDoorAnim)
	{
		this->Draw_Object(
			RoofAnim,
			this->GetHealthPercentage() <= RulesClass::Instance->ConditionYellow ? 1 : 0,
			pPixel,
			pBound,
			DirType::North, 256,  // rotation, scale
			-Game::AdjustHeight(this->GetZ()),  // height adjust
			ZGradient::Ground, 1,  // ZGradient, useZBuffer
			 (int16)this->Type->ExtraLight + pBuildingCell->Color1.Red,
			tintColor,
			0, 0, 0, 0, BlitterFlags::None  // z-shape params
		);
	}

    return;
}
