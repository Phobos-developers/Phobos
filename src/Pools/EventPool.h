#pragma once

#include "Pool.h"

#include <FootClass.h>

class EventPool : public Pool
{
public:
	virtual void OnUpdate() = 0;
	virtual void OnReceiveDamage(int* pDamage, int nDistanceFromEpicenter, WarheadTypeClass* pWH,
		ObjectClass* pAttacker, bool bIgnoreDefenses, bool bPreventPassengerEscape, HouseClass* pAttackingHouse) = 0;
	virtual void OnFire(AbstractClass* pTarget, int weaponIndex) = 0;
};