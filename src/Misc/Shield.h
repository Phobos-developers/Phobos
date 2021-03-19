#pragma once

// TODO : Support Armor Logic

#include <GeneralStructures.h>
#include "../Ext/TechnoType/Body.h"

class TechnoClass;
class WarheadTypeClass;

class ShieldTechnoClass
{
public:
    ShieldTechnoClass();
    ShieldTechnoClass(TechnoClass* pTechno);
    ~ShieldTechnoClass() = default;

    int ReceiveDamage(int nDamage, WarheadTypeClass* pWH);
    void Update();
    void DrawShieldBar(int iLength, Point2D* pLocation, RectangleStruct* pBound);

    void Load(IStream* Stm);
    void Save(IStream* Stm);
    
private:
    // static constexpr int ScanInterval = 15;		//!< Minimum delay between scans in frames.

    const TechnoTypeExt::ExtData* GetExt();

    void RespawnShield();
    void SelfHealing();
    int GetPercentageAmount(double iStatus);
    void BreakShield();
    void DrawShield();
    void DrawShieldBarBuilding(int iLength, Point2D* pLocation, RectangleStruct* pBound);
    void DrawShieldBarOther(int iLength, Point2D* pLocation, RectangleStruct* pBound);

    /// Properties ///

    TechnoClass* Techno;
    int HP;
    TimerStruct Timer_SelfHealing;
    TimerStruct Timer_Respawn;
    AnimClass* Image;
    //SHPStruct* Image;
    //LightConvertClass* Convert;
};