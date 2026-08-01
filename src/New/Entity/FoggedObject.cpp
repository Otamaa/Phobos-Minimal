#include "FoggedObject.h"

#include <Ext/Building/Body.h>
#include <Ext/BuildingType/Body.h>

#include <Ext/TechnoType/Body.h>
#include <Ext/TerrainType/Body.h>
#include <Ext/Cell/Body.h>

#include <TacticalClass.h>
#include <SmudgeTypeClass.h>
#include <AnimClass.h>
#include <TerrainClass.h>
#include <HouseClass.h>

#include <algorithm>

HelperedVector<FoggedObject*> FoggedObject::FoggedObjects;
char FoggedObject::BuildingVXLDrawer[sizeof(BuildingClass)];
bool FoggedObject::BuildingVXLDrawerReady = false;

HRESULT FoggedObject::SaveGlobal(IStream* pStm)
{
	HRESULT hr;
	const int _szWrite = FoggedObjects.size();

	hr = pStm->Write(&_szWrite, sizeof(_szWrite), nullptr);
	if (FAILED(hr)) return hr;

	for (auto const pObject : FoggedObjects) {
		hr = pStm->Write(&pObject, sizeof(pObject), nullptr);
		if (FAILED(hr)) return hr;
		hr = pObject->Save(pStm);
		if (FAILED(hr)) return hr;
	}

	hr = pStm->Write(BuildingVXLDrawer, sizeof(BuildingVXLDrawer), nullptr);
	if (FAILED(hr)) return hr;

	return pStm->Write(&BuildingVXLDrawerReady, sizeof(BuildingVXLDrawerReady), nullptr);
}

HRESULT FoggedObject::LoadGlobal(IStream* pStm)
{
	HRESULT hr;
	int count;
	hr = pStm->Read(&count, sizeof(count), nullptr);
	if (FAILED(hr)) return hr;

	for (int i = 0; i < count; ++i) {
		auto pObject = GameCreate<FoggedObject>();
		long pOldObject;
		hr = pStm->Read(&pOldObject, sizeof(pOldObject), nullptr);
		if (FAILED(hr)) return hr;

		SwizzleManagerClass::Instance->Here_I_Am(pOldObject, pObject);
		hr = pObject->Load(pStm);
		if (FAILED(hr)) return hr;
	}

	hr = pStm->Read(BuildingVXLDrawer, sizeof(BuildingVXLDrawer), nullptr);
	if (FAILED(hr)) return hr;

	return pStm->Read(&BuildingVXLDrawerReady, sizeof(BuildingVXLDrawerReady), nullptr);
}

void FoggedObject::Clear()
{
	auto snap = FoggedObjects;
	FoggedObjects.clear();

	for (auto const pObject : snap)
		GameDelete(pObject);

	BuildingVXLDrawerReady = false;
}

FoggedObject::FoggedObject() noexcept
{
	FoggedObjects.push_back(this);
}

