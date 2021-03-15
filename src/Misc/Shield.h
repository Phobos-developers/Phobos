#pragma once

// TODO : Support Armor Logic

#include <GeneralStructures.h>
#include "../Ext/TechnoType/Body.h"

class TechnoClass;
class WarheadTypeClass;

class ShieldTechnoClass
{
public:
    ShieldTechnoClass(TechnoClass* pTechno);
    ~ShieldTechnoClass() = default;

    int ReceiveDamage(int nDamage, WarheadTypeClass* pWH);
    void Update();

    void Load(IStream* Stm);
    void Save(IStream* Stm);

    int GetHP();
    
private:
    // static constexpr int ScanInterval = 15;		//!< Minimum delay between scans in frames.

    TechnoTypeExt::ExtData* GetExt();

    void RespawnShield();
    void SelfHealing();
    void BreakShield();
    //void DrawIt();


    /// Properties ///

    TechnoClass* Techno;
    int HP;
    TimerStruct Timer_SelfHealing;
    TimerStruct Timer_Respawn;
    //SHPStruct* Image;
    //LightConvertClass* Convert;
};