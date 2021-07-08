#include "ShieldClass.h"

#include <Ext/Rules/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/WarheadType/Body.h>

#include <Utilities/GeneralUtils.h>

#include <AnimClass.h>
#include <HouseClass.h>
#include <RadarEventClass.h>

ShieldClass::ShieldClass() :
	Techno { nullptr },
	HP { 0 },
	Timers { }
{ }

ShieldClass::ShieldClass(TechnoClass* pTechno) :
	Techno { pTechno },
	IdleAnim { nullptr },
	Timers { },
	Cloak { false },
	Online { true },
	Temporal { false },
	Available { true }
{
	this->UpdateType();
	this->HP = this->Type->Strength;
	strcpy(this->TechnoID, this->Techno->get_ID());
}

void ShieldClass::UpdateType()
{
	this->Type = TechnoTypeExt::ExtMap.Find(this->Techno->GetTechnoType())->ShieldType;
}

template <typename T>
bool ShieldClass::Serialize(T& Stm)
{
	return Stm
		.Process(this->Techno)
		.Process(this->TechnoID)
		.Process(this->IdleAnim)
		.Process(this->Timers.SelfHealing)
		.Process(this->Timers.Respawn)
		.Process(this->HP)
		.Process(this->Cloak)
		.Process(this->Online)
		.Process(this->Temporal)
		.Process(this->Available)
		.Success();
}

bool ShieldClass::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	return Serialize(Stm);
}

bool ShieldClass::Save(PhobosStreamWriter& Stm) const
{
	return const_cast<ShieldClass*>(this)->Serialize(Stm);
}

// Is used for DeploysInto/UndeploysInto
void ShieldClass::SyncShieldToAnother(TechnoClass* pFrom, TechnoClass* pTo)
{
	const auto pFromExt = TechnoExt::ExtMap.Find(pFrom);
	const auto pToExt = TechnoExt::ExtMap.Find(pTo);

	if (pFromExt->Shield)
	{
		pToExt->Shield = std::make_unique<ShieldClass>(pTo);
		strcpy(pToExt->Shield->TechnoID, pFromExt->Shield->TechnoID);
		pToExt->Shield->Available = pFromExt->Shield->Available;
		pToExt->Shield->HP = pFromExt->Shield->HP;
	}

	if (pFrom->WhatAmI() == AbstractType::Building && pFromExt->Shield)
		pFromExt->Shield = nullptr;
}

int ShieldClass::ReceiveDamage(args_ReceiveDamage* args)
{
	const auto pWHExt = WarheadTypeExt::ExtMap.Find(args->WH);

	if (!this->HP || *args->Damage == 0 || this->Techno->IsIronCurtained() || pWHExt->PenetratesShield)
		return *args->Damage;

	int nDamage = 0;

	if (pWHExt->CanTargetHouse(args->SourceHouse, this->Techno) && !args->WH->Temporal)
	{
		if (*args->Damage > 0)
			nDamage = MapClass::GetTotalDamage(*args->Damage, args->WH, this->Type->Armor, args->DistanceToEpicenter);
		else
			nDamage = -MapClass::GetTotalDamage(-*args->Damage, args->WH, this->Type->Armor, args->DistanceToEpicenter);
	}

	if (nDamage > 0)
	{
		this->Timers.SelfHealing.Start(this->Type->SelfHealing_Rate); // when attacked, restart the timer
		this->ResponseAttack();

		int residueDamage = nDamage - this->HP;
		if (residueDamage >= 0 || pWHExt->BreaksShield)
		{

			if (pWHExt->BreaksShield && residueDamage < 0)
				residueDamage = 0;

			residueDamage = int((double)residueDamage /
				GeneralUtils::GetWarheadVersusArmor(args->WH, this->Type->Armor)); //only absord percentage damage

			this->BreakShield();
			return this->Type->AbsorbOverDamage ? 0 : residueDamage;
		}
		else
		{
			this->WeaponNullifyAnim();
			this->HP = -residueDamage;
			return 0;
		}
	}
	else if (nDamage < 0)
	{
		const int nLostHP = this->Type->Strength - this->HP;
		if (!nLostHP)
		{
			int result = *args->Damage;
			const int armor = static_cast<int>(this->Techno->GetTechnoType()->Armor);
			if (result * GeneralUtils::GetWarheadVersusArmor(args->WH, armor) > 0)
				result = 0;

			return result;
		}

		const int nRemainLostHP = nLostHP + nDamage;
		if (nRemainLostHP < 0)
			this->HP = this->Type->Strength;
		else
			this->HP -= nDamage;

		return 0;
	}

	// else if (nDamage == 0)
	return 0;
}

