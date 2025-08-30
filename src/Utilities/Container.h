#pragma once

#include <CCINIClass.h>
#include <SwizzleManagerClass.h>
#include <Memory.h>

#include "Debug.h"
#include "Stream.h"
#include "Swizzle.h"

#include <string>
#include <ExtraHeaders/MemoryPool.h>

class AbstractClass;
static COMPILETIMEEVAL size_t AbstractExtOffset = 0x18;

struct AbstractExtended {

	AbstractClass* AttachedToObject;
	InitState Initialized;

public:

	//normal assigned AO
	AbstractExtended(AbstractClass* abs) : AttachedToObject(abs) , Initialized(InitState::Blank) { };

	//withnoint_t , with less instasiation
	AbstractExtended(AbstractClass* abs, noinit_t) : AttachedToObject(abs) {};

	~AbstractExtended() { };

	void Internal_LoadFromStream(PhobosStreamReader& Stm) {
		Stm.Process(Initialized);
	}

	void Internal_SaveToStream(PhobosStreamWriter& Stm) const {
		Stm.Process(Initialized);
	}

public:

	virtual AbstractType WhatIam() const { return AbstractType::Abstract; }
	virtual int GetSize() const { return sizeof(AbstractExtended); };

	virtual AbstractClass* This() const = 0;
	virtual const AbstractClass* This_Const() const = 0;

	virtual void InvalidatePointer(AbstractClass* ptr, bool bRemoved) = 0;
	virtual void LoadFromStream(PhobosStreamReader& Stm) = 0;
	virtual void SaveToStream(PhobosStreamWriter& Stm) const = 0;

	virtual void CalculateCRC(CRCEngine& crc) const = 0;

private:
	AbstractExtended(const AbstractExtended&) = delete;
	void operator = (const AbstractExtended&) = delete;
};

template <class T>
concept HasAbsID = requires(T) { T::AbsID; };

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
private:

	using base_type = typename T::base_type;
	using extension_type = typename T;
	using base_type_ptr = base_type*;
	using const_base_type_ptr = const base_type*;
	using extension_type_ptr = extension_type*;
	using extension_type_ref_ptr = extension_type**;
	using const_extension_type_ptr = const extension_type*;

public:
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
		if (extension_type_ptr val = DLLCreate<extension_type>(key)) {
			val->AttachedToObject = key;
		}

		return nullptr;
	}

	extension_type_ptr AllocateNoInit(base_type_ptr key)
	{
		if (extension_type_ptr val = DLLCreate<extension_type>(key, noinit_t())) {
			val->AttachedToObject = key;
		}

		return nullptr;
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
			DLLCallDTOR<false>(Item);
			this->ClearExtAttribute(key);
		}
	}

	void RemoveExtOf(base_type_ptr key , extension_type_ptr Item) {
		DLLCallDTOR<false>(Item);
		this->ClearExtAttribute(key);
	}

	void LoadFromINI(base_type_ptr key, CCINIClass* pINI, bool parseFailAddr)
	{
		if COMPILETIMEEVAL (CanLoadFromINIFile<T>)
		{
			if (extension_type_ptr ptr = this->TryFind(key))
			{
				if (!pINI) {
					//Debug::LogInfo("[%s] LoadFrom INI Called WithInvalid CCINIClass ptr ! ", typeid(T).name());
					return;
				}

				switch (ptr->Initialized) {
					case InitState::Blank:
					{
						if COMPILETIMEEVAL (Initable<T>)
							ptr->Initialize();

						ptr->Initialized = InitState::Inited;

						if COMPILETIMEEVAL (CanLoadFromRulesFile<T>) {
							if (pINI == CCINIClass::INI_Rules) {
								ptr->LoadFromRulesFile(pINI);
							}
						}

						//Load from rules INI File
						ptr->LoadFromINI(pINI, parseFailAddr);
						ptr->Initialized = InitState::Ruled;
					}
					break;
					case InitState::Ruled:
					case InitState::Constanted:
					{
						//load anywhere other than rules
						ptr->LoadFromINI(pINI, parseFailAddr);
						//this function can be called again multiple time but without need to re-init the data
						ptr->Initialized = InitState::Ruled;
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

public: //array Operation

	static void Clear()
	{
	}

	static bool SaveToGlobal()
	{

	}

	static bool LoadFromGlobal()
	{

	}

	static void Swizzle()
	{

	}

	static void InvalidatePointer(AbstractClass* const ptr, bool bRemoved)	{
		for (auto& ext : Array) {
			ext->InvalidatePointer(ptr, bRemoved);
		}
	}

public: //not sure if these needed ?

	virtual bool WriteDataToTheByteStream(base_type_ptr key, IStream* pStm) = 0;
	virtual bool ReadDataFromTheByteStream(base_type_ptr key, IStream * pStm) = 0;

};
