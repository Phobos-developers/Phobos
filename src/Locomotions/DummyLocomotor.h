#pragma once

#include "Locomotions.h"

// __declspec(uuid("0C2F47D2-34F5-445a-A38A-D66C70329658"))
DEFINE_GUID(DummyLocomotorClass_CLSID, 0xC2F47D2, 0x34F5, 0x445A, 0xA3, 0x8A, 0xD6, 0x6C, 0x70, 0x32, 0x96, 0x58);
class DummyLocomotorClass : public PhobosLocomotionClass
{
public:
	static constexpr CLSID ClassID { 0xC2F47D2, 0x34F5, 0x445A, {0xA3, 0x8A, 0xD6, 0x6C, 0x70, 0x32, 0x96, 0x58} };

	DummyLocomotorClass() : PhobosLocomotionClass()
	{

	}
};

class DummyLocomotorFactory : public PhobosLocomotorFactory
{
public:
	using Locomotion = DummyLocomotorClass;
	virtual STDMETHODIMP CreateInstance(LPUNKNOWN, REFIID, LPVOID FAR*);
};