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

template<typename T>
concept CanBeAbstract = std::is_base_of<AbstractClass, T>::value;

template<typename T>
concept CanBeTechno = std::is_base_of<TechnoClass, T>::value;

template<typename T>
concept CanBeAbstractType = std::is_base_of<AbstractTypeClass, T>::value;
