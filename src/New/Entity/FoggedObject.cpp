#include "FoggedObject.h"

#include <Ext/TechnoType/Body.h>

#include <TacticalClass.h>
#include <SmudgeTypeClass.h>
#include <AnimClass.h>
#include <TerrainClass.h>
#include <HouseClass.h>

DynamicVectorClass<FoggedObject*> FoggedObject::FoggedObjects;
char FoggedObject::BuildingVXLDrawer[sizeof(BuildingClass)];

void FoggedObject::SaveGlobal(IStream* pStm)
{
	pStm->Write(&FoggedObjects.Count, sizeof(FoggedObjects.Count), nullptr);
	for (auto const pObject : FoggedObjects)
	{
		pStm->Write(&pObject, sizeof(pObject), nullptr);
		pObject->Save(pStm);
	}

	pStm->Write(BuildingVXLDrawer, sizeof(BuildingVXLDrawer), nullptr);
}

void FoggedObject::LoadGlobal(IStream* pStm)
{
	int nCount;
	pStm->Read(&nCount, sizeof(nCount), nullptr);
	while (nCount--)
	{
		auto pObject = GameCreate<FoggedObject>();
		long pOldObject;
		pStm->Read(&pOldObject, sizeof(pOldObject), nullptr);
		SwizzleManagerClass::Instance->Here_I_Am(pOldObject, pObject);
		pObject->Load(pStm);
	}

	pStm->Read(BuildingVXLDrawer, sizeof(BuildingVXLDrawer), nullptr);
}

FoggedObject::FoggedObject() noexcept
{
	FoggedObjects.AddItem(this);
}

FoggedObject::FoggedObject(BuildingClass* pBld, bool IsVisible) noexcept
{
	CoveredType = CoveredType::Building;

	Location = pBld->Location;
	Visible = IsVisible;
	BuildingData.Owner = pBld->Owner;
	BuildingData.Type = pBld->Type;
	BuildingData.ShapeFrame = pBld->GetShapeNumber();
	BuildingData.PrimaryFacing = pBld->PrimaryFacing;
	BuildingData.BarrelFacing = pBld->BarrelFacing;
	BuildingData.TurretRecoil = pBld->TurretRecoil;
	BuildingData.BarrelRecoil = pBld->BarrelRecoil;
	BuildingData.IsFirestormWall = pBld->Type->FirestormWall;
	BuildingData.TurretAnimFrame = pBld->TurretAnimFrame;
	pBld->GetRenderDimensions(&Bound);

	memset(BuildingData.Anims, 0, sizeof(BuildingData.Anims));
	auto pAnimData = BuildingData.Anims;
	for (auto pAnim : pBld->Anims)
	{
		if (pAnim)
		{
			pAnimData->AnimType = pAnim->Type;
			pAnimData->AnimFrame = pAnim->Animation.Value + pAnimData->AnimType->Start;
			pAnimData->ZAdjust = pAnim->ZAdjust + pAnimData->AnimType->YDrawOffset - TacticalClass::AdjustForZ(pAnim->Location.Z);
			pAnimData->ZAdjust -= pAnimData->AnimType->Flat ? 3 : 2;
			++pAnimData;

			pAnim->IsFogged = true;
			RectangleStruct buffer;
			pAnim->GetDimensions(&buffer);
			Bound = std::move(Drawing::Union(Bound, buffer));
		}
	}

	TacticalClass::Instance->RegisterDirtyArea(Bound, false);
	Bound.X += TacticalClass::Instance->TacticalPos.X;
	Bound.Y += TacticalClass::Instance->TacticalPos.Y;
	pBld->IsFogged = true;

	if (!BuildingVXLDrawer[0] && pBld->Type->TurretAnimIsVoxel || pBld->Type->BarrelAnimIsVoxel)
	{
		memcpy(BuildingVXLDrawer, pBld, sizeof(BuildingClass));
		reinterpret_cast<BuildingClass*>(BuildingVXLDrawer)->BeingWarpedOut = false;
		reinterpret_cast<BuildingClass*>(BuildingVXLDrawer)->WarpFactor = 0.0f;
	}

	FoggedObjects.AddItem(this);
}

