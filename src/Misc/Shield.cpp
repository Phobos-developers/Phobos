#include "Shield.h"

#include <Ext/Rules/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/WarheadType/Body.h>

#include <AnimClass.h>

ShieldTechnoClass::ShieldTechnoClass() :Techno{ nullptr }, HP{ 0 }, Timer_Respawn{}, Timer_SelfHealing{}{};

ShieldTechnoClass::ShieldTechnoClass(TechnoClass* pTechno) :
    Techno{ pTechno },
    HP{ this->GetExt()->Shield_Strength },
    Timer_Respawn{},
    Timer_SelfHealing{},
    Image{ nullptr }
{
    this->DrawShield();
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
        .Success();
}

int ShieldTechnoClass::ReceiveDamage(args_ReceiveDamage* args)
{
    //UNREFERENCED_PARAMETER(pWH);
    auto pWHExt = WarheadTypeExt::ExtMap.Find(args->WH);

    if (!this->HP || *args->Damage == 0)
        return *args->Damage;

    int nDamage = 0;

    if (pWHExt && pWHExt->CanTargetHouse(args->SourceHouse, this->Techno))
        nDamage = MapClass::GetTotalDamage(*args->Damage, args->WH, this->GetExt()->Shield_Armor, args->DistanceToEpicenter);

    if (nDamage > 0) {
        this->Timer_SelfHealing.Start(int(this->GetExt()->Shield_SelfHealingDelay * 900)); //when attacked, restart the timer

        auto residueDamage = nDamage - this->HP;
        if (residueDamage >= 0)
        {
            this->BreakShield();
            return this->GetExt()->Shield_AbsorbOverDamage ? 0 : residueDamage;
        }
        else
        {
            this->HP = -residueDamage;
            return 0;
        }
    }
    else {
        return nDamage; //might change in future
    }
}

bool ShieldTechnoClass::CanBeTargeted(WeaponTypeClass* pWeapon, TechnoClass* pSource)
{
    auto pWHExt = WarheadTypeExt::ExtMap.Find(pWeapon->Warhead);
    UNREFERENCED_PARAMETER(pWHExt);
    bool result =
        ((MapClass::GetTotalDamage(pWeapon->Damage, pWeapon->Warhead, this->GetExt()->Shield_Armor, 0) != 0) && pWeapon->Damage) 
        || !pWeapon->Damage; // we could check how is a warhead vs shield's armor 
    return this->HP ? result : true;
}

void ShieldTechnoClass::Update()
{
    this->RespawnShield();
    this->SelfHealing();
}

void ShieldTechnoClass::RespawnShield()
{
    if (this->HP <= 0 && this->Timer_Respawn.Completed())
    {
        this->Timer_Respawn.Stop();
        this->HP = this->GetPercentageAmount(this->GetExt()->Shield_Respawn);
        this->DrawShield();
    }
}

void ShieldTechnoClass::SelfHealing()
{
    auto nSelfHealingAmount = this->GetPercentageAmount(this->GetExt()->Shield_SelfHealing);
    if (nSelfHealingAmount > 0 && this->HP < this->GetExt()->Shield_Strength && this->Timer_SelfHealing.StartTime == -1)
        this->Timer_SelfHealing.Start(int(this->GetExt()->Shield_SelfHealingDelay * 900));

    if (nSelfHealingAmount > 0 && this->HP > 0 && this->Timer_SelfHealing.Completed())
    {
        this->Timer_SelfHealing.Start(int(this->GetExt()->Shield_SelfHealingDelay * 900));
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
            return int(this->GetExt()->Shield_Strength * iStatus);

        if (iStatus < 0)
        {
            iStatus *= -1;
            iStatus = (int)iStatus;
            iStatus *= -1;
        }
        return (int)iStatus;

    }
    return 0;
}

void ShieldTechnoClass::BreakShield()
{
    this->HP = 0;
    if (this->GetExt()->Shield_Respawn > 0) this->Timer_Respawn.Start(int(this->GetExt()->Shield_RespawnDelay * 900));
    this->Timer_SelfHealing.Stop();

    if (this->Image) 
    {
        GameDelete(this->Image);
        this->Image = nullptr;
    }
    if (this->GetExt()->Shield_BreakImage.isset())
    {
        if (auto pAnimType = this->GetExt()->Shield_BreakImage)
        {
            auto pAnim = GameCreate<AnimClass>(pAnimType, this->Techno->GetCoords());
            if (pAnim)
                pAnim->SetOwnerObject(this->Techno);
        }
    }
}

void ShieldTechnoClass::DrawShield()
{
    if (this->GetExt()->Shield_Image.isset() && this->HP > 0)
    {
        if (auto pAnimType = this->GetExt()->Shield_Image)
        {
            this->Image = GameCreate<AnimClass>(pAnimType, this->Techno->GetCoords());
            if (this->Image)
                this->Image->SetOwnerObject(this->Techno);
        }
    }
}

