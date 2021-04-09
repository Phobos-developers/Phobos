#include "PoolMap.h"

#include <Memory.h>

Pool* PoolMap::get(PoolIdentifier& id)
{
	auto itr = _dict.find(id);
	return itr == _dict.end() ? nullptr : itr->second;
}

void PoolMap::insert(Pool* pool)
{
	_dict[pool->GetIdentifier()] = pool;
}

size_t PoolMap::erase(PoolIdentifier& id)
{
	return _dict.erase(id);
}

size_t PoolMap::erase(Pool* pool)
{
	return _dict.erase(pool->GetIdentifier());
}

template<typename ThePool> requires std::is_base_of_v<Pool, ThePool>
ThePool* PoolMap::CreatePool(PoolIdentifier& id)
{
	ThePool* instance = DLLCreate<ThePool>();
	instance->CreatePool(id);
	_dict.insert(instance);
}

void PoolMap::RemovePool(PoolIdentifier& id)
{
	auto itr = _dict.find(id);
	if (itr != _dict.end())
	{
		auto pPool = itr->second;
		_dict.erase(itr);
		DLLDelete(pPool);
	}
}

void PoolMap::RemovePool(Pool* pool)
{
	this->erase(pool);
	DLLDelete(pool);
}