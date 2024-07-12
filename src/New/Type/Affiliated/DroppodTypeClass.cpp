#include "DroppodTypeClass.h"
#include <DropPodLocomotionClass.h>

#include <Ext/Anim/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/WeaponType/Body.h>

void DroppodTypeClass::LoadFromINI(CCINIClass* pINI, const char* pSection)
{
	INI_EX exINI(pINI);
	this->Angle.Read(exINI, pSection, "DropPod.Angle");
	this->AtmosphereEntry.Read(exINI, pSection, "DropPod.AtmosphereEntry");
	ValueableVector<AnimTypeClass*> anims;
	anims.Read(exINI, pSection, "DropPod.GroundAnim");
	if (!anims.empty())
	{
		this->GroundAnim[0] = anims[0];
		this->GroundAnim[1] = anims[anims.size() > 1];
	}
	this->AirImage.Read(exINI, pSection, "DropPod.AirImage");
	this->Height.Read(exINI, pSection, "DropPod.Height");
	this->Puff.Read(exINI, pSection, "DropPod.Puff");
	this->Speed.Read(exINI, pSection, "DropPod.Speed");
	this->Trailer.Read(exINI, pSection, "DropPod.Trailer");
	this->Trailer_SpawnDelay.Read(exINI, pSection, "DropPod.Trailer.SpawnDelay");
	this->Trailer_Attached.Read(exINI, pSection, "DropPod.Trailer.Attached");
	this->Weapon.Read<true>(exINI, pSection, "DropPod.Weapon");
	this->Weapon_HitLandOnly.Read(exINI, pSection, "DropPod.Weapon.HitLandOnly");
}

#pragma region(save/load)

template <class T>
bool DroppodTypeClass::Serialize(T& stm)
{
	return stm
		.Process(this->Angle)
		.Process(this->GroundAnim[0])
		.Process(this->GroundAnim[1])
		.Process(this->AtmosphereEntry)
		.Process(this->Height)
		.Process(this->Puff)
		.Process(this->Speed)
		.Process(this->Trailer)
		.Process(this->Trailer_SpawnDelay)
		.Process(this->Trailer_Attached)
		.Process(this->Weapon)
		.Process(this->Weapon_HitLandOnly)
		.Process(this->AirImage)
		.Success();
}

bool DroppodTypeClass::Load(PhobosStreamReader& stm, bool registerForChange)
{
	return this->Serialize(stm);
}

bool DroppodTypeClass::Save(PhobosStreamWriter& stm) const
{
	return const_cast<DroppodTypeClass*>(this)->Serialize(stm);
}

#pragma endregion(save/load)

// DropPod loco is so far the easiest loco to rewrite completely

