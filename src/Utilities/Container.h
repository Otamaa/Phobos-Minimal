#pragma once

#include <CCINIClass.h>
#include <Memory.h>

#include "Debug.h"
#include "SavegameDef.h"
#include "Swizzle.h"

#include <Utilities/ClassInterfaces.h>
#include <Utilities/Concepts.h>

#include <Base/Always.h>

class AbstractClass;
static COMPILETIMEEVAL size_t AbstractExtOffset = 0x18;

template <typename T>
concept CanLoadFromStream =
	requires (PhobosStreamReader & stm) { T::LoadFromStream(stm); };

template <typename T>
concept CanSaveToStream =
	requires (PhobosStreamWriter & stm) { T::SaveToStream(stm); };

template <typename T>
concept CanThisLoadFromStream =
	requires (T t, PhobosStreamReader & stm) { t.LoadFromStream(stm); };

template <typename T>
concept CanThisSaveToStream =
	requires (T t, PhobosStreamWriter & stm) { t.SaveToStream(stm); };

template <typename T>
concept CanThisPtrLoadFromStream =
	requires (T t, PhobosStreamReader & stm) { t->LoadFromStream(stm); };

template <typename T>
concept CanThisPtrSaveToStream =
	requires (T t, PhobosStreamWriter & stm) { t->SaveToStream(stm); };

template<typename T>
struct ContainerExtHandler
{
	static COMPILETIMEEVAL FORCEDINLINE void ClearExtAttribute(T::base_type* key) {
		(*(uintptr_t*)((char*)key + AbstractExtOffset)) = 0;
	}

	static COMPILETIMEEVAL FORCEDINLINE void SetExtAttribute(T::base_type* key, T* val) {
		(*(uintptr_t*)((char*)key + AbstractExtOffset)) = (uintptr_t)val;
	}

	static COMPILETIMEEVAL FORCEDINLINE T* GetExtAttribute(T::base_type* key) {
		return (T*)(*(uintptr_t*)((char*)key + AbstractExtOffset));
	}

	static COMPILETIMEEVAL FORCEDINLINE T* Find(T::base_type* key) {
		return ContainerExtHandler<T>::GetExtAttribute(key);
	}

	static COMPILETIMEEVAL FORCEDINLINE T* TryFind(T::base_type* key) {
		if (!key)
			return nullptr;

		return ContainerExtHandler<T>::GetExtAttribute(key);
	}

	static void RemoveExtOf(T::base_type* key, T* Item) {
		delete Item;
		ContainerExtHandler<T>::ClearExtAttribute(key);
	}
};

template<typename T>
struct UuidFirstPart {
	static constexpr unsigned int value = __uuidof(T).Data1;
};

template <unsigned int V>
static COMPILETIMEEVAL auto to_hex_string() {
	COMPILETIMEEVAL char lut[] = "0123456789ABCDEF";
	std::array<char, 11> out {}; // "0x" + 8 hex + '\0'

	out[0] = '0';
	out[1] = 'x';

	for (int i = 0; i < 8; ++i)
	{
		out[9 - i] = lut[(V >> (i * 4)) & 0xF];
	}
	out[10] = '\0';
	return out;
}

struct AbstractExtended {
public:

	AbstractClass* AttachedToObject;
	AbstractType AbsType;
	InitState Initialized;

	//normal assigned AO
	AbstractExtended(AbstractClass* abs);

	//with noint_t less instasiation
	AbstractExtended(AbstractClass* abs, noinit_t);

	virtual ~AbstractExtended() = default;

	void Internal_LoadFromStream(PhobosStreamReader& Stm);
	void Internal_SaveToStream(PhobosStreamWriter& Stm) const;

	FORCEDINLINE InitState GetInitState() const { return Initialized; }
	FORCEDINLINE void SetInitState(InitState state) { Initialized = state; }
	FORCEDINLINE void SetAttached(AbstractClass* abs) { AttachedToObject = abs; }
public:

	AbstractClass* This() const { return const_cast<AbstractClass*>(AttachedToObject); }
	const AbstractClass* This_Const() const { return AttachedToObject; }

	virtual void InvalidatePointer(AbstractClass* ptr, bool bRemoved, AbstractType  type) = 0;
	virtual void LoadFromStream(PhobosStreamReader& Stm) = 0;
	virtual void SaveToStream(PhobosStreamWriter& Stm) = 0;
	virtual void CalculateCRC(CRCEngine& crc) const = 0;

private:
	AbstractExtended(const AbstractExtended&) = delete;
	void operator = (const AbstractExtended&) = delete;
};

