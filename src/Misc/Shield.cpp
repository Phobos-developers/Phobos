#include "Shield.h"

#include <Ext/Rules/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/WarheadType/Body.h>

#include <Utilities/GeneralUtils.h>

#include <AnimClass.h>
#include <HouseClass.h>
#include <RadarEventClass.h>

ShieldClass::ShieldClass() :
	Techno{ nullptr },
	HP{ 0 },
	Timers{ }
{ }

ShieldClass::ShieldClass(TechnoClass* pTechno) :
	Techno{ pTechno },
	Available{ true },
	HP{ this->GetType()->Strength },
	IdleAnim{ nullptr },
	Temporal{ false },
	Timers{ }
{
	strcpy(this->TechnoID, this->Techno->get_ID());
	this->CreateAnim();
}

const ShieldTypeClass* ShieldClass::GetType()
{
	return TechnoTypeExt::ExtMap.Find(this->Techno->GetTechnoType())->Shield;
}

template <typename T1, typename T2>
bool ShieldClass::Serialize(T1 pThis, T2& Stm)
{
	return Stm
		.Process(pThis->Techno)
		.Process(pThis->Available)
		.Process(pThis->TechnoID)
		.Process(pThis->IdleAnim)
		.Process(pThis->HP)
		.Process(pThis->Timers.SelfHealing)
		.Process(pThis->Timers.Respawn)
		.Process(pThis->Temporal)
		.Success();
}

bool ShieldClass::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	return Serialize(this, Stm);
}

bool ShieldClass::Save(PhobosStreamWriter& Stm) const
{
	return Serialize(this, Stm);
}

void ShieldClass::SyncShieldToAnother(TechnoClass* pFrom, TechnoClass* pTo)
{
	const auto pFromExt = TechnoExt::ExtMap.Find(pFrom);
	const auto pToExt = TechnoExt::ExtMap.Find(pTo);
	const auto pToTypeExt = TechnoTypeExt::ExtMap.Find(pTo->GetTechnoType());

	pToExt->ShieldData = std::make_unique<ShieldClass>(pTo);
	pToExt->ShieldData->HP = int(pFromExt->ShieldData->GetShieldRatio() * pToTypeExt->Shield->Strength);
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
			nDamage = MapClass::GetTotalDamage(*args->Damage, args->WH, this->GetType()->Armor, args->DistanceToEpicenter);
		else
			nDamage = -MapClass::GetTotalDamage(-*args->Damage, args->WH, this->GetType()->Armor, args->DistanceToEpicenter);
	}

	if (nDamage > 0)
	{
		this->Timers.SelfHealing.Start(this->GetType()->SelfHealing_Rate); //when attacked, restart the timer
		this->ResponseAttack();

		int residueDamage = nDamage - this->HP;
		if (residueDamage >= 0 || pWHExt->BreaksShield)
		{

			if (pWHExt->BreaksShield && residueDamage < 0)
				residueDamage = 0;

			residueDamage = int((double)residueDamage /
				GeneralUtils::GetWarheadVersusArmor(args->WH, this->GetType()->Armor)); //only absord percentage damage

			this->BreakShield();
			return this->GetType()->AbsorbOverDamage ? 0 : residueDamage;
		}
		else
		{
			this->WeaponNullifyAnim();
			this->HP = -residueDamage;
			return 0;
		}
	}
	else if (!nDamage)
	{
		return 0;
	}

	else
	{
		const int nLostHP = this->GetType()->Strength - this->HP;
		if (!nLostHP)
		{
			auto result = *args->Damage;
			if (*args->Damage * GeneralUtils::GetWarheadVersusArmor(args->WH,
				static_cast<int>(this->Techno->GetTechnoType()->Armor)) > 0)
				result = 0;
			return result;
		}

		const int nRemainLostHP = nLostHP + nDamage;
		if (nRemainLostHP < 0)
			this->HP = this->GetType()->Strength;
		else
			this->HP -= nDamage;

		return 0;
	}
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
	if (this->GetType()->HitAnim.isset())
		GameCreate<AnimClass>(this->GetType()->HitAnim, this->Techno->GetCoords());
}

bool ShieldClass::CanBeTargeted(WeaponTypeClass* pWeapon)
{
	const auto pWHExt = WarheadTypeExt::ExtMap.Find(pWeapon->Warhead);

	if (pWHExt->PenetratesShield)
		return true;

	bool result = GeneralUtils::GetWarheadVersusArmor(pWeapon->Warhead, this->GetType()->Armor) != 0.0;

	return this->HP ? result : true;
}