FoggedObject::FoggedObject(BuildingClass* pBld, bool IsVisible) noexcept
{
	CoveredType = CoveredType::Building;

	Location = pBld->Location;
	Visible = IsVisible;
	BuildingData.Owner = pBld->Owner;
	BuildingData.Type = pBld->Type;
	BuildingData.BaseCoords = pBld->GetMapCoords();
	BuildingData.ShapeFrame = pBld->GetShapeNumber();
	BuildingData.PrimaryFacing = pBld->PrimaryFacing;
	BuildingData.SecondaryFacing = pBld->SecondaryFacing;
	BuildingData.BarrelFacing = pBld->BarrelFacing;
	BuildingData.TurretRecoil = pBld->TurretRecoil;
	BuildingData.BarrelRecoil = pBld->BarrelRecoil;
	BuildingData.IsFirestormWall = pBld->Type->FirestormWall;
	BuildingData.TurretAnimFrame = pBld->TurretAnimFrame;
	pBld->GetRenderDimensions(&Bound);

	memset(BuildingData.Anims, 0, sizeof(BuildingData.Anims));
	auto pAnimData = BuildingData.Anims;
	for (auto pAnim : pBld->Anims) {
		if (pAnim) {
			pAnimData->AnimType = pAnim->Type;
			pAnimData->AnimFrame = pAnim->Animation.Stage + pAnimData->AnimType->Start;
			pAnimData->ZAdjust = pAnim->ZAdjust + pAnimData->AnimType->YDrawOffset - Game::AdjustHeight(pAnim->Location.Z);
			pAnimData->ZAdjust -= pAnimData->AnimType->Flat ? 3 : 2;
			++pAnimData;

			pAnim->IsFogged = true;
			RectangleStruct buffer;
			pAnim->GetDimensions(&buffer);
			Bound = std::move(Drawing::Union(Bound, buffer));
		}
	}

	// Bound is still view-local here, which is what RegisterDirtyArea wants.
	TacticalClass::Instance->RegisterDirtyArea(Bound, false);

	// Bound is stored as (view-local + TacticalPos). Render() converts it back with
	// (+ ViewBounds - TacticalPos), so no ViewBounds term may be folded in here.
	Bound.X += TacticalClass::Instance->TacticalPos.X;
	Bound.Y += TacticalClass::Instance->TacticalPos.Y;
	pBld->IsFogged = true;

	if (!BuildingVXLDrawerReady && (pBld->Type->TurretAnimIsVoxel || pBld->Type->BarrelAnimIsVoxel))
	{
		memcpy(BuildingVXLDrawer, pBld, sizeof(BuildingClass));
		reinterpret_cast<BuildingClass*>(BuildingVXLDrawer)->BeingWarpedOut = false;
		reinterpret_cast<BuildingClass*>(BuildingVXLDrawer)->WarpFactor = 0.0f;
		BuildingVXLDrawerReady = true;
	}

	FoggedObjects.push_back(this);
}

FoggedObject::FoggedObject(TerrainClass* pTerrain) noexcept
{
	Location = pTerrain->Location;
	CoveredType = CoveredType::Terrain;

	pTerrain->GetRenderDimensions(&Bound);

	TacticalClass::Instance->RegisterDirtyArea(Bound, false);

	Bound.X += TacticalClass::Instance->TacticalPos.X;
	Bound.Y += TacticalClass::Instance->TacticalPos.Y;

	TerrainData.Type = pTerrain->Type;
	TerrainData.Frame = 0;
	if (TerrainData.Type->IsAnimated)
		TerrainData.Frame = pTerrain->Animation.Stage;
	else if (pTerrain->TimeToDie)
		TerrainData.Frame = pTerrain->Animation.Stage + 1;
	else if (pTerrain->Health < 2)
		TerrainData.Frame = 2;

	TerrainData.Flat = TerrainData.Type->IsAnimated || pTerrain->TimeToDie;

	FoggedObjects.push_back(this);
}

FoggedObject::FoggedObject(CellClass* pCell, bool IsOverlay) noexcept
{
	pCell->GetCoords(&Location);
	if (IsOverlay)
	{
		CoveredType = CoveredType::Overlay;

		RectangleStruct containingRect;
		RectangleStruct shapeRect = pCell->GetOverlayShapeRect();
		pCell->GetContainingRect(&containingRect);
		Bound = std::move(Drawing::Union(shapeRect, containingRect));

		TacticalClass::Instance->RegisterDirtyArea(Bound, false);

		Bound.X += TacticalClass::Instance->TacticalPos.X;
		Bound.Y += TacticalClass::Instance->TacticalPos.Y;

		OverlayData.Overlay = pCell->OverlayTypeIndex;
		OverlayData.OverlayData = pCell->OverlayData;
	}
	else
	{
		CoveredType = CoveredType::Smudge;

		Location.Z = pCell->Level * Unsorted::LevelHeight;
		Point2D position = TacticalClass::Instance->CoordsToClient(Location);

		RectangleStruct local { position.X - 30, position.Y - 15, 60, 30 };
		TacticalClass::Instance->RegisterDirtyArea(local, false); // EXTENSION, as above

		Bound = RectangleStruct
		{
			local.X + TacticalClass::Instance->TacticalPos.X,
			local.Y + TacticalClass::Instance->TacticalPos.Y,
			60,
			30
		};

		SmudgeData.Smudge = pCell->SmudgeTypeIndex;
		SmudgeData.SmudgeData = pCell->SmudgeData;
		SmudgeData.Height = Location.Z;
	}

	FoggedObjects.push_back(this);
}