FoggedObject::FoggedObject(TerrainClass* pTerrain) noexcept
{
	Location = pTerrain->Location;
	CoveredType = CoveredType::Terrain;

	pTerrain->GetRenderDimensions(&Bound);
	Bound.X += TacticalClass::Instance->TacticalPos.X - DSurface::ViewBounds->X;
	Bound.Y += TacticalClass::Instance->TacticalPos.Y - DSurface::ViewBounds->Y;

	TerrainData.Type = pTerrain->Type;
	TerrainData.Frame = 0;
	if (TerrainData.Type->IsAnimated)
		TerrainData.Frame = pTerrain->Animation.Value;
	else if (pTerrain->TimeToDie)
		TerrainData.Frame = pTerrain->Animation.Value + 1;
	else if (pTerrain->Health < 2)
		TerrainData.Frame = 2;

	TerrainData.Flat = TerrainData.Type->IsAnimated || pTerrain->TimeToDie;

	FoggedObjects.AddItem(this);
}

FoggedObject::FoggedObject(CellClass* pCell, bool IsOverlay) noexcept
{
	pCell->GetCoords(&Location);
	if (IsOverlay)
	{
		CoveredType = CoveredType::Overlay;

		RectangleStruct containingRect;
		RectangleStruct shapeRect;
		pCell->ShapeRect(&shapeRect);
		pCell->GetContainingRect(&containingRect);
		Bound = std::move(Drawing::Union(shapeRect, containingRect));
		Bound.X += TacticalClass::Instance->TacticalPos.X;
		Bound.Y += TacticalClass::Instance->TacticalPos.Y;

		OverlayData.Overlay = pCell->OverlayTypeIndex;
		OverlayData.OverlayData = pCell->OverlayData;
	}
	else
	{
		CoveredType = CoveredType::Smudge;

		Location.Z = pCell->Level * Unsorted::LevelHeight;
		Point2D position;
		TacticalClass::Instance->CoordsToClient(Location, &position);

		Bound = RectangleStruct
		{
			position.X - 30 + TacticalClass::Instance->TacticalPos.X,
			position.Y - 15 + TacticalClass::Instance->TacticalPos.Y,
			60,
			30
		};

		SmudgeData.Smudge = pCell->SmudgeTypeIndex;
		SmudgeData.SmudgeData = pCell->SmudgeData;
		SmudgeData.Height = Location.Z;
	}

	FoggedObjects.AddItem(this);
}

void FoggedObject::Load(IStream* pStm)
{
	pStm->Read(this, sizeof(FoggedObject), nullptr);\

	if (CoveredType == CoveredType::Building)
	{
		SWIZZLE(BuildingData.Owner);
		SWIZZLE(BuildingData.Type);
		for (auto& Anim : BuildingData.Anims)
		{
			if (!Anim.AnimType)
				break;
			SWIZZLE(Anim.AnimType);
		}
	}
	else if (CoveredType == CoveredType::Terrain)
	{
		SWIZZLE(TerrainData.Type);
	}
}

void FoggedObject::Save(IStream* pStm)
{
	pStm->Write(this, sizeof(FoggedObject), nullptr);
}

FoggedObject::~FoggedObject()
{
	FoggedObjects.Remove(this);

	if (this->CoveredType == CoveredType::Building)
	{
		auto pCell = MapClass::Instance->GetCellAt(Location);
		if (auto pBld = pCell->GetBuilding())
		{
			pBld->IsFogged = false;
			for (auto pAnim : pBld->Anims)
				if (pAnim)
					pAnim->IsFogged = false;
		}
	}
}

void FoggedObject::Render(const RectangleStruct& viewRect) const
{
	if (!Visible)
		return;

	RectangleStruct buffer = Bound;
	buffer.X += DSurface::ViewBounds->X - TacticalClass::Instance->TacticalPos.X;
	buffer.Y += DSurface::ViewBounds->Y - TacticalClass::Instance->TacticalPos.Y;
	RectangleStruct finalRect = Drawing::Intersect(buffer, viewRect);
	if (finalRect.Width <= 0 || finalRect.Height <= 0)
		return;

	switch (CoveredType)
	{
	case CoveredType::Building:
		RenderAsBuilding(viewRect);
		break;

	case CoveredType::Overlay:
		RenderAsOverlay(viewRect);
		break;

	case CoveredType::Terrain:
		RenderAsTerrain(viewRect);
		break;

	case CoveredType::Smudge:
		RenderAsSmudge(viewRect);
		break;
	}
}