void ShieldClass::AI()
{
	this->ConvertCheck();

	if (!this->Available)
		return;

	this->TemporalCheck();

	if (!this->Techno || this->Techno->InLimbo || this->Techno->IsImmobilized || this->Techno->Transporter)
		return;

	this->RespawnShield();
	this->SelfHealing();
	this->CreateAnim();
}

void ShieldClass::TemporalCheck()
{
	if (this->Techno->TemporalTargetingMe && !this->Temporal)
	{
		this->Temporal = true;
		if (this->HP == 0)
			this->Timers.Respawn.Pause();
		else
			this->Timers.SelfHealing.Pause();
	}
	else if (!this->Techno->TemporalTargetingMe && this->Temporal)
	{
		this->Temporal = false;
		if
			(this->HP == 0) this->Timers.Respawn.Resume();
		else
			this->Timers.SelfHealing.Resume();
	}
}

// Is used for DeploysInto/UndeploysInto
void ShieldClass::ConvertCheck()
{
	const auto newID = this->Techno->get_ID();

	if (strcmp(this->TechnoID, newID) == 0)
		return;

	const auto pType = this->GetType();

	if (pType->Strength && this->Available)
	{ // Update this shield for the new owner
		const auto pFrom_TechnoType = TechnoTypeClass::Find(this->TechnoID);
		const auto pFromType = TechnoTypeExt::ExtMap.Find(pFrom_TechnoType)->Shield;

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
	const auto pType = this->GetType();
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
	if (iStatus)
	{
		if (iStatus >= -1.0 && iStatus <= 1.0)
			return int(this->GetType()->Strength * iStatus);

		if (iStatus < 0)
		{
			// ensure correct flooring I guess? - Kerbiter
			iStatus *= -1;
			iStatus = (int)iStatus;
			iStatus *= -1;
		}

		return (int)iStatus;
	}

	return 0;
}

void ShieldClass::InvalidatePointer(void* ptr)
{
	if (this->IdleAnim == ptr)
		this->KillAnim();
}

void ShieldClass::BreakShield()
{
	this->HP = 0;

	if (this->GetType()->Respawn > 0)
		this->Timers.Respawn.Start(this->GetType()->Respawn_Rate);

	this->Timers.SelfHealing.Stop();

	this->KillAnim();
	if (this->GetType()->BreakAnim.isset())
	{
		if (const auto pAnimType = this->GetType()->BreakAnim)
		{
			if (const auto pAnim = GameCreate<AnimClass>(pAnimType, this->Techno->GetCoords()))
				pAnim->SetOwnerObject(this->Techno);
		}
	}
}

void ShieldClass::RespawnShield()
{
	if (this->HP <= 0 && this->Timers.Respawn.Completed())
	{
		this->Timers.Respawn.Stop();
		this->HP = this->GetPercentageAmount(this->GetType()->Respawn);
		this->CreateAnim();
	}
}

void ShieldClass::CreateAnim()
{
	if (this->Techno->CloakState == CloakState::Cloaked
		|| this->Techno->CloakState == CloakState::Cloaking)
		return;

	if (!this->IdleAnim && this->GetType()->IdleAnim.isset())
	{
		if (auto const pAnimType = this->GetType()->IdleAnim)
		{
			this->IdleAnim = GameCreate<AnimClass>(pAnimType, this->Techno->Location);
			if (auto const pAnim = this->IdleAnim)
			{
				pAnim->SetOwnerObject(this->Techno);
				pAnim->Owner = this->Techno->Owner;
				pAnim->RemainingIterations = 0xFFu;
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
	if (this->HP > 0 || this->GetType()->Respawn)
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

	int iCurrent = int(this->GetShieldRatio() * iLength);
	int min = this->HP != 0;

	if (iCurrent < min)
		iCurrent = min;
	if (iCurrent > iLength)
		iCurrent = iLength;

	int iTotal = iCurrent;
	int frame = this->DrawShieldBar_Pip(true);

	if (iCurrent > 0)
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

		iCurrent = iTotal;
	}

	if (iCurrent < iLength)
	{
		int frameIdx, deltaX, deltaY;
		for (frameIdx = iLength - iTotal, deltaX = 4 * iTotal, deltaY = -2 * iCurrent;
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
	YOffset = this->Techno->GetTechnoType()->PixelSelectionBracketDelta + this->GetType()->BracketDelta;
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

	int iTotal = int(this->GetShieldRatio() * iLength);
	int min = this->HP != 0;

	if (iTotal < min)
		iTotal = min;
	if (iTotal > iLength)
		iTotal = iLength;

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
	const auto strength = this->GetType()->Strength;
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

int ShieldClass::GetHP()
{
	return this->HP;
}

double ShieldClass::GetShieldRatio()
{
	return double(this->HP) / double(this->GetType()->Strength);
}

bool ShieldClass::IsAvailable()
{
	return this->Available;
}
