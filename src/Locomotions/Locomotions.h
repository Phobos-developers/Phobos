/*
* This header is the base of our new locomotors, it's based on VK's locomotor code at
* https://www.ppmforums.com/topic-15345/new-locomotor/?highlight=locomotor
* - secsome
*/

#pragma once

#include <LocomotionClass.h>

class PhobosLocomotorFactory : public IClassFactory
{
protected:
	ULONG	m_cRef;
public:
	PhobosLocomotorFactory();
	~PhobosLocomotorFactory();

	//IUnknown
	STDMETHODIMP			QueryInterface(REFIID, LPVOID FAR*);
	STDMETHODIMP_(ULONG)	AddRef();
	STDMETHODIMP_(ULONG)	Release();

	//IClassFactory members
	STDMETHODIMP		CreateInstance(LPUNKNOWN, REFIID, LPVOID FAR*);
	STDMETHODIMP		LockServer(BOOL);
};



class PhobosLocomotionClass : public LocomotionClass
{
public:

	template<typename T> requires std::is_base_of_v<PhobosLocomotionClass, T>
	static YRComPtr<T> CreateInstance(const CLSID& rclsid)
	{
		return LocomotionClass::CreateInstance(rclsid);
	}


};