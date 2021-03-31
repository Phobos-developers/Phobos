#pragma once

// TODO : Support Armor Logic

#include <GeneralStructures.h>
#include <SpecificStructures.h>
#include "../Ext/TechnoType/Body.h"

class TechnoClass;
class WarheadTypeClass;

class ShieldTechnoClass
{
public:
    ShieldTechnoClass();
    ShieldTechnoClass(TechnoClass* pTechno);
    ~ShieldTechnoClass() = default;

    int ReceiveDamage(args_ReceiveDamage* args);
    bool CanBeTargeted(WeaponTypeClass* pWeapon, TechnoClass* pSource);
    void Update();
    void DrawShieldBar(int iLength, Point2D* pLocation, RectangleStruct* pBound);
    void InvalidatePointer(void* ptr);

    bool Load(PhobosStreamReader& Stm, bool RegisterForChange);
    bool Save(PhobosStreamWriter& Stm) const;
    
private:
    // static constexpr int ScanInterval = 15;		//!< Minimum delay between scans in frames.
    struct UninitAnim {
        void operator() (AnimClass* const pAnim) const;
    };

    const TechnoTypeExt::ExtData* GetExt();

    void RespawnShield();
    void SelfHealing();
    int GetPercentageAmount(double iStatus);
    void BreakShield();
    void DrawShield();
    void CreateAnim();
    void KillAnim();
    void DrawShieldBarBuilding(int iLength, Point2D* pLocation, RectangleStruct* pBound);
    void DrawShieldBarOther(int iLength, Point2D* pLocation, RectangleStruct* pBound);
    int DrawShieldBar_Pip();

    /// Properties ///

    TechnoClass* Techno;
    int HP;
    TimerStruct Timer_SelfHealing;
    TimerStruct Timer_Respawn;
    Handle<AnimClass*, UninitAnim> Image;
    bool HaveAnim;
    //SHPStruct* Image;
    //LightConvertClass* Convert;
};