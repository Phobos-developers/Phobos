#pragma once

/* ! What is a pool?
*  ! Pool is a helper class for all TechnoClass items to interact with others.
*  ! A TechnoClass item would be attached to one or more pools.
*  ! Kerb should do this himself. Ask Kerb for details - secsome
*  !
*  ! A specific pool class will track its items, for example:
*  ! class ShieldPool : public KerbPool
*  ! {
*  !	// some overrides
*  !	
*  !	static std::vector<ShieldPool*> Array; stores all class instances which inherits from ShieldPool
*  ! }
*  !
*  ! All pools should have the same set of virtual functions and overrides them, so
*  ! we can easily manage them by iterate the KerbPool::Array and run each of them.
*  ! For example: 
*  ! virtual bool InteractWithPool(PoolIdentifier identifier, KerbPool* pPoolToInteract) = 0;
*  ! **may** be a good example for that idea.
*  ! 
*  ! Kerb should build up this foundation class by himself - secsome
*/

#include <FootClass.h>

#include <vector>
#include <unordered_map>

enum class PoolIdentifier : int
{
	KerbPool = -1 // the basic pool
};

// The foundation for other implemented pools.
// secsome named this pool.
class KerbPool
{
	TechnoClass* pTechno;
	
public:
	explicit KerbPool(TechnoClass* _pTechno) : pTechno{ _pTechno } {}
	virtual ~KerbPool() = 0;
	
	virtual PoolIdentifier WhatPoolAmI() = 0;

	// Statics
public:
	static std::vector<KerbPool*> Array;
};