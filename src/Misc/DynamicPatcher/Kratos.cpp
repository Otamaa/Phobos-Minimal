#include "Kratos.h"

#include <Utilities/Debug.h>
#include "Helpers/Container.h"

#include <Misc/DynamicPatcher/Common/EventSystems/EventSystem.h>

#include <Misc/DynamicPatcher/Extension/AnimExt.h>
#include <Misc/DynamicPatcher/Extension/AnimTypeExt.h>
#include <Misc/DynamicPatcher/Extension/BulletExt.h>
#include <Misc/DynamicPatcher/Extension/BulletTypeExt.h>
#include <Misc/DynamicPatcher/Extension/SuperWeaponExt.h>
#include <Misc/DynamicPatcher/Extension/SuperWeaponTypeExt.h>
#include <Misc/DynamicPatcher/Extension/TechnoExt.h>
#include <Misc/DynamicPatcher/Extension/TechnoTypeExt.h>
#include <Misc/DynamicPatcher/Extension/TerrainTypeExt.h>
#include <Misc/DynamicPatcher/Extension/VoxelAnimExt.h>
#include <Misc/DynamicPatcher/Extension/VoxelAnimTypeExt.h>
#include <Misc/DynamicPatcher/Extension/WarheadTypeExt.h>
#include <Misc/DynamicPatcher/Extension/WeaponTypeExt.h>

class GeneraHook
{
public:
	GeneraHook()
	{
		//EventSystems::General.AddHandler(Events::ScenarioStartEvent, FireSuperManager::Clear);
		//EventSystems::General.AddHandler(Events::ScenarioStartEvent, PrintTextManager::Clear);
		EventSystems::General.AddHandler(Events::ScenarioClearClassesEvent, ExtTypeRegistryClear);
		//EventSystems::Logic.AddHandler(Events::LogicUpdateEvent, FireSuperManager::Update);
	}
};

static GeneraHook _GeneraHook;

class GScreenHook
{
public:
	GScreenHook()
	{
		//EventSystems::Render.AddHandler(Events::GScreenRenderEvent, PrintTextManager::PrintRollingText);
	}
};

static GScreenHook _gScreenHook;

class SaveGameHook
{
public:
	SaveGameHook()
	{
		//EventSystems::SaveLoad.AddHandler(Events::SaveGameEvent, FireSuperManager::SaveSuperQueue);
		//EventSystems::SaveLoad.AddHandler(Events::LoadGameEvent, FireSuperManager::LoadSuperQueue);
		//EventSystems::SaveLoad.AddHandler(Events::LoadGameEvent, TechnoExt::ClearAllArray);
	}
};

static SaveGameHook _saveGameHook;

bool Kratos::IsScenarioClear = false;

void Kratos::Initialize()
{
	Debug::Log("Initialized version: " KRATOS_VERSION_SHORT_STR "\n");
}

#pragma region Implementation details

#pragma region Concepts

// a hack to check if some type can be used as a specialization of a template
template <template <class...> class Template, class... Args>
void DerivedFromSpecialization(const Template<Args...>&);

template <class T, template <class...> class Template>
concept DerivedFromSpecializationOf =
	requires(const T & t) { DerivedFromSpecialization<Template>(t); };

template<typename TExt>
concept HasExtMap = requires { { TExt::ExtMap } -> DerivedFromSpecializationOf<ExtMapCointainer>; };

template <typename T>
concept Clearable = requires { T::Clear(); };

template <typename T>
concept DetachSubscribable =
	requires (void* ptr, bool all) { T::ObjectWantDetach(ptr, all); };

template <typename T>
concept GlobalSaveLoadable = requires
{
	T::LoadGlobals(std::declval<PhobosStreamReader&>());
	T::SaveGlobals(std::declval<PhobosStreamWriter&>());
};

template <typename TAction, typename TProcessed, typename... ArgTypes>
concept DispatchesAction =
	requires (ArgTypes... args) { TAction::template Process<TProcessed>(args...); };

#pragma endregion

// calls:
// T::Clear()
// T::ExtMap.Clear()
struct ClearAction
{
	template <typename T>
	static bool Process()
	{
		if constexpr (Clearable<T>)
			T::Clear();
		else if constexpr (HasExtMap<T>)
			T::ExtMap.Clear();

		return true;
	}
};

