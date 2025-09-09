#pragma once

#include <CCINIClass.h>
#include <Memory.h>

#include "Debug.h"
#include "SavegameDef.h"
#include "Swizzle.h"

class AbstractClass;
static COMPILETIMEEVAL size_t AbstractExtOffset = 0x18;

template<typename T>
struct UuidFirstPart {
	static constexpr unsigned int value = __uuidof(T).Data1;
};


struct AbstractExtended {
private:
	AbstractClass* AttachedToObject;
	InitState Initialized;
	FixedString<0x24> Name;

public:

	//normal assigned AO
	AbstractExtended(AbstractClass* abs);

	//with noint_t less instasiation
	AbstractExtended(AbstractClass* abs, noinit_t);

	~AbstractExtended() = default;

	void Internal_LoadFromStream(PhobosStreamReader& Stm);
	void Internal_SaveToStream(PhobosStreamWriter& Stm) const;

	FORCEDINLINE InitState GetInitState() const { return Initialized; }
	FORCEDINLINE void SetInitState(InitState state) { Initialized = state; }
	FORCEDINLINE void SetAttached(AbstractClass* abs) { AttachedToObject = abs; }
	FORCEDINLINE void SetName(const char* name) { Name = name; }
	FORCEDINLINE const char* GetAttachedObjectName() const { return Name.data(); }

public:

	virtual AbstractType WhatIam() const { return AbstractType::Abstract; }
	virtual int GetSize() const { return sizeof(AbstractExtended); };

	virtual AbstractClass* This() const { return const_cast<AbstractClass*>(AttachedToObject); }
	virtual const AbstractClass* This_Const() const { return AttachedToObject; }

	virtual void InvalidatePointer(AbstractClass* ptr, bool bRemoved) = 0;
	virtual void LoadFromStream(PhobosStreamReader& Stm) = 0;
	virtual void SaveToStream(PhobosStreamWriter& Stm) = 0;

	virtual void CalculateCRC(CRCEngine& crc) const = 0;

private:
	AbstractExtended(const AbstractExtended&) = delete;
	void operator = (const AbstractExtended&) = delete;
};

template <class T>
concept HasAbsID = requires(T) { T::AbsID; };

template <class T>
concept HasMarker = requires(T) { T::Marker; };

template <class T>
concept HasTypeBase = requires(T) { T::AbsTypeBase; };

template <typename T>
concept Initable = requires(T t) { t.Initialize(); };

template <typename T>
concept CanLoadFromINIFile =
	requires (T t , CCINIClass* pINI, bool parseFailAddr) { t.LoadFromINI(pINI, parseFailAddr); };

template <typename T>
concept CanWriteToINIFile =
	requires (T t, CCINIClass * pINI) { t.WriteToINI(pINI); };


template <typename T>
concept CanLoadFromRulesFile =
	requires (T t, CCINIClass * pINI) { t.LoadFromRulesFile(pINI); };

template <typename T>
concept CTORInitable =
	requires (T t) { t.InitializeConstant(); };


template <typename T>
concept PointerInvalidationSubscribable =
	requires (AbstractClass* ptr, bool removed) { T::InvalidatePointer(ptr, removed); };

template <typename T>
concept ThisPointerInvalidationSubscribable =
	requires (T t, AbstractClass* ptr, bool removed) { t.InvalidatePointer(ptr, removed); };

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


template <typename T>
class Container
{
public:

	using base_type = typename T::base_type;
	using extension_type = typename T;
	using base_type_ptr = base_type*;
	using const_base_type_ptr = const base_type*;
	using extension_type_ptr = extension_type*;
	using extension_type_ref_ptr = extension_type**;
	using const_extension_type_ptr = const extension_type*;

	static std::vector<extension_type_ptr> Array;

	COMPILETIMEEVAL FORCEDINLINE void ClearExtAttribute(base_type_ptr key) {
		(*(uintptr_t*)((char*)key + AbstractExtOffset)) = 0;
	}

	COMPILETIMEEVAL FORCEDINLINE void SetExtAttribute(base_type_ptr key, extension_type_ptr val) {
		(*(uintptr_t*)((char*)key + AbstractExtOffset)) = (uintptr_t)val;
	}

	COMPILETIMEEVAL FORCEDINLINE extension_type_ptr GetExtAttribute(base_type_ptr key) {
		return (extension_type_ptr)(*(uintptr_t*)((char*)key + AbstractExtOffset));
	}

	COMPILETIMEEVAL FORCEDINLINE extension_type_ptr Find(base_type_ptr key) {
		return this->GetExtAttribute(key);
	}

