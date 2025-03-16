#include <Utilities/Macro.h>

#include <New/Entity/ElectricBoltClass.h>
#include <New/Entity/FlyingStrings.h>

#ifdef COMPILE_PORTED_DP_FEATURES_
#include <Misc/DynamicPatcher/Others/TextManager.h>
#endif

#include <FootClass.h>
#include <TechnoClass.h>
#include <ObjectClass.h>

#include <IonBlastClass.h>
#include <VeinholeMonsterClass.h>

#include <Commands/ShowTechnoNames.h>
#include <Commands/ShowAnimNames.h>

#include <TagTypeClass.h>
#include <BitFont.h>

#include <Ext/House/Body.h>

#include <New/HugeBar.h>

bool IsOnMyView(CoordStruct& coords)
{
	auto const Point = TacticalClass::Instance->CoordsToView(coords);
	return Point.X > Drawing::SurfaceDimensions_Hidden().X
		&& Point.Y > Drawing::SurfaceDimensions_Hidden().Y
		&& Point.X < Drawing::SurfaceDimensions_Hidden().X + Drawing::SurfaceDimensions_Hidden().Width
		&& Point.Y < Drawing::SurfaceDimensions_Hidden().Y + Drawing::SurfaceDimensions_Hidden().Height;
}

ASMJIT_PATCH(0x6D4684, TacticalClass_Draw_Addition, 6)
{
	/*auto const pOWner = HouseExtData::FindFirstCivilianHouse();

	for (int i = 0; i < MapClass::Instance->Cells.Capacity; ++i){
		if (auto pCell = MapClass::Instance->Cells[i]) {
			if (pCell->AttachedTag) {
				if (!IsOnMyView(pCell->GetCoords()))
					continue;

				if (pCell->IsFogged() || pCell->IsShrouded())
					continue;

				std::wstring pText((size_t)(0x18 + 1), L'#');
				mbstowcs(&pText[0], pCell->AttachedTag->Type->ID, 0x18);

				if (pText.empty())
					continue;

				Point2D pixelOffset = Point2D::Empty;
				int width = 0, height = 0;
				BitFont::Instance->GetTextDimension(pText.c_str(), &width, &height, 120);
				pixelOffset.X -= (width / 2);

				auto pos = TacticalClass::Instance->CoordsToView(pCell->GetCoords());
				pos += pixelOffset;
				auto bound = DSurface::Temp->Get_Rect_WithoutBottomBar();

				if (!(pos.X < 0 || pos.Y < 0 || pos.X > bound.Width || pos.Y > bound.Height))
				{
					Point2D tmp { 0,0 };
					ColorStruct _color{};
					ColorScheme::Array->Items[pResultOwner->ColorSchemeIndex]->BaseColor.ToColorStruct(&_color);
					Simple_Text_Print_Wide(&tmp, pText.c_str(), DSurface::Temp(), &bound, &pos, _color.ToInit() , 0, TextPrintType::Center, 1);
				}
			}
		}
	}*/

	FlyingStrings::UpdateAll();
	ShowTechnoNameCommandClass::AI();
	ShowAnimNameCommandClass::AI();
	return 0;
}