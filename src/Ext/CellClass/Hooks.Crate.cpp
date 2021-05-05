#include <ScenarioClass.h>
#include <HouseClass.h>
#include <SuperClass.h>
#include <WarheadTypeClass.h>
#include <BulletTypeClass.h>
#include <BulletClass.h>
#include <AnimClass.h>

#include "Body.h"

#include <Utilities/Macro.h>
#include <Ext/TechnoType/Body.h>
#include <Enum/CrateTypeClass.h>
#include <Ext/Techno/Body.h>


/*
            Crate Power Up Extended
https://www.modenc.renegadeprojects.com/Powerups
Format Original :
    Crate_Type = Chance ,Animation ,Water Allowed ,	Special Parameter

*/
namespace CrateStufs
{
    enum ePowerup : unsigned char
    {
        Money,
        Unit,
        HealBase,
        Cloak,
        Explosion,
        Napalm,
        Squad,
        Darkness,
        Reveal,
        Armor,
        Speed,
        Firepower,
        ICBM,
        Invulnerability,
        Veteran,
        IonStorm,
        Gas,
        Tiberium,
        Pod
    };

    unsigned char EnumOfIndex(int i)
    {
        return static_cast<ePowerup>(i);
    }

    //dont need to push YRPP with this
    bool Place_Crate(CellStruct where, int whatcrate)
    {
        auto map = MapClass::Instance;
        EPILOG_THISCALL
            _asm {mov ecx, map}
        _asm {mov eax, 0x56BEC0}
        _asm {jmp eax}
    }

}