RectangleStruct FoggedObject::Union(const RectangleStruct& rect1, const RectangleStruct& rect2)
{
	if (rect1.Width <= 0 || rect1.Height <= 0)
		return rect2;
	if (rect2.Width <= 0 || rect2.Height <= 0)
		return rect1;

	return
	{
		std::min(rect1.X,rect2.X),
		std::min(rect1.Y,rect2.Y),
		std::max(rect1.X + rect1.Width,rect2.X + rect2.Width),
		std::max(rect1.Y + rect1.Height,rect2.Y + rect2.Height)
	};
}

int FoggedObject::GetIndexID() const
{
	int x = Location.X / 256;
	int y = Location.Y / 256;
	return (y - ((x + y) << 9) - x) - static_cast<int>(CoveredType) * 0x80000 + INT_MAX;
}

void FoggedObject::RenderAsBuilding(const RectangleStruct& viewRect) const
{
	auto const pType = BuildingData.Type;
	if (pType->InvisibleInGame)
		return;

	auto pScheme = ColorScheme::Array->GetItem(BuildingData.Owner->ColorSchemeIndex);
	auto pSHP = pType->GetImage();
	CoordStruct coord = { Location.X - 128,Location.Y - 128,Location.Z };
	Point2D point;
	TacticalClass::Instance->CoordsToClient(coord, &point);
	point.X += DSurface::ViewBounds->X - viewRect.X;
	point.Y += DSurface::ViewBounds->Y - viewRect.Y;

	auto pCell = MapClass::Instance->GetCellAt(Location);
	ConvertClass* pConvert;
	if (!pType->TerrainPalette)
		pConvert = pScheme->LightConvert;
	else
	{
		if (!pCell->LightConvert)
			pCell->InitLightConvert();
		pConvert = pCell->LightConvert;
	}
	if (pType->Palette)
		pConvert = pType->Palette->GetItem(BuildingData.Owner->ColorSchemeIndex)->LightConvert;

	if (pSHP)
	{
		RectangleStruct rect = viewRect;
		int height = point.Y + pSHP->Height / 2;
		if (rect.Height > height)
			rect.Height = height;
		int ZAdjust = -2 - TacticalClass::AdjustForZ(Location.Z);
		int Intensity = pCell->Intensity_Normal + pType->ExtraLight;
		if (rect.Height > 0)
		{
			if (BuildingData.IsFirestormWall)
			{
				CC_Draw_Shape(DSurface::Temp, pConvert, pSHP, BuildingData.ShapeFrame, &point, &rect,
					BlitterFlags::ZReadWrite | BlitterFlags::Alpha | BlitterFlags::bf_400 | BlitterFlags::Centered,
					0, ZAdjust, ZGradient::Ground, Intensity, 0, nullptr, 0, 0, 0);
			}
			else
			{
				CC_Draw_Shape(DSurface::Temp, pConvert, pSHP, BuildingData.ShapeFrame, &point, &rect,
					BlitterFlags::ZReadWrite | BlitterFlags::Alpha | BlitterFlags::bf_400 | BlitterFlags::Centered,
					0, ZAdjust, ZGradient::Deg90, Intensity, 0, nullptr, 0, 0, 0);
				CC_Draw_Shape(DSurface::Temp, pConvert, pSHP, BuildingData.ShapeFrame + pSHP->Frames / 2, &point, &rect,
					BlitterFlags::ZReadWrite | BlitterFlags::Alpha | BlitterFlags::bf_400 | BlitterFlags::Centered | BlitterFlags::Darken,
					0, ZAdjust, ZGradient::Ground, 1000, 0, nullptr, 0, 0, 0);
				if (pType->BibShape)
				{
					CC_Draw_Shape(DSurface::Temp, pConvert, pType->BibShape, BuildingData.ShapeFrame, &point, &viewRect,
						BlitterFlags::ZReadWrite | BlitterFlags::Alpha | BlitterFlags::bf_400 | BlitterFlags::Centered,
						0, ZAdjust - 1, ZGradient::Deg90, Intensity, 0, nullptr, 0, 0, 0);
				}
			}
		}
		Point2D turretPoint
		{
			point.X + pType->GetBuildingAnim(BuildingAnimSlot::Turret).Position.X,
			point.Y + pType->GetBuildingAnim(BuildingAnimSlot::Turret).Position.Y
		};
		if (pType->TurretAnimIsVoxel || pType->BarrelAnimIsVoxel)
		{
			auto pVXLDrawer = reinterpret_cast<BuildingClass*>(BuildingVXLDrawer);
			pVXLDrawer->Type = pType;
			pVXLDrawer->SecondaryFacing = BuildingData.PrimaryFacing;
			pVXLDrawer->BarrelFacing = BuildingData.BarrelFacing;
			pVXLDrawer->TurretRecoil = BuildingData.TurretRecoil;
			pVXLDrawer->BarrelRecoil = BuildingData.BarrelRecoil;
			pVXLDrawer->Owner = BuildingData.Owner;
			pVXLDrawer->Location = Location;
			pVXLDrawer->TurretAnimFrame = BuildingData.TurretAnimFrame;

			auto const primaryDir = BuildingData.PrimaryFacing.current();
			int turretFacing = 0;
			int barrelFacing = 0;
			if (pType->TurretVoxel.HVA)
				turretFacing = BuildingData.TurretAnimFrame % pType->TurretVoxel.HVA->FrameCount;
			if (pType->BarrelVoxel.HVA)
				barrelFacing = BuildingData.TurretAnimFrame % pType->BarrelVoxel.HVA->FrameCount;
			int val32 = primaryDir.value32();
			int turretExtra = ((unsigned char)turretFacing << 16) | val32;
			int barrelExtra = ((unsigned char)barrelFacing << 16) | val32;

			if (pType->TurretVoxel.VXL)
			{
				Matrix3D matrixturret;
				matrixturret.MakeIdentity();
				matrixturret.RotateZ(static_cast<float>(primaryDir.get_radian()));
				TechnoTypeExt::ApplyTurretOffset(pType, &matrixturret, 0.125);

				Vector3D<float> negativevector = { -matrixturret.Row[0].W ,-matrixturret.Row[1].W,-matrixturret.Row[2].W };
				Vector3D<float> vector = { matrixturret.Row[0].W ,matrixturret.Row[1].W,matrixturret.Row[2].W };
				Matrix3D matrixbarrel = matrixturret;
				if (BuildingData.TurretRecoil.State != RecoilData::RecoilState::Inactive)
				{
					matrixturret.TranslateX(-BuildingData.TurretRecoil.TravelSoFar);
					turretExtra = -1;
				}
				Matrix3D::MatrixMultiply(&matrixturret, &Matrix3D::VoxelDefaultMatrix, &matrixturret);

				bool bDrawBarrel = pType->BarrelVoxel.VXL && pType->BarrelVoxel.HVA;
				if (bDrawBarrel)
				{
					matrixbarrel.Translate(negativevector);
					if (BuildingData.BarrelRecoil.State != RecoilData::RecoilState::Inactive)
					{
						matrixbarrel.TranslateX(-BuildingData.BarrelRecoil.TravelSoFar);
						barrelExtra = -1;
					}
					matrixbarrel.RotateY(-static_cast<float>(BuildingData.BarrelFacing.current().get_radian()));
					matrixbarrel.Translate(vector);
					Matrix3D::MatrixMultiply(&matrixbarrel, &Matrix3D::VoxelDefaultMatrix, &matrixbarrel);
				}

				int facetype = (((((*(unsigned int*)&primaryDir) >> 13) + 1) >> 1) & 3);
				if (facetype == 0 || facetype == 3) // Draw barrel first
				{
					if (bDrawBarrel)
						pVXLDrawer->DrawVoxel(BuildingData.Type->BarrelVoxel, 0, (short)turretExtra,
							BuildingData.Type->VoxelCaches[3], viewRect, turretPoint, matrixbarrel,
							pCell->Intensity_Normal, 0, 0);

					pVXLDrawer->DrawVoxel(BuildingData.Type->TurretVoxel, turretFacing, (short)turretExtra,
						BuildingData.Type->VoxelCaches[1], viewRect, turretPoint, matrixturret,
						pCell->Intensity_Normal, 0, 0);
				}
				else
				{
					pVXLDrawer->DrawVoxel(BuildingData.Type->TurretVoxel, turretFacing, (short)turretExtra,
						BuildingData.Type->VoxelCaches[1], viewRect, turretPoint, matrixturret,
						pCell->Intensity_Normal, 0, 0);

					if (bDrawBarrel)
						pVXLDrawer->DrawVoxel(BuildingData.Type->BarrelVoxel, 0, (short)turretExtra,
							BuildingData.Type->VoxelCaches[3], viewRect, turretPoint, matrixbarrel,
							pCell->Intensity_Normal, 0, 0);
				}
			}
			else if (pType->BarrelVoxel.VXL && pType->BarrelVoxel.HVA)
			{
				Matrix3D matrixbarrel;
				matrixbarrel.MakeIdentity();
				Vector3D<float> negativevector = { -matrixbarrel.Row[0].W ,-matrixbarrel.Row[1].W,-matrixbarrel.Row[2].W };
				Vector3D<float> vector = { matrixbarrel.Row[0].W ,matrixbarrel.Row[1].W,matrixbarrel.Row[2].W };
				matrixbarrel.Translate(negativevector);
				matrixbarrel.RotateZ(static_cast<float>(BuildingData.PrimaryFacing.current().get_radian()));
				matrixbarrel.RotateY(-static_cast<float>(BuildingData.BarrelFacing.current().get_radian()));
				matrixbarrel.Translate(vector);
				Matrix3D::MatrixMultiply(&matrixbarrel, &Matrix3D::VoxelDefaultMatrix, &matrixbarrel);
				pVXLDrawer->DrawVoxel(BuildingData.Type->BarrelVoxel, barrelFacing, (short)barrelExtra,
					BuildingData.Type->VoxelCaches[3], viewRect, turretPoint, matrixbarrel,
					pCell->Intensity_Normal, 0, 0);
			}
		}
	}

	for (const auto& AnimData : BuildingData.Anims)
	{
		if (!AnimData.AnimType)
			break;

		auto pAnimType = AnimData.AnimType;
		if (auto pAnimSHP = pAnimType->GetImage())
		{
			ConvertClass* pAnimConvert = pAnimType->ShouldUseCellDrawer ? pScheme->LightConvert : FileSystem::ANIM_PAL();

			CC_Draw_Shape(DSurface::Temp, pAnimConvert, pAnimSHP, AnimData.AnimFrame, &point, &viewRect,
				BlitterFlags::ZReadWrite | BlitterFlags::Alpha | BlitterFlags::bf_400 | BlitterFlags::Centered,
				0, AnimData.ZAdjust, pAnimType->Flat ? ZGradient::Ground : ZGradient::Deg90,
				pAnimType->UseNormalLight ? 1000 : pCell->Intensity_Normal, 0, nullptr, 0, 0, 0);
			if (pAnimType->Shadow)
			{
				CC_Draw_Shape(DSurface::Temp, pAnimConvert, pAnimSHP, AnimData.AnimFrame + pAnimSHP->Frames / 2, &point, &viewRect,
					BlitterFlags::ZReadWrite | BlitterFlags::Alpha | BlitterFlags::bf_400 | BlitterFlags::Centered | BlitterFlags::Darken,
					0, AnimData.ZAdjust, ZGradient::Deg90, 1000, 0, nullptr, 0, 0, 0);
			}
		}
	}
}

