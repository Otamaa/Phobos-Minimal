#pragma once
#include <TeamTypeClass.h>

#include <Helpers/Macro.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>
#include <Utilities/Macro.h>

class TeamTypeExt
{
public:
	static constexpr size_t Canary = 0xBEE79008;
	using base_type = TeamTypeClass;
#ifdef ENABLE_NEWHOOKS
	//static constexpr size_t ExtOffset = sizeof(base_type);
#endif

	class ExtData final : public Extension<TeamTypeClass>
	{
	public:

		Nullable<int> AI_SafeDIstance;
		Nullable<int> AI_FriendlyDistance;
		Valueable<bool> AttackWaypoint_AllowCell;

		ExtData(TeamTypeClass* OwnerObject) : Extension<TeamTypeClass>(OwnerObject)
			, AI_SafeDIstance { }
			, AI_FriendlyDistance { }
			, AttackWaypoint_AllowCell { true }
		{ }

		virtual ~ExtData() = default;
		virtual void LoadFromINIFile(CCINIClass* pINI) override;
		virtual void InvalidatePointer(void* ptr, bool bRemoved) override { }
		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;
		virtual size_t Size() const { return sizeof(*this); }

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<TeamTypeExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;

	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);
};