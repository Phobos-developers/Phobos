#include "Shield.h"

#include "../Ext/Techno/Body.h"
#include "../Ext/TechnoType/Body.h"

ShieldTechnoClass::ShieldTechnoClass() :Techno{ nullptr }, HP{ 0 }, Timer_Respawn{}, Timer_SelfHealing{}{};

ShieldTechnoClass::ShieldTechnoClass(TechnoClass* pTechno) : Techno{ pTechno }
{
    this->HP = this->GetExt()->Shield_Strength;
    this->Timer_Respawn.Stop();
    this->Timer_SelfHealing.Stop();
}

inline TechnoTypeExt::ExtData* ShieldTechnoClass::GetExt()
{
    return TechnoTypeExt::ExtMap.Find(Techno->GetTechnoType());
}

void ShieldTechnoClass::Load(IStream* Stm)
{
    PhobosStreamReader::ProcessPointer(Stm, this->Techno, true);
    PhobosStreamReader::Process(Stm, this->HP);
    PhobosStreamReader::Process(Stm, this->Timer_Respawn);
    PhobosStreamReader::Process(Stm, this->Timer_SelfHealing);
}

void ShieldTechnoClass::Save(IStream* Stm)
{
    PhobosStreamWriter::Process(Stm, this->Techno);
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

    // Extra Draws Here
    //this->DrawIt();
}

void ShieldTechnoClass::RespawnShield()
{
    if (this->HP <= 0 && this->Timer_Respawn.Completed())
    {
        this->Timer_Respawn.Stop();
        this->HP = this->GetExt()->Shield_Strength;
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
}