#include "Shield.h"

#include "../Ext/Techno/Body.h"
#include "../Ext/TechnoType/Body.h"

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

void ShieldTechnoClass::Load(IStream* Stm)
{
    PhobosStreamReader::ProcessPointer(Stm, this->Techno, true);
    PhobosStreamReader::ProcessPointer(Stm, this->Image, true);
}

void ShieldTechnoClass::Save(IStream* Stm)
{
    PhobosStreamWriter::Process(Stm, this->Techno);
    PhobosStreamWriter::Process(Stm, this->Image);
}

int ShieldTechnoClass::ReceiveDamage(int nDamage, WarheadTypeClass* pWH)
{
    UNREFERENCED_PARAMETER(pWH);

    if (!this->HP || nDamage == 0)
        return nDamage;

    if (nDamage > 0) //when attacked, restart the timer
        this->Timer_SelfHealing.Start(this->GetExt()->Shield_SelfHealingDelay);

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
        this->HP = this->GetExt()->Shield_Respawn;
        this->DrawShield();
    }
}

void ShieldTechnoClass::SelfHealing()
{
    auto nSelfHealingAmount = this->GetExt()->Shield_SelfHealing;
    if (nSelfHealingAmount > 0 && this->HP < this->GetExt()->Shield_Strength && this->Timer_SelfHealing.StartTime == -1)
        this->Timer_SelfHealing.Start(this->GetExt()->Shield_SelfHealingDelay);

    if (nSelfHealingAmount > 0 && this->HP > 0 && this->Timer_SelfHealing.Completed())
    {
        this->Timer_SelfHealing.Start(this->GetExt()->Shield_SelfHealingDelay);
        this->HP += nSelfHealingAmount;
        if (this->HP > this->GetExt()->Shield_Strength) 
        {
            this->HP = this->GetExt()->Shield_Strength;
            this->Timer_SelfHealing.Stop();
        }
    }
}

void ShieldTechnoClass::BreakShield()
{
    this->HP = 0;
    if (this->GetExt()->Shield_Respawn > 0) this->Timer_Respawn.Start(this->GetExt()->Shield_RespawnDelay);
    this->Timer_SelfHealing.Stop();

    if (this->Image)
        GameDelete(this->Image);
    if(this->GetExt()->Shield_BreakImage.isset())
        if (auto pAnimType = this->GetExt()->Shield_BreakImage)
        {
            this->Image = GameCreate<AnimClass>(pAnimType, this->Techno->GetCoords());
            if (this->Image)
                this->Image->SetOwnerObject(this->Techno);
        }

}

void ShieldTechnoClass::DrawShield()
{
    if (this->GetExt()->Shield_Image.isset() && this->HP > 0)
        if (auto pAnimType = this->GetExt()->Shield_Image)
        {
            this->Image = GameCreate<AnimClass>(pAnimType, this->Techno->GetCoords());
            if (this->Image)
                this->Image->SetOwnerObject(this->Techno);
        }
}

void ShieldTechnoClass::DrawShieldBar(int iLength, Point2D* pLocation, RectangleStruct* pBound)
{
    if (this->HP > 0 || this->GetExt()->Shield_Respawn) {
        if (this->Techno->WhatAmI() == AbstractType::Building)
            this->DrawShieldBarBuilding(iLength, pLocation, pBound);
        else
            this->DrawShieldBarOther();
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
    int frame = 5;
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
            //Debug::Log("[Phobos/Shield] %s Drawing Shield Bar: vPos is {%d, %d}.\n", this->Techno->GetTechnoType()->ID, vPos.X, vPos.Y);
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

void ShieldTechnoClass::DrawShieldBarOther() {

}