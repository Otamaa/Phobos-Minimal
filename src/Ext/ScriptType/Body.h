#pragma once

#include <ScriptTypeClass.h>

#include <Utilities/Container.h>
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
#ifdef ENABLE_NEWHOOKS
	static constexpr size_t ExtOffset = sizeof(base_type);
#endif

	class ExtData final : public Extension<ScriptTypeClass>
	{
	public:

		ValueableVector<ScriptActionNode> PhobosNode;
		ExtData(ScriptTypeClass* OwnerObject) : Extension<ScriptTypeClass>(OwnerObject)
			, PhobosNode { }
		{ }

		virtual ~ExtData() = default;
		// void InvalidatePointer(void* ptr, bool bRemoved) {}
		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;
		void LoadFromINIFile(CCINIClass* pINI);
		void InitializeConstants() {
			PhobosNode.reserve(50);
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
		void InvalidatePointer(void* ptr, bool bRemoved);
	};

	static ExtContainer ExtMap;

	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);
};