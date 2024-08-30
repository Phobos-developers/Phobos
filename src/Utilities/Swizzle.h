#pragma once

// Ares has hooked the SwizzleManagerClass,
// so what we need to do is just call the original functions.

#include <type_traits>

#include <SwizzleManagerClass.h>

class PhobosSwizzle
{
public:
	/**
	* pass in the *address* of the pointer you want to have changed
	* caution, after the call *p will be NULL
	*/
	inline static HRESULT RegisterForChange(void** p)
	{
		return SwizzleManagerClass::Instance->Swizzle(p);
	}

	/**
	* the original game objects all save their `this` pointer to the save stream
	* that way they know what ptr they used and call this function with that old ptr and `this` as the new ptr
	*/
	inline static HRESULT RegisterChange(void* was, void* is)
	{
		return SwizzleManagerClass::Instance->Here_I_Am((long)was, is);
	}

	template<typename T>
	inline static void RegisterPointerForChange(T*& ptr)
	{
		auto pptr = const_cast<std::remove_cv_t<T>**>(&ptr);
		RegisterForChange(reinterpret_cast<void**>(pptr));
	}
};

struct Swizzle {
	template <typename T>
	Swizzle(T& object)
	{
		if constexpr (std::is_pointer_v<T>)
			PhobosSwizzle::RegisterPointerForChange(object);
	}
};