HRESULT FoggedObject::Load(IStream* pStm)
{
	HRESULT hr;

	hr = pStm->Read(this, sizeof(FoggedObject), nullptr);
	if (FAILED(hr)) return hr;

	if (CoveredType == CoveredType::Building)
	{
		SWIZZLE(BuildingData.Owner);
		SWIZZLE(BuildingData.Type);
		for (auto& Anim : BuildingData.Anims) {
			if (!Anim.AnimType)
				break;
			SWIZZLE(Anim.AnimType);
		}
	} else if (CoveredType == CoveredType::Terrain) {
		SWIZZLE(TerrainData.Type);
	}

	return hr;
}

HRESULT FoggedObject::Save(IStream* pStm)
{
	return pStm->Write(this, sizeof(FoggedObject), nullptr);
}

FoggedObject::~FoggedObject()
{
	FoggedObjects.remove(this);

	if (this->CoveredType == CoveredType::Building)
	{
		BuildingClass* pBld = nullptr;

		if (auto const pType = BuildingData.Type) {
			for (auto pFoundation = pType->GetFoundationData(false);
				!pBld && (pFoundation->X != 0x7FFF || pFoundation->Y != 0x7FFF);
				++pFoundation) {
				CellStruct const mapCoord
				{
					static_cast<short>(BuildingData.BaseCoords.X + pFoundation->X),
					static_cast<short>(BuildingData.BaseCoords.Y + pFoundation->Y)
				};

				if (auto const pCell = MapClass::Instance->TryGetCellAt(mapCoord))
					pBld = pCell->GetBuilding();
			}
		}

		if (!pBld) {
			if (auto const pCentre = MapClass::Instance->TryGetCellAt(Location))
				pBld = pCentre->GetBuilding();
		}

		if (pBld) {
			pBld->IsFogged = false;
			for (auto pAnim : pBld->Anims)
				if (pAnim)
					pAnim->IsFogged = false;

			pBld->NeedsRedraw = true;

			if (TacticalClass::Instance()) {
				RectangleStruct dirty;
				pBld->GetRenderDimensions(&dirty);
				TacticalClass::Instance->RegisterDirtyArea(dirty, false);
			}
		}
	}
}

void FoggedObject::SortForRender()
{
	auto const layerOf = [](const FoggedObject* pObject) noexcept -> int
		{
			switch (pObject->CoveredType)
			{
			case CoveredType::Smudge:   return 0;
			case CoveredType::Overlay:  return 1;
			case CoveredType::Terrain:  return 2;
			case CoveredType::Building: return 3;
			default:                    return 4;
			}
		};

	std::stable_sort(FoggedObjects.begin(), FoggedObjects.end(),
		[&layerOf](const FoggedObject* pA, const FoggedObject* pB) noexcept
		{
			int const layerA = layerOf(pA);
			int const layerB = layerOf(pB);
			if (layerA != layerB)
				return layerA < layerB;

			int const depthA = (pA->Location.X >> 8) + (pA->Location.Y >> 8);
			int const depthB = (pB->Location.X >> 8) + (pB->Location.Y >> 8);
			if (depthA != depthB)
				return depthA < depthB;

			return pA->Location.Z < pB->Location.Z;
		});
}

void FoggedObject::RenderAll(const RectangleStruct& viewRect)
{
	SortForRender();

	for (auto const pObject : FoggedObjects)
		pObject->Render(viewRect);
}

void FoggedObject::Render(const RectangleStruct& viewRect) const
{
	if (!Visible)
		return;

	RectangleStruct buffer = Bound;
	buffer.X += DSurface::ViewBounds->X - TacticalClass::Instance->TacticalPos.X;
	buffer.Y += DSurface::ViewBounds->Y - TacticalClass::Instance->TacticalPos.Y;
	RectangleStruct finalRect = buffer.IntersectWith(viewRect);
	if (finalRect.Width <= 0 || finalRect.Height <= 0)
		return;

	switch (CoveredType)
	{
	case CoveredType::Building:
		RenderAsBuilding(viewRect);
		break;

	case CoveredType::Overlay:
		RenderAsOverlay(viewRect);
		break;

	case CoveredType::Terrain:
		RenderAsTerrain(viewRect);
		break;

	case CoveredType::Smudge:
		RenderAsSmudge(viewRect);
		break;
	}
}

