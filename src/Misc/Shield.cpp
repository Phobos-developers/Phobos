#include "Shield.h"

#include <Ext/Rules/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/WarheadType/Body.h>

#include <AnimClass.h>
#include <HouseClass.h>
#include <RadarEventClass.h>

ShieldTechnoClass::ShieldTechnoClass() :Techno { nullptr }, HP { 0 }, Timer_Respawn {}, Timer_SelfHealing {}{};

ShieldTechnoClass::ShieldTechnoClass(TechnoClass* pTechno) :
    Techno { pTechno },
    HP { this->GetExt()->Shield_Strength },
    Timer_Respawn {},
    Timer_SelfHealing {},
    Image { nullptr },
    HaveAnim { true },
    Temporal { false }
    //Broken{ false }
{
    this->CreateAnim();
}
const TechnoTypeExt::ExtData* ShieldTechnoClass::GetExt()
{
    return TechnoTypeExt::ExtMap.Find(Techno->GetTechnoType());
}

bool ShieldTechnoClass::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
    return Stm
        .Process(this->Techno)
        .Process(this->Image)
        .Process(this->HP)
        .Process(this->Timer_SelfHealing)
        .Process(this->Timer_Respawn)
        .Process(this->HaveAnim)
        .Process(this->Temporal)
        .Success();
}

bool ShieldTechnoClass::Save(PhobosStreamWriter& Stm) const
{
    return Stm
        .Process(this->Techno)
        .Process(this->Image)
        .Process(this->HP)
        .Process(this->Timer_SelfHealing)
        .Process(this->Timer_Respawn)
        .Process(this->HaveAnim)
        .Process(this->Temporal)
        .Success();
}

void ShieldTechnoClass::SyncShieldToAnother(TechnoClass* pFrom, TechnoClass* pTo)
{
    auto pFromExt = TechnoExt::ExtMap.Find(pFrom);
    auto pToExt = TechnoExt::ExtMap.Find(pTo);
    auto pToTypeExt = TechnoTypeExt::ExtMap.Find(pTo->GetTechnoType());
    if (pFromExt->ShieldData && pToTypeExt->Shield_Strength)
    {
        pToExt->ShieldData = std::make_unique<ShieldTechnoClass>(pTo);
        pToExt->ShieldData->HP = int(pFromExt->ShieldData->GetShieldRatio() * pToTypeExt->Shield_Strength);
    }
}

int ShieldTechnoClass::ReceiveDamage(args_ReceiveDamage* args)
{
    //UNREFERENCED_PARAMETER(pWH);
    auto pWHExt = WarheadTypeExt::ExtMap.Find(args->WH);

    if (!this->HP || *args->Damage == 0 || this->Techno->IsIronCurtained())
    {
        return *args->Damage;
    }

    if (pWHExt && pWHExt->PenetratesShield)
    {
        return *args->Damage;
    }

    int nDamage = 0;

    if (pWHExt->CanTargetHouse(args->SourceHouse, this->Techno) && !args->WH->Temporal)
    {
        if (*args->Damage > 0)
            nDamage = MapClass::GetTotalDamage(*args->Damage, args->WH, this->GetExt()->Shield_Armor, args->DistanceToEpicenter);
        else
            nDamage = -MapClass::GetTotalDamage(-*args->Damage, args->WH, this->GetExt()->Shield_Armor, args->DistanceToEpicenter);
    }

    if (nDamage > 0)
    {

        this->Timer_SelfHealing.Start(int(this->GetExt()->Shield_SelfHealing_Rate * 900)); //when attacked, restart the timer
        this->ResponseAttack();

        auto residueDamage = nDamage - this->HP;
        if (residueDamage >= 0 || pWHExt->BreaksShield)
        {

            if (pWHExt->BreaksShield && residueDamage < 0)
                residueDamage = 0;

            this->BreakShield();
            return this->GetExt()->Shield_AbsorbOverDamage ? 0 : residueDamage;
        }
        else
        {
            this->WeaponNullifyAnim();
            this->HP = -residueDamage;
            return 0;
        }
    }
    else
    {
        auto LostHP = this->GetExt()->Shield_Strength - this->HP;
        if (!LostHP) return *args->Damage;

        auto RemainLostHP = LostHP + nDamage;
        if (RemainLostHP < 0)
            this->HP = this->GetExt()->Shield_Strength;
        else
            this->HP -= nDamage;
        return 0;
    }
}

