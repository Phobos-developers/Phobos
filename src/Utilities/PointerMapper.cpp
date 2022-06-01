#include "PointerMapper.h"

std::map<long, long> PointerMapper::Map;

void PointerMapper::AddMapping(void* was, void* is)
{
	Map[reinterpret_cast<long>(was)] = reinterpret_cast<long>(is);
}

void* PointerMapper::Mapping(void* was)
{
	if (Map.count(reinterpret_cast<long>(was)))
		return reinterpret_cast<void*>(Map[reinterpret_cast<long>(was)]);
	return nullptr;
}

bool PointerMapper::Exist(void* was)
{
	return Map.find(reinterpret_cast<long>(was)) != Map.end();
}