void ShieldTechnoClass::DrawShieldBar(int iLength, Point2D* pLocation, RectangleStruct* pBound)
{
    if (this->HP > 0 || this->GetExt()->Shield_Respawn) {
        if (this->Techno->WhatAmI() == AbstractType::Building)
            this->DrawShieldBarBuilding(iLength, pLocation, pBound);
        else
            this->DrawShieldBarOther(iLength, pLocation, pBound);
    }
}

void ShieldTechnoClass::DrawShieldBarBuilding(int iLength, Point2D* pLocation, RectangleStruct* pBound) {
    int iCurrent = int((double)this->HP / this->GetExt()->Shield_Strength * iLength);
    int iTotal = iCurrent;
    if (iCurrent < 0) {
        iCurrent = 0;
        iTotal = 0;
    }
    if (iCurrent > iLength) {
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
    if (iCurrent > 0) {
        int deltaX = 0;
        int deltaY = 0;
        int frameIdx = iTotal;
        for (; frameIdx; frameIdx--) {
            vPos.X = vPos2.X + vLoc.X + 4 * iLength + 3 - deltaX;
            vPos.Y = vPos2.Y + vLoc.Y - 2 * iLength + 4 - deltaY;
            DSurface::Temp->DrawSHP(FileSystem::PALETTE_PAL, FileSystem::PIPS_SHP, frame, &vPos, pBound, BlitterFlags(0x600), 0, 0, 0, 1000, 0, 0, 0, 0, 0);
            deltaX += 4;
            deltaY -= 2;
        }
        iCurrent = iTotal;
    }
    if (iCurrent < iLength) {
        int deltaX = 4 * iTotal;
        int deltaY = -2 * iCurrent;
        int frameIdx = iLength - iTotal;
        for (; frameIdx; frameIdx--) {
            vPos.X = vPos2.X + vLoc.X + 4 * iLength + 3 - deltaX;
            vPos.Y = vPos2.Y + vLoc.Y - 2 * iLength + 4 - deltaY;
            DSurface::Temp->DrawSHP(FileSystem::PALETTE_PAL, FileSystem::PIPS_SHP, 0, &vPos, pBound, BlitterFlags(0x600), 0, 0, 0, 1000, 0, 0, 0, 0, 0);
            deltaX += 4;
            deltaY -= 2;
        }
    }
}

void ShieldTechnoClass::DrawShieldBarOther(int iLength, Point2D* pLocation, RectangleStruct* pBound) {
    Point2D vPos = { 0,0 };
    Point2D vLoc = *pLocation;
    int frame, XOffset, YOffset;
    YOffset = this->Techno->GetTechnoType()->PixelSelectionBracketDelta + this->GetExt()->Shield_BracketDelta;
    vLoc.Y -= 5;
    if (iLength == 8) {
        vPos.X = vLoc.X + 11;
        vPos.Y = vLoc.Y - 25 + YOffset;
        frame = FileSystem::PIPBRD_SHP->Frames > 2 ? 3 : 1;
        XOffset = -5;
        YOffset -= 24;
    }
    else {
        vPos.X = vLoc.X + 1;
        vPos.Y = vLoc.Y - 26 + YOffset;
        frame = FileSystem::PIPBRD_SHP->Frames > 2 ? 2 : 0;
        XOffset = -15;
        YOffset -= 25;
    }
    if (this->Techno->IsSelected) DSurface::Temp->DrawSHP(FileSystem::PALETTE_PAL, FileSystem::PIPBRD_SHP, frame, &vPos, pBound, BlitterFlags(0xE00), 0, 0, 0, 1000, 0, 0, 0, 0, 0);

    int iTotal = int((double)this->HP / this->GetExt()->Shield_Strength * iLength);
    if (iTotal < 0) {
        iTotal = 0;
    }
    if (iTotal > iLength) {
        iTotal = iLength;
    }
    frame = this->DrawShieldBar_Pip();
    for (int i = 0; i < iTotal; ++i) {
        vPos.X = vLoc.X + XOffset + 2 * i;
        vPos.Y = vLoc.Y + YOffset;
        DSurface::Temp->DrawSHP(FileSystem::PALETTE_PAL, FileSystem::PIPS_SHP, frame, &vPos, pBound, BlitterFlags(0x600), 0, 0, 0, 1000, 0, 0, 0, 0, 0);
    }
}

int ShieldTechnoClass::DrawShieldBar_Pip() {
    auto ShieldPip = RulesExt::Global()->Shield_PipsForOther.Get();
    if (this->Techno->WhatAmI() == AbstractType::Building)
        ShieldPip = RulesExt::Global()->Shield_PipsForBuidling;

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