// calls:
// T::PointerGotInvalid(void*, bool)
// T::ExtMap.PointerGotInvalid(void*, bool)
struct InvalidatePointerAction
{
	template <typename T>
	static bool Process(void* ptr)
	{
		if constexpr (PointerInvalidationSubscribable<T>)
			T::PointerGotInvalid(ptr);
		else if constexpr (HasExtMap<T>)
			T::ExtMap.PointerGotInvalid(ptr);

		return true;
	}
};

// calls:
// T::Detach(void*, bool)
// T::ExtMap.Detach(void*, bool)
struct DetachAllAction
{
	template <typename T>
	static bool Process(void* ptr, bool all)
	{
		if constexpr (DetachSubscribable<T>)
			T::ObjectWantDetach(ptr, all);
		else if constexpr (HasExtMap<T>)
			T::ExtMap.ObjectWantDetach(ptr, all);

		return true;
	}
};

// calls:
// T::LoadGlobals(PhobosStreamReader&)
struct LoadGlobalsAction
{
	template <typename T>
	static bool Process(IStream* pStm)
	{
		if constexpr (GlobalSaveLoadable<T>)
		{
			ExByteStream stm(0);
			stm.ReadBlockFromStream(pStm);
			PhobosStreamReader reader(stm);

			return T::LoadGlobals(reader) && reader.ExpectEndOfBlock();
		}
		else
		{
			return true;
		}
	}
};

// calls:
// T::SaveGlobals(PhobosStreamWriter&)
struct SaveGlobalsAction
{
	template <typename T>
	static bool Process(IStream* pStm)
	{
		if constexpr (GlobalSaveLoadable<T>)
		{
			ExByteStream stm;
			PhobosStreamWriter writer(stm);

			return T::SaveGlobals(writer) && stm.WriteBlockToStream(pStm);
		}
		else
		{
			return true;
		}
	}
};

// this is a complicated thing that calls methods on classes. add types to the
// instantiation of this type, and the most appropriate method for each type
// will be called with no overhead of virtual functions.
template <typename... RegisteredTypes>
struct TypeRegistry
{
	__forceinline static void Clear()
	{
		dispatch_mass_action<ClearAction>();
	}

	__forceinline static void DetachAll(void* ptr, bool all)
	{
		dispatch_mass_action<DetachAllAction>(ptr, all);
	}

	__forceinline static bool LoadGlobals(IStream* pStm)
	{
		return dispatch_mass_action<LoadGlobalsAction>(pStm);
	}

	__forceinline static bool SaveGlobals(IStream* pStm)
	{
		return dispatch_mass_action<SaveGlobalsAction>(pStm);
	}

private:
	// TAction: the method dispatcher class to call with each type
	// ArgTypes: the argument types to call the method dispatcher's Process() method
	template <typename TAction, typename... ArgTypes>
		requires (DispatchesAction<TAction, RegisteredTypes, ArgTypes...> && ...)
	__forceinline static bool dispatch_mass_action(ArgTypes... args)
	{
		// (pack expression op ...) is a fold expression which
		// unfolds the parameter pack into a full expression
		return (TAction::template Process<RegisteredTypes>(args...) && ...);
	}
};

#pragma endregion

// Add more class names as you like
using ExtTypeRegistry = TypeRegistry<

	// Ext classes
	AnimTypeExt,
	AnimExt,
	BulletExt,
	BulletTypeExt,
	TechnoExt,
	TechnoTypeExt,
	TerrainTypeExt,
	VoxelAnimExt,
	VoxelAnimTypeExt,
	WarheadTypeExt,
	WeaponTypeExt
	// New classes
>;

void ExtTypeRegistryClear(EventSystem* sender, Event e, void* args)
{
	ExtTypeRegistry::Clear();
}

void DetachAll(EventSystem* sender, Event e, void* args)
{
	auto const& argsArray = reinterpret_cast<void**>(args);
	AbstractClass* pItem = (AbstractClass*)argsArray[0];
	bool all = argsArray[1];
	ExtTypeRegistry::DetachAll(pItem, all);
}