RectangleStruct FoggedObject::Union(const RectangleStruct& rect1, const RectangleStruct& rect2)
{
	if (rect1.Width <= 0 || rect1.Height <= 0)
		return rect2;
	if (rect2.Width <= 0 || rect2.Height <= 0)
		return rect1;

	int const left = std::min(rect1.X, rect2.X);
	int const top = std::min(rect1.Y, rect2.Y);
	int const right = std::max(rect1.X + rect1.Width, rect2.X + rect2.Width);
	int const bottom = std::max(rect1.Y + rect1.Height, rect2.Y + rect2.Height);

	return { left, top, right - left, bottom - top };
}

int FoggedObject::GetIndexID() const
{
	int x = Location.X / 256;
	int y = Location.Y / 256;
	return (y - ((x + y) << 9) - x) - static_cast<int>(CoveredType) * 0x80000 + INT_MAX;
}

void FoggedObject::RenderAsBuilding(const RectangleStruct& viewRect) const
{
	auto const pType = BuildingData.Type;
	if (pType->InvisibleInGame)
		return;

	auto pScheme = ColorScheme::Array->Items[BuildingData.Owner->ColorSchemeIndex];
	auto pSHP = pType->GetImage();
	CoordStruct coord = { Location.X - 128,Location.Y - 128,Location.Z };
	Point2D point = TacticalClass::Instance->CoordsToClient(coord);
	point.X += DSurface::ViewBounds->X - viewRect.X;
	point.Y += DSurface::ViewBounds->Y - viewRect.Y;

	auto pCell = MapClass::Instance->GetCellAt(Location);
	ConvertClass* pConvert;
	if (!pType->TerrainPalette)
		pConvert = pScheme->LightConvert;
	else {
		if (!pCell->LightConvert)
			pCell->InitLightConvert();
		pConvert = pCell->LightConvert;
	}
	if (pType->Palette)
		pConvert = pType->Palette->Items[BuildingData.Owner->ColorSchemeIndex]->LightConvert;

	if (pSHP) {
		RectangleStruct rect = viewRect;
		int height = point.Y + pSHP->Height / 2;
		if (rect.Height > height)
			rect.Height = height;
		int ZAdjust = -2 - Game::AdjustHeight(Location.Z);
		int Intensity = pCell->Color1.Red + pType->ExtraLight;
		if (rect.Height > 0) {
			if (BuildingData.IsFirestormWall) {
				CC_Draw_Shape(DSurface::Temp, pConvert, pSHP, BuildingData.ShapeFrame, &point, &rect,
					BlitterFlags::ZReadWrite | BlitterFlags::Alpha | BlitterFlags::bf_400 | BlitterFlags::Centered,
					0, ZAdjust, ZGradient::Ground, Intensity, 0, nullptr, 0, 0, 0);
			} else {
				CC_Draw_Shape(DSurface::Temp, pConvert, pSHP, BuildingData.ShapeFrame, &point, &rect,
					BlitterFlags::ZReadWrite | BlitterFlags::Alpha | BlitterFlags::bf_400 | BlitterFlags::Centered,
					0, ZAdjust, ZGradient::Deg90, Intensity, 0, nullptr, 0, 0, 0);
				CC_Draw_Shape(DSurface::Temp, pConvert, pSHP, BuildingData.ShapeFrame + pSHP->Frames / 2, &point, &rect,
					BlitterFlags::ZReadWrite | BlitterFlags::Alpha | BlitterFlags::bf_400 | BlitterFlags::Centered | BlitterFlags::Darken,
					0, ZAdjust, ZGradient::Ground, 1000, 0, nullptr, 0, 0, 0);
				if (pType->BibShape)
				{
					CC_Draw_Shape(DSurface::Temp, pConvert, pType->BibShape, BuildingData.ShapeFrame, &point, &viewRect,
						BlitterFlags::ZReadWrite | BlitterFlags::Alpha | BlitterFlags::bf_400 | BlitterFlags::Centered,
						0, ZAdjust - 1, ZGradient::Deg90, Intensity, 0, nullptr, 0, 0, 0);
				}
			}
		}

		Point2D turretPoint
		{
			point.X + pType->GetBuildingAnim(BuildingAnimSlot::Turret).Position.X,
			point.Y + pType->GetBuildingAnim(BuildingAnimSlot::Turret).Position.Y
		};

		if ((pType->TurretAnimIsVoxel || pType->BarrelAnimIsVoxel) && BuildingVXLDrawerReady) {
			auto pVXLDrawer = reinterpret_cast<BuildingClass*>(BuildingVXLDrawer);
			pVXLDrawer->Type = pType;
			pVXLDrawer->PrimaryFacing = BuildingData.PrimaryFacing;
			pVXLDrawer->SecondaryFacing = BuildingData.SecondaryFacing;
			pVXLDrawer->BarrelFacing = BuildingData.BarrelFacing;
			pVXLDrawer->TurretRecoil = BuildingData.TurretRecoil;
			pVXLDrawer->BarrelRecoil = BuildingData.BarrelRecoil;
			pVXLDrawer->Owner = BuildingData.Owner;
			pVXLDrawer->Location = Location;
			pVXLDrawer->TurretAnimFrame = BuildingData.TurretAnimFrame;

			auto const primaryDir = BuildingData.SecondaryFacing.Current();
			int turretFacing = 0;
			int barrelFacing = 0;
			if (pType->TurretVoxel.HVA)
				turretFacing = BuildingData.TurretAnimFrame % pType->TurretVoxel.HVA->FrameCount;
			if (pType->BarrelVoxel.HVA)
				barrelFacing = BuildingData.TurretAnimFrame % pType->BarrelVoxel.HVA->FrameCount;
			int val32 = primaryDir.Getvalue32();
			int turretExtra = ((unsigned char)turretFacing << 16) | val32;
			int barrelExtra = ((unsigned char)barrelFacing << 16) | val32;

			if (pType->TurretVoxel.VXL) {
				Matrix3D matrixturret;
				matrixturret.MakeIdentity();
				matrixturret.RotateZ(static_cast<float>(primaryDir.GetRadian()));
				TechnoTypeExtContainer::Instance.Find(pType)->ApplyTurretOffset(&matrixturret, 0.125);

				Vector3D<float> negativevector = { -matrixturret.Row[0].W ,-matrixturret.Row[1].W,-matrixturret.Row[2].W };
				Vector3D<float> vector = { matrixturret.Row[0].W ,matrixturret.Row[1].W,matrixturret.Row[2].W };
				Matrix3D matrixbarrel = matrixturret;
				if (BuildingData.TurretRecoil.State != RecoilData::RecoilState::Inactive) {
					matrixturret.TranslateX(-BuildingData.TurretRecoil.TravelSoFar);
					turretExtra = -1;
				}

				Matrix3D::MatrixMultiply(&matrixturret, &Game::VoxelDefaultMatrix(), &matrixturret);

				bool bDrawBarrel = pType->BarrelVoxel.VXL && pType->BarrelVoxel.HVA;
				if (bDrawBarrel) {
					matrixbarrel.Translate(negativevector);
					if (BuildingData.BarrelRecoil.State != RecoilData::RecoilState::Inactive)
					{
						matrixbarrel.TranslateX(-BuildingData.BarrelRecoil.TravelSoFar);
						barrelExtra = -1;
					}
					matrixbarrel.RotateY(-static_cast<float>(BuildingData.BarrelFacing.Current().GetRadian()));
					matrixbarrel.Translate(vector);
					Matrix3D::MatrixMultiply(&matrixbarrel, &Game::VoxelDefaultMatrix(), &matrixbarrel);
				}

				int facetype = (((((*(unsigned int*)&primaryDir) >> 13) + 1) >> 1) & 3);
				if (facetype == 0 || facetype == 3)
				{
					if (bDrawBarrel)
						pVXLDrawer->DrawVoxel(BuildingData.Type->BarrelVoxel, barrelFacing, (short)barrelExtra,
							BuildingData.Type->VoxelCaches_[3], viewRect, turretPoint, matrixbarrel,
							pCell->Color1.Red, 0, 0);

					pVXLDrawer->DrawVoxel(BuildingData.Type->TurretVoxel, turretFacing, (short)turretExtra,
						BuildingData.Type->VoxelCaches_[1], viewRect, turretPoint, matrixturret,
						pCell->Color1.Red, 0, 0);
				} else {
					pVXLDrawer->DrawVoxel(BuildingData.Type->TurretVoxel, turretFacing, (short)turretExtra,
						BuildingData.Type->VoxelCaches_[1], viewRect, turretPoint, matrixturret,
						pCell->Color1.Red, 0, 0);

					if (bDrawBarrel)
						pVXLDrawer->DrawVoxel(BuildingData.Type->BarrelVoxel, barrelFacing, (short)barrelExtra,
							BuildingData.Type->VoxelCaches_[3], viewRect, turretPoint, matrixbarrel,
							pCell->Color1.Red, 0, 0);
				}
			} else if (pType->BarrelVoxel.VXL && pType->BarrelVoxel.HVA) {
				Matrix3D matrixbarrel;
				matrixbarrel.MakeIdentity();
				Vector3D<float> negativevector = { -matrixbarrel.Row[0].W ,-matrixbarrel.Row[1].W,-matrixbarrel.Row[2].W };
				Vector3D<float> vector = { matrixbarrel.Row[0].W ,matrixbarrel.Row[1].W,matrixbarrel.Row[2].W };
				matrixbarrel.Translate(negativevector);
				matrixbarrel.RotateZ(static_cast<float>(primaryDir.GetRadian()));
				matrixbarrel.RotateY(-static_cast<float>(BuildingData.BarrelFacing.Current().GetRadian()));
				matrixbarrel.Translate(vector);
				Matrix3D::MatrixMultiply(&matrixbarrel, &Game::VoxelDefaultMatrix(), &matrixbarrel);
				pVXLDrawer->DrawVoxel(BuildingData.Type->BarrelVoxel, barrelFacing, (short)barrelExtra,
					BuildingData.Type->VoxelCaches_[3], viewRect, turretPoint, matrixbarrel,
					pCell->Color1.Red, 0, 0);
			}
		}
	}

	for (const auto& AnimData : BuildingData.Anims) {
		if (!AnimData.AnimType)
			break;

		auto pAnimType = AnimData.AnimType;

		if (auto pAnimSHP = pAnimType->GetImage()) {
			ConvertClass* pAnimConvert = pAnimType->ShouldUseCellDrawer ? pScheme->LightConvert : FileSystem::ANIM_PAL();

			CC_Draw_Shape(DSurface::Temp, pAnimConvert, pAnimSHP, AnimData.AnimFrame, &point, &viewRect,
				BlitterFlags::ZReadWrite | BlitterFlags::Alpha | BlitterFlags::bf_400 | BlitterFlags::Centered,
				0, AnimData.ZAdjust, pAnimType->Flat ? ZGradient::Ground : ZGradient::Deg90,
				pAnimType->UseNormalLight ? 1000 : pCell->Color1.Red, 0, nullptr, 0, 0, 0);
			if (pAnimType->Shadow)
			{
				CC_Draw_Shape(DSurface::Temp, pAnimConvert, pAnimSHP, AnimData.AnimFrame + pAnimSHP->Frames / 2, &point, &viewRect,
					BlitterFlags::ZReadWrite | BlitterFlags::Alpha | BlitterFlags::bf_400 | BlitterFlags::Centered | BlitterFlags::Darken,
					0, AnimData.ZAdjust, ZGradient::Deg90, 1000, 0, nullptr, 0, 0, 0);
			}
		}
	}
}