void FoggedObject::RenderAsSmudge(const RectangleStruct& viewRect) const
{
	auto const pSmudge = SmudgeTypeClass::Array->GetItem(SmudgeData.Smudge);
	Point2D position
	{
			this->Bound.X - TacticalClass::Instance->TacticalPos.X - viewRect.X + DSurface::ViewBounds->X + 30,
			this->Bound.Y - TacticalClass::Instance->TacticalPos.Y - viewRect.Y + DSurface::ViewBounds->Y
	};
	CellStruct MapCoord = CellClass::Coord2Cell(Location);
	pSmudge->DrawIt(position, viewRect, SmudgeData.SmudgeData, SmudgeData.Height, MapCoord);
}

void FoggedObject::RenderAsOverlay(const RectangleStruct& viewRect) const
{
	if (auto pCell = MapClass::Instance->TryGetCellAt(Location))
	{
		CoordStruct coords =
		{
			(((pCell->MapCoords.X << 8) + 128) / 256) << 8,
			(((pCell->MapCoords.Y << 8) + 128) / 256) << 8,
			0
		};
		Point2D position;
		TacticalClass::Instance->CoordsToClient(coords, &position);
		position.X -= 30;

		std::swap(pCell->OverlayTypeIndex, const_cast<FoggedObject*>(this)->OverlayData.Overlay);
		std::swap(pCell->OverlayData, const_cast<FoggedObject*>(this)->OverlayData.OverlayData);
		pCell->DrawOverlay(position, viewRect);
		pCell->DrawOverlayShadow(position, viewRect);

		std::swap(pCell->OverlayTypeIndex, const_cast<FoggedObject*>(this)->OverlayData.Overlay);
		std::swap(pCell->OverlayData, const_cast<FoggedObject*>(this)->OverlayData.OverlayData);
	}
}

