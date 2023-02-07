#pragma once

#include <ScriptTypeClass.h>

#include <Ext/Abstract/Body.h>

#include <Utilities/Constructs.h>
#include <Utilities/Template.h>
#include <Utilities/Debug.h>
#include <Helpers/Macro.h>
#include <Utilities/TemplateDef.h>

class ScriptTypeExt
{
public:
	static constexpr size_t Canary = 0x414B4B41;
	using base_type = ScriptTypeClass;

	class ExtData final : public Extension<ScriptTypeClass>
	{
	public:

		ValueableVector<ScriptActionNode> PhobosNode;
		ExtData(ScriptTypeClass* OwnerObject) : Extension<ScriptTypeClass>(OwnerObject)
			, PhobosNode {}
		{ }

		virtual ~ExtData() override = default;
		virtual bool InvalidateIgnorable(void* const ptr) const override { return true; }
		virtual void InvalidatePointer(void* ptr, bool bRemoved) override {}
		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;
		virtual void LoadFromINIFile(CCINIClass* pINI) override;
		virtual void InitializeConstants() override {
			PhobosNode.reserve(ScriptTypeClass::MaxActions);
		}

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<ScriptTypeExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;

	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);

};