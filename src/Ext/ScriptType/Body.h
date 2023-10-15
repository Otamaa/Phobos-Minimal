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
	/*
	class ExtData final : public Extension<ScriptTypeClass>
	{
	public:
		static constexpr size_t Canary = 0x414B4B41;
		using base_type = ScriptTypeClass;

	public:

		ValueableVector<ScriptActionNode> PhobosNode {};
		ExtData(ScriptTypeClass* OwnerObject) : Extension<ScriptTypeClass>(OwnerObject) {
			PhobosNode.reserve(ScriptTypeClass::MaxActions);
		}

		virtual ~ExtData() override = default;
		void LoadFromStream(PhobosStreamReader& Stm) { this->Serialize(Stm); }
		void SaveToStream(PhobosStreamWriter& Stm) { this->Serialize(Stm); }

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<ScriptTypeExt::ExtData>
	{
	public:
		CONSTEXPR_NOCOPY_CLASS(ScriptTypeExt::ExtData, "ScriptTypeClass");
	};

	static ExtContainer ExtMap;
	*/
};