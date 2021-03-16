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

    if (nDamage > 0)
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
        this->HP = this->GetExt()->Shield_Strength;
        this->DrawShield();
    }
}

void ShieldTechnoClass::SelfHealing()
{
    auto nSelfHealingAmount = this->GetExt()->Shield_SelfHealing;
    if (nSelfHealingAmount > 0 && this->HP > 0 && this->Timer_SelfHealing.Completed())
    {
        this->Timer_SelfHealing.Start(this->GetExt()->Shield_SelfHealingDelay);
        this->HP += nSelfHealingAmount;
        if (this->HP > this->GetExt()->Shield_Strength)
            this->HP = this->GetExt()->Shield_Strength;
    }
}

void ShieldTechnoClass::BreakShield()
{
    this->HP = 0;
    this->Timer_Respawn.Start(this->GetExt()->Shield_RespawnDelay);

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