void FoggedObject::RenderAsSmudge(const RectangleStruct& viewRect) const
{
	auto const pSmudge = SmudgeTypeClass::Array->Items[SmudgeData.Smudge];
	Point2D position
	{
			this->Bound.X - TacticalClass::Instance->TacticalPos.X - viewRect.X + DSurface::ViewBounds->X + 30,
			this->Bound.Y - TacticalClass::Instance->TacticalPos.Y - viewRect.Y + DSurface::ViewBounds->Y
	};
	CellStruct MapCoord = CellClass::Coord2Cell(Location);
	pSmudge->DrawIt(position, viewRect, SmudgeData.SmudgeData, SmudgeData.Height, MapCoord);
}

void FoggedObject::RenderAsOverlay(const RectangleStruct& viewRect) const
{
	if (OverlayData.Overlay == -1)
		return;

	auto pCell = MapClass::Instance->TryGetCellAt(Location);
	if (!pCell)
		return;

	CoordStruct coords =
	{
		(((pCell->MapCoords.X << 8) + 128) / 256) << 8,
		(((pCell->MapCoords.Y << 8) + 128) / 256) << 8,
		0
	};
	Point2D position = TacticalClass::Instance->CoordsToClient(coords);
	position.X -= 30;

	position.X += DSurface::ViewBounds->X;
	position.Y += DSurface::ViewBounds->Y;

	bool const bThrottled = (pCell->Flags & CellFlags::BridgeOwner) != CellFlags::Empty;
	DWORD const savedRedrawFrame = pCell->RedrawFrame;
	RectangleStruct const savedViewportRect = pCell->InViewportRect;
	char const savedRedrawCount = pCell->RedrawCountMAYBE;

	std::swap(pCell->OverlayTypeIndex, const_cast<FoggedObject*>(this)->OverlayData.Overlay);
	std::swap(pCell->OverlayData, const_cast<FoggedObject*>(this)->OverlayData.OverlayData);

	pCell->DrawOverlay(position, viewRect);
	pCell->DrawOverlayShadow(position, viewRect);

	std::swap(pCell->OverlayTypeIndex, const_cast<FoggedObject*>(this)->OverlayData.Overlay);
	std::swap(pCell->OverlayData, const_cast<FoggedObject*>(this)->OverlayData.OverlayData);

	if (bThrottled)
	{
		pCell->RedrawFrame = savedRedrawFrame;
		pCell->InViewportRect = savedViewportRect;
		pCell->RedrawCountMAYBE = savedRedrawCount;
	}
}