void ShieldClass::ResponseAttack()
{
	if (this->Techno->Owner != HouseClass::Player)
		return;

	if (this->Techno->WhatAmI() == AbstractType::Building)
	{
		const auto pBld = abstract_cast<BuildingClass*>(this->Techno);
		this->Techno->Owner->BuildingUnderAttack(pBld);
	}
	else if (this->Techno->WhatAmI() == AbstractType::Unit)
	{
		const auto pUnit = abstract_cast<UnitClass*>(this->Techno);
		if (pUnit->Type->Harvester)
		{
			const auto pPos = pUnit->GetDestination(pUnit);
			if (RadarEventClass::Create(RadarEventType::HarvesterAttacked, { (short)pPos.X / 256,(short)pPos.Y / 256 }))
				VoxClass::Play("EVA_OreMinerUnderAttack");
		}
	}
}

void ShieldClass::WeaponNullifyAnim()
{
	if (this->Type->HitAnim.isset())
		GameCreate<AnimClass>(this->Type->HitAnim, this->Techno->GetCoords());
}

bool ShieldClass::CanBeTargeted(WeaponTypeClass* pWeapon)
{
	const auto pWHExt = WarheadTypeExt::ExtMap.Find(pWeapon->Warhead);

	if ((pWHExt && pWHExt->PenetratesShield.Get()) || !this->HP)
		return true;

	return GeneralUtils::GetWarheadVersusArmor(pWeapon->Warhead, this->Type->Armor) != 0.0;
}

void ShieldClass::AI_Temporal()
{
	if (!this->Temporal)
	{
		this->Temporal = true;

		const auto timer = (this->HP <= 0) ? &this->Timers.Respawn : &this->Timers.SelfHealing;
		timer->Pause();

		this->CloakCheck();

		if (this->IdleAnim)
		{
			switch (this->Type->IdleAnim_TemporalAction)
			{
			case AttachedAnimFlag::Hides:
				this->KillAnim();
				break;

			case AttachedAnimFlag::Temporal:
				this->IdleAnim->UnderTemporal = true;
				break;

			case AttachedAnimFlag::Paused:
				this->IdleAnim->Pause();
				break;

			case AttachedAnimFlag::PausedTemporal:
				this->IdleAnim->Pause();
				this->IdleAnim->UnderTemporal = true;
				break;
			}
		}
	}
}

void ShieldClass::AI()
{
	if (!this->Techno || this->Techno->InLimbo || this->Techno->IsImmobilized || this->Techno->Transporter)
		return;

	if (this->Techno->Health <= 0 || !this->Techno->IsAlive)
	{
		if (auto pTechnoExt = TechnoExt::ExtMap.Find(this->Techno))
		{
			pTechnoExt->Shield = nullptr;
			return;
		}
	}

	this->UpdateType();
	this->CloakCheck();
	this->ConvertCheck();

	if (!this->Available)
		return;

	this->TemporalCheck();
	if (this->Temporal)
		return;

	this->OnlineCheck();
	this->RespawnShield();
	this->SelfHealing();

	if (!this->Cloak && !this->Temporal && this->Online && (this->HP > 0 && this->Techno->Health > 0))
		this->CreateAnim();
}

// The animation is automatically destroyed when the associated unit receives the isCloak statute.
// Therefore, we must zero out the invalid pointer
void ShieldClass::CloakCheck()
{
	const auto cloakState = this->Techno->CloakState;
	this->Cloak = cloakState == CloakState::Cloaked || cloakState == CloakState::Cloaking;

	if (this->Cloak)
		this->IdleAnim = nullptr;
}

