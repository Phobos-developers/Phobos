#pragma once

#include <GeneralStructures.h>
#include <PCX.h>
#include <TacticalClass.h>

#include <Utilities/SavegameDef.h>
#include <Utilities/Constructs.h>
#include <Utilities/Template.h>
#include <Helpers/Template.h>
#include <Utilities/Enum.h>

#include <New/Type/BannerTypeClass.h>

class BannerClass
{
public:
	static DynamicVectorClass<BannerClass*> Array;

	BannerTypeClass* Type;
	int Id;
	CoordStruct Position;

	BannerClass(BannerTypeClass* pBannerType, int id, CoordStruct position) :
		Type(pBannerType),
		Id(id),
		Position(position)
	{
		BannerClass::Array.AddItem(this);
	}

	BannerClass() :
		Type(),
		Id(),
		Position()
	{
		BannerClass::Array.AddItem(this);
	}

	void Render();

	void InvalidatePointer(void* ptr) { };

	bool Load(PhobosStreamReader& Stm, bool RegisterForChange);
	bool Save(PhobosStreamWriter& Stm) const;

private:
	template <typename T>
	bool Serialize(T& Stm);
};