void FoggedObject::RenderAsTerrain(const RectangleStruct& viewRect) const
{
	auto pCell = MapClass::Instance->GetCellAt(Location);
	if (auto pSHP = TerrainData.Type->GetImage())
	{
		int nZAdjust = -TacticalClass::AdjustForZ(Location.Z);
		Point2D point;
		TacticalClass::Instance->CoordsToClient(Location, &point);
		point.X += DSurface::ViewBounds->X - viewRect.X;
		point.Y += DSurface::ViewBounds->Y - viewRect.Y;
		if (!pCell->LightConvert)
			pCell->InitLightConvert();

		BlitterFlags blitterFlag = BlitterFlags::Centered | BlitterFlags::bf_400 | BlitterFlags::Alpha;
		if (TerrainData.Flat)
			blitterFlag |= BlitterFlags::Flat;
		else
			blitterFlag |= BlitterFlags::ZReadWrite;

		ConvertClass* pConvert;
		int nIntensity;

		if (TerrainData.Type->SpawnsTiberium)
		{
			pConvert = FileSystem::GRFTXT_TIBERIUM_PAL;
			nIntensity = pCell->Intensity_Normal;
			point.Y -= 16;
		}
		else
		{
			pConvert = pCell->LightConvert;
			nIntensity = pCell->Intensity_Terrain;
		}

		CC_Draw_Shape(DSurface::Temp, pConvert, pSHP, TerrainData.Frame, &point,
			&viewRect, blitterFlag, 0, nZAdjust - 12, ZGradient::Deg90, nIntensity,
			0, 0, 0, 0, 0);
		if (Game::bDrawShadow)
			CC_Draw_Shape(DSurface::Temp, pConvert, pSHP, TerrainData.Frame + pSHP->Frames / 2, &point,
				&viewRect, blitterFlag | BlitterFlags::Darken, 0, nZAdjust - 3,
				ZGradient::Ground, 1000, 0, 0, 0, 0, 0);
	}
}

DEFINE_HOOK(0x67D32C, Put_All_FoggedObjects, 0x5)
{
	GET(IStream*, pStm, ESI);

	FoggedObject::SaveGlobal(pStm);

	return 0;
}

DEFINE_HOOK(0x67E826, Decode_All_FoggedObjects, 0x6)
{
	GET(IStream*, pStm, ESI);

	FoggedObject::LoadGlobal(pStm);

	return 0;
}

DEFINE_HOOK(0x685659, Clear_Scenario_FoggedObjects, 0xA)
{
	FoggedObject::FoggedObjects.Clear();

	return 0;
}