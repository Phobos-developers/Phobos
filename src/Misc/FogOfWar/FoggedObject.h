#pragma once

#include "../SavegameDef.h"

#include <BuildingClass.h>

class FoggedObject
{
public:
	CoordStruct Location;
	RectangleStruct Bound;

public:
	virtual ~FoggedObject();
	virtual void Draw(RectangleStruct& rect) = 0;
	virtual int GetType();
	virtual BuildingTypeClass* GetBuildingType();
	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange);
	virtual bool Save(PhobosStreamWriter& Stm) const;
};

class FoggedSmudge : public FoggedObject
{
public:
	int Smudge;

public:
	virtual ~FoggedSmudge();
	virtual void Draw(RectangleStruct& rect) override;
	virtual int GetType() override;
	virtual BuildingTypeClass* GetBuildingType();
	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;
	virtual bool Save(PhobosStreamWriter& Stm) const override;
};

class FoggedTerrain : public FoggedObject
{
public:
	int Terrain;

public:
	virtual ~FoggedTerrain();
	virtual void Draw(RectangleStruct& rect) override;
	virtual int GetType() override;
	virtual BuildingTypeClass* GetBuildingType();
	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;
	virtual bool Save(PhobosStreamWriter& Stm) const override;
};

class FoggedOverlay : public FoggedObject
{
public:
	int Overlay;
	unsigned char OverlayData;

public:
	virtual ~FoggedOverlay();
	virtual void Draw(RectangleStruct& rect) override;
	virtual int GetType() override;
	virtual BuildingTypeClass* GetBuildingType();
	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;
	virtual bool Save(PhobosStreamWriter& Stm) const override;
};

class FoggedBuilding : public FoggedObject
{
public:
	HouseClass* Owner;
	BuildingTypeClass* Type;
	int ShapeNumber;
	FacingStruct PrimaryFacing;
	FacingStruct BarrelFacing;
	RecoilData TurretRecoil;
	RecoilData BarrelRecoil;
	BYTE FireStormWall;
	BYTE gap_A5[3];
	int unknown_A8;
	int unknown_AC;
	int unknown_B0;
	int TypeArrayIndex;

public:
	virtual ~FoggedBuilding();
	virtual void Draw(RectangleStruct& rect) override;
	virtual int GetType();
	virtual BuildingTypeClass* GetBuildingType() override;
	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;
	virtual bool Save(PhobosStreamWriter& Stm) const override;
};