#ifndef IS_RELEASE_VER
#ifndef BELONIT_IS_NOT_HAPPY_ABOUT_THESE
#include <Utilities/Macro.h>
#include <Utilities/Debug.h>
#include <Ext/RadSite/Body.h>
#include <Ext/Bullet/Body.h>
#include <Ext/Anim/Body.h>

#include <ScenarioClass.h>
#include <InfantryClass.h>
#include <EventClass.h>
#include <FPSCounter.h>
#include <GameOptionsClass.h>
#include <CCINIClass.h>
#include <CRC.h>

// the INI Format is fucked up

#include <nlohmann/json.hpp>

#include <fstream>
#include <string>

// -MPDEBUG cmd option only and devbuild only, let's begin with this and 
void __fastcall DesyncLogging_MPDEBUG(int, EventClass*);
DEFINE_JUMP(CALL,0x64735F,GET_OFFSET(DesyncLogging_MPDEBUG))
DEFINE_JUMP(CALL,0x64CC98,GET_OFFSET(DesyncLogging_MPDEBUG))


struct DesyncLogger
{
	using json = nlohmann::json;

	int FrameIdx;
	EventClass* OffendingEvent;

	json MainFile;

	explicit DesyncLogger(int slot, EventClass* offendingEvent):
		FrameIdx { slot }, OffendingEvent { offendingEvent },
		MainFile {}
	{	}



	void WriteMetaInfo()
	{
		json metaInfo;

		metaInfo["CurrentFrame"] = Unsorted::CurrentFrame();
		metaInfo["AverageFPS"] = FPSCounter::GetAverageFrameRate();
		metaInfo["MaxAhead"] = Game::Network.MaxAhead();
		metaInfo["MaxMaxAhead"] = Game::Network.MaxMaxAhead();
		metaInfo["LatencyFudge"] = Game::Network.LatencyFudge();
		metaInfo["GameSpeed"] = GameOptionsClass::Instance->GameSpeed;
		metaInfo["FrameSendRate"] = Game::Network.FrameSendRate();

		json crcs = json::array();
		for (auto i = 0; i < 0x100; ++i)
			crcs.push_back(EventClass::LatestFramesCRC[i]);
		metaInfo["LatestFramesCRC"] = std::move(crcs);

		if (SessionClass::Instance->GameMode == GameMode::Internet)
		{
			metaInfo["RulesHash"] = CCINIClass::RulesHash();// 679D90
			metaInfo["ArtHash"] = CCINIClass::ArtHash();// 679EC0
			metaInfo["AIHash"] = CCINIClass::AIHash();// 679ED0
		}
		else if(SessionClass::Instance->GameMode == GameMode::LAN)
		{
			metaInfo["RulesHash"] = *reinterpret_cast<DWORD*>(0xAC026C);
			metaInfo["ArtHash"] = *reinterpret_cast<DWORD*>(0xAC0270);
			metaInfo["AIHash"] = *reinterpret_cast<DWORD*>(0xAC0274);
		}

		MainFile["Network"] = std::move(metaInfo);
		MainFile["ModName"] = GameStrings::YURI_S_REVENGE();// TODO : TO BE PARSED
		MainFile["Version"] = "3.3.6"; // TODO : TO BE PARSED
		MainFile["CurrentPlayerName"] = HouseClass::CurrentPlayer->PlainName;
		// TODO: more info

	}

	void WriteOffendingEvent()
	{
		if (!OffendingEvent)return;
		json offend;

		offend["Type"] = EventClass::EventNames[(int)OffendingEvent->Type];
		offend["Frame"] = OffendingEvent->Frame;
		offend["House"] = (int)OffendingEvent->HouseIndex;
		if (OffendingEvent->Type == EventType::FrameInfo)
		{
			offend["CommandCount"] = OffendingEvent->FrameInfo.CommandCount;
			offend["CRC"] = OffendingEvent->FrameInfo.CRC;
			offend["Delay"] = OffendingEvent->FrameInfo.Delay;
		}

		MainFile["OffendingEvent"] = std::move(offend);
		//...
	}

	void WriteRNG()
	{
		json randomizer;
		auto const& random = ScenarioClass::Instance->Random;
		randomizer["unknown_00"] = random.unknown_00;
		randomizer["Next1"] = random.Next1;
		randomizer["Next2"] = random.Next2;

		json randomizer_table = json::array();
		for (auto const& i : random.Table)
			randomizer_table.push_back(i);
		randomizer["Table"] = std::move(randomizer_table);

		MainFile["Randomizer"] = std::move(randomizer);
	}

	static json FromAbstract(AbstractClass* abst)
	{
		json jAbs;
		jAbs["UniqueID"] = abst->UniqueID;
		jAbs["Dirty"] = abst->Dirty;
		CRCEngine c; abst->ComputeCRC(c);
		jAbs["Checksum"] = c.CRC;
		jAbs["RTTI"] = abst->GetClassName();
		return jAbs;
	}

	static json FromObject(ObjectClass* obj)
	{
		json jObj = FromAbstract(obj);
		if (obj->NextObject)
			jObj["NextObject"] = obj->NextObject->UniqueID;
		if (obj->AttachedTag)
			jObj["AttachedTag"] = obj->AttachedTag->UniqueID;
		jObj["Health"] = obj->Health;
		jObj["IsOnMap"] = obj->IsOnMap;
		jObj["InLimbo"] = obj->InLimbo;
		jObj["HasParachute"] = obj->HasParachute;
		jObj["OnBridge"] = obj->OnBridge;
		jObj["IsFallingDown"] = obj->IsFallingDown;
		jObj["IsABomb"] = obj->IsABomb;
		jObj["IsAlive"] = obj->IsAlive;
		jObj["Location"] = { obj->Location.X,obj->Location.Y,obj->Location.Z };
		return jObj;
	}

