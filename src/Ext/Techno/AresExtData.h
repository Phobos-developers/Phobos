#pragma once
#include <TechnoClass.h>

class AresTechnoExt
{
public:
	using base_type = TechnoClass;
	static constexpr size_t ExtPointerOffset = 0x154;

	class ExtData final
	{
	public:
		char unknow[0x9E];
		bool PayloadCreated;
	};

	static ExtData* FindExtData(base_type* key)
	{
		if (!key)
			return nullptr;

		return (ExtData*)(*(uintptr_t*)((char*)key + ExtPointerOffset));
	}
};
