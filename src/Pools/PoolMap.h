#pragma once

#include "Pool.h"

#include <unordered_map>

class PoolMap
{
	std::unordered_map<PoolIdentifier, Pool*> _dict;

public:
	Pool* get(PoolIdentifier& id);
	void insert(Pool* pool);
	size_t erase(PoolIdentifier& id);
	size_t erase(Pool* pool);

	template<typename ThePool> requires std::is_base_of_v<Pool, ThePool>
	ThePool* CreatePool(PoolIdentifier& id);

	void RemovePool(PoolIdentifier& id);
	void RemovePool(Pool* pool);

	// For Save/Load
	bool Load(PhobosStreamReader& Stm, bool RegisterForChange)
	{
		return Stm
			.Process(_dict)
			.Success();
	}

	bool Save(PhobosStreamWriter& Stm) const
	{
		return Stm
			.Process(_dict)
			.Success();
	}
};