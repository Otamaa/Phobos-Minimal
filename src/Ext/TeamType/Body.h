#pragma once
#include <TeamTypeClass.h>

#include <Helpers/Macro.h>
#include <Ext/Abstract/Body.h>
#include <Utilities/TemplateDef.h>
#include <Utilities/Macro.h>

class TeamTypeExt
{
public:

	static constexpr size_t Canary = 0xBEE79008;
	using base_type = TeamTypeClass;

	class ExtData final : public TExtension<TeamTypeClass>
	{
	public:

		Nullable<int> AI_SafeDIstance;
		Nullable<int> AI_FriendlyDistance;
		Nullable<bool> AttackWaypoint_AllowCell;

		ExtData(TeamTypeClass* OwnerObject) : TExtension<TeamTypeClass>(OwnerObject)
			, AI_SafeDIstance { }
			, AI_FriendlyDistance { }
			, AttackWaypoint_AllowCell { }
		{ }

		virtual ~ExtData() = default;
		void LoadFromINIFile(CCINIClass* pINI);
		// void InvalidatePointer(void* ptr, bool bRemoved) { }
		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public TExtensionContainer<TeamTypeExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;

	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);

};