	COMPILETIMEEVAL FORCEDINLINE extension_type_ptr TryFind(base_type_ptr key)
	{
		if (!key)
			return nullptr;

		return this->GetExtAttribute(key);
	}

public:

	extension_type_ptr Allocate(base_type_ptr key) {
		auto pExt = new extension_type(key);
		this->SetExtAttribute(key ,pExt);
		Array.emplace_back(pExt);
		return pExt;
	}

	extension_type_ptr AllocateNoInit(base_type_ptr key) {
		auto pExt = new extension_type(key, noinit_t());
		Array.emplace_back(pExt);
		return pExt;
	}

	extension_type_ptr FindOrAllocate(base_type_ptr key)
	{
		if (extension_type_ptr const ptr = TryFind(key))
			return ptr;

		return this->Allocate(key);
	}

	void Remove(base_type_ptr key)
	{
		if (extension_type_ptr Item = TryFind(key)) {
			Array.erase(std::find(Array.begin() , Array.end(), Item));
			delete Item;
			this->ClearExtAttribute(key);
		}
	}

	void RemoveExtOf(base_type_ptr key , extension_type_ptr Item) {
		delete Item;
		this->ClearExtAttribute(key);
	}

	void LoadFromINI(base_type_ptr key, CCINIClass* pINI, bool parseFailAddr)
	{
		if COMPILETIMEEVAL (CanLoadFromINIFile<T>)
		{
			if (extension_type_ptr ptr = this->Find(key))
			{
				if (!pINI) {
					//Debug::LogInfo("[%s] LoadFrom INI Called WithInvalid CCINIClass ptr ! ", typeid(T).name());
					return;
				}

				switch (ptr->GetInitState()) {
					case InitState::Blank:
					{
						ptr->SetInitState(InitState::Inited);

						if COMPILETIMEEVAL (CanLoadFromRulesFile<T>) {
							if (pINI == CCINIClass::INI_Rules) {
								ptr->LoadFromRulesFile(pINI);
							}
						}

						//Load from rules INI File
						ptr->LoadFromINI(pINI, parseFailAddr);
						ptr->SetInitState(InitState::Ruled);
					}
					break;
					case InitState::Ruled:
					case InitState::Constanted:
					{
						//load anywhere other than rules
						ptr->LoadFromINI(pINI, parseFailAddr);
						//this function can be called again multiple time but without need to re-init the data
						ptr->SetInitState(InitState::Ruled);
					}
					break;
					{
					default:
						break;
					}
				}
			}
		}
	}

	void WriteToINI(base_type_ptr key, CCINIClass* pINI)
	{
		if COMPILETIMEEVAL(CanWriteToINIFile<T>)
		{
			if (extension_type_ptr ptr = this->TryFind(key))
			{
				if (!pINI)
				{
					//Debug::LogInfo("[%s] LoadFrom INI Called WithInvalid CCINIClass ptr ! ", typeid(T).name());
					return;
				}

				ptr->WriteToINI(pINI);
			}
		}
	}

	void InvalidatePointerFor(base_type_ptr key, AbstractClass* const ptr, bool bRemoved)
	{
		if (extension_type_ptr Extptr = this->TryFind(key))
				Extptr->InvalidatePointer(ptr, bRemoved);
	}

public : //default Save/Load functions
		//define your own with same name to override these

	static void Clear();

	static bool SaveGlobalArrayData(PhobosStreamWriter& Stm)
	{
		//save it as int instead of size_t
		const int Count = (int)Array.size();
		Stm.Save(Count);

		for (int i = 0; i < Count; ++i) {
			Stm.Save((long)Array[i]); // important !
			Array[i]->SaveToStream(Stm); // call the internal Ext save load function
		}

		return true;
	}

	static bool LoadGlobalArrayData(PhobosStreamReader& Stm)
	{
		Clear(); //clear the global data

		int Count = 0;

		if (Stm.Load(Count)) {
			if (Count > 0) {

				//reserve !
				Array.reserve(Count);

				const auto name = PhobosCRT::GetTypeIDName<T>();

				for (int i = 0; i < Count; ++i) {

					long oldPtr = 0l;

					if (!Stm.Load(oldPtr))
						return false;

					auto newPtr = new T(nullptr, noinit_t());

					PHOBOS_SWIZZLE_REGISTER_POINTER(oldPtr, newPtr, name.c_str())
					ExtensionSwizzleManager::RegisterExtensionPointer((void*)oldPtr, newPtr);
					newPtr->LoadFromStream(Stm);
					Array.push_back(newPtr);
				}
			}

			return true;
		}

		return false;
	}
};

