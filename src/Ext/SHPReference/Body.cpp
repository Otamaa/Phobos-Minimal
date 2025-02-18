#include "Body.h"

/*
void SHPRefExt::ExtData::Initialize()
{
	//TODO : WTF ?
	//auto pThis = this->Get();
	//std::string nFileName = pThis->Filename;
	//nFileName.erase(nFileName.begin() + nFileName.find(".SHP"));
	//nFileName += ".APH";

	//if (!this->Alpha) {
	//	if (auto pAlpha = GameCreate<SHPReference>(nFileName.c_str()))
	//	{
	//		pAlpha->Load();

	//		if (!pAlpha->Loaded)
	//		{
	//			GameDelete<true>(pAlpha);
	//			return;
	//		}

	//		if (pAlpha->Data->Width != pThis->Data->Width || pAlpha->Data->Height != pThis->Data->Height || pAlpha->Data->Frames != pThis->Data->Frames)
	//		{
	//			Debug::LogInfo("Mismatched alpha file. %s.", nFileName.c_str());
	//			GameDelete<true>(pAlpha);
	//			return;
	//		}

	//		this->Alpha = pAlpha;

	//		if (this->Data.X != this->Data.Y && this->Data.Width != this->Data.Height)
	//		{
	//			Debug::LogInfo("File %s alpha has been loaded successfully.", pThis->Filename);
	//		}
	//	}
	//} else {
	//	if (pThis) {
	//		if (!pThis->Loaded) {
	//			pThis->Load();
	//		}

	//		if (this->Alpha->Loaded && pThis->Loaded) {
	//			Debug::LogInfo("File %s alpha has been loaded successfully.", pThis->Filename);
	//		}
	//	}
	//}

}
*/
// =============================
// container
//SHPRefExt::ExtContainer SHPRefExt::ExtMap;

//SHPRefExt::ExtContainer::ExtContainer() : Container("SHPReference") { }
//SHPRefExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

/*
DEFINE_HOOK(0x69E4F0, SHPReference_CTOR, 0x5)
{
	GET(SHPReference*, pItem, ESI);

	SHPRefExt::ExtMap.JustAllocate(pItem , pItem , "Trying To Allocate From nullptr !");
	return 0;
}

DEFINE_HOOK(0x69E509, SHPReference_DTOR, 0x5)
{
	GET(SHPReference*, pItem, ECX);

	SHPRefExt::ExtMap.Remove(pItem);
	return 0;
}*/