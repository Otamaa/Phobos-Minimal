#include "Kratos.h"

#include <Utilities/Stream.h>

#include <AbstractClass.h>

#include <Misc/Kratos/Extension/AnimExt.h>
#include <Misc/Kratos/Extension/AnimTypeExt.h>
#include <Misc/Kratos/Extension/BulletExt.h>
#include <Misc/Kratos/Extension/BulletTypeExt.h>
#include <Misc/Kratos/Extension/SuperWeaponExt.h>
#include <Misc/Kratos/Extension/SuperWeaponTypeExt.h>
#include <Misc/Kratos/Extension/TechnoExt.h>
#include <Misc/Kratos/Extension/TechnoTypeExt.h>
#include <Misc/Kratos/Extension/WarheadTypeExt.h>
#include <Misc/Kratos/Extension/WeaponTypeExt.h>

#pragma region Implementation details

#pragma region Concepts

// a hack to check if some type can be used as a specialization of a template
template <template <class...> class Template, class... Args>
void DerivedFromSpecialization(const Template<Args...>&);

template <class T, template <class...> class Template>
concept DerivedFromSpecializationOf =
	requires(const T & t) { DerivedFromSpecialization<Template>(t); };

template<typename TExt>
concept HasExtMap = requires { { TExt::ExtMap } -> DerivedFromSpecializationOf<KratosContainer>; };

template <typename T>
concept Clearable = requires { T::Clear(); };

template <typename T>
concept PointerInvalidationSubscribable =
	requires (void* ptr) { T::PointerGotInvalid(ptr); };

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
			PhobosByteStream stm(0);
			stm.ReadFromStream(pStm);
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
			PhobosByteStream stm;
			PhobosStreamWriter writer(stm);

			return T::SaveGlobals(writer) && stm.WriteToStream(pStm);
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

	__forceinline static void InvalidatePointer(void* ptr)
	{
		dispatch_mass_action<InvalidatePointerAction>(ptr);
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
	WarheadTypeExt,
	WeaponTypeExt
	// New classes
>;

void ExtTypeRegistryClear(EventSystem* sender, Event e, void* args)
{
	ExtTypeRegistry::Clear();
}
void InvalidatePointer(EventSystem* sender, Event e, void* args)
{
	ExtTypeRegistry::InvalidatePointer(args);
}

void DetachAll(EventSystem* sender, Event e, void* args)
{
	auto const& argsArray = reinterpret_cast<void**>(args);
	AbstractClass* pItem = (AbstractClass*)argsArray[0];
	bool all = argsArray[1];
	ExtTypeRegistry::DetachAll(pItem, all);
}

void Kratos::ExeRun(EventSystem* sender, Event e, void* args)
{
	EventSystems::General.Broadcast(Events::ExeRun);
}

#include <Misc/Kratos/Ext/Helper/MathEx.h>

void Kratos::LogicUpdate_Early() {
	EventSystems::Logic.Broadcast(Events::LogicUpdateEvent);
}

void Kratos::LogicUpdate_End() {
	EventSystems::Logic.Broadcast(Events::LogicUpdateEvent, EventArgsLate);
}

void Kratos::EnsureSeeded(unsigned long seed) {
	Random::SetRandomSeed(seed);

	EventSystems::General.Broadcast(Events::ScenarioStartEvent);
}

void Kratos::ScenarioClearFlagSet() { KratosCommon::IsScenarioClear = true; }

void Kratos::ScenarioClearFlagUnset() {
	EventSystems::General.Broadcast(Events::ScenarioClearClassesEvent);
	KratosCommon::IsScenarioClear = false;
}

void Kratos::DetachFromAll(AbstractClass* const pInvalid, bool const removed) {
	void* args[] = { pInvalid, (void*)removed };

	// 该广播会执行三次
	// (01) DetachAll
	// (01) Limbo
	// (01) DetachAll
	// (02) DetachAll
	// (02) Delete
	EventSystems::General.Broadcast(Events::DetachAll, &args);

}

void Kratos::GScreenRender_Early() {
	EventSystems::Render.Broadcast(Events::GScreenRenderEvent);
}

void Kratos::GSCreenRender_End() {
	EventSystems::Render.Broadcast(Events::GScreenRenderEvent, EventArgsLate);
}
