#pragma once

#include <GeneralStructures.h>
#include <LaserDrawClass.h>
#include <HouseClass.h>

#include <New/Type/LaserTrailTypeClass.h>

#include <vector>

class LaserTrailClass
{
public:
    Valueable<LaserTrailTypeClass*> Type;
    Valueable<bool> Visible;
	Valueable<CoordStruct> FLH;
	Valueable<bool> IsOnTurret;
	Valueable<ColorStruct> CurrentColor;
    Nullable<CoordStruct> LastLocation;

    LaserTrailClass(LaserTrailTypeClass* pTrailType, HouseClass* pHouse = nullptr,
		CoordStruct flh = { 0, 0, 0 }, bool isOnTurret = false) :
        Type(pTrailType),
        Visible(true),
		FLH(flh),
		IsOnTurret(isOnTurret),
		CurrentColor(pTrailType->Color),
        LastLocation()
    {
        if (this->Type->IsHouseColor && pHouse)
            this->CurrentColor = pHouse->LaserColor;
    }

    LaserTrailClass() :
        Type(),
        Visible(),
		FLH(),
		IsOnTurret(),
		CurrentColor(),
        LastLocation()
    { }

	virtual void LoadFromStream(PhobosStreamReader& Stm);
	virtual void SaveToStream(PhobosStreamWriter& Stm);

    bool Draw(CoordStruct location);

private:
	template <typename T>
	void Serialize(T& Stm);
};