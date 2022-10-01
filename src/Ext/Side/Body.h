#pragma once
#include <SideClass.h>

#include <Helpers/Macro.h>
#include <Ext/Abstract/Body.h>
#include <Utilities/TemplateDef.h>

class SideExt
{
public:
	static constexpr size_t Canary = 0x05B10501;
	using base_type = SideClass;
#ifdef ENABLE_NEWHOOKS
	static constexpr size_t ExtOffset = sizeof(base_type);
#endif

	class ExtData final : public TExtension<SideClass>
	{
	public:
		Valueable<int> ArrayIndex;
		Valueable<bool> Sidebar_GDIPositions;
		Valueable<int> IngameScore_WinTheme;
		Valueable<int> IngameScore_LoseTheme;
		Valueable<Point2D> Sidebar_HarvesterCounter_Offset;
		Valueable<ColorStruct> Sidebar_HarvesterCounter_Yellow;
		Valueable<ColorStruct> Sidebar_HarvesterCounter_Red;
		Valueable<Point2D> Sidebar_ProducingProgress_Offset;
		Valueable<Point2D> Sidebar_PowerDelta_Offset;
		Valueable<ColorStruct> Sidebar_PowerDelta_Green;
		Valueable<ColorStruct> Sidebar_PowerDelta_Yellow;
		Valueable<ColorStruct> Sidebar_PowerDelta_Red;
		Valueable<TextAlign> Sidebar_PowerDelta_Align;

		Nullable<ColorStruct> ToolTip_Background_Color;
		Nullable<int> ToolTip_Background_Opacity;
		Nullable<float> ToolTip_Background_BlurSize;

		ExtData(SideClass* OwnerObject) : TExtension<SideClass>(OwnerObject)
			, ArrayIndex { -1 }
			, Sidebar_GDIPositions { false }
			, IngameScore_WinTheme { -2 }
			, IngameScore_LoseTheme { -2 }
			, Sidebar_HarvesterCounter_Offset { { 0,0 } }
			, Sidebar_HarvesterCounter_Yellow { { 255,255,0 } }
			, Sidebar_HarvesterCounter_Red { { 255,0,0 } }
			, Sidebar_ProducingProgress_Offset { { 0,0 } }
			, Sidebar_PowerDelta_Offset { { 0,0 } }
			, Sidebar_PowerDelta_Green { { 0,255,0 } }
			, Sidebar_PowerDelta_Yellow { { 255,255,0 } }
			, Sidebar_PowerDelta_Red { { 255,0,0 } }
			, Sidebar_PowerDelta_Align { TextAlign::Left }

			, ToolTip_Background_Color { }
			, ToolTip_Background_Opacity { }
			, ToolTip_Background_BlurSize { }
		{ }

		virtual ~ExtData() = default;

		void LoadFromINIFile(CCINIClass* pINI);
		void Initialize();
		void InitializeConstants() { }
		// void InvalidatePointer(void* ptr, bool bRemoved) { }
		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public TExtensionContainer<SideExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
		void InvalidatePointer(void* ptr, bool bRemoved);

	};

	static ExtContainer ExtMap;
	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);

	static bool isNODSidebar();
};
