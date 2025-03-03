#include <AlphaShapeClass.h>
#include <TacticalClass.h>

#include <Ext/Building/Body.h>
#include <Utilities/AresFunctions.h>

DEFINE_PATCH(0x5F3E70, 0x83, 0xEC, 0x10, 0x55, 0x56) // Disbale Ares::ObjectClass_Update_AlphaLight

static void __fastcall UpdateAlphaShape(ObjectClass* pSource)
{
	const auto pSourceType = pSource->GetType();

	if (!pSourceType)
		return;

	auto pImage = pSourceType->AlphaImage;

	if (!pImage)
		return;

	const Point2D* tacticalPos = &TacticalClass::Instance->TacticalPos;
	const Point2D off = { tacticalPos->X - (pImage->Width / 2), tacticalPos->Y - (pImage->Height / 2) };

	// for animations attached to the owner object, consider
	// the owner object as source, so the display is refreshed
	// whenever the owner object moves.
	auto pOwner = pSource;
	const auto pAnim = abstract_cast<AnimClass*>(pSource);

	if (pAnim && pAnim->OwnerObject)
		pOwner = pAnim->OwnerObject;

	if (const auto pFoot = abstract_cast<FootClass*>(pOwner))
	{
		if (pFoot->LastMapCoords != pFoot->CurrentMapCoords)
		{
			// we moved - need to redraw the area we were in
			// alas, we don't have the precise XYZ we were in, only the cell we were last seen in
			// so we need to add the cell's dimensions to the dirty area just in case
			CoordStruct lastCellCoords = CellClass::Cell2Coord(pFoot->LastMapCoords);
			const auto xyTL = TacticalClass::Instance->CoordsToClient(lastCellCoords).first;
			// because the coord systems are different - xyz is x/, y\, xy is x-, y|
			lastCellCoords.X += 256;
			lastCellCoords.Y += 256;
			const auto xyBR = TacticalClass::Instance->CoordsToClient(lastCellCoords).first;
			const auto cellDimensions = xyBR - xyTL;
			Point2D point = xyTL;
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
			if (const auto pDisguise = pTechno->GetDisguise(true))
			{
				if (pDisguise->AlphaImage)
					pImage = pDisguise->AlphaImage;
				else
					inactive = true;
			}
		}
	}

	const auto pBuilding = abstract_cast<BuildingClass*>(pSource);

	if (pBuilding && !inactive)
	{
		if (pBuilding->GetCurrentMission() != Mission::Selling)
			inactive |= !pBuilding->IsPowerOnline() || BuildingExt::ExtMap.Find(pBuilding)->LimboID != -1;
	}

	auto& alphaExt = *AresFunctions::AlphaExtMap;

	if (inactive)
	{
		if (auto pAlpha = alphaExt.get_or_default(pSource))
		{
			GameDelete(std::exchange(pAlpha, nullptr));
			// pSource is erased from map
		}
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
		RectangleStruct dirty = { point.X - tacticalPos->X, point.Y - tacticalPos->Y, pImage->Width, pImage->Height };
		TacticalClass::Instance->RegisterDirtyArea(dirty, true);
	}
}

DEFINE_HOOK(0x5F3E78, ObjectClass_AI_UpdateAlphaShape, 0x6)
{
	GET(ObjectClass*, pThis, ESI);

	UpdateAlphaShape(pThis);

	return 0;
}
