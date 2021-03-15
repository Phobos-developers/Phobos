#include "Shield.h"

#include "../Ext/Techno/Body.h"
#include "../Ext/TechnoType/Body.h"

ShieldTechnoClass::ShieldTechnoClass(TechnoClass* pTechno)
{
    // auto pExt = TechnoExt::ExtMap.Find(pTechno);
    this->Ext = TechnoTypeExt::ExtMap.Find(pTechno->GetTechnoType());

    this->HP = this->Ext->Shield_Strength;
    this->Timer_Respawn.Stop();
    this->Timer_SelfHealing.Stop();

}

void ShieldTechnoClass::Load(IStream* Stm)
{
    PhobosStreamReader::Process(Stm, this->Techno);
    PhobosStreamReader::Process(Stm, this->Ext);
    PhobosStreamReader::Process(Stm, this->HP);
    PhobosStreamReader::Process(Stm, this->Timer_Respawn);
    PhobosStreamReader::Process(Stm, this->Timer_SelfHealing);
}

void ShieldTechnoClass::Save(IStream* Stm)
{
    PhobosStreamWriter::Process(Stm, this->Techno);
    PhobosStreamReader::Process(Stm, this->Ext);
    PhobosStreamWriter::Process(Stm, this->HP);
    PhobosStreamWriter::Process(Stm, this->Timer_Respawn);
    PhobosStreamWriter::Process(Stm, this->Timer_SelfHealing);
}

int ShieldTechnoClass::ReceiveDamage(int nDamage, WarheadTypeClass* pWH)
{
    UNREFERENCED_PARAMETER(pWH);

    if (!this->HP || nDamage == 0)
        return nDamage;

    if (nDamage > 0)
        this->Timer_SelfHealing.Start(this->Ext->Shield_SelfHealingDelay);

    auto residueDamage = nDamage - this->HP;
    if (residueDamage >= 0)
    {
        this->BreakShield();
        if (this->Ext->Shield_AbsorbOverDamage)
            return 0;
    }
    else
    {
        this->HP = -residueDamage;
        return 0;
    }

    return residueDamage;
}

void ShieldTechnoClass::Update()
{
    this->RespawnShield();
    this->SelfHealing();

    // Extra Draws Here
    //this->DrawIt();
}

int ShieldTechnoClass::GetHP() {
    return this->HP;
}

void ShieldTechnoClass::RespawnShield()
{
    if (this->HP <= 0 && this->Timer_Respawn.Completed()) // should be -1
    {
        this->Timer_Respawn.Stop();
        this->HP = this->Ext->Shield_Strength;
    }
}

void ShieldTechnoClass::SelfHealing()
{
    auto nSelfHealingAmount = this->Ext->Shield_SelfHealing;
    if (nSelfHealingAmount > 0 && this->HP > 0 && this->Timer_SelfHealing.Completed())
    {
        this->Timer_SelfHealing.Start(this->Ext->Shield_SelfHealingDelay);
        this->HP += nSelfHealingAmount;
        if (this->HP > this->Ext->Shield_Strength)
            this->HP = this->Ext->Shield_Strength;
    }
}

void ShieldTechnoClass::BreakShield()
{
    this->HP = 0;
    this->Timer_Respawn.Start(this->Ext->Shield_RespawnDelay);
}