void ShieldTechnoClass::ResponseAttack()
{
    if (this->Techno->Owner != HouseClass::Player)
        return;
    if (this->Techno->WhatAmI() == AbstractType::Building)
    {
        auto pBld = abstract_cast<BuildingClass*>(this->Techno);
        this->Techno->Owner->BuildingUnderAttack(pBld);
    }
    else if (this->Techno->WhatAmI() == AbstractType::Unit)
    {
        auto pUnit = abstract_cast<UnitClass*>(this->Techno);
        if (pUnit->Type->Harvester)
        {
            auto pPos = pUnit->GetDestination(pUnit);
            if (RadarEventClass::Create(RadarEventType::HarvesterAttacked, { (short)pPos.X / 256,(short)pPos.Y / 256 }))
            {
                VoxClass::Play("EVA_OreMinerUnderAttack");
            }
        }
    }
}

void ShieldTechnoClass::WeaponNullifyAnim()
{
    if (this->GetExt()->Shield_HitAnim.isset())
    {
        GameCreate<AnimClass>(this->GetExt()->Shield_HitAnim, this->Techno->GetCoords());
    }
}

bool ShieldTechnoClass::CanBeTargeted(WeaponTypeClass* pWeapon, TechnoClass* pSource)
{
    auto pWHExt = WarheadTypeExt::ExtMap.Find(pWeapon->Warhead);
    UNREFERENCED_PARAMETER(pWHExt);

    if (pWHExt->PenetratesShield)
    {
        return true;
    }

    bool result =
        ((MapClass::GetTotalDamage(pWeapon->Damage, pWeapon->Warhead, this->GetExt()->Shield_Armor, 0) != 0) && pWeapon->Damage)
        || !pWeapon->Damage // we couldn't check how is a warhead vs shield's armor for now - Uranusian
        || pWeapon->Damage < 0;

    return this->HP ? result : true;
}

void ShieldTechnoClass::AI()
{
    this->TemporalCheck();

    if (!this->Techno || this->Techno->InLimbo || this->Techno->IsImmobilized || this->Techno->Transporter)
    {
        return;
    }

    this->DrawShield();
    this->RespawnShield();
    this->SelfHealing();
}

void ShieldTechnoClass::TemporalCheck()
{
    if (this->Techno->TemporalTargetingMe && !this->Temporal)
    {
        this->Temporal = true;
        if (this->HP == 0) this->Timer_Respawn.Pause();
        else this->Timer_SelfHealing.Pause();
    }
    else if (!this->Techno->TemporalTargetingMe && this->Temporal)
    {
        this->Temporal = false;
        if (this->HP == 0) this->Timer_Respawn.Resume();
        else this->Timer_SelfHealing.Resume();
    }
}

void ShieldTechnoClass::SelfHealing()
{
    auto nSelfHealingAmount = this->GetPercentageAmount(this->GetExt()->Shield_SelfHealing);
    if (nSelfHealingAmount > 0 && this->HP < this->GetExt()->Shield_Strength && this->Timer_SelfHealing.StartTime == -1)
        this->Timer_SelfHealing.Start(int(this->GetExt()->Shield_SelfHealing_Rate * 900));

    if (nSelfHealingAmount > 0 && this->HP > 0 && this->Timer_SelfHealing.Completed())
    {
        this->Timer_SelfHealing.Start(int(this->GetExt()->Shield_SelfHealing_Rate * 900));
        this->HP += nSelfHealingAmount;
        if (this->HP > this->GetExt()->Shield_Strength)
        {
            this->HP = this->GetExt()->Shield_Strength;
            this->Timer_SelfHealing.Stop();
        }
    }
}