//Shroud easy to handle without breaking everything else
//jump goes to the end of the function because need to replace the animation
//for more fit with the stuffs
DEFINE_HOOK(481F87, CellClass_CrateCollected_Shroud_Override, 7)
{
    if (CrateTypeClass::Array.empty())
    {
        Debug::Log("CrateType is empty return 0 \n");
        return 0;
    }

    GET(TechnoClass*, Collector, EAX);

    bool pass = false;//return default

    if (auto pHouse = Collector->Owner)
    {
        auto& CrateType = CrateTypeClass::Array;
        //accesing thru Powerups::Anim causing access violation crash
        auto Powerups_Animarray = reinterpret_cast<int*>(0x81DAD8);

        CellStruct BufferCellStruct = { 0,0 };
        BufferCellStruct.X = static_cast<short>(R->EDX());
        BufferCellStruct.Y = static_cast<short>(R->ECX());

        auto Cell = MapClass::Instance->TryGetCellAt(BufferCellStruct);
        auto animCoord = CellClass::Cell2Coord(BufferCellStruct, 200 + MapClass::Instance->GetCellFloorHeight(CellClass::Cell2Coord(BufferCellStruct)));

        auto dice = ScenarioClass::Instance->Random.RandomRanged(0, CrateType.size() - 1); //Pick  random from array
        auto pickedupDice = ScenarioClass::Instance->Random.RandomRanged(0, RulesClass::Instance->CrateMaximum);
        //chance 0 cause crash fix
        bool allowspawn = abs(CrateType[dice]->Chance.Get()) < pickedupDice && abs(CrateType[dice]->Chance.Get()) > 0;
        bool LandTypeEligible = false;
        if (CellExt::ExtMap.Find(Cell)->NewPowerups > -1)
        {
            dice = abs(CellExt::ExtMap.Find(Cell)->NewPowerups);
            dice = dice - 1;
            Debug::Log("Crate type Check cell which to spawn [%d]\n", dice);
            allowspawn = true; //forced
        }

        auto type = CrateType[dice]->Type.Get();
        auto pType = CrateType[dice]->Anim.Get();
        auto pSound = CrateType[dice]->Sound.Get();
        auto pEva = CrateType[dice]->Eva.Get();
        bool NotObserver = !pHouse->IsObserver() && !pHouse->IsPlayerObserver();
        auto isWater = Cell->LandType == LandType::Water && Cell->Tile_Is_Water() && Cell->Tile_Is_Wet();
        LandTypeEligible = CrateType[dice]->AllowWater && isWater;

        if (allowspawn && !LandTypeEligible)
        {
            switch (type)
            {
            case 1: //Super Weapon
            {
                auto SWIDX = CrateType[dice]->Super.Get();
                auto pSuper = pHouse->Supers.GetItem(SWIDX);

                if (CrateType[dice]->SuperGrant.Get())
                {
                    if (pSuper->Grant(true, NotObserver, false))
                    {
                        if (NotObserver && pHouse == HouseClass::Player)
                        {
                            if (MouseClass::Instance->AddCameo(AbstractType::Special, SWIDX))
                                MouseClass::Instance->RepaintSidebar(1);
                        }
                    }
                }
                else
                {
                    //Abused By AI ?
                    pSuper->IsCharged = true;
                    pSuper->Launch(Cell->MapCoords, true);

                }
                pass = true;
            }
            break;
            case 2: //Weapon
            {
                auto Weapon = CrateType[dice]->WeaponType.Get();
                if (Weapon && (!Weapon->Warhead->MindControl || !Weapon->LimboLaunch))
                {
                    if (auto pBulletC = Weapon->Projectile->CreateBullet(Cell, Collector, Weapon->Damage, Weapon->Warhead, Weapon->Speed, Weapon->Bright))
                    {
                        pBulletC->SetWeaponType(Weapon);
                        pBulletC->SetLocation(CellClass::Cell2Coord(BufferCellStruct));
                        if (Weapon->Projectile->ShrapnelCount > 0)
                            pBulletC->Shrapnel();
                        pBulletC->Detonate(CellClass::Cell2Coord(BufferCellStruct));
                        pBulletC->Remove();
                        pBulletC->UnInit();

                        pass = true;
                    }
                }
            }
            break;
            case 3: //case 3 is overrided reshroud
            {
                if (!pType)
                    pType = AnimTypeClass::Array->GetItem(Powerups_Animarray[7]);
                MapClass::Instance->Reshroud(pHouse);
                pass = true;
            }
            break;
            case 4: //random Unit
            {
                if (auto Unit = CrateType[dice]->Unit.GetElements())
                {
                    auto const pUnit = static_cast<TechnoClass*>(Unit.at(ScenarioClass::Instance->Random.Random() % Unit.size())->CreateObject(pHouse));

                    ++Unsorted::IKnowWhatImDoing;
                    auto succes = pUnit->Put(CellClass::Cell2Coord(BufferCellStruct), static_cast<short>(ScenarioClass::Instance->Random.RandomRanged(0, 255)));
                    --Unsorted::IKnowWhatImDoing;

                    if (!succes)
                    {
                        GameDelete(pUnit);
                    }
                    else
                    {
                        if (!pUnit->InLimbo)
                        {
                            pUnit->NeedsRedraw = true;
                            pUnit->Update();
                            pUnit->QueueMission(Mission::Guard, 1);
                            pUnit->NextMission();
                        }

                        if (NotObserver)
                            pHouse->RecheckTechTree = true;

                        pass = true;
                    }
                }
            }
            break;
            case 5: //Money
            {
                if (!pType)
                    pType = AnimTypeClass::Array->GetItem(Powerups_Animarray[0]);
                if (!pSound)
                    pSound = RulesClass::Instance->CrateMoneySound;

                auto MoneyMin = abs(CrateType[dice]->MoneyMin.Get());
                auto MoneyMax = abs(CrateType[dice]->MoneyMax.Get());
                if (MoneyMax > MoneyMin)
                {
                    pHouse->GiveMoney(ScenarioClass::Instance->Random.RandomRanged(MoneyMin, MoneyMax));
                    pass = true;
                }
            }
            break;
            case 6: //Heall All
            {
                if (!pType)
                    pType = AnimTypeClass::Array->GetItem(Powerups_Animarray[2]);
                if (!pSound)
                    pSound = RulesClass::Instance->HealCrateSound;

                for (auto const& pTechno : *TechnoClass::Array)
                {
                    bool Allowed = !pTechno->InLimbo || !pTechno->TemporalTargetingMe || !pTechno->IsSinking || !pTechno->IsCrashing;

                    if (pTechno->Owner == pHouse && pTechno->IsAlive && Allowed)
                    {
                        auto damage = (pTechno->GetTechnoType()->Strength * pTechno->Health) * -1;

                        pTechno->ReceiveDamage(&damage, 0, RulesClass::Instance->C4Warhead, Collector, true, true, pHouse);
                        pTechno->Flash(10);
                    }
                }

                pass = true;
            }
            break;
            default:
                break;
            }

            if (pass)
            {
                if (pType)
                    if (auto anim = GameCreate<AnimClass>(pType, animCoord))
                        anim->Owner = pHouse;

                if (pSound)
                    VocClass::PlayAt(pSound, animCoord, nullptr);

                if (pEva && pHouse->ControlledByPlayer() && NotObserver)
                    VoxClass::PlayAtPos(pEva, &animCoord);
            }
        }
        else
        {
            return 0x481AD3; //reroll it instead
        //	pass = true;
        }

    }

    return pass ? 0x483389 : 0x0;
}

