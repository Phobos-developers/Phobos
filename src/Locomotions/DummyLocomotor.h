/*
* This file is an example for you to know how to setup a new locomotor
* - secsome
*/

#pragma once

#include "Locomotions.h"

// 1. Define GUID here
// __declspec(uuid("0C2F47D2-34F5-445a-A38A-D66C70329658"))
DEFINE_GUID(DummyLocomotorClass_CLSID, 0xC2F47D2, 0x34F5, 0x445A, 0xA3, 0x8A, 0xD6, 0x6C, 0x70, 0x32, 0x96, 0x58);
class DummyLocomotorClass : public PhobosLocomotionClass // You can also inherit from some custom locomotors, just like
															// SecsomeLocomotorClass : KerbLocomotorClass : PhobosLocomotorClass
{
public:
	// 2. Make This ClassID same as the one above, it will be used by our templates
	static constexpr CLSID ClassID { 0xC2F47D2, 0x34F5, 0x445A, {0xA3, 0x8A, 0xD6, 0x6C, 0x70, 0x32, 0x96, 0x58} };

	// 3. This is the constructor
	DummyLocomotorClass() : PhobosLocomotionClass()
	{

	}

	// 4. You can add extra property here

	// 5. Also, extra functions can be implemented here

	// 6. Don't forget to implement virtual functions from base "LocomotionClass", it's quite important!
};

// 7. Remember to add this for it's factory definition
LOCO_FACTORY(DummyLocomotorFactory, DummyLocomotorClass);

// 8. Go back to Locomotions.cpp and add this LocomotorFactory to the LocomotorCollection called Locomotors, like
/*

auto Locomotors = LocomotorCollection<
	DummyLocomotorFactory
>();

If you have more locomotors, it should be like:
auto Locomotors = LocomotorCollection<
	SecsomeLocomotorFactory,
	KerbLocomotorFactory,
	UranusianLocomotorFactory
>();
and so on.

*/