int ShieldTechnoClass::GetPercentageAmount(double iStatus)
{
    if (iStatus)
    {
        if (iStatus >= -1.0 && iStatus <= 1.0)
        {
            return int(this->GetExt()->Shield_Strength * iStatus);
        }

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

void ShieldTechnoClass::InvalidatePointer(void* ptr)
{
    if (this->Techno == ptr)
    {
        this->Techno = nullptr;
    }

    if (this->Image == ptr)
    {
        this->KillAnim();
    }
}

void ShieldTechnoClass::UninitAnim::operator() (AnimClass* const pAnim) const
{
    auto buffer = abstract_cast<TechnoClass*>(pAnim->OwnerObject);
    pAnim->SetOwnerObject(nullptr);

    if (buffer)
    {
        pAnim->UnInit();
    }
}

void ShieldTechnoClass::BreakShield()
{
    this->HP = 0;

    if (this->GetExt()->Shield_Respawn > 0)
    {
        this->Timer_Respawn.Start(int(this->GetExt()->Shield_Respawn_Rate * 900));
    }

    this->Timer_SelfHealing.Stop();

    //if (this->GetExt()->Shield_RespawnAnim.isset()) this->Broken = true;

    if (this->GetExt()->Shield_BreakAnim.isset())
    {
        if (auto pAnimType = this->GetExt()->Shield_BreakAnim)
        {
            auto pAnim = GameCreate<AnimClass>(pAnimType, this->Techno->GetCoords());

            if (pAnim)
            {
                pAnim->SetOwnerObject(this->Techno);
            }
        }
    }
}

void ShieldTechnoClass::RespawnShield()
{
    if (this->HP <= 0 && this->Timer_Respawn.Completed())
    {
        this->Timer_Respawn.Stop();
        this->HP = this->GetPercentageAmount(this->GetExt()->Shield_Respawn);
        /*
        if (this->GetExt()->Shield_RespawnAnim.isset() && this->Broken) {
            if (AnimTypeClass* const pAnimType = this->GetExt()->Shield_RespawnAnim) {
                this->Image.reset(GameCreate<AnimClass>(pAnimType, this->Techno->Location));
                if (AnimClass* const pAnim = this->Image) {
                    pAnim->SetOwnerObject(this->Techno);
                    pAnim->Owner = this->Techno->Owner;
                }
            }
        }
        */
    }
}

void ShieldTechnoClass::DrawShield()
{
    if (this->Techno->CloakState != CloakState::Uncloaked
        || this->HP < 1)
    {
        if (this->HaveAnim)
        {
            this->KillAnim();
            this->HaveAnim = false;
        }
    }
    else
    {
        if (!this->HaveAnim)
        {
            this->CreateAnim();
            this->HaveAnim = true;
        }
    }
}

void ShieldTechnoClass::CreateAnim()
{
    if (this->Techno->CloakState != CloakState::Uncloaked)
    {
        return;
    }
    if (this->GetExt()->Shield_IdleAnim.isset())
    {
        if (AnimTypeClass* const pAnimType = this->GetExt()->Shield_IdleAnim)
        {
            this->Image.reset(GameCreate<AnimClass>(pAnimType, this->Techno->Location));
            if (AnimClass* const pAnim = this->Image)
            {
                pAnim->SetOwnerObject(this->Techno);
                pAnim->RemainingIterations = 0xFFu;
                pAnim->Owner = this->Techno->Owner;
            }
        }
    }
}

void ShieldTechnoClass::KillAnim()
{
    this->Image.clear();
}

void ShieldTechnoClass::DrawShieldBar(int iLength, Point2D* pLocation, RectangleStruct* pBound)
{
    if (this->HP > 0 || this->GetExt()->Shield_Respawn)
    {
        if (this->Techno->WhatAmI() == AbstractType::Building)
            this->DrawShieldBarBuilding(iLength, pLocation, pBound);
        else
            this->DrawShieldBarOther(iLength, pLocation, pBound);
    }
}

void ShieldTechnoClass::DrawShieldBarBuilding(int iLength, Point2D* pLocation, RectangleStruct* pBound)
{
    int iCurrent = int((double)this->HP / this->GetExt()->Shield_Strength * iLength);
    int iTotal = iCurrent;
    if (iCurrent < 0)
    {
        iCurrent = 0;
        iTotal = 0;
    }
    if (iCurrent > iLength)
    {
        iCurrent = iLength;
        iTotal = iLength;
    }
    int frame = this->DrawShieldBar_Pip();
    Point2D vPos = { 0,0 };
    CoordStruct vCoords = { 0,0,0 };
    this->Techno->GetTechnoType()->Dimension2(&vCoords);
    Point2D vPos2 = { 0,0 };
    CoordStruct vCoords2 = { -vCoords.X / 2, vCoords.Y / 2,vCoords.Z };
    TacticalClass::Instance->CoordsToScreen(&vPos2, &vCoords2);
    Point2D vLoc = *pLocation;
    vLoc.X -= 5;
    vLoc.Y -= 3;
    if (iCurrent > 0)
    {
        int deltaX = 0;
        int deltaY = 0;
        int frameIdx = iTotal;
        for (; frameIdx; frameIdx--)
        {
            vPos.X = vPos2.X + vLoc.X + 4 * iLength + 3 - deltaX;
            vPos.Y = vPos2.Y + vLoc.Y - 2 * iLength + 4 - deltaY;
            DSurface::Temp->DrawSHP(FileSystem::PALETTE_PAL, FileSystem::PIPS_SHP, frame, &vPos, pBound, BlitterFlags(0x600), 0, 0, 0, 1000, 0, 0, 0, 0, 0);
            deltaX += 4;
            deltaY -= 2;
        }
        iCurrent = iTotal;
    }
    if (iCurrent < iLength)
    {
        int deltaX = 4 * iTotal;
        int deltaY = -2 * iCurrent;
        int frameIdx = iLength - iTotal;
        for (; frameIdx; frameIdx--)
        {
            vPos.X = vPos2.X + vLoc.X + 4 * iLength + 3 - deltaX;
            vPos.Y = vPos2.Y + vLoc.Y - 2 * iLength + 4 - deltaY;
            DSurface::Temp->DrawSHP(FileSystem::PALETTE_PAL, FileSystem::PIPS_SHP, 0, &vPos, pBound, BlitterFlags(0x600), 0, 0, 0, 1000, 0, 0, 0, 0, 0);
            deltaX += 4;
            deltaY -= 2;
        }
    }
}

void ShieldTechnoClass::DrawShieldBarOther(int iLength, Point2D* pLocation, RectangleStruct* pBound)
{
    Point2D vPos = { 0,0 };
    Point2D vLoc = *pLocation;
    int frame, XOffset, YOffset;
    YOffset = this->Techno->GetTechnoType()->PixelSelectionBracketDelta + this->GetExt()->Shield_BracketDelta;
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
    if (this->Techno->IsSelected) DSurface::Temp->DrawSHP(FileSystem::PALETTE_PAL, FileSystem::PIPBRD_SHP, frame, &vPos, pBound, BlitterFlags(0xE00), 0, 0, 0, 1000, 0, 0, 0, 0, 0);

    int iTotal = int((double)this->HP / this->GetExt()->Shield_Strength * iLength);
    if (iTotal < 0)
    {
        iTotal = 0;
    }
    if (iTotal > iLength)
    {
        iTotal = iLength;
    }
    frame = this->DrawShieldBar_Pip();
    for (int i = 0; i < iTotal; ++i)
    {
        vPos.X = vLoc.X + XOffset + 2 * i;
        vPos.Y = vLoc.Y + YOffset;
        DSurface::Temp->DrawSHP(FileSystem::PALETTE_PAL, FileSystem::PIPS_SHP, frame, &vPos, pBound, BlitterFlags(0x600), 0, 0, 0, 1000, 0, 0, 0, 0, 0);
    }
}

int ShieldTechnoClass::DrawShieldBar_Pip()
{
    auto ShieldPip = RulesExt::Global()->Pips_Shield.Get();
    if (this->Techno->WhatAmI() == AbstractType::Building)
        ShieldPip = RulesExt::Global()->Pips_Shield_Buildings;

    if (this->HP > RulesClass::Instance->ConditionYellow * this->GetExt()->Shield_Strength && ShieldPip.X != -1)
        return ShieldPip.X;
    else if (this->HP > RulesClass::Instance->ConditionRed * this->GetExt()->Shield_Strength && (ShieldPip.Y != -1 || ShieldPip.X != -1))
        return ShieldPip.Y == -1 ? ShieldPip.X : ShieldPip.Y;
    else if (ShieldPip.Z != -1 || ShieldPip.X != -1)
        return ShieldPip.Z == -1 ? ShieldPip.X : ShieldPip.Z;

    if (this->Techno->WhatAmI() == AbstractType::Building)
        return 5;
    else
        return 16;
}

int ShieldTechnoClass::GetShieldHP()
{
    return this->HP;
}

double ShieldTechnoClass::GetShieldRatio()
{
    return double(this->HP) / double(this->GetExt()->Shield_Strength);
}
