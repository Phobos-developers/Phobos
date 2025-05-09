#include <AlphaShapeClass.h>
#include <TacticalClass.h>

#include <Ext/Building/Body.h>
#include <Utilities/AresFunctions.h>

DEFINE_PATCH(0x5F3E70, 0x83, 0xEC, 0x10, 0x55, 0x56) // Disbale Ares::ObjectClass_Update_AlphaLight

static void __fastcall UpdateAlphaShape(ObjectClass* pSource)
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
	const auto pAnim = abstract_cast<AnimClass*, true>(pSource);

	if (pAnim && pAnim->OwnerObject)
		pOwner = pAnim->OwnerObject;

	const Point2D* tacticalPos = &TacticalClass::Instance->TacticalPos;
	const Point2D off = { tacticalPos->X - ((pImage->Width + 1) / 2), tacticalPos->Y - ((pImage->Height + 1) / 2) };

	if (const auto pFoot = abstract_cast<FootClass*, true>(pOwner))
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

	if (const auto pTechno = abstract_cast<TechnoClass*, true>(pSource))
	{
		inactive |= pTechno->Deactivated || pTechno->CloakState == CloakState::Cloaked || pTechno->GetHeight() < -10;

		if (!inactive && pTechno->IsDisguised())
		{
			const auto pDisguise = pTechno->GetDisguise(true);
			inactive |= pDisguise && pDisguise->WhatAmI() == AbstractType::TerrainType;
		}
	}

	const auto pBuilding = abstract_cast<BuildingClass*, true>(pSource);

	if (pBuilding)
	{
		const auto currMission = pBuilding->GetCurrentMission();

		if (currMission != Mission::Construction && currMission != Mission::Selling)
			inactive |= !pBuilding->IsPowerOnline() || BuildingExt::ExtMap.Find(pBuilding)->LimboID != -1;
	}

	auto& alphaExt = *AresFunctions::AlphaExtMap;

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

		++Unsorted::ScenarioInit;
		GameCreate<AlphaShapeClass>(pSource, point.X, point.Y);
		--Unsorted::ScenarioInit;
		//int Margin = 40;
		RectangleStruct Dirty = { point.X - tacticalPos->X, point.Y - tacticalPos->Y, pImage->Width, pImage->Height };
		TacticalClass::Instance->RegisterDirtyArea(Dirty, true);
	}
}

DEFINE_HOOK(0x5F3E78, ObjectClass_AI_UpdateAlphaShape, 0x6)
{
	if (AresFunctions::AlphaExtMap)
	{
		GET(ObjectClass*, pThis, ESI);
		UpdateAlphaShape(pThis);
	}

	return 0;
}
