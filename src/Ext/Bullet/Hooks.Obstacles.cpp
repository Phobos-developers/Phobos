#include "Body.h"

#include <Utilities/Macro.h>

// Ares reimplements the bullet obstacle logic so need to get creative to add any new functionality for that in Phobos.
// Not named PhobosTrajectoryHelper to avoid confusion with actual custom trajectory logic.
class BulletObstacleHelper
{
public:

	static CellClass* GetObstacle(CellClass* pSourceCell, CellClass* pTargetCell, CellClass* pCurrentCell, CoordStruct currentCoords, AbstractClass const* const pSource,
		AbstractClass const* const pTarget, HouseClass* pOwner, BulletTypeClass* pBulletType, BulletTypeExt::ExtData*& pBulletTypeExt, bool isTargetingCheck = false)
	{
		CellClass* pObstacleCell = nullptr;

		if (SubjectToObstacles(pBulletType, pBulletTypeExt))
		{
			if (SubjectToTerrain(pCurrentCell, pBulletType, pBulletTypeExt, isTargetingCheck))
				pObstacleCell = pCurrentCell;
		}

		return pObstacleCell;
	}

	static CellClass* FindFirstObstacle(CoordStruct const& pSourceCoords, CoordStruct const& pTargetCoords, AbstractClass const* const pSource,
		AbstractClass const* const pTarget, HouseClass* pOwner, BulletTypeClass* pBulletType, bool isTargetingCheck = false)
	{
		BulletTypeExt::ExtData* pBulletTypeExt = BulletTypeExt::ExtMap.Find(pBulletType);

		if (SubjectToObstacles(pBulletType, pBulletTypeExt))
		{
			auto sourceCell = CellClass::Coord2Cell(pSourceCoords);
			auto const pSourceCell = MapClass::Instance->GetCellAt(sourceCell);
			auto targetCell = CellClass::Coord2Cell(pTargetCoords);
			auto const pTargetCell = MapClass::Instance->GetCellAt(targetCell);

			auto const sub = sourceCell - targetCell;
			auto const delta = CellStruct { (short)std::abs(sub.X), (short)std::abs(sub.Y) };
			auto const maxDelta = static_cast<size_t>(std::max(delta.X, delta.Y));
			auto const step = !maxDelta ? CoordStruct::Empty : (pTargetCoords - pSourceCoords) * (1.0 / maxDelta);
			CoordStruct crdCur = pSourceCoords;
			auto pCellCur = pSourceCell;

			for (size_t i = 0; i < maxDelta + isTargetingCheck; ++i)
			{
				if (auto const pCell = GetObstacle(pSourceCell, pTargetCell, pCellCur, crdCur, pSource, pTarget, pOwner, pBulletType, pBulletTypeExt, isTargetingCheck))
					return pCell;

				crdCur += step;
				pCellCur = MapClass::Instance->GetCellAt(crdCur);
			}
		}

		return nullptr;
	}

	static CellClass* FindFirstImpenetrableObstacle(CoordStruct const& pSourceCoords, CoordStruct const& pTargetCoords, AbstractClass const* const pSource,
		AbstractClass const* const pTarget, HouseClass* pOwner, WeaponTypeClass* pWeapon, bool isTargetingCheck = false)
	{
		// Does not currently need further checks.
		return FindFirstObstacle(pSourceCoords, pTargetCoords, pSource, pTarget, pOwner, pWeapon->Projectile, isTargetingCheck);
	}

	static bool SubjectToObstacles(BulletTypeClass* pBulletType, BulletTypeExt::ExtData*& pBulletTypeExt)
	{
		bool subjectToTerrain = pBulletTypeExt->SubjectToLand.isset() || pBulletTypeExt->SubjectToWater.isset();

		return subjectToTerrain ? true : pBulletType->Level;
	}

