#pragma once
#include <TeamTypeClass.h>

//#include <Helpers/Macro.h>
//#include <Utilities/Container.h>
//#include <Utilities/TemplateDef.h>
//#include <Utilities/Macro.h>

class TeamTypeExt
{
public:
	/*
	class ExtData final : public Extension<TeamTypeClass>
	{
	public:
		static constexpr size_t Canary = 0xBEE79008;
		using base_type = TeamTypeClass;

	public:

		Nullable<int> AI_SafeDIstance { };
		Nullable<int> AI_FriendlyDistance { };
		Nullable<bool> AttackWaypoint_AllowCell { };

		ExtData(TeamTypeClass* OwnerObject) : Extension<TeamTypeClass>(OwnerObject)
		{ }

		virtual ~ExtData() override = default;
		void LoadFromINIFile(CCINIClass* pINI, bool parseFailAddr);
		void LoadFromStream(PhobosStreamReader& Stm) { this->Serialize(Stm); }
		void SaveToStream(PhobosStreamWriter& Stm) { this->Serialize(Stm); }

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<TeamTypeExt::ExtData>
	{
	public:
		CONSTEXPR_NOCOPY_CLASS(TeamTypeExt::ExtData, "TeamTypeClass");
	};

	static ExtContainer ExtMap;
	*/
};