	static json FromAnim(AnimClass* anim)
	{
		json jAnim = FromObject(anim);
		if (anim->OwnerObject)
			jAnim["OwnerObject"] = anim->OwnerObject->UniqueID;
		if (anim->Owner)
			jAnim["Owner"] = anim->Owner->PlainName;
		jAnim["TintColor"] = anim->TintColor;
		jAnim["ZAdjust"] = anim->ZAdjust;
		jAnim["YSortAdjust"] = anim->YSortAdjust;
		jAnim["LoopDelay"] = anim->LoopDelay;
		jAnim["Accum"] = anim->Accum;
		jAnim["AnimFlags"] = (int)anim->AnimFlags;
		jAnim["HasExtras"] = anim->HasExtras;
		jAnim["RemainingIterations"] = anim->RemainingIterations;
		jAnim["Type"] = anim->Type->ID;

		auto const aext = AnimExt::ExtMap.Find(anim);
		if (aext->Invoker)
			jAnim["Invoker"] = aext->Invoker->UniqueID;
		if (aext->InvokerHouse)
			jAnim["InvokerHouse"] = aext->InvokerHouse->PlainName;


		return jAnim;
	}

	static json FromRad(RadSiteClass* rad)
	{
		json jRad = FromAbstract(rad);
		// not crc-ed
		auto radext = RadSiteExt::ExtMap.Find(rad);
		if (radext->RadHouse)
			jRad["RadHouse"] = radext->RadHouse->PlainName;
		if (radext->RadInvoker)
			jRad["RadInvoker"] = radext->RadInvoker->UniqueID;
		jRad["Type"] = radext->Type->Name.data();


		return jRad;
	}

	static json FromBullet(BulletClass* bullet)
	{
		json jBul = FromObject(bullet);
		// not crc-ed
		if (bullet->Owner)
			jBul["Owner"] = bullet->Owner->UniqueID;
		jBul["Type"] = bullet->Type->ID;
		if (bullet->Target)
			jBul["Target"] = bullet->Target->UniqueID;
		auto const bext = BulletExt::ExtMap.Find(bullet);
		jBul["CurrentStrength"] = bext->CurrentStrength;
		if (bext->FirerHouse)
			jBul["FirerHouse"] = bext->FirerHouse->PlainName;


		return jBul;
	}

	static json FromTechno(TechnoClass* techno)
	{
		json jTech = FromObject(techno);
		// MissionClass
		jTech["CurrentMission"] = MissionControlClass::FindName(techno->CurrentMission);
		jTech["SuspendedMission"] = MissionControlClass::FindName(techno->SuspendedMission);
		jTech["QueuedMission"] = MissionControlClass::FindName(techno->QueuedMission);
		jTech["MissionStatus"] = techno->MissionStatus;
		jTech["UpdateTimerTimeLeft"] = techno->UpdateTimer.GetTimeLeft();
		jTech["CurrentMissionStartTime"] = techno->CurrentMissionStartTime;

		// RadioClass

		// TechnoClass

		// BuildingClass
		
		 
		// FootClass

		// Infantry
		//
		// 
		// Unit
		//
		// 
		// Aircraft

		jTech["Type"] = techno->get_ID();
		return jTech;
	}


	static json FromTeam(TeamClass* team)
	{
		json jTeam = FromAbstract(team);





		return jTeam;
	}

	void WriteAnims()
	{
		json jAnims = json::array();
		for (auto anim : *AnimClass::Array)
			jAnims.push_back(FromAnim(anim));
		MainFile["Animations"] = std::move(jAnims);
	}

	void WriteRadSites()
	{
		json jRads = json::array();
		for (auto rad : *RadSiteClass::Array)
			jRads.push_back(FromRad(rad));
		MainFile["RadSites"] = std::move(jRads);
	}

	void WriteBullets()
	{
		json jBullets = json::array();
		for (auto rad : *RadSiteClass::Array)
			jBullets.push_back(FromRad(rad));
		MainFile["Bullets"] = std::move(jBullets);
	}

	void WriteTechnos()
	{
		json jTechnos = json::array();
		for (auto techno : *TechnoClass::Array)
			jTechnos.push_back(FromTechno(techno));
		MainFile["RadSites"] = std::move(jTechnos);
	}

	void WriteTeams()
	{

	}
};



void __fastcall DesyncLogging_MPDEBUG(int slot, EventClass* pOffendingEvent)
{

	std::ofstream oss { std::format("SYNC{:01d}_{:03d}.json", HouseClass::CurrentPlayer->ArrayIndex, slot) };
	if (oss.is_open())
	{
		DesyncLogger logger { slot, pOffendingEvent };
		logger.WriteMetaInfo();
		logger.WriteOffendingEvent();
		logger.WriteRNG();
		logger.WriteAnims();
		logger.WriteRadSites();
		logger.WriteBullets();
		logger.WriteTechnos();




		oss << logger.MainFile.dump(4) << std::endl;
	}
	oss.close();

#ifndef I_DONT_WANT_TO_FUCK_UP_IO_TIME
	reinterpret_cast<decltype(DesyncLogging_MPDEBUG)*>(0x6516F0)(slot, pOffendingEvent);
#elif STARKKU_HAS_INSOMNIA

#endif
}

#endif
#endif
