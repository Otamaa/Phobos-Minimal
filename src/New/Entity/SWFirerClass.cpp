#include "SWFirerClass.h"

#include <SuperClass.h>

#include <Phobos.SaveGame.h>
#include <Utilities/SavegameDef.h>

SWFirerManagerClass SWFirerManagerClass::Instance;

SWFirerClass::SWFirerClass(SuperClass* SW, int deferment, CellStruct cell, bool playerControl, int oldstart, int oldleft) :
	SW { SW },
	deferment {},
	cell { cell },
	playerControl { playerControl },
	oldstart { oldstart },
	oldleft { MaxImpl(oldleft - deferment, 0) }
{
	this->SW->Reset();
	this->deferment.Start(deferment);
}

void SWFirerManagerClass::Update()
{
	Array.remove_all_if([](auto& item) {
		if (item.deferment.Completed()) {
			item.SW->SetReadiness(true);
			item.SW->Launch(item.cell, item.playerControl);
			item.SW->Reset();
			item.SW->RechargeTimer.StartTime = item.oldstart;
			item.SW->RechargeTimer.TimeLeft = item.oldleft;
			return true;
		}

		return false;
	});
}

void SWFirerManagerClass::Clear()
{
	Array.clear();
}

bool SWFirerManagerClass::LoadGlobal(const json& root)
{
	this->Clear();

	if (root.contains("SWFirerManagerClass")) {
		size_t dataSize = root["collectionsize"].get<size_t>();
		Debug::Log("[ExtSave] Loading SWFirerManagerClass %d\n", dataSize);

		if (dataSize > 0) {
			Array.reserve(dataSize);

			for (auto& _item : root["collection"]) {
				auto& newPtr = Array.emplace_back();
				size_t itemdatasize = _item["itemdatasize"].get<size_t>();
				std::string encoded = _item["itemdata"].get<std::string>();

				PhobosByteStream loader(itemdatasize);
				loader.data = std::move(Base64Handler::decodeBase64(encoded, itemdatasize));
				PhobosStreamReader reader(loader);

				if (!newPtr.Load(reader, false) || !reader.ExpectEndOfBlock())
					return false;
			}
		}

		return true;
	}

	return false;
}

bool SWFirerManagerClass::SaveGlobal(json& root)
{
	Debug::Log("[ExtSave] Saving SWFirerManagerClass: %d\n", Array.size());

	json entry_first;

	entry_first["collectionsize"] = Array.size();

	if (!Array.empty()) {

		entry_first["collection"] = json::array();
		auto& _arrayOfData = entry_first["collection"];

		for (auto& _item : Array)
		{
			json entry_sec;
			PhobosByteStream _saver(0);
			PhobosStreamWriter writer(_saver);

			if (!_item.Save(writer))
				return false;

			entry_sec["itemdatasize"] = _saver.data.size();
			entry_sec["itemdata"] = Base64Handler::encodeBase64(_saver.data);

			_arrayOfData.push_back(std::move(entry_sec));
		}
	}

	root["SWFirerManagerClass"] = std::move(entry_first);
	return true;
}