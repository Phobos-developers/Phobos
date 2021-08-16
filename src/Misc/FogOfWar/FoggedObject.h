#pragma once

#include <Utilities/SavegameDef.h>

#define FOGGED_BRIGHTNESS 250

class AnimTypeClass;
class BuildingTypeClass;
class HouseClass;

class FoggedObject
{
public:
	AbstractType CoveredRTTIType;
	CoordStruct Location;
	RectangleStruct Bound;
	bool Visible;

public:
	FoggedObject(ObjectClass* pObject);
	FoggedObject() = default;

	virtual ~FoggedObject();
	virtual void Draw(RectangleStruct& rect) { }
	virtual int GetType();
	virtual BuildingTypeClass* GetBuildingType();
	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange);
	virtual bool Save(PhobosStreamWriter& Stm) const;
};

class FoggedSmudge : public FoggedObject
{
public:
	int Smudge;
	unsigned char SmudgeData;

public:
	FoggedSmudge(CellClass* pCell, int smudge, unsigned char smudgeData);

	virtual ~FoggedSmudge() override;
	virtual void Draw(RectangleStruct& rect) override;
	virtual int GetType() override;
	virtual BuildingTypeClass* GetBuildingType() override;
	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;
	virtual bool Save(PhobosStreamWriter& Stm) const override;
};

class FoggedTerrain : public FoggedObject
{
public:
	int Terrain;
	int FrameIndex;

public:
	FoggedTerrain(ObjectClass* pObject, int terrain, int frameIdx);

	virtual ~FoggedTerrain() override;
	virtual void Draw(RectangleStruct& rect) override;
	virtual int GetType() override;
	virtual BuildingTypeClass* GetBuildingType() override;
	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;
	virtual bool Save(PhobosStreamWriter& Stm) const override;
};

class FoggedOverlay : public FoggedObject
{
public:
	int Overlay;
	unsigned char OverlayData;

public:
	FoggedOverlay(CellClass* pCell, int overlay, unsigned char overlayData);

	virtual ~FoggedOverlay() override;
	virtual void Draw(RectangleStruct& rect) override;
	virtual int GetType() override;
	virtual BuildingTypeClass* GetBuildingType() override;
	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;
	virtual bool Save(PhobosStreamWriter& Stm) const override;
};

class FoggedBuilding : public FoggedObject
{
public:
	struct FoggedBuildingAnimStruct
	{
		AnimTypeClass* AnimType;
		RectangleStruct OriginalBound;
		RectangleStruct Bound;
		bool Visible;
		int FrameIndex;
		int ZAdjust;
		int YSort;

		bool Load(PhobosStreamReader& stm, bool registerForChange);
		bool Save(PhobosStreamWriter& stm) const;

	private:
		template <typename T>
		bool Serialize(T& stm);
	};

	HouseClass* Owner;
	BuildingTypeClass* Type;
	RectangleStruct OriginalBound;
	std::vector<FoggedBuildingAnimStruct> BuildingAnims;
	int FrameIndex;
	bool FireStormWall;

public:
	FoggedBuilding(BuildingClass* pObject, bool bTranslucent);

	void DrawBuildingAnim(FoggedBuildingAnimStruct& Anim, RectangleStruct& rect);

	virtual ~FoggedBuilding() override;
	virtual void Draw(RectangleStruct& rect) override;
	virtual int GetType() override;
	virtual BuildingTypeClass* GetBuildingType() override;
	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;
	virtual bool Save(PhobosStreamWriter& Stm) const override;
};