void FoggedObject::RenderAsTerrain(const RectangleStruct& viewRect) const
{
	auto pCell = MapClass::Instance->GetCellAt(Location);
	if (auto pSHP = TerrainData.Type->GetImage())
	{
		int nZAdjust = -Game::AdjustHeight(Location.Z);
		Point2D point = TacticalClass::Instance->CoordsToClient(Location);
		point.X += DSurface::ViewBounds->X - viewRect.X;
		point.Y += DSurface::ViewBounds->Y - viewRect.Y;
		if (!pCell->LightConvert)
			pCell->InitLightConvert();

		BlitterFlags blitterFlag = BlitterFlags::Centered | BlitterFlags::bf_400 | BlitterFlags::Alpha;
		if (TerrainData.Flat)
			blitterFlag |= BlitterFlags::Flat;
		else
			blitterFlag |= BlitterFlags::ZReadWrite;

		ConvertClass* pConvert;
		int nIntensity;

		if (TerrainData.Type->SpawnsTiberium)
		{
			pConvert = FileSystem::GRFTXT_TIBERIUM_PAL;
			nIntensity = pCell->Color1.Red;
			point.Y -= 16;
		}
		else
		{
			pConvert = pCell->LightConvert;
			nIntensity = pCell->Color1.Green;
		}

		CC_Draw_Shape(DSurface::Temp, pConvert, pSHP, TerrainData.Frame, &point,
			&viewRect, blitterFlag, 0, nZAdjust - 12, ZGradient::Deg90, nIntensity,
			0, 0, 0, 0, 0);
		if (Game::bDrawShadow())
			CC_Draw_Shape(DSurface::Temp, pConvert, pSHP, TerrainData.Frame + pSHP->Frames / 2, &point,
				&viewRect, blitterFlag | BlitterFlags::Darken, 0, nZAdjust - 3,
				ZGradient::Ground, 1000, 0, 0, 0, 0, 0);
	}
}