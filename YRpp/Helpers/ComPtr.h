#pragma once

#include <YRCom.h>
#include <Unsorted.h>

#include <type_traits>

// a managed COM pointer like the game uses it

template <typename T>
struct YRComPtr {
	using pointer = T*;

	static_assert(std::is_base_of<IUnknown, T>::value, "T has to be derived from IUnknown.");

	YRComPtr() : value(pointer()) { }

	explicit YRComPtr(T* pInterface) : value(_addref(pInterface)) { }

	template <typename TIn>
	explicit YRComPtr(TIn* pIUnknown) : value(pointer()) {
		static_assert(std::is_base_of<IUnknown, TIn>::value, "TIn has to be derived from IUnknown.");

		if(pIUnknown) {
			auto hr = pIUnknown->QueryInterface(__uuidof(T), reinterpret_cast<void**>(&this->value));
			this->raise_if_failed(hr);
		}
	}

	YRComPtr(const IID& rclsid, LPUNKNOWN pUnkOuter, DWORD dwClsContext = CLSCTX_INPROC_SERVER
		| CLSCTX_INPROC_HANDLER | CLSCTX_LOCAL_SERVER) : value(pointer())
	{
		HRESULT hr = YRComHelpers::CreateInstance<T>(rclsid, pUnkOuter, dwClsContext, this->value);
		this->raise_if_failed(hr);
	}

	template <typename TIn>
	YRComPtr(const YRComPtr<TIn>& other) : YRComPtr(other.get()) {	}

	YRComPtr(const YRComPtr& other) : YRComPtr(other.value) { }

	YRComPtr(YRComPtr&& other) : value(other.release()) { }

	~YRComPtr() {
		this->_release();
	}

	YRComPtr& operator = (const YRComPtr& other) {
		this->reset(other.value);
		return *this;
	}

	YRComPtr& operator = (YRComPtr&& other) noexcept {
		this->_release();
		this->value = other.release();
		return *this;
	}

	explicit operator bool() const {
		return this->value != pointer();
	}

	__forceinline T* operator -> () const {
		this->raise_if_empty();
		return this->value;
	}

	__forceinline T& operator * () const {
		this->raise_if_empty();
		return *this->value;
	}

	T* get() const {
		return this->value;
	}

	T** pointer_to() {
		return &this->value;
	}

	void reset(T* pInterface = pointer()) {
		auto pNew = this->_addref(pInterface);
		this->_release();
		this->value = pNew;
	}

	T* release() {
		auto ret = this->value;
		this->value = pointer();
		return ret;
	}

private:
	T* _addref(T* ptr) {
		if(ptr) {
			ptr->AddRef();
		}
		return ptr;
	}

	void _release() {
		if(this->value) {
			this->value->Release();
		}
	}

	void raise_if_failed(HRESULT hr) const {
		if(FAILED(hr) && hr != E_NOINTERFACE) {
			Game::RaiseError(hr);
		}
	}

	void raise_if_empty() const {
		if(!this->value) {
			Game::RaiseError(E_POINTER);
		}
	}

	T* value;
};

/*
 * Copyright (C) 2014 Patrick Mours
 * SPDX-License-Identifier: BSD-3-Clause OR MIT
 */

#include <cassert>
#include <utility>

template <typename T>
class com_ptr
{
public:
	com_ptr()
		: _object(nullptr) { }
	com_ptr(std::nullptr_t)
		: _object(nullptr) { }
	com_ptr(T* object, bool own = false)
		: _object(object)
	{
		if (!own && _object != nullptr)
			_object->AddRef();
	}
	com_ptr(const com_ptr<T>& ptr)
		: _object(nullptr) { reset(ptr._object); }
	com_ptr(com_ptr<T>&& ptr)
		: _object(nullptr) { operator=(std::move(ptr)); }
	~com_ptr() { reset(); }

	/// <summary>
	/// Returns the stored pointer to the managed object.
	/// </summary>
	T* get() const { return _object; }

	/// <summary>
	/// Returns the current COM reference count of the managed object.
	/// </summary>
	unsigned long ref_count() const
	{
		assert(_object != nullptr);
		return _object->AddRef(), _object->Release();
	}

	/// <summary>
	/// Returns the stored pointer and releases ownership without decreasing the reference count.
	/// </summary>
	T* release()
	{
		T* const object = _object;
		_object = nullptr;
		return object;
	}

	/// <summary>
	/// Replaces the managed object.
	/// </summary>
	/// <param name="object">The new object to manage and take ownership and add a reference to.</param>
	void reset(T* object = nullptr)
	{
		if (_object != nullptr)
			_object->Release();
		_object = object;
		if (_object != nullptr)
			_object->AddRef();
	}

	// Overloaded pointer operators which operate on the managed object.
	T& operator*() const
	{
		assert(_object != nullptr);
		return *_object;
	}
	T* operator->() const
	{
		assert(_object != nullptr);
		return _object;
	}

	// This should only be called on uninitialized objects, e.g. when passed into 'QueryInterface' or creation functions.
	T** operator&()
	{
		assert(_object == nullptr);
		return &_object;
	}

	com_ptr<T>& operator=(T* object)
	{
		reset(object);
		return *this;
	}
	com_ptr<T>& operator=(const com_ptr<T>& copy)
	{
		reset(copy._object);
		return *this;
	}
	com_ptr<T>& operator=(com_ptr<T>&& move)
	{
		// Clear the current object first
		if (_object != nullptr)
			_object->Release();

		_object = move._object;
		move._object = nullptr;

		return *this;
	}

	bool operator==(const T* rhs) const { return _object == rhs; }
	bool operator==(const com_ptr<T>& rhs) const { return _object == rhs._object; }
	friend bool operator==(const T* lhs, const com_ptr<T>& rhs) { return rhs.operator==(lhs); }
	bool operator!=(const T* rhs) const { return _object != rhs; }
	bool operator!=(const com_ptr<T>& rhs) const { return _object != rhs._object; }
	friend bool operator!=(const T* lhs, const com_ptr<T>& rhs) { return rhs.operator!=(lhs); }

	// Default operator used for sorting
	friend bool operator< (const com_ptr<T>& lhs, const com_ptr<T>& rhs) { return lhs._object < rhs._object; }

private:
	T* _object;
};

#include <functional> // std::hash

namespace std
{
	template <typename T>
	struct hash<com_ptr<T>>
	{
		size_t operator()(const com_ptr<T>& ptr) const
		{
			return std::hash<T*>()(ptr.get());
		}
	};
}