void ShieldClass::OnlineCheck()
{
	if (!this->Type->Powered)
		return;

	const auto timer = (this->HP <= 0) ? &this->Timers.Respawn : &this->Timers.SelfHealing;

	auto pTechno = this->Techno;
	bool isActive = !(pTechno->Deactivated || pTechno->IsUnderEMP());

	if (isActive && this->Techno->WhatAmI() == AbstractType::Building)
	{
		auto const pBuilding = static_cast<BuildingClass const*>(this->Techno);
		isActive = pBuilding->IsPowerOnline();
	}

	if (!isActive)
	{
		this->Online = false;
		timer->Pause();

		if (this->IdleAnim)
		{
			switch (this->Type->IdleAnim_OfflineAction)
			{
			case AttachedAnimFlag::Hides:
				this->KillAnim();
				break;

			case AttachedAnimFlag::Temporal:
				this->IdleAnim->UnderTemporal = true;
				break;

			case AttachedAnimFlag::Paused:
				this->IdleAnim->Pause();
				break;

			case AttachedAnimFlag::PausedTemporal:
				this->IdleAnim->Pause();
				this->IdleAnim->UnderTemporal = true;
				break;
			}
		}
	}
	else
	{
		this->Online = true;
		timer->Resume();

		if (this->IdleAnim)
		{
			this->IdleAnim->UnderTemporal = false;
			this->IdleAnim->Unpause();
		}
	}
}

void ShieldClass::TemporalCheck()
{
	if (!this->Temporal)
		return;

	this->Temporal = false;

	const auto timer = (this->HP <= 0) ? &this->Timers.Respawn : &this->Timers.SelfHealing;
	timer->Resume();

	if (this->IdleAnim)
	{
		this->IdleAnim->UnderTemporal = false;
		this->IdleAnim->Unpause();
	}
}

// Is used for DeploysInto/UndeploysInto and DeploysInto/UndeploysInto
void ShieldClass::ConvertCheck()
{
	const auto newID = this->Techno->get_ID();

	if (strcmp(this->TechnoID, newID) == 0)
		return;

	const auto pType = this->Type;

	if (pType->Strength && this->Available)
	{ // Update this shield for the new owner
		const auto pFrom_TechnoType = TechnoTypeClass::Find(this->TechnoID);
		const auto pFromType = TechnoTypeExt::ExtMap.Find(pFrom_TechnoType)->ShieldType;

		if (pFromType->IdleAnim.Get() != pType->IdleAnim.Get())
			this->KillAnim();

		this->HP = (int)round(
			(double)this->HP /
			(double)pFromType->Strength *
			(double)pType->Strength
		);
	}
	else
	{
		const auto timer = (this->HP <= 0) ? &this->Timers.Respawn : &this->Timers.SelfHealing;
		if (pType->Strength && !this->Available)
		{ // Resume this shield when became Available
			timer->Resume();
			this->Available = true;
		}
		else if (this->Available)
		{ // Pause this shield when became unAvailable
			timer->Pause();
			this->Available = false;
			this->KillAnim();
		}
	}

	strcpy(this->TechnoID, newID);
}

void ShieldClass::SelfHealing()
{
	const auto pType = this->Type;
	const auto timer = &this->Timers.SelfHealing;
	const auto percentageAmount = this->GetPercentageAmount(pType->SelfHealing);
	if (percentageAmount > 0)
	{
		if (this->HP < pType->Strength && timer->StartTime == -1)
			timer->Start(pType->SelfHealing_Rate);

		if (this->HP > 0 && timer->Completed())
		{
			timer->Start(pType->SelfHealing_Rate);
			this->HP += percentageAmount;

			if (this->HP > pType->Strength)
			{
				this->HP = pType->Strength;
				timer->Stop();
			}
		}
	}
}