DEFINE_HOOK(0x4B5B70, DroppodLocomotionClass_ILoco_Process, 0x5)
{
	GET_STACK(ILocomotion*, iloco, 0x4);
	__assume(iloco != nullptr);
	auto const lThis = static_cast<DropPodLocomotionClass*>(iloco);
	auto const pLinked = lThis->LinkedTo;
	auto const linkedExt = TechnoExt::ExtMap.Find(pLinked);
	const auto podType = linkedExt->TypeExtData->DroppodType.get();

	if (!podType)
		return 0;//You're not welcome

	CoordStruct oldLoc = pLinked->Location;

	const double angle = podType->Angle.Get(RulesClass::Instance->DropPodAngle);
	// exponentially decelerate
	int speed = std::max(podType->Speed.Get(RulesClass::Instance->DropPodSpeed), pLinked->GetHeight() / 10 + 2);

	CoordStruct coords = pLinked->Location;
	coords.X += int(Math::cos(angle) * speed * (lThis->OutOfMap ? 1 : -1));
	coords.Z -= int(Math::sin(angle) * speed);

	auto dWpn = podType->Weapon.Get(RulesClass::Instance->DropPodWeapon);

	if (pLinked->GetHeight() > 0)
	{
		pLinked->SetLocation(coords);
		const int timeSinceCreation = Unsorted::CurrentFrame - pLinked->ParalysisTimer.StartTime;//or anything

		if (timeSinceCreation % podType->Trailer_SpawnDelay == 1)
		{
			if (auto trailerType = podType->Trailer.Get(RulesExt::Global()->DropPodTrailer))
			{
				auto trailer = GameCreate<AnimClass>(trailerType, oldLoc);
				trailer->Owner = pLinked->Owner;
				if (podType->Trailer_Attached)
					trailer->SetOwnerObject(pLinked);
			}
		}

		if (dWpn && !podType->Weapon_HitLandOnly)
		{
			if (timeSinceCreation % std::max(dWpn->ROF, 3) == 1)
			{
				auto cell = MapClass::Instance->GetCellAt(lThis->DestinationCoords);
				auto techno = cell->FindTechnoNearestTo({ 0,0 }, false);
				if (!pLinked->Owner->IsAlliedWith(techno))
				{
					// really? not firing for real?
					auto locnear = MapClass::GetRandomCoordsNear(lThis->DestinationCoords, 85, false);
					if (int count = dWpn->Report.Count)
						VocClass::PlayAt(dWpn->Report[Randomizer::Global->Random() % count], coords);
					MapClass::DamageArea(locnear, 2 * dWpn->Damage, pLinked, dWpn->Warhead, true, pLinked->Owner);
					if (auto dmgAnim = MapClass::SelectDamageAnimation(2 * dWpn->Damage, dWpn->Warhead, LandType::Clear, locnear))
						GameCreate<AnimClass>(dmgAnim, locnear, 0, 1, 0x2600, -15, 0)->Owner = pLinked->Owner;
				}
			}
		}
	}
	else
	{
		pLinked->SetHeight(0);
		pLinked->Limbo();
		lThis->AddRef();
		lThis->End_Piggyback(&pLinked->Locomotor);

		if (pLinked->Unlimbo(pLinked->Location, DirType::North))
		{
			if (auto puff = podType->Puff.Get(RulesClass::Instance->DropPodPuff))
				GameCreate<AnimClass>(puff, pLinked->Location)->Owner = pLinked->Owner;

			if (auto podAnim = podType->GroundAnim[lThis->OutOfMap].Get(RulesClass::Instance->DropPod[lThis->OutOfMap]))
				GameCreate<AnimClass>(podAnim, pLinked->Location)->Owner = pLinked->Owner;

			if (dWpn && podType->Weapon_HitLandOnly)
				WeaponTypeExt::DetonateAt(dWpn, pLinked->Location, pLinked, pLinked->Owner);

			auto& vec = linkedExt->LaserTrails;
			if (!vec.empty())
				vec.erase(std::remove_if(vec.begin(), vec.end(), [](auto& trail) { return trail.Type->DroppodOnly; }));

			pLinked->Mark(MarkType::Down);
			pLinked->SetHeight(0);
			pLinked->EnterIdleMode(false, true);
			pLinked->NextMission();
			pLinked->Scatter(CoordStruct::Empty, false, false);
		}
		else
		{
			MapClass::DamageArea(coords, 100, pLinked, RulesClass::Instance->C4Warhead, true, pLinked->Owner);
			if (auto dmgAnim = MapClass::SelectDamageAnimation(100, RulesClass::Instance->C4Warhead, LandType::Clear, coords))
				GameCreate<AnimClass>(dmgAnim, coords, 0, 1, 0x2600, -15, 0)->Owner = pLinked->Owner;
		}
		lThis->Release();
	}


	R->AL(true);
	return 0x4B6036;
}

DEFINE_HOOK(0x4B607D, DroppodLocomotionClass_ILoco_MoveTo, 0x8)
{
	GET(ILocomotion*, iloco, EDI);
	REF_STACK(CoordStruct, to, STACK_OFFSET(0x1C, 0x8));
	__assume(iloco != nullptr);

	auto const lThis = static_cast<DropPodLocomotionClass*>(iloco);
	auto const pLinked = lThis->LinkedTo;
	const auto podType= TechnoTypeExt::ExtMap.Find(pLinked->GetTechnoType())->DroppodType.get();

	if (!podType)
		return 0;

	lThis->DestinationCoords = to;
	lThis->DestinationCoords.Z = MapClass::Instance->GetCellFloorHeight(to);

	const int height = podType->Height.Get(RulesClass::Instance->DropPodHeight);
	const double angle = podType->Angle.Get(RulesClass::Instance->DropPodAngle);

	lThis->OutOfMap = !MapClass::Instance->IsWithinUsableArea(to);
	to.X -= (lThis->OutOfMap ? 1 : -1) * int(height / Math::tan(angle));
	to.Z += height;

	pLinked->SetLocation(to);

	if (pLinked->Unlimbo(to, DirType::South))
	{
		pLinked->PrimaryFacing.SetCurrent(DirStruct { DirType::South });
		if (auto entryAnim = podType->AtmosphereEntry.Get(RulesClass::Instance->AtmosphereEntry))
			GameCreate<AnimClass>(entryAnim, to)->Owner = pLinked->Owner;
	}

	return 0x4B61F0;
}

DEFINE_HOOK(0x519168, InfantryClass_Draw_Droppod, 0x5)
{
	GET(InfantryClass*, pThis, EBP);
	REF_STACK(SHPStruct*, shp, STACK_OFFSET(0x54, -0x34));

	auto const podType = TechnoTypeExt::ExtMap.Find(pThis->Type)->DroppodType.get();
	shp = podType->AirImage.Get(RulesExt::Global()->PodImage);

	return 0x519176;
}