void Kratos::LoadGameGlobal(IStream* pStm)
{
	ExtTypeRegistry::LoadGlobals(pStm);
}

void Kratos::SaveGameGlobal(IStream* pStm)
{
	ExtTypeRegistry::SaveGlobals(pStm);
}

void Kratos::ClearScenarioStart()
{
	Kratos::IsScenarioClear = true;
}

void Kratos::ClearScenarioEnd()
{
	EventSystems::General.Broadcast(Events::ScenarioClearClassesEvent);
	Kratos::IsScenarioClear = false;
}

void Kratos::ScenarioStart()
{
	EventSystems::General.Broadcast(Events::ScenarioStartEvent);
}

DEFINE_HOOK(0x55AFB3, LogicClass_Update, 0x6)
{
	EventSystems::Logic.Broadcast(Events::LogicUpdateEvent);
	return 0;
}

DEFINE_HOOK(0x55B719, LogicClass_Update_Late, 0x5)
{
	EventSystems::Logic.Broadcast(Events::LogicUpdateEvent, EventArgsLate);
	return 0;
}

void Kratos::Render()
{
	EventSystems::Render.Broadcast(Events::GScreenRenderEvent);
}
void Kratos::RenderLate()
{
	EventSystems::Render.Broadcast(Events::GScreenRenderEvent, EventArgsLate);
}

#pragma region SaveGame

void Kratos::SaveGameStart(const char* name)
{
	SaveGameEventArgs args { name, true };
	EventSystems::SaveLoad.Broadcast(Events::SaveGameEvent, &args);
}

void Kratos::SaveGameEnd(const char* name)
{
	SaveGameEventArgs args { name, false };
	EventSystems::SaveLoad.Broadcast(Events::SaveGameEvent, &args);
}

void Kratos::SaveGameInStream(IStream* stream)
{
	SaveGameEventArgs args { stream, true };
	EventSystems::SaveLoad.Broadcast(Events::SaveGameEvent, &args);
}
void Kratos::SaveGameInStreamEnd(IStream* stream)
{
	SaveGameEventArgs args { stream, false };
	EventSystems::SaveLoad.Broadcast(Events::SaveGameEvent, &args);

}

#pragma endregion

#pragma region LoadGame

void Kratos::LoadGameStart(const char* name)
{
	LoadGameEventArgs args { name, true };
	EventSystems::SaveLoad.Broadcast(Events::LoadGameEvent, &args);
}

void Kratos::LoadGameEnd(const char* name)
{
	LoadGameEventArgs args { name, false };
	EventSystems::SaveLoad.Broadcast(Events::LoadGameEvent, &args);
}


void Kratos::LoadGameInStream(IStream* stream)
{
	LoadGameEventArgs args { stream, true };
	EventSystems::SaveLoad.Broadcast(Events::LoadGameEvent, &args);
}

void Kratos::LoadGameInStreamEnd(IStream* stream)
{
	LoadGameEventArgs args { stream, false };
	EventSystems::SaveLoad.Broadcast(Events::LoadGameEvent, &args);
}
#pragma endregion

DEFINE_HOOK(0x4101F0, AbstractClass_DTOR, 0x6)
{
	GET(ObjectClass*, pObject, ECX);
	EventSystems::General.Broadcast(Events::PointerExpireEvent, pObject);
	return 0;
}

DEFINE_HOOK(0x5F65F1, ObjectClass_UnInit, 0x5)
{
	GET(ObjectClass*, pObject, ECX);
	// Debug::Log("ObjectClass [%s]%d ready to delete.\n", pObject->GetType()->ID, pObject);
	EventSystems::General.Broadcast(Events::ObjectUnInitEvent, pObject);
	return 0;
}

// this function is a Object want Detach_All when Limbo or Delete
DEFINE_HOOK(0x7258D0, DetachThisFromAll, 0x6)
{
	GET(AbstractClass* const, pInvalid, ECX);
	GET(bool const, removed, EDX);
	void* args[] = { pInvalid, (void*)removed };

	// 该广播会执行三次
	// (01) DetachAll
	// (01) Limbo
	// (01) DetachAll
	// (02) DetachAll
	// (02) Delete
	EventSystems::General.Broadcast(Events::DetachAll, &args);

	return 0;
}