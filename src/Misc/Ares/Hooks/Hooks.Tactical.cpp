#include <AbstractClass.h>
#include <TechnoClass.h>
#include <FootClass.h>
#include <UnitClass.h>
#include <Utilities/Macro.h>
#include <Helpers/Macro.h>
#include <Base/Always.h>

#include <HouseClass.h>
#include <Utilities/Debug.h>

#include <Ext/Anim/Body.h>
#include <Ext/AnimType/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/VoxelAnim/Body.h>
#include <Ext/Terrain/Body.h>
#include <Ext/Techno/Body.h>

#include <TerrainTypeClass.h>
#include <New/Type/ArmorTypeClass.h>

#include <Notifications.h>

#include <Conversions.h>

DEFINE_OVERRIDE_HOOK(0x420F75, AlphaLightClass_UpdateScreen_ShouldDraw, 5)
{
	GET(AlphaShapeClass*, pAlpha, ECX);

	bool shouldDraw = !pAlpha->IsObjectGone;

	if(shouldDraw) {
		if(auto pTechno = abstract_cast<TechnoClass*>(pAlpha->AttachedTo)) {
			shouldDraw =
			pTechno->VisualCharacter(VARIANT_TRUE, pTechno->Owner) == VisualType::Normal
			&& !pTechno->Disguised;
		}
	}

	return shouldDraw ? 0x420F80 : 0x42132A;

}

DEFINE_OVERRIDE_HOOK(0x4210AC, AlphaLightClass_UpdateScreen_Header, 5)
{
	GET(AlphaShapeClass*, pAlpha, EDX);
	GET(SHPStruct *, pImage, ECX);

	if(const auto pTechno = abstract_cast<TechnoClass*>(pAlpha->AttachedTo)) {
		unsigned int idx = 0;
		if (pImage->Frames > 0) {
			const int countFrames = Conversions::Int2Highest(pImage->Frames);
			const DirStruct PrimaryFacing = pTechno->PrimaryFacing.Current();
			idx = (PrimaryFacing.Raw >> (16 - countFrames));
		}

		R->Stack(0x0, idx);
	}

	return 0;
}

DEFINE_OVERRIDE_HOOK(0x4211AC, AlphaLightClass_UpdateScreen_Body, 8)
{
	GET_STACK(int, AlphaLightIndex, STACK_OFFS(0xDC, 0xB4));
	GET_STACK(SHPStruct*, pImage, STACK_OFFS(0xDC, 0x6C));

	const auto pAlpha = AlphaShapeClass::Array->Items[AlphaLightIndex];

	if(const auto pTechno = abstract_cast<TechnoClass*>(pAlpha->AttachedTo)) {
		unsigned int idx = 0;
		if (pImage->Frames > 0) {
			const int countFrames = Conversions::Int2Highest(pImage->Frames);
			const DirStruct PrimaryFacing = pTechno->PrimaryFacing.Current();
			idx = (PrimaryFacing.Raw >> (16 - countFrames));
		}

		R->Stack(0x0, idx);
	}

	return 0;
}

DEFINE_OVERRIDE_HOOK(0x42146E, TacticalClass_UpdateAlphasInRectangle_Header, 5)
{
	GET(int, AlphaLightIndex, EBX);
	GET(RectangleStruct*, buffer, EDX);
	GET(SHPStruct*, pImage, EDI);

	const auto pAlpha = AlphaShapeClass::Array->Items[AlphaLightIndex];
	unsigned int idx = 0;

	if (const auto pTechno = abstract_cast<TechnoClass*>(pAlpha->AttachedTo))  {
		if (pImage->Frames > 0) {
			const int countFrames = Conversions::Int2Highest(pImage->Frames);
			const DirStruct PrimaryFacing = pTechno->PrimaryFacing.Current();
			idx = (PrimaryFacing.Raw >> (16 - countFrames));
		}
	}

	R->EAX(pImage->GetFrameBounds_ptr(buffer, idx));
	return 0x421478;
}

DEFINE_OVERRIDE_HOOK(0x42152C, TacticalClass_UpdateAlphasInRectangle_Body, 8)
{
	GET_STACK(int, AlphaLightIndex, STACK_OFFS(0xA4, 0x78));
	GET(SHPStruct*, pImage, ECX);

	const auto pAlpha = AlphaShapeClass::Array->Items[AlphaLightIndex];
	if (const auto pTechno = abstract_cast<TechnoClass*>(pAlpha->AttachedTo)) {
		if (pImage->Frames > 0) {
			const int countFrames = Conversions::Int2Highest(pImage->Frames);
			const DirStruct PrimaryFacing = pTechno->PrimaryFacing.Current();
			R->ESP((PrimaryFacing.Raw >> (16 - countFrames)));
		}
	}

	return 0;
}

DEFINE_OVERRIDE_HOOK(0x421371, TacticalClass_UpdateAlphasInRectangle_ShouldDraw, 5)
{
	GET(int, AlphaLightIndex, EBX);
	auto pAlpha = AlphaShapeClass::Array->Items[AlphaLightIndex];

	bool shouldDraw = !pAlpha->IsObjectGone;

	if (shouldDraw) {
		if (const auto pTechno = abstract_cast<TechnoClass*>(pAlpha->AttachedTo)) {
			shouldDraw = pTechno->VisualCharacter(VARIANT_TRUE, pTechno->Owner) == VisualType::Normal &&
				!pTechno->Disguised;
		}
	}

	return shouldDraw ? 0 : 0x421694;
}