int ShieldClass::GetPercentageAmount(double iStatus)
{
	if (iStatus == 0)
		return 0;

	if (iStatus >= -1.0 && iStatus <= 1.0)
		return (int)round(this->Type->Strength * iStatus);

	return (int)trunc(iStatus);
}

void ShieldClass::InvalidatePointer(void* ptr)
{
	if (this->IdleAnim == ptr)
		this->KillAnim();
}

void ShieldClass::BreakShield()
{
	this->HP = 0;

	if (this->Type->Respawn)
		this->Timers.Respawn.Start(this->Type->Respawn_Rate);

	this->Timers.SelfHealing.Stop();

	this->KillAnim();
	if (this->Type->BreakAnim.isset())
	{
		if (const auto pAnimType = this->Type->BreakAnim.Get())
		{
			if (auto const pAnim = GameCreate<AnimClass>(pAnimType, this->Techno->Location))
			{
				pAnim->SetOwnerObject(this->Techno);
				pAnim->Owner = this->Techno->Owner;
			}
		}
	}
}

void ShieldClass::RespawnShield()
{
	if (this->HP <= 0 && this->Timers.Respawn.Completed())
	{
		this->Timers.Respawn.Stop();
		this->HP = this->GetPercentageAmount(this->Type->Respawn);
	}
}

void ShieldClass::CreateAnim()
{
	if (!this->IdleAnim && this->Type->IdleAnim.isset())
	{
		if (auto const pAnimType = this->Type->IdleAnim.Get())
		{
			if (auto const pAnim = GameCreate<AnimClass>(pAnimType, this->Techno->Location))
			{
				pAnim->SetOwnerObject(this->Techno);
				pAnim->Owner = this->Techno->Owner;
				pAnim->RemainingIterations = 0xFFu;
				this->IdleAnim = pAnim;
			}
		}
	}
}

void ShieldClass::KillAnim()
{
	if (this->IdleAnim)
	{
		GameDelete(this->IdleAnim);
		this->IdleAnim = nullptr;
	}
}

void ShieldClass::DrawShieldBar(int iLength, Point2D* pLocation, RectangleStruct* pBound)
{
	if (this->HP > 0 || this->Type->Respawn)
	{
		if (this->Techno->WhatAmI() == AbstractType::Building)
			this->DrawShieldBar_Building(iLength, pLocation, pBound);
		else
			this->DrawShieldBar_Other(iLength, pLocation, pBound);
	}
}

void ShieldClass::DrawShieldBar_Building(int iLength, Point2D* pLocation, RectangleStruct* pBound)
{
	CoordStruct vCoords = { 0, 0, 0 };
	this->Techno->GetTechnoType()->Dimension2(&vCoords);

	Point2D vPos2 = { 0, 0 };
	CoordStruct vCoords2 = { -vCoords.X / 2, vCoords.Y / 2,vCoords.Z };
	TacticalClass::Instance->CoordsToScreen(&vPos2, &vCoords2);

	Point2D vLoc = *pLocation;
	vLoc.X -= 5;
	vLoc.Y -= 3;

	Point2D vPos = { 0, 0 };

	const int iTotal = DrawShieldBar_PipAmount(iLength);
	int frame = this->DrawShieldBar_Pip(true);

	if (iTotal > 0)
	{
		int frameIdx, deltaX, deltaY;
		for (frameIdx = iTotal, deltaX = 0, deltaY = 0;
			frameIdx;
			frameIdx--, deltaX += 4, deltaY -= 2)
		{
			vPos.X = vPos2.X + vLoc.X + 4 * iLength + 3 - deltaX;
			vPos.Y = vPos2.Y + vLoc.Y - 2 * iLength + 4 - deltaY;

			DSurface::Temp->DrawSHP(FileSystem::PALETTE_PAL, FileSystem::PIPS_SHP,
				frame, &vPos, pBound, BlitterFlags(0x600), 0, 0, 0, 1000, 0, 0, 0, 0, 0);
		}
	}

	if (iTotal < iLength)
	{
		int frameIdx, deltaX, deltaY;
		for (frameIdx = iLength - iTotal, deltaX = 4 * iTotal, deltaY = -2 * iTotal;
			frameIdx;
			frameIdx--, deltaX += 4, deltaY -= 2)
		{
			vPos.X = vPos2.X + vLoc.X + 4 * iLength + 3 - deltaX;
			vPos.Y = vPos2.Y + vLoc.Y - 2 * iLength + 4 - deltaY;

			DSurface::Temp->DrawSHP(FileSystem::PALETTE_PAL, FileSystem::PIPS_SHP,
				0, &vPos, pBound, BlitterFlags(0x600), 0, 0, 0, 1000, 0, 0, 0, 0, 0);
		}
	}
}

