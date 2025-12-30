/*
 *  C++ 20 standard introduces this, might be useful.
 *  However, IntelliSense do not recognize them... for now, so do modules.
 *  When it's possible, I might try to make the codes fit with C++ 20 then.
 *
 *  So this file is for further use.
 *  Author : secsome
 */

#pragma once

#include <type_traits>

class AbstractClass;
class TechnoClass;
class AbstractTypeClass;
class TecnoTypeClass;
class CCINIClass;

template<typename T>
concept CanBeAbstract = std::is_base_of<AbstractClass, T>::value;

template<typename T>
concept CanBeTechno = std::is_base_of<TechnoClass, T>::value;

template<typename T>
concept CanBeAbstractType = std::is_base_of<AbstractTypeClass, T>::value;

template<typename T>
concept IsTechno = std::is_base_of<TecnoTypeClass, T>::value;

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
	requires (T t, CCINIClass * pINI, bool parseFailAddr) { t.LoadFromINI(pINI, parseFailAddr); };

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
	requires (AbstractClass * ptr, bool removed) { T::InvalidatePointer(ptr, removed); };

template <typename T>
concept ThisPointerInvalidationSubscribable =
	requires (T t, AbstractClass * ptr, bool removed) { t.InvalidatePointer(ptr, removed); };