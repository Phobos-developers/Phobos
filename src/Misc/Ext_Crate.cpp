#include <HouseClass.h>
#include <MapClass.h>
#include <CellClass.h>
#include <AnimTypeClass.h>
#include <AnimClass.h>
#include <ScenarioClass.h>
#include <WeaponTypeClass.h>
#include <WarheadTypeClass.h>
#include <BulletClass.h>
#include <BulletTypeClass.h>
#include <SuperClass.h>

#include "../Ext/TechnoType/Body.h"
#include "../Enum/CrateTypes.h"

/*
			Crate Power Up Extended 
https://www.modenc.renegadeprojects.com/Powerups
Format Original :
	Crate_Type = Chance ,Animation ,Water Allowed ,	Special Parameter
		1.Reshourd = normal
		2.Super weapon , chance , anim , water allowed , SWType 

		Collected Check : ??
		//481D52 - pass
		//481C86 - override with Money

		Collected Play anim = 0x481A00

		Crate_Type :
		Explosives = 482565
		Napalm = 48271E
		Re-Shroud = 481F6D 
		Map-Reveal = 481F9D
		ICBM = 482CA1 
		Veterancy = 482972
		NAPALM = 481DE7
		Tiberium = 481E99
		Heal  = 482B8F
		Unit = 482041
		Invulnerability = X 
		Squad = X
		IonStorm = x
		Gas = 481DE7
		Pod = x
		Parabomb = x
		Sonar = x
		Money = 482465
		//Armor , Cloak , Firepower , speed is occupied by Ares
*/

//Shroud easy to handle without breaking everything else
//jump goes to the end of the function because need to replace the animation
//for more fit with the stuffs
DEFINE_HOOK(481F87, CellClass_CrateCollected_Shroud_Override, 6) {

	GET(TechnoClass*, Collector, EAX);
	
	auto pX = static_cast<short>(R->EDX());
	auto pY = static_cast<short>(R->ECX());

	bool pass = false;//default behaviour return
	Debug::Log("Crate at  [x : %d][y : %d] Collected by [%s]\n", pX, pY,Collector->get_ID());
	
	auto pHouse = Collector->Owner;

		if (pHouse) {

			CellStruct Bcell = { 0,0 };
			Bcell.X = pX;
			Bcell.Y = pY;

			auto& random = ScenarioClass::Instance->Random;
			const int size = CrateType::Array.size();
			Debug::Log("CrateType Array [%d]\n", size);
			if (size > 0) {
				
				auto pcell = MapClass::Instance->TryGetCellAt(Bcell);
				auto animCoord = CellClass::Cell2Coord(Bcell);
				animCoord.Z = animCoord.Z + 200;
				
				int dice = random.RandomRanged(0, size); //Pick  random from array
				const int diceb = random.RandomRanged(1, 110); //randomize it again 
				dice = dice == 0 ? 0 : dice-1; //dont let it out of bound , jeez
				Debug::Log("Crate Dice [%d]\n", dice);
				Debug::Log("Crate Diceb [%d]\n", diceb);

			if ( diceb > CrateType::Array[dice]->Chance){
				
			   auto pType = CrateType::Array[dice]->Anim;
			   auto ANID = pType ? CrateType::Array[dice]->Anim->ID : "<None>";
			   

			   int type = CrateType::Array[dice]->Tp;
			   Debug::Log("Crate type [%d]\n", type);

					switch (type)
					{
					case 1:
					{
						auto SWID = CrateType::Array[dice]->SWs ? CrateType::Array[dice]->SWs->ID : "<None>";
						Debug::Log("Crate selected [%s] , SW [%s] , Anim [%s] \n", CrateType::Array[dice]->Name, SWID, ANID);
						auto SWSS = SuperWeaponTypeClass::FindIndex(CrateType::Array[dice]->SWs->ID);
						Debug::Log("Crate type SW IDX [%d]\n", SWSS);
						//auto pSuper = SuperClass::Array->GetItem(SWSS);

						if (CrateType::Array[dice]->SWGrant) {

							if (pHouse->Supers.GetItem(SWSS)->Grant(true, true, false)) {
								MouseClass::Instance->AddCameo(AbstractType::Special, SWSS);
								MouseClass::Instance->RepaintSidebar(1);
							}

						}
						else
						{
							if (!pHouse->Supers.GetItem(SWSS)->IsCharged) { pHouse->Supers.GetItem(SWSS)->IsCharged = true;}
							pHouse->Supers.GetItem(SWSS)->Launch(pcell->MapCoords, true);
							
						}

			
						pass = true;
					}
					break;
					case 2:
					{
						auto WPID = CrateType::Array[dice]->WeaponType ? CrateType::Array[dice]->WeaponType->ID : "<None>";
						auto pBullet = CrateType::Array[dice]->WeaponType;
						Debug::Log("Crate selected [%s] , WP [%s] , Chance [%d] , Anim [%s] \n", CrateType::Array[dice]->Name, WPID, CrateType::Array[dice]->Chance, ANID);
						Debug::Log("Crate type WP IDX [%d][%s]\n", WeaponTypeClass::FindIndex(WPID),WPID);
						if (pBullet && !pBullet->Warhead->MindControl) {
							auto pBulletC = pBullet->Projectile->CreateBullet(pcell, Collector, pBullet->Damage, pBullet->Warhead, pBullet->Speed, pBullet->Bright);

							if (pBulletC && !pBullet->LimboLaunch) {
								//fixing RadSite crash
								if (pBulletC->GetWeaponType() == nullptr) {
									pBulletC->SetWeaponType(pBullet);
								}
								pBulletC->Detonate(CellClass::Cell2Coord(pcell->MapCoords));
								pBulletC->Remove();
								pBulletC->UnInit();
							}
						}

						pass = true;
					}
					break;
					case 3:
					{
						pass = false;
					}
					break;
					default:
						break;
					}

					if(pType && pass)
					GameCreate<AnimClass>(pType, animCoord);	
				
			}
		  }
		}

	Debug::Log("Crate override result [%d]\n", pass);
    return pass ? 0x483389 : 0x0;
}

