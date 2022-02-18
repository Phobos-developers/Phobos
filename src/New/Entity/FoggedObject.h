#pragma once

#include <AbstractClass.h>
#include <BuildingClass.h>

#include <Ext/Cell/Body.h>

class TerrainClass;

class FoggedObject
{
public:
	static DynamicVectorClass<FoggedObject*> FoggedObjects;

	static void SaveGlobal(IStream* pStm);
	static void LoadGlobal(IStream* pStm);

	explicit FoggedObject() noexcept;
	explicit FoggedObject(BuildingClass* pBld, bool IsVisible) noexcept;
	explicit FoggedObject(TerrainClass* pTerrain) noexcept;

	// Handles Smudge and Overlay Construct
	explicit FoggedObject(CellClass* pCell, bool IsOverlay) noexcept;

	void Load(IStream* pStm);
	void Save(IStream* pStm);

	//Destructor
	virtual ~FoggedObject();

	void Render(const RectangleStruct& viewRect) const;

	static RectangleStruct Union(const RectangleStruct& rect1, const RectangleStruct& rect2);
protected:
	inline int GetIndexID() const;

	void RenderAsBuilding(const RectangleStruct& viewRect) const;
	void RenderAsSmudge(const RectangleStruct& viewRect) const;
	void RenderAsOverlay(const RectangleStruct& viewRect) const;
	void RenderAsTerrain(const RectangleStruct& viewRect) const;

	static char BuildingVXLDrawer[sizeof(BuildingClass)];
public:
	enum class CoveredType : char
	{
		Building = 1,
		Terrain,
		Smudge,
		Overlay
	};

	CoordStruct Location;
	CoveredType CoveredType;
	RectangleStruct Bound;
	bool Visible { true };

	union
	{
		struct
		{
			int Overlay;
			unsigned char OverlayData;
		} OverlayData;
		struct
		{
			TerrainTypeClass* Type;
			int Frame;
			bool Flat;
		} TerrainData;
		struct
		{
			HouseClass* Owner;
			BuildingTypeClass* Type;
			int ShapeFrame;
			FacingStruct PrimaryFacing;
			FacingStruct BarrelFacing;
			RecoilData TurretRecoil;
			RecoilData BarrelRecoil;
			bool IsFirestormWall;
			int TurretAnimFrame;
			struct
			{
				AnimTypeClass* AnimType;
				int AnimFrame;
				int ZAdjust;
			} Anims[21];
		} BuildingData;
		struct
		{
			int Smudge;
			int SmudgeData;
			int Height;
		} SmudgeData;
	};
};