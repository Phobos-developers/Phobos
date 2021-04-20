#pragma once

#include "../SavegameDef.h"
#include <BuildingClass.h>

class FoggedObject
{
public:
	CoordStruct Location;
	RectangleStruct Bound;

public:
	virtual ~FoggedObject() = 0;
	virtual void Draw(RectangleStruct& rect) = 0;
	virtual int GetType() = 0;
	virtual BuildingTypeClass* GetBuildingType() = 0;
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
	virtual BuildingTypeClass* GetBuildingType() override;
	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;
	virtual bool Save(PhobosStreamWriter& Stm) const override;
};

class FoggedTerrain :public FoggedObject
{
public:
	int Terrain;

public:
	virtual ~FoggedTerrain();
	virtual void Draw(RectangleStruct& rect) override;
	virtual int GetType() override;
	virtual BuildingTypeClass* GetBuildingType() override;
	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;
	virtual bool Save(PhobosStreamWriter& Stm) const override;
};

class FoggedOverlay :public FoggedObject
{
public:
	int Overlay;
	int OverlayData;

public:
	virtual ~FoggedOverlay();
	virtual void Draw(RectangleStruct& rect) override;
	virtual int GetType() override;
	virtual BuildingTypeClass* GetBuildingType() override;
	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;
	virtual bool Save(PhobosStreamWriter& Stm) const override;
};