void ShieldClass::DrawShieldBar_Other(int iLength, Point2D* pLocation, RectangleStruct* pBound)
{
	Point2D vPos = { 0,0 };
	Point2D vLoc = *pLocation;
	int frame, XOffset, YOffset;
	YOffset = this->Techno->GetTechnoType()->PixelSelectionBracketDelta + this->Type->BracketDelta;
	vLoc.Y -= 5;

	if (iLength == 8)
	{
		vPos.X = vLoc.X + 11;
		vPos.Y = vLoc.Y - 25 + YOffset;
		frame = FileSystem::PIPBRD_SHP->Frames > 2 ? 3 : 1;
		XOffset = -5;
		YOffset -= 24;
	}
	else
	{
		vPos.X = vLoc.X + 1;
		vPos.Y = vLoc.Y - 26 + YOffset;
		frame = FileSystem::PIPBRD_SHP->Frames > 2 ? 2 : 0;
		XOffset = -15;
		YOffset -= 25;
	}

	if (this->Techno->IsSelected)
	{
		DSurface::Temp->DrawSHP(FileSystem::PALETTE_PAL, FileSystem::PIPBRD_SHP,
			frame, &vPos, pBound, BlitterFlags(0xE00), 0, 0, 0, 1000, 0, 0, 0, 0, 0);
	}

	const int iTotal = DrawShieldBar_PipAmount(iLength);

	frame = this->DrawShieldBar_Pip(false);

	for (int i = 0; i < iTotal; ++i)
	{
		vPos.X = vLoc.X + XOffset + 2 * i;
		vPos.Y = vLoc.Y + YOffset;

		DSurface::Temp->DrawSHP(FileSystem::PALETTE_PAL, FileSystem::PIPS_SHP,
			frame, &vPos, pBound, BlitterFlags(0x600), 0, 0, 0, 1000, 0, 0, 0, 0, 0);
	}
}

int ShieldClass::DrawShieldBar_Pip(const bool isBuilding)
{
	const auto strength = this->Type->Strength;
	const auto shieldPip = isBuilding ?
		RulesExt::Global()->Pips_Shield_Buildings.Get() :
		RulesExt::Global()->Pips_Shield.Get();

	if (this->HP > RulesClass::Instance->ConditionYellow * strength && shieldPip.X != -1)
		return shieldPip.X;
	else if (this->HP > RulesClass::Instance->ConditionRed * strength && (shieldPip.Y != -1 || shieldPip.X != -1))
		return shieldPip.Y == -1 ? shieldPip.X : shieldPip.Y;
	else if (shieldPip.Z != -1 || shieldPip.X != -1)
		return shieldPip.Z == -1 ? shieldPip.X : shieldPip.Z;

	return isBuilding ? 5 : 16;
}

int ShieldClass::DrawShieldBar_PipAmount(int iLength)
{
	return this->IsActive()
		? Math::clamp((int)round(this->GetHealthRatio() * iLength), 0, iLength)
		: 0;
}

double ShieldClass::GetHealthRatio()
{
	return static_cast<double>(this->HP) / this->Type->Strength;
}

int ShieldClass::GetHP()
{
	return this->HP;
}

bool ShieldClass::IsActive()
{
	return
		this->Available &&
		this->HP > 0 &&
		this->Online;
}

bool ShieldClass::IsAvailable()
{
	return this->Available;
}
