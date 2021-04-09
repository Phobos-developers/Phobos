#pragma once

#include "PoolIdentifier.h"
#include "../Misc/Stream.h"

class Pool
{
	PoolIdentifier _id;

public:
	PoolIdentifier GetIdentifier() const
		{ return _id; }

	virtual Pool* CreatePool(PoolIdentifier& id, ...) = 0;

	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange)
	{
		return Stm
			.Process(_id)
			.Success();
	}

	virtual bool Save(PhobosStreamWriter& Stm) const
	{
		return Stm
			.Process(_id)
			.Success();
	}
};