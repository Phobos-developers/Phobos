#pragma once
#include <map>

class PointerMapper
{
public:
	static std::map<long, long> Map;

	static void AddMapping(void* was, void* is);
	static void* Mapping(void* was);
	static bool Exist(void* was);
};