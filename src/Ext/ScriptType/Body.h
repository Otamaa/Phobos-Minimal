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

	class ExtData final : public TExtension<ScriptTypeClass>
	{
	public:

		ValueableVector<ScriptActionNode> PhobosNode;
		ExtData(ScriptTypeClass* OwnerObject) : TExtension<ScriptTypeClass>(OwnerObject)
			, PhobosNode { }
		{ }

		virtual ~ExtData() = default;
		// void InvalidatePointer(void* ptr, bool bRemoved) {}
		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;
		void LoadFromINIFile(CCINIClass* pINI);
		void InitializeConstants() {
			PhobosNode.reserve(ScriptTypeClass::MaxActions);
		}

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public TExtensionContainer<ScriptTypeExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
		void InvalidatePointer(void* ptr, bool bRemoved);
	};

	static ExtContainer ExtMap;

	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);

};