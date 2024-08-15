#include <AlphaShapeClass.h>
#include <AnimClass.h>
#include <BuildingClass.h>
#include <CellClass.h>
#include <FootClass.h>
#include <TacticalClass.h>
#include <TechnoClass.h>

#include <Ext/Building/Body.h>

#include <Utilities/Macro.h>
#include <Utilities/AresHelper.h>
#include <Utilities/TemplateDef.h>

DEFINE_PATCH(0x5F3E70, 0x83, 0xEC, 0x10, 0x55, 0x56) // Disbale Ares::ObjectClass_Update_AlphaLight

void UpdateAlphaShape(ObjectClass* pSource)
{
	if (!pSource || !pSource->IsAlive)
		return;

	const auto pSourceType = pSource->GetType();

	if (!pSourceType)
		return;

	const auto pImage = pSourceType->AlphaImage;

	if (!pImage)
		return;

	// for animations attached to the owner object, consider
	// the owner object as source, so the display is refreshed
	// whenever the owner object moves.
	auto pOwner = pSource;
	const auto pAnim = abstract_cast<AnimClass*>(pSource);

	if (pAnim && pAnim->OwnerObject)
		pOwner = pAnim->OwnerObject;

	const Point2D* tacticalPos = &TacticalClass::Instance->TacticalPos;
	const Point2D off = { tacticalPos->X - ((pImage->Width + 1) / 2), tacticalPos->Y - ((pImage->Height + 1) / 2) };

	if (const auto pFoot = abstract_cast<FootClass*>(pOwner))
	{
		if (pFoot->LastMapCoords != pFoot->CurrentMapCoords)
		{
			// we moved - need to redraw the area we were in
			// alas, we don't have the precise XYZ we were in, only the cell we were last seen in
			// so we need to add the cell's dimensions to the dirty area just in case
			CoordStruct lastCellCoords = CellClass::Cell2Coord(pFoot->LastMapCoords);
			const auto xyTL = TacticalClass::Instance->CoordsToClient(lastCellCoords);
			// because the coord systems are different - xyz is x/, y\, xy is x-, y|
			lastCellCoords.X += 256;
			lastCellCoords.Y += 256;
			const auto xyBR = TacticalClass::Instance->CoordsToClient(lastCellCoords);
			const auto cellDimensions = xyBR.first - xyTL.first;
			Point2D point = xyTL.first;
			point.X += cellDimensions.X / 2;
			point.Y += cellDimensions.Y / 2;
			point += off;

			RectangleStruct dirty = { point.X - tacticalPos->X - cellDimensions.X,
				point.Y - tacticalPos->Y - cellDimensions.Y,
				pImage->Width + cellDimensions.X * 2,
				pImage->Height + cellDimensions.Y * 2 };
			TacticalClass::Instance->RegisterDirtyArea(dirty, true);
		}
	}

	bool inactive = pSource->InLimbo;

	if (const auto pTechno = abstract_cast<TechnoClass*>(pSource))
	{
		inactive |= pTechno->Deactivated || pTechno->CloakState == CloakState::Cloaked || pTechno->GetHeight() < -10;

		if (!inactive && pTechno->IsDisguised())
		{
			const auto pDisguise = pTechno->GetDisguise(true);
			inactive |= pDisguise && pDisguise->WhatAmI() == AbstractType::TerrainType;
		}
	}

	const auto pBuilding = abstract_cast<BuildingClass*>(pSource);

	if (pBuilding)
	{
		const auto currMission = pBuilding->GetCurrentMission();

		if (currMission != Mission::Construction && currMission != Mission::Selling)
			inactive |= !pBuilding->IsPowerOnline() || BuildingExt::ExtMap.Find(pBuilding)->LimboID != -1;
	}

	auto& alphaExt = Make_Global<PhobosMap<ObjectClass*, AlphaShapeClass*>>(AresHelper::AresFunctionOffsetsFinal["TechnoExt_AlphaExt"]);

	if (inactive)
	{
		if (auto pAlpha = alphaExt.get_or_default(pSource))
			GameDelete(pAlpha);

		return;
	}

	if (Unsorted::CurrentFrame % 2) // lag reduction - don't draw a new alpha every frame
	{
		if (alphaExt.get_or_default(pSource) && pBuilding && (pImage->Frames <= 1 || !pBuilding->HasTurret() || !pBuilding->TurretIsRotating))
			return;

		Point2D point = TacticalClass::Instance->CoordsToClient(pSource->GetCoords()).first;
		point += off;

		++Unsorted::IKnowWhatImDoing;
		GameCreate<AlphaShapeClass>(pSource, point.X, point.Y);
		--Unsorted::IKnowWhatImDoing;
		//int Margin = 40;
		RectangleStruct Dirty = { point.X - tacticalPos->X, point.Y - tacticalPos->Y, pImage->Width, pImage->Height };
		TacticalClass::Instance->RegisterDirtyArea(Dirty, true);
	}
}

DEFINE_HOOK(0x5F3E78, ObjectClass_AI_UpdateAlphaShape, 0x6)
{
	GET(ObjectClass*, pThis, ESI);

	if (AresHelper::CanUseAres)
		UpdateAlphaShape(pThis);

	return 0;
}
