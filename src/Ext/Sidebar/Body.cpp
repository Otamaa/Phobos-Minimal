#include "Body.h"

#include <Ext/House/Body.h>
#include <Ext/Side/Body.h>

IStream* SidebarExt::g_pStm = nullptr;
std::array<SHPReference*, 4u> SidebarExt::TabProducingProgress {};
 std::unique_ptr<SidebarExt::ExtData> SidebarExt::Data = nullptr;

void SidebarExt::Allocate(SidebarClass* pThis)
{
	Data = std::make_unique<SidebarExt::ExtData>(pThis);
}

void SidebarExt::Remove(SidebarClass* pThis)
{
	Data = nullptr;
}

void SidebarExt::DrawProducingProgress()
{
	const auto pPlayer = HouseClass::CurrentPlayer();

	if (HouseExt::IsObserverPlayer(pPlayer))
		return;

	if (Phobos::UI::ShowProducingProgress) {
		const auto pSideExt = SideExt::ExtMap.Find(SideClass::Array->GetItem(pPlayer->SideIndex));

		if (!pSideExt)
			return;

		const int XOffset = pSideExt->Sidebar_GDIPositions ? 29 : 32;
		const int XBase = (pSideExt->Sidebar_GDIPositions ? 26 : 20) + pSideExt->Sidebar_ProducingProgress_Offset.Get().X;
		const int YBase = 197 + pSideExt->Sidebar_ProducingProgress_Offset.Get().Y;

		for (int i = 0; i < (int)SidebarExt::TabProducingProgress.size(); i++) {
			if (auto pSHP = SidebarExt::TabProducingProgress[i]) {

				const auto rtti = i == 0 || i == 1 ? AbstractType::BuildingType : AbstractType::InfantryType;
				FactoryClass* pFactory = nullptr;

				if (i != 3) {
					pFactory = pPlayer->GetPrimaryFactory(rtti, false, i == 1 ? BuildCat::Combat : BuildCat::DontCare);
				} else {
					pFactory = pPlayer->GetPrimaryFactory(AbstractType::UnitType, false, BuildCat::DontCare);
					if (!pFactory || !pFactory->Object)
						pFactory = pPlayer->GetPrimaryFactory(AbstractType::UnitType, true, BuildCat::DontCare);
					if (!pFactory || !pFactory->Object)
						pFactory = pPlayer->GetPrimaryFactory(AbstractType::AircraftType, false, BuildCat::DontCare);
				}

				if(pFactory) {

					const int idxFrame = (int)(((double)pFactory->GetProgress() / 54) * (pSHP->Frames - 1)) ;
					Point2D vPos = { XBase + i * XOffset, YBase };
					RectangleStruct sidebarRect = DSurface::Sidebar()->Get_Rect();

					if (idxFrame != -1)
					{
						DSurface::Sidebar()->DrawSHP(FileSystem::SIDEBAR_PAL, pSHP, idxFrame, &vPos,
							&sidebarRect, BlitterFlags::bf_400, 0, 0, ZGradient::Ground, 1000, 0, 0, 0, 0, 0);
					}
				}
			}
		}
	}
}

// =============================
// load / save

template <typename T>
void SidebarExt::ExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->Initialized)
		;
}

// =============================
// container hooks

DEFINE_HOOK(0x6A4F0B, SidebarClass_CTOR, 0x5)
{
	GET(SidebarClass*, pItem, EAX);

	SidebarExt::Allocate(pItem);

	return 0;
}

DEFINE_HOOK(0x6AC82F, SidebarClass_DTOR, 0x5)
{
	GET(SidebarClass*, pItem, EBX);

	SidebarExt::Remove(pItem);
	return 0;
}

DEFINE_HOOK_AGAIN(0x6AC5D0, SidebarClass_SaveLoad_Prefix, 0x5)
DEFINE_HOOK(0x6AC5E0, SidebarClass_SaveLoad_Prefix, 0x5)
{
	GET_STACK(IStream*, pStm, 0x4);

	SidebarExt::g_pStm = pStm;

	return 0;
}

DEFINE_HOOK(0x6AC5DA, SidebarClass_Load_Suffix, 0x6)
{
	auto buffer = SidebarExt::Global();

	PhobosByteStream Stm(0);
	if (Stm.ReadBlockFromStream(SidebarExt::g_pStm))
	{
		PhobosStreamReader Reader(Stm);

		if (Reader.Expect(SidebarExt::ExtData::Canary) && Reader.RegisterChange(buffer))
			buffer->LoadFromStream(Reader);
	}

	return 0;
}

DEFINE_HOOK(0x6AC5EA, SidebarClass_Save_Suffix, 0x6)
{
	auto buffer = SidebarExt::Global();
	PhobosByteStream saver(sizeof(*buffer));
	PhobosStreamWriter writer(saver);

	writer.Expect(SidebarExt::ExtData::Canary);
	writer.RegisterChange(buffer);

	buffer->SaveToStream(writer);
	saver.WriteBlockToStream(SidebarExt::g_pStm);

	return 0;
}