template <typename T>
class Container : public ContainerExtHandler<T> , public ContainerInterfaces
{
public:

	//the container is not handling the memory
	//the object has the extension handling the memory
	std::vector<T*> Array;
	using ExtT = T;
	using ExtTptr = T*;

public:

	Container() = default;
	virtual ~Container() = default;

	T* Allocate(T::base_type* key) {
		auto pExt = new T(key);
		ContainerExtHandler<T>::SetExtAttribute(key ,pExt);
		Array.emplace_back(pExt);
		return pExt;
	}

	T* AllocateNoInit(T::base_type* key) {
		ContainerExtHandler<T>::ClearExtAttribute(key);
		auto pExt = new T(key, noinit_t());
		ContainerExtHandler<T>::SetExtAttribute(key, pExt);
		Array.emplace_back(pExt);
		return pExt;
	}

	T* AllocateNoInit() {
		auto pExt = new T(nullptr, noinit_t());
		Array.emplace_back(pExt);
		return pExt;
	}

	T* FindOrAllocate(T::base_type* key) {
		if (T* const ptr = ContainerExtHandler<T>::TryFind(key))
			return ptr;

		return this->Allocate(key);
	}

	void Remove(T::base_type* key) {
		if (T* Item = ContainerExtHandler<T>::TryFind(key)) {

			auto iter = std::ranges::find(Array, Item);

			if(iter != Array.end())
				Array.erase(iter);

			ContainerExtHandler<T>::RemoveExtOf(key, Item);
		}
	}

	//void InvalidatePointerFor(T::base_type* key, AbstractClass* const ptr, bool bRemoved) {
	//	if (T* Extptr = ContainerExtHandler<T>TryFind(key)) {
	//			Extptr->InvalidatePointer(ptr, bRemoved);
	//	}
	//}

public :

	virtual void Clear() {
		for (auto& _item : Array) {
			if (_item) {
				delete _item;
			}
		}

		Array.clear();
	}

	virtual void ClearNullAttachedObj() final {
		if constexpr (std::is_base_of_v<AbstractExtended , ExtT>){
			Array.erase(
				std::remove_if(Array.begin(), Array.end(),
					[](T* p)
					{
						if (!p)
							return true;

						if (!p->AttachedToObject) {
							delete  p;
							return true;
						}

						return false;
					}),
				Array.end());
		}
	}

};

template<typename T , bool haNameItem>
struct ContainerSaveLoad {

	virtual bool LoadAll(PhobosStreamReader& stm)
	{
		int Count = 0;
		if (!stm.Load(Count))
			return false;

		for (int i = 0; i < Count; ++i) {
			uintptr_t savedPtr {};
			if (!stm.Load(savedPtr))
				return false;

			auto buffer = new T::ExtT(nullptr, noinit_t());
			Debug::Log("Loading %s [At (%d) - %x] to stream\n",
					T::ClassName, i, savedPtr);

			buffer->LoadFromStream(stm);
			if (!stm.Success())
				return false;

			//specifically for immedietely restore the extension pointers 
			ExtensionSwizzleManager::RegisterExtensionPointer((void*)savedPtr, buffer);
			//for others that need to be remapped using vanilla swizzle manager
			PhobosSwizzleManager.Here_I_Am(savedPtr, buffer);
			((T*)this)->Array.emplace_back(buffer);
		}

		return true;
	}

	virtual bool SaveAll(PhobosStreamWriter& stm)
	{
		const int Count = (int)((T*)this)->Array.size();
		if (!stm.Save(Count))
			return false;

		for (int i = 0; i < Count; ++i) {
			uintptr_t savedPtr = (uintptr_t)((T*)this)->Array[i];

			if constexpr (haNameItem) { 
				Debug::Log("Saving %s [%s (%d) - %x] to stream\n",
					T::ClassName, ((T*)this)->Array[i]->Name.c_str(), i, savedPtr);
			} else {
				Debug::Log("Saving %s [At Index %d - %x] to stream\n",
						T::ClassName, i, savedPtr);
			}

			if (!stm.Save(savedPtr))
				return false;

			((T*)this)->Array[i]->SaveToStream(stm);
			if (!stm.Success())
				return false;
		}

		return true;
	}
};