#pragma once

#include <GeneralStructures.h>
#include <LaserDrawClass.h>
#include <HouseClass.h>

#include <New/Type/LaserTrailTypeClass.h>

#include <vector>

class LaserTrailClass
{
public:
	LaserTrailTypeClass* Type;
	bool Visible;
	CoordStruct FLH;
	bool IsOnTurret;
	ColorStruct CurrentColor;
	Nullable<CoordStruct> LastLocation;

	LaserTrailClass(LaserTrailTypeClass* pTrailType, HouseClass* pHouse = nullptr,
		CoordStruct flh = { 0, 0, 0 }, bool isOnTurret = false) :
		Type { pTrailType }
		, Visible { true }
		, FLH { flh }
		, IsOnTurret { isOnTurret }
		, CurrentColor { pTrailType->Color }
		, LastLocation {}
	{
		if (this->Type->IsHouseColor && pHouse)
			this->CurrentColor = pHouse->LaserColor;
	}

	LaserTrailClass() :
		Type {},
		Visible {},
		FLH {},
		IsOnTurret {},
		CurrentColor {},
		LastLocation {}
	{ }

	bool Update(CoordStruct location);

	bool Load(PhobosStreamReader& stm, bool registerForChange);
	bool Save(PhobosStreamWriter& stm) const;

private:
	template <typename T>
	bool Serialize(T& stm);
};
