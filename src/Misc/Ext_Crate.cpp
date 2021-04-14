#include <HouseClass.h>
#include <MapClass.h>
#include <AnimTypeClass.h>
#include <AnimClass.h>
#include <ScenarioClass.h>
#include <WeaponTypeClass.h>
#include <WarheadTypeClass.h>
#include <BulletClass.h>
#include <BulletTypeClass.h>
#include <SuperClass.h>
#include <BuildingTypeClass.h>
#include <BuildingClass.h>
#include <Powerups.h>

#include <Utilities/Macro.h>

#include "../Ext/CellClass/Body.h"
#include "../Ext/TechnoType/Body.h"

#include "../Enum/CrateTypes.h"

/*
			Crate Power Up Extended
https://www.modenc.renegadeprojects.com/Powerups
Format Original :
	Crate_Type = Chance ,Animation ,Water Allowed ,	Special Parameter

*/
namespace CrateStufs
{
	enum ePowerup : unsigned char {
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
}
//Shroud easy to handle without breaking everything else
//jump goes to the end of the function because need to replace the animation
//for more fit with the stuffs
DEFINE_HOOK(481F87, CellClass_CrateCollected_Shroud_Override, 7) {

	if (CrateType::Array.empty())
	{
		Debug::Log("CrateType is empty return 0 \n");
		return 0;
	}

	GET(TechnoClass*, Collector, EAX);

	bool pass = false;//return default

	if (auto pHouse = Collector->Owner)
	{
		auto& arr = CrateType::Array;
		//accesing thru Powerups::Anim causing access violation crash
		auto C_Animarr = reinterpret_cast<int*>(0x81DAD8);

		CellStruct Bcell = { 0,0 };
		Bcell.X = static_cast<short>(R->EDX());
		Bcell.Y = static_cast<short>(R->ECX());

		auto& map = MapClass::Instance;
		auto pcell = map->TryGetCellAt(Bcell);
		auto animCoord = CellClass::Cell2Coord(Bcell, 200 + map->GetCellFloorHeight(CellClass::Cell2Coord(Bcell)));
		Debug::Log("Crate at  [x : %d][y : %d] Collected by [%s]\n", Bcell.X, Bcell.Y, Collector->get_ID());

		auto& random = ScenarioClass::Instance->Random;
		bool landtypenoteligible = false;

		auto dice = random.RandomRanged(0, arr.size() - 1); //Pick  random from array
		const int crateChance = abs(arr[dice]->Chance);
		//110 is vanilla value for yr ? 
		auto pickedupDice = random.RandomRanged(0, 110);
		//chance 0 cause crash fix
		bool allowspawn = crateChance < abs(pickedupDice) && crateChance > 0;

		auto cEXT = CellExt::ExtMap.Find(pcell);
		if (cEXT->NewPowerups > -1)
		{
			dice = abs(cEXT->NewPowerups);
			dice = dice - 1;
			Debug::Log("Crate type Check cell which to spawn [%d]\n", dice);
			allowspawn = true; //forced
		}

		auto type = arr[dice]->Tp;
		auto pType = arr[dice]->Anim;
		auto pSound = arr[dice]->Sound;

		//crate desnt allow water
		auto isWater = pcell->LandType == LandType::Water && pcell->Tile_Is_Water() && pcell->Tile_Is_Wet();
		landtypenoteligible = !arr[dice]->Water && isWater;

		if (allowspawn && !landtypenoteligible)
		{
			switch (type)
			{
			case 1:
			{
				auto SWIDX = arr[dice]->SWs;

				if (arr[dice]->SWGrant)
				{
					if (pHouse->Supers.GetItem(SWIDX)->Grant(true, true, false))
					{
						if (!pHouse->IsObserver() && !pHouse->IsPlayerObserver() && (pHouse == HouseClass::Player))
						{
							if (MouseClass::Instance->AddCameo(AbstractType::Special, SWIDX))
								MouseClass::Instance->RepaintSidebar(1);
						}
					}
				}
				else {

					//Abused By AI ?
					if (!pHouse->Supers.GetItem(SWIDX)->IsCharged) { pHouse->Supers.GetItem(SWIDX)->IsCharged = true; }
					pHouse->Supers.GetItem(SWIDX)->Launch(Bcell, true);
					pHouse->Supers.GetItem(SWIDX)->IsCharged = false;
				}
				pass = true;
			}
			break;
			case 2:
			{
				auto Weapon = arr[dice]->WeaponType;
				if (Weapon && !Weapon->Warhead->MindControl)
				{
					auto pBulletC = Weapon->Projectile->CreateBullet(pcell, Collector, Weapon->Damage, Weapon->Warhead, Weapon->Speed, Weapon->Bright);

					if (pBulletC && !Weapon->LimboLaunch)
					{
						pBulletC->SetWeaponType(Weapon);
						pBulletC->Detonate(CellClass::Cell2Coord(Bcell));
						pBulletC->Remove();
						pBulletC->UnInit();
					}
				}
				pass = true;
			}
			break;
			case 3: //case 3 is overrided reshroud
			{
				if (!pType)
					pType = AnimTypeClass::Array->GetItem(C_Animarr[7]);
				MapClass::Instance->Reshroud(pHouse);
				pass = true;
			}
			break;
			default:
				break;
			}
		}
		else
		{
			pType = AnimTypeClass::Array->GetItem(C_Animarr[0]);
			pSound = RulesClass::Instance->CrateMoneySound;
			pHouse->GiveMoney(RulesClass::Instance->SoloCrateMoney);
			pass = true;
		}

		if (pass)
		{
			if (pType)
				if (auto anim = GameCreate<AnimClass>(pType, animCoord))
				{
					anim->Owner = pHouse;
				}

			if (pSound)
				VocClass::PlayAt(pSound, animCoord, nullptr);
		}
	}

	Debug::Log("Crate Override Result %d\n", pass);
	return pass ? 0x483389 : 0x0;
}

/*
		Building And Unit Spesific Crates
		ToDo: Infantry Spesific Crate;
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
	GET(CellStruct, W, EAX);
	GET(UnitClass*, U, ESI);

	auto crate = TechnoTypeExt::ExtMap.Find(U->GetTechnoType())->CrateType;
	crate = abs(crate);

	if (MapClass::Instance->Place_Crate(W, crate))
	{
		Debug::Log("Unit[%s] to crate [%d] X [%d] Y[%d]\n", U->Type->ID, crate, W.X, W.Y);
	}

	return 0x738457;
}

DEFINE_HOOK(442219, BuildingTypeClass_Destroy_PlaceCrate_override, 7)
{
	GET(BuildingTypeClass*, pB, EDX);
	GET(BuildingClass *, BB, EBX);

	auto crate = TechnoTypeExt::ExtMap.Find((pB))->CrateType;
	crate = abs(crate);

	R->AL(MapClass::Instance->Place_Crate(BB->GetMapCoords(), crate));
	Debug::Log("Building[%s] to crate [%d]\n", pB->ID, crate);

	return 0x442226;
}


/*	Important Part to further Extend the Crate Types without rewrite whole functions
	NeedCheck : How Does this react with CellChainReact from Ares ?
*/

DEFINE_LJMP(0x56BFC2, 0x56BFC7);

//are the size okay ?
DEFINE_HOOK(56C1D3, MapClass_RemoveCrate_Override, 4)
{
	GET(CellClass*, C, EBX);

	C->OverlayData = static_cast<unsigned char>(0); //?
	auto cEXT = CellExt::ExtMap.Find(C);
	cEXT->NewPowerups = -1;
	return 0x56C1DA;
}

DEFINE_HOOK(56BFF9, MapClass_PlaceCrate_Override, 6)
{
	GET(CellClass*, C, EAX);
	GET(int, pOw, EDX);
	auto cEXT = CellExt::ExtMap.Find(C);

	//ugly XD
	//20 is random , dont occupy that 
	//custom powerup
	auto tempPow = pOw > 20 ? (pOw - 20) : (-1);
	//original powerup
	pOw = pOw > 20 ? 20 : pOw;
	pOw = pOw == 19 ? 0 : pOw; //19 is empty replace it with money instead

	//18,6,15,13 absolute crate replace to speed
	pOw = pOw == 13 || pOw == 18 || pOw == 6 || pOw == 15 ? 10 : pOw;

	C->OverlayData = CrateStufs::EnumOfIndex(pOw);
	cEXT->NewPowerups = tempPow;
	Debug::Log("MapClass_PlaceCrate_Override NewPowerUps [%d] Original PowerUps[%d] cell NewPowerups [%d] \n", tempPow, pOw, cEXT->NewPowerups);
	return 0x56BFFF;
}

//are the size okay ?
DEFINE_HOOK(481ACE, CellHasPowerUp_Override, 3)
{
	GET(CellClass*, C, ESI);

	enum Ret : DWORD { keeproll = 0x481AD3, SpawnSpesific = 0x481B22 };

	DWORD ret = Ret::keeproll; //default
	auto pOw = C->OverlayData;
	auto cEXT = CellExt::ExtMap.Find(C);
	auto cPow = cEXT->NewPowerups;
	Debug::Log("CellHasPowerUp_Override Original PowerUps [%d] cell NewPowerups [%d] \n", pOw, cPow);

	if (cPow > -1)
	{
		R->EBX(CrateStufs::EnumOfIndex(7));//force spawn Darkness
		ret = Ret::SpawnSpesific;
	}
	else
	{
		if (pOw < 19)
		{
			R->EBX(pOw);
			ret = Ret::SpawnSpesific;
		}
	}
	return ret;
}

//4A18F0 , CrateImage
//put random crate -> set crate + image after rol 
//-> set powerup type/new powerup 
//->collected 
//->spawn spesific