	static bool SubjectToTerrain(CellClass* pCurrentCell, BulletTypeClass* pBulletType, BulletTypeExt::ExtData*& pBulletTypeExt, bool isTargetingCheck)
	{
		bool isCellWater = pCurrentCell->LandType == LandType::Water || pCurrentCell->LandType == LandType::Beach;
		bool isLevel = pBulletType->Level ? pCurrentCell->IsOnFloor() : false;

		if (isLevel && !pBulletTypeExt->SubjectToLand.isset() && !pBulletTypeExt->SubjectToWater.isset())
			return true;
		else if (!isCellWater && pBulletTypeExt->SubjectToLand.Get(false))
			return !isTargetingCheck ? pBulletTypeExt->SubjectToLand_Detonate : true;
		else if (isCellWater && pBulletTypeExt->SubjectToWater.Get(false))
			return !isTargetingCheck ? pBulletTypeExt->SubjectToWater_Detonate : true;

		return false;
	}
};

// Hooks

DEFINE_HOOK(0x4688A9, BulletClass_Unlimbo_Obstacles, 0x6)
{
	enum { SkipGameCode = 0x468A3F, Continue = 0x4688BD };

	GET(BulletClass*, pThis, EBX);
	GET(CoordStruct const* const, sourceCoords, EDI);
	REF_STACK(CoordStruct const, targetCoords, STACK_OFFSET(0x54, -0x10));

	if (pThis->Type->Inviso)
	{
		auto const pOwner = pThis->Owner ? pThis->Owner->Owner : BulletExt::ExtMap.Find(pThis)->FirerHouse;
		const auto pObstacleCell = BulletObstacleHelper::FindFirstObstacle(*sourceCoords, targetCoords, pThis->Owner, pThis->Target, pOwner, pThis->Type, false);

		if (pObstacleCell)
		{
			pThis->SetLocation(pObstacleCell->GetCoords());
			pThis->Speed = 0;
			pThis->Velocity = BulletVelocity::Empty;

			return SkipGameCode;
		}

		return Continue;
	}

	return 0;
}

DEFINE_HOOK(0x468C86, BulletClass_ShouldExplode_Obstacles, 0xA)
{
	enum { SkipGameCode = 0x468C90, Explode = 0x468C9F };

	GET(BulletClass*, pThis, ESI);

	BulletTypeExt::ExtData* pBulletTypeExt = BulletTypeExt::ExtMap.Find(pThis->Type);

	if (BulletObstacleHelper::SubjectToObstacles(pThis->Type, pBulletTypeExt))
	{
		auto const pCellSource = MapClass::Instance->GetCellAt(pThis->SourceCoords);
		auto const pCellTarget = MapClass::Instance->GetCellAt(pThis->TargetCoords);
		auto const pCellCurrent = MapClass::Instance->GetCellAt(pThis->LastMapCoords);
		auto const pOwner = pThis->Owner ? pThis->Owner->Owner : BulletExt::ExtMap.Find(pThis)->FirerHouse;
		const auto pObstacleCell = BulletObstacleHelper::GetObstacle(pCellSource, pCellTarget, pCellCurrent, pThis->Location, pThis->Owner, pThis->Target, pOwner, pThis->Type, pBulletTypeExt, false);

		if (pObstacleCell)
			return Explode;
	}


	// Restore overridden instructions.
	R->EAX(pThis->GetHeight());
	return SkipGameCode;
}

namespace InRangeTemp
{
	TechnoClass* Techno = nullptr;
}

DEFINE_HOOK(0x6F7261, TechnoClass_InRange_SetContext, 0x5)
{
	GET(TechnoClass*, pThis, ESI);

	InRangeTemp::Techno = pThis;

	return 0;
}

DEFINE_HOOK(0x6F7647, TechnoClass_InRange_Obstacles, 0x5)
{
	GET_BASE(WeaponTypeClass*, pWeapon, 0x10);
	GET(CoordStruct const* const, pSourceCoords, ESI);
	REF_STACK(CoordStruct const, targetCoords, STACK_OFFSET(0x3C, -0x1C));
	GET_BASE(AbstractClass* const, pTarget, 0xC);
	GET(CellClass*, pResult, EAX);

	auto pObstacleCell = pResult;
	auto pTechno = InRangeTemp::Techno;

	if (!pObstacleCell)
		pObstacleCell = BulletObstacleHelper::FindFirstImpenetrableObstacle(*pSourceCoords, targetCoords, pTechno, pTarget, pTechno->Owner, pWeapon, true);

	InRangeTemp::Techno = nullptr;

	R->EAX(pObstacleCell);
	return 0;
}