/*
        Building And Unit Spesific Crates
        ToDo: Infantry Spesific Crate;
             allow put crate on top of creaters
*/

//force spawn crate for testings
/*disable Scenario checking
DEFINE_HOOK(738390, CellClass_Crate_To_Create, 6)
{
    GET(UnitTypeClass*, pUnit, EAX);
    return pUnit->CarriesCrate ? 0x7383D4 : 0x738457;
}
*/

DEFINE_HOOK(73844A, UnitClass_ReceiveDamage_PlaceCrate_override, 6)
{
    GET(CellStruct, Where, EAX);
    GET(UnitClass*, pthis, ESI);

    auto CrateType = TechnoTypeExt::ExtMap.Find(pthis->GetTechnoType())->CrateType;
    CrateType = abs(CrateType);

    auto Success = CrateStufs::Place_Crate(Where, CrateType);
    Debug::Log("Unit[%s] to crate [%d] X [%d] Y[%d] succes [%d]\n", pthis->Type->ID, CrateType, Where.X, Where.Y, Success);
    return 0x738457;
}

DEFINE_HOOK(442215, BuildingTypeClass_Destroy_PlaceCrate_override, 7)
{
    GET(BuildingTypeClass*, pBldType, EDX);
    GET(BuildingClass *, Building, EBX);

    auto CrateType = TechnoTypeExt::ExtMap.Find((pBldType))->CrateType;
    CrateType = abs(CrateType);

    auto Success = CrateStufs::Place_Crate(Building->GetMapCoords(), CrateType);
    Debug::Log("Building[%s] to crate [%d] succes ? [%d] \n", pBldType->ID, CrateType, Success);
    R->AL(Success);

    return 0x442226;
}


/*	Important Part to further Extend the Crate Types without rewrite whole functions
    NeedCheck : How Does this react with CellChainReact from Ares ?
*/
/**/
DEFINE_LJMP(0x56BFC2, 0x56BFC7);

DEFINE_HOOK(56C1D3, MapClass_RemoveCrate_Override, 9)
{
    GET(CellClass*, pThis, EBX);

    CellExt::ExtMap.Find(pThis)->NewPowerups = -1;

    return 0;
}

DEFINE_HOOK(56BFF9, MapClass_PlaceCrate_Override, 6)
{
    GET(CellClass*, pThis, EAX);
    GET(int, OverlayData, EDX);
    auto CellExt = CellExt::ExtMap.Find(pThis);

    //ugly XD
    //20 is random , dont occupy that 
    //custom powerup
    auto NewPowerUp = OverlayData > 20 ? (OverlayData - 20) : (-1);
    //original powerup
    OverlayData = OverlayData > 20 ? 20 : OverlayData;
    OverlayData = OverlayData == 19 ? 0 : OverlayData; //19 is empty replace it with money instead

    //18,6,15,13 absolute crate replace to speed
  //  OverlayData = OverlayData == 13 || OverlayData == 18 || OverlayData == 6 || OverlayData == 15 ? 10 : OverlayData;

    pThis->OverlayData = CrateStufs::EnumOfIndex(OverlayData);
    CellExt->NewPowerups = NewPowerUp;

    Debug::Log("MapClass_PlaceCrate_Override NewPowerUps [%d] Original PowerUps[%d] cell NewPowerups [%d] \n", NewPowerUp, OverlayData, CellExt->NewPowerups);
    return 0x56BFFF;
}

DEFINE_HOOK(481ACE, CellHasPowerUp_Override, 5)
{
    GET(CellClass*, pThis, ESI);

    enum Ret : DWORD { keeproll = 0x481AD3, SpawnSpesific = 0x481B22 };

    DWORD Return = Ret::keeproll; //default

    auto OverlayData = pThis->OverlayData;
    auto CellExt = CellExt::ExtMap.Find(pThis);
    auto NewPowerup = CellExt->NewPowerups;

    Debug::Log("CellHasPowerUp_Override Original PowerUps [%d] cell NewPowerups [%d] \n", OverlayData, NewPowerup);

    if (NewPowerup > -1)
    {
        R->EBX(CrateStufs::EnumOfIndex(7));//force spawn Darkness
        Return = Ret::SpawnSpesific;
    }
    else
    {
        if (OverlayData < 19)
        {
            R->EBX(OverlayData);
            Return = Ret::SpawnSpesific;
        }
    }

    return Return;
}

//4A18F0 , CrateImage
//put random crate -> set crate + image after rol 
//-> set powerup type/new powerup 
//->collected 
//->spawn spesific