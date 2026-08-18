#include "Body.h"

#include "NewSWType/NewSWType.h"

SWTypeExt::ExtContainer SWTypeExt::ExtMap;

void SWTypeExt::Initialize()
{
	this->EVA_InsufficientFunds = VoxClass::FindIndex(GameStrings::EVA_InsufficientFunds);
	this->EVA_SelectTarget = VoxClass::FindIndex("EVA_SelectTarget");

	this->Message_CannotFire = CSFText("MSG:CannotFire");
}

// =============================
// load / save

template <typename T>
void SWTypeExt::Serialize(T& Stm)
{
	Stm
		.Process(this->TypeID)
		.Process(this->Money_Amount)
		.Process(this->EVA_Impatient)
		.Process(this->EVA_InsufficientFunds)
		.Process(this->EVA_SelectTarget)
		.Process(this->SW_UseAITargeting)
		.Process(this->SW_AutoFire)
		.Process(this->SW_ManualFire)
		.Process(this->SW_ShowCameo)
		.Process(this->SW_Unstoppable)
		.Process(this->SW_AllowPlayer)
		.Process(this->SW_AllowAI)
		.Process(this->SW_Inhibitors)
		.Process(this->SW_AnyInhibitor)
		.Process(this->SW_Designators)
		.Process(this->SW_AnyDesignator)
		.Process(this->SW_RangeMinimum)
		.Process(this->SW_RangeMaximum)
		.Process(this->SW_RequiredHouses)
		.Process(this->SW_ForbiddenHouses)
		.Process(this->SW_AuxBuildings)
		.Process(this->SW_NegBuildings)
		.Process(this->SW_AuxTechnos)
		.Process(this->SW_NegTechnos)
		.Process(this->SW_TechLevel)
		.Process(this->SW_InitialReady)
		.Process(this->SW_PostDependent)
		.Process(this->SW_MaxCount)
		.Process(this->SW_Shots)
		.Process(this->Message_CannotFire)
		.Process(this->Message_InsufficientFunds)
		.Process(this->Message_ColorScheme)
		.Process(this->Message_FirerColor)
		.Process(this->UIDescription)
		.Process(this->CameoPriority)
		.Process(this->LimboDelivery_Types)
		.Process(this->LimboDelivery_IDs)
		.Process(this->LimboDelivery_RandomWeightsData)
		.Process(this->LimboDelivery_RollChances)
		.Process(this->LimboKill_AffectsHouse)
		.Process(this->LimboKill_IDs)
		.Process(this->LimboKill_Counts)
		.Process(this->RandomBuffer)
		.Process(this->Detonate_Warhead)
		.Process(this->Detonate_Weapon)
		.Process(this->Detonate_Damage)
		.Process(this->Detonate_Warhead_Full)
		.Process(this->Detonate_AtFirer)
		.Process(this->SW_Next)
		.Process(this->SW_Next_RealLaunch)
		.Process(this->SW_Next_IgnoreInhibitors)
		.Process(this->SW_Next_IgnoreDesignators)
		.Process(this->SW_Next_RandomWeightsData)
		.Process(this->SW_Next_RollChances)
		.Process(this->ShowTimer_Priority)
		.Process(this->ShowTimer_Percentage)
		.Process(this->Convert_Pairs)
		.Process(this->ShowDesignatorRange)
		.Process(this->TabIndex)
		.Process(this->SuperWeaponSidebar_Allow)
		.Process(this->SuperWeaponSidebar_PriorityHouses)
		.Process(this->SuperWeaponSidebar_RequiredHouses)
		.Process(this->SuperWeaponSidebar_Significance)
		.Process(this->SidebarPal)
		.Process(this->SidebarPCX)
		.Process(this->UseWeeds)
		.Process(this->UseWeeds_Amount)
		.Process(this->UseWeeds_StorageTimer)
		.Process(this->UseWeeds_ReadinessAnimationPercentage)
		.Process(this->EMPulse_WeaponIndex)
		.Process(this->EMPulse_SuspendOthers)
		.Process(this->EMPulse_Cannons)
		.Process(this->EMPulse_TargetSelf)
		.Process(this->SW_Link)
		.Process(this->SW_Link_Grant)
		.Process(this->SW_Link_Ready)
		.Process(this->SW_Link_Reset)
		.Process(this->SW_Link_RandomWeightsData)
		.Process(this->SW_Link_RollChances)
		.Process(this->Message_LinkedSWAcquired)
		.Process(this->EVA_LinkedSWAcquired)
		.Process(this->Message_Activated_Owner)
		.Process(this->Message_Activated_Allies)
		.Process(this->Message_Activated_Enemies)
		.Process(this->EVA_Activated_Owner)
		.Process(this->EVA_Activated_Allies)
		.Process(this->EVA_Activated_Enemies)
		.Process(this->DropshipLoadout_OpenWindow)
		.Process(this->DropshipLoadout_Launch)
		.Process(this->DropshipLoadout_PersistentCargo)
		.Process(this->DropshipLoadout_PreloadCargo)
		.Process(this->DropshipLoadout_AddUnusedMoneyToPlayer)
		.Process(this->DropshipLoadout_RememberPurchasedCargo)
		.Process(this->DropshipLoadout_Palette)
		.Process(this->DropshipLoadout_Carrier)
		.Process(this->DropshipLoadout_AllowableUnits)
		.Process(this->DropshipLoadout_AllowableUnitMaximums)
		.Process(this->DropshipLoadout_Money)
		.Process(this->DropshipLoadout_Theme)
		.Process(this->DropshipLoadout_StartEVA)
		.Process(this->DropshipLoadout_SizeLimit)
		.Process(this->DropshipLoadout_BackgroundPCX)
		.Process(this->DropshipLoadout_UpArrowPCX)
		.Process(this->DropshipLoadout_DownArrowPCX)
		.Process(this->DropshipLoadout_UpArrowLocation)
		.Process(this->DropshipLoadout_DownArrowLocation)
		.Process(this->DropshipLoadout_SidebarCameosCount)
		.Process(this->DropshipLoadout_SidebarCameosLocations)
		.Process(this->DropshipLoadout_PilotLitPCX)
		.Process(this->DropshipLoadout_PilotLitLocation)
		.Process(this->DropshipLoadout_LoadoutPCX)
		.Process(this->DropshipLoadout_LoadoutLocation)
		.Process(this->DropshipLoadout_DGreenListPCX)
		.Process(this->DropshipLoadout_Background)
		.Process(this->DropshipLoadout_UpArrow)
		.Process(this->DropshipLoadout_DownArrow)
		.Process(this->DropshipLoadout_Loadout)
		.Process(this->DropshipLoadout_PilotLit)
		.Process(this->DropshipLoadout_DGreenList)
		.Process(this->DropshipLoadout_DGreenAnimationsCount)
		.Process(this->DropshipLoadout_DGreenLocations)
		.Process(this->DropshipLoadout_DropshipCameosCount)
		.Process(this->DropshipLoadout_DropshipCameosLocations)
		.Process(this->DropshipLoadout_FixedUnits)
		.Process(this->DropshipLoadout_InitialUnits)
		.Process(this->DropshipLoadout_BuyClickSound)
		.Process(this->DropshipLoadout_SellClickSound)
		.Process(this->DropshipLoadout_ArrowsClickSound)
		.Process(this->DropshipLoadout_StartingDragDropSound)
		.Process(this->DropshipLoadout_EndingDragDropSound)
		.Process(this->DropshipLoadout_VeteranLevel)
		;
}

void SWTypeExt::LoadFromINIFile(CCINIClass* const pINI)
{
	auto pThis = this->OwnerObject();
	const char* pSection = pThis->ID;
	INI_EX exINI(pINI);

	this->TypeID.Read(pINI, pSection, "Type");

	// from ares
	this->Money_Amount.Read(exINI, pSection, "Money.Amount");
	this->EVA_Impatient.Read(exINI, pSection, "EVA.Impatient");
	this->EVA_InsufficientFunds.Read(exINI, pSection, "EVA.InsufficientFunds");
	this->EVA_SelectTarget.Read(exINI, pSection, "EVA.SelectTarget");
	this->SW_UseAITargeting.Read(exINI, pSection, "SW.UseAITargeting");
	this->SW_AutoFire.Read(exINI, pSection, "SW.AutoFire");
	this->SW_ManualFire.Read(exINI, pSection, "SW.ManualFire");
	this->SW_ShowCameo.Read(exINI, pSection, "SW.ShowCameo");
	this->SW_Unstoppable.Read(exINI, pSection, "SW.Unstoppable");
	this->SW_AllowPlayer.Read(exINI, pSection, "SW.AllowPlayer");
	this->SW_AllowAI.Read(exINI, pSection, "SW.AllowAI");
	this->SW_Inhibitors.Read(exINI, pSection, "SW.Inhibitors");
	this->SW_AnyInhibitor.Read(exINI, pSection, "SW.AnyInhibitor");
	this->SW_Designators.Read(exINI, pSection, "SW.Designators");
	this->SW_AnyDesignator.Read(exINI, pSection, "SW.AnyDesignator");
	this->SW_RangeMinimum.Read(exINI, pSection, "SW.RangeMinimum");
	this->SW_RangeMaximum.Read(exINI, pSection, "SW.RangeMaximum");
	this->SW_RequiredHouses = pINI->ReadHouseTypesList(pSection, "SW.RequiredHouses", this->SW_RequiredHouses);
	this->SW_ForbiddenHouses = pINI->ReadHouseTypesList(pSection, "SW.ForbiddenHouses", this->SW_ForbiddenHouses);
	this->SW_AuxBuildings.Read(exINI, pSection, "SW.AuxBuildings");
	this->SW_NegBuildings.Read(exINI, pSection, "SW.NegBuildings");
	this->SW_AuxTechnos.Read(exINI, pSection, "SW.AuxTechnos");
	this->SW_NegTechnos.Read(exINI, pSection, "SW.NegTechnos");
	this->SW_TechLevel.Read(exINI, pSection, "SW.TechLevel");
	this->SW_InitialReady.Read(exINI, pSection, "SW.InitialReady");
	this->SW_PostDependent.Read(exINI, pSection, "SW.PostDependent");
	this->SW_MaxCount.Read(exINI, pSection, "SW.MaxCount");
	this->SW_Shots.Read(exINI, pSection, "SW.Shots");

	this->Message_CannotFire.Read(exINI, pSection, "Message.CannotFire");
	this->Message_InsufficientFunds.Read(exINI, pSection, "Message.InsufficientFunds");

	// messages and their properties
	this->Message_FirerColor.Read(exINI, pSection, "Message.FirerColor");
	this->Message_ColorScheme.Read(exINI, pSection, "Message.Color");

	this->UIDescription.Read(exINI, pSection, "UIDescription");
	this->CameoPriority.Read(exINI, pSection, "CameoPriority");
	this->LimboDelivery_Types.Read(exINI, pSection, "LimboDelivery.Types");
	this->LimboDelivery_IDs.Read(exINI, pSection, "LimboDelivery.IDs");
	this->LimboDelivery_RollChances.Read(exINI, pSection, "LimboDelivery.RollChances");
	if (exINI.ReadString(pSection, "LimboKill.Affected") > 0)
	{
		Debug::Log("[Developer warning][%s] LimboKill.Affected is deprecated and has been replaced by LimboKill.AffectsHouse! If both are set, the latter will be used.\n", pSection);
	}
	this->LimboKill_AffectsHouse.Read(exINI, pSection, "LimboKill.Affected"); // Temporary solution for the INI tags renaming issue, see #2093
	this->LimboKill_AffectsHouse.Read(exINI, pSection, "LimboKill.AffectsHouse");
	this->LimboKill_IDs.Read(exINI, pSection, "LimboKill.IDs");
	this->LimboKill_Counts.Read(exINI, pSection, "LimboKill.Counts");
	this->SW_Next.Read(exINI, pSection, "SW.Next");
	this->SW_Next_RealLaunch.Read(exINI, pSection, "SW.Next.RealLaunch");
	this->SW_Next_IgnoreInhibitors.Read(exINI, pSection, "SW.Next.IgnoreInhibitors");
	this->SW_Next_IgnoreDesignators.Read(exINI, pSection, "SW.Next.IgnoreDesignators");
	this->SW_Next_RollChances.Read(exINI, pSection, "SW.Next.RollChances");

	this->ShowTimer_Priority.Read(exINI, pSection, "ShowTimer.Priority");
	this->ShowTimer_Percentage.Read(exINI, pSection, "ShowTimer.Percentage");

	this->EMPulse_WeaponIndex.Read(exINI, pSection, "EMPulse.WeaponIndex");
	this->EMPulse_SuspendOthers.Read(exINI, pSection, "EMPulse.SuspendOthers");
	this->EMPulse_Cannons.Read(exINI, pSection, "EMPulse.Cannons");
	this->EMPulse_TargetSelf.Read(exINI, pSection, "EMPulse.TargetSelf");

	char tempBuffer[32];
	// LimboDelivery.RandomWeights
	for (size_t i = 0; ; ++i)
	{
		ValueableVector<int> weights;
		_snprintf_s(tempBuffer, sizeof(tempBuffer), "LimboDelivery.RandomWeights%d", i);
		weights.Read(exINI, pSection, tempBuffer);

		if (!weights.size())
			break;

		if (this->LimboDelivery_RandomWeightsData.size() > i)
			this->LimboDelivery_RandomWeightsData[i] = std::move(weights);
		else
			this->LimboDelivery_RandomWeightsData.emplace_back(std::move(weights));
	}

	ValueableVector<int> weights;
	weights.Read(exINI, pSection, "LimboDelivery.RandomWeights");
	if (weights.size())
	{
		if (this->LimboDelivery_RandomWeightsData.size())
			this->LimboDelivery_RandomWeightsData[0] = std::move(weights);
		else
			this->LimboDelivery_RandomWeightsData.emplace_back(std::move(weights));
	}

	// SW.Next.RandomWeights
	for (size_t i = 0; ; ++i)
	{
		ValueableVector<int> weights2;
		_snprintf_s(tempBuffer, sizeof(tempBuffer), "SW.Next.RandomWeights%d", i);
		weights2.Read(exINI, pSection, tempBuffer);

		if (!weights2.size())
			break;

		if (this->SW_Next_RandomWeightsData.size() > i)
			this->SW_Next_RandomWeightsData[i] = std::move(weights2);
		else
			this->SW_Next_RandomWeightsData.emplace_back(std::move(weights2));
	}

	ValueableVector<int> weights2;
	weights2.Read(exINI, pSection, "SW.Next.RandomWeights");
	if (weights2.size())
	{
		if (this->SW_Next_RandomWeightsData.size())
			this->SW_Next_RandomWeightsData[0] = std::move(weights2);
		else
			this->SW_Next_RandomWeightsData.emplace_back(std::move(weights2));
	}

	this->SW_Link.Read(exINI, pSection, "SW.Link");
	this->SW_Link_Grant.Read(exINI, pSection, "SW.Link.Grant");
	this->SW_Link_Ready.Read(exINI, pSection, "SW.Link.Ready");
	this->SW_Link_Reset.Read(exINI, pSection, "SW.Link.Reset");
	this->Message_LinkedSWAcquired.Read(exINI, pSection, "Message.LinkedSWAcquired");
	this->EVA_LinkedSWAcquired.Read(exINI, pSection, "EVA.LinkedSWAcquired");
	this->SW_Link_RollChances.Read(exINI, pSection, "SW.Link.RollChances");

	this->Message_Activated_Owner.Read(exINI, pSection, "Message.Activated.Owner");
	this->Message_Activated_Allies.Read(exINI, pSection, "Message.Activated.Allies");
	this->Message_Activated_Enemies.Read(exINI, pSection, "Message.Activated.Enemies");
	this->EVA_Activated_Owner.Read(exINI, pSection, "EVA.Activated.Owner");
	this->EVA_Activated_Allies.Read(exINI, pSection, "EVA.Activated.Allies");
	this->EVA_Activated_Enemies.Read(exINI, pSection, "EVA.Activated.Enemies");

	// SW.Link.RandomWeights
	for (size_t i = 0; ; ++i)
	{
		ValueableVector<int> weights3;
		_snprintf_s(tempBuffer, sizeof(tempBuffer), "SW.Link.RandomWeights%d", i);
		weights3.Read(exINI, pSection, tempBuffer);

		if (!weights3.size())
			break;

		this->SW_Link_RandomWeightsData.emplace_back(std::move(weights3));
	}
	ValueableVector<int> weights3;
	weights3.Read(exINI, pSection, "SW.Link.RandomWeights");
	if (weights3.size())
	{
		if (this->SW_Link_RandomWeightsData.size())
			this->SW_Link_RandomWeightsData[0] = std::move(weights3);
		else
			this->SW_Link_RandomWeightsData.emplace_back(std::move(weights3));
	}

	this->Detonate_Warhead.Read<true>(exINI, pSection, "Detonate.Warhead");
	this->Detonate_Weapon.Read<true>(exINI, pSection, "Detonate.Weapon");
	this->Detonate_Damage.Read(exINI, pSection, "Detonate.Damage");
	this->Detonate_Warhead_Full.Read(exINI, pSection, "Detonate.Warhead.Full");
	this->Detonate_AtFirer.Read(exINI, pSection, "Detonate.AtFirer");

	// Convert.From & Convert.To
	TypeConvertGroup::Parse(this->Convert_Pairs, exINI, pSection, AffectedHouse::Owner);

	this->ShowDesignatorRange.Read(exINI, pSection, "ShowDesignatorRange");

	this->TabIndex.Read(exINI, pSection, "TabIndex");
	GeneralUtils::IntValidCheck(&this->TabIndex, pSection, "TabIndex", 1, 0, 3);

	this->SuperWeaponSidebar_Allow.Read(exINI, pSection, "SuperWeaponSidebar.Allow");
	this->SuperWeaponSidebar_PriorityHouses = pINI->ReadHouseTypesList(pSection, "SuperWeaponSidebar.PriorityHouses", this->SuperWeaponSidebar_PriorityHouses);
	this->SuperWeaponSidebar_RequiredHouses = pINI->ReadHouseTypesList(pSection, "SuperWeaponSidebar.RequiredHouses", this->SuperWeaponSidebar_RequiredHouses);
	this->SuperWeaponSidebar_Significance.Read(exINI, pSection, "SuperWeaponSidebar.Significance");

	this->SidebarPal.LoadFromINI(pINI, pSection, "SidebarPalette");
	this->SidebarPCX.Read(pINI, pSection, "SidebarPCX");

	this->UseWeeds.Read(exINI, pSection, "UseWeeds");
	this->UseWeeds_Amount.Read(exINI, pSection, "UseWeeds.Amount");
	this->UseWeeds_StorageTimer.Read(exINI, pSection, "UseWeeds.StorageTimer");
	this->UseWeeds_ReadinessAnimationPercentage.Read(exINI, pSection, "UseWeeds.ReadinessAnimationPercentage");

	int newidx = NewSWType::GetNewSWTypeIdx(TypeID.data());

	if (newidx != -1)
	{
		NewSWType* pNewSWType = NewSWType::GetNthItem(newidx);
		pNewSWType->Initialize(const_cast<SWTypeExt*>(this), OwnerObject());
		pNewSWType->LoadFromINI(const_cast<SWTypeExt*>(this), OwnerObject(), pINI);
	}

	this->DropshipLoadout_OpenWindow.Read(exINI, pSection, "DropshipLoadout.OpenWindow");
	this->DropshipLoadout_Launch.Read(exINI, pSection, "DropshipLoadout.Launch");
	this->DropshipLoadout_PersistentCargo.Read(exINI, pSection, "DropshipLoadout.PersistentCargo");
	this->DropshipLoadout_PreloadCargo.Read(exINI, pSection, "DropshipLoadout.PreloadCargo");
	this->DropshipLoadout_AddUnusedMoneyToPlayer.Read(exINI, pSection, "DropshipLoadout.AddUnusedMoneyToPlayer");
	this->DropshipLoadout_RememberPurchasedCargo.Read(exINI, pSection, "DropshipLoadout.RememberPurchasedCargo");

	if (pINI->ReadString(pSection, "DropshipLoadout.Palette", "", Phobos::readBuffer) != 0)
		this->DropshipLoadout_Palette = FileSystem::LoadPALFile(Phobos::readBuffer, DSurface::Hidden);
	this->DropshipLoadout_Carrier.Read(exINI, pSection, "DropshipLoadout.Carrier");

	this->DropshipLoadout_AllowableUnits.Read(exINI, pSection, "DropshipLoadout.AllowableUnits");
	this->DropshipLoadout_AllowableUnitMaximums.Read(exINI, pSection, "DropshipLoadout.AllowableUnitMaximums");
	this->DropshipLoadout_Money.Read(exINI, pSection, "DropshipLoadout.Money");
	this->DropshipLoadout_VeteranLevel.Read(exINI, pSection, "DropshipLoadout.VeteranLevel");
	this->DropshipLoadout_StartEVA.Read(exINI, pSection, "DropshipLoadout.StartEVA");
	this->DropshipLoadout_SizeLimit.Read(exINI, pSection, "DropshipLoadout.SizeLimit");

	if (pINI->ReadString(pSection, "DropshipLoadout.Theme", "", Phobos::readBuffer) > 0)
		this->DropshipLoadout_Theme = pINI->ReadTheme(pSection, "DropshipLoadout.Theme", -1);

	if (pINI->ReadString(pSection, "DropshipLoadout.BackgroundPCX", "", Phobos::readBuffer) != 0)
	{
		this->DropshipLoadout_BackgroundPCXPattern = Phobos::readBuffer;
		char filename[260];
		_snprintf_s(filename, sizeof(filename), Phobos::readBuffer, 1);
		this->DropshipLoadout_BackgroundPCX = PhobosPCXFile(_strdup(filename));
	}

	pINI->ReadString(pSection, "DropshipLoadout.UpArrowPCX", "", Phobos::readBuffer);
	this->DropshipLoadout_UpArrowPCX = PhobosPCXFile(Phobos::readBuffer);

	pINI->ReadString(pSection, "DropshipLoadout.DownArrowPCX", "", Phobos::readBuffer);
	this->DropshipLoadout_DownArrowPCX = PhobosPCXFile(Phobos::readBuffer);

	pINI->ReadString(pSection, "DropshipLoadout.LoadoutPCX", "", Phobos::readBuffer);
	this->DropshipLoadout_LoadoutPCX = PhobosPCXFile(Phobos::readBuffer);

	pINI->ReadString(pSection, "DropshipLoadout.PilotLitPCX", "", Phobos::readBuffer);
	this->DropshipLoadout_PilotLitPCX = PhobosPCXFile(Phobos::readBuffer);

	char* context = nullptr;

	if (pINI->ReadString(pSection, "DropshipLoadout.DGreenListPCX", "", Phobos::readBuffer) > 0)
	{
		this->DropshipLoadout_DGreenListPCX.clear();

		for (char* cur = strtok_s(Phobos::readBuffer, Phobos::readDelims, &context); cur; cur = strtok_s(nullptr, Phobos::readDelims, &context))
		{
			if (auto pFrames = GeneralUtils::GetAnimationPCX(cur))
			{
				for (auto& frame : *pFrames)
				{
					this->DropshipLoadout_DGreenListPCX.emplace_back(std::move(frame));
				}
			}
		}
	}

	this->DropshipLoadout_Background.Read(exINI, pSection, "DropshipLoadout.Background");
	this->DropshipLoadout_UpArrow.Read(exINI, pSection, "DropshipLoadout.UpArrow");
	this->DropshipLoadout_DownArrow.Read(exINI, pSection, "DropshipLoadout.DownArrow");
	this->DropshipLoadout_Loadout.Read(exINI, pSection, "DropshipLoadout.Loadout");
	this->DropshipLoadout_PilotLit.Read(exINI, pSection, "DropshipLoadout.PilotLit");
	context = nullptr;

	if (pINI->ReadString(pSection, "DropshipLoadout.DGreenList", "", Phobos::readBuffer) > 0)
	{
		this->DropshipLoadout_DGreenList.clear();

		for (char* cur = strtok_s(Phobos::readBuffer, Phobos::readDelims, &context); cur; cur = strtok_s(nullptr, Phobos::readDelims, &context))
		{
			std::string Result = cur;

			if (Result.size() < 4 || !std::equal(Result.end() - 4, Result.end(), ".shp", [](char input, char expected) { return std::tolower(input) == expected; }))
				Result += ".shp";

			if (auto const pImage = FileSystem::LoadSHPFile(Result.c_str()))
				this->DropshipLoadout_DGreenList.push_back(pImage);
			else
				Debug::Log("Failed to find file %s referenced by [%s]DropshipLoadout.DGreenList=%s\n", Result.c_str(), pSection, cur);
		}
	}

	if (pINI->ReadString(pSection, "DropshipLoadout.DGreenAnimationsCount", "", Phobos::readBuffer) > 0)
	{
		this->DropshipLoadout_DGreenAnimationsCount.Read(exINI, pSection, "DropshipLoadout.DGreenAnimationsCount");
		this->DropshipLoadout_DGreenLocations.clear();

		for (int i = 0; i < this->DropshipLoadout_DGreenAnimationsCount.Get(0); i++)
		{
			char tempBuffer[256];
			Point2D location = Point2D::Empty;
			_snprintf_s(tempBuffer, sizeof(tempBuffer), "DropshipLoadout.DGreenLocation%d", i);
			pINI->ReadPoint2D(location, pSection, tempBuffer, location);
			this->DropshipLoadout_DGreenLocations.push_back(location);
		}
	}
	else
	{
		this->DropshipLoadout_DGreenAnimationsCount.Read(exINI, pSection, "DropshipLoadout.DGreenAnimationsCount");
	}

	this->DropshipLoadout_LoadoutLocation.Read(exINI, pSection, "DropshipLoadout.LoadoutLocation");
	this->DropshipLoadout_PilotLitLocation.Read(exINI, pSection, "DropshipLoadout.PilotLitLocation");
	this->DropshipLoadout_UpArrowLocation.Read(exINI, pSection, "DropshipLoadout.UpArrowLocation");
	this->DropshipLoadout_DownArrowLocation.Read(exINI, pSection, "DropshipLoadout.DownArrowLocation");

	if (pINI->ReadString(pSection, "DropshipLoadout.SidebarCameosCount", "", Phobos::readBuffer) > 0)
	{
		this->DropshipLoadout_SidebarCameosCount.Read(exINI, pSection, "DropshipLoadout.SidebarCameosCount");
		this->DropshipLoadout_SidebarCameosLocations.clear();

		for (int i = 0; i < this->DropshipLoadout_SidebarCameosCount; i++)
		{
			char tempBuffer[256];
			Point2D location = Point2D::Empty;
			_snprintf_s(tempBuffer, sizeof(tempBuffer), "DropshipLoadout.SidebarCameoLocation%d", i);
			pINI->ReadPoint2D(location, pSection, tempBuffer, location);
			this->DropshipLoadout_SidebarCameosLocations.push_back(location);
		}
	}
	else
	{
		this->DropshipLoadout_SidebarCameosCount.Read(exINI, pSection, "DropshipLoadout.SidebarCameosCount");
	}

	this->DropshipLoadout_DropshipCameosCount.Read(exINI, pSection, "DropshipLoadout.DropshipCameosCount");
	int cameosCount = this->DropshipLoadout_DropshipCameosCount.Get(0);
	char countKey[256];
	_snprintf_s(countKey, sizeof(countKey), "DropshipLoadout.Dropship0.CameosCount");
	cameosCount = pINI->ReadInteger(pSection, countKey, cameosCount);

	if (cameosCount > 0)
	{
		this->DropshipLoadout_DropshipCameosCount = cameosCount;
		this->DropshipLoadout_DropshipCameosLocations.clear();

		for (int j = 0; j < cameosCount; j++)
		{
			char tempBuffer[256];
			Point2D location = Point2D::Empty;

			// Try DropshipLoadout.CameoLocation%d (preferred clean format)
			_snprintf_s(tempBuffer, sizeof(tempBuffer), "DropshipLoadout.CameoLocation%d", j);

			if (pINI->Exists(pSection, tempBuffer))
			{
				pINI->ReadPoint2D(location, pSection, tempBuffer, location);
				this->DropshipLoadout_DropshipCameosLocations.push_back(location);
				continue;
			}

			// Try DropshipLoadout.Dropship0.CameoLocation%d (matches DropshipN.CameoLocationM index 0)
			_snprintf_s(tempBuffer, sizeof(tempBuffer), "DropshipLoadout.Dropship0.CameoLocation%d", j);

			if (pINI->Exists(pSection, tempBuffer))
			{
				pINI->ReadPoint2D(location, pSection, tempBuffer, location);
				this->DropshipLoadout_DropshipCameosLocations.push_back(location);
				continue;
			}

			// Default: DropshipLoadout.Dropship.CameoLocation%d (legacy dot format)
			_snprintf_s(tempBuffer, sizeof(tempBuffer), "DropshipLoadout.Dropship.CameoLocation%d", j);
			pINI->ReadPoint2D(location, pSection, tempBuffer, location);
			this->DropshipLoadout_DropshipCameosLocations.push_back(location);
		}
	}

	this->DropshipLoadout_FixedUnits.clear();

	if (pINI->ReadString(pSection, "DropshipLoadout.FixedUnits", "", Phobos::readBuffer) > 0)
	{
		char* ctx = nullptr;

		for (char* cur = strtok_s(Phobos::readBuffer, Phobos::readDelims, &ctx); cur; cur = strtok_s(nullptr, Phobos::readDelims, &ctx))
		{
			if (auto pType = TechnoTypeClass::Find(cur))
				this->DropshipLoadout_FixedUnits.push_back(pType);
		}
	}

	this->DropshipLoadout_InitialUnits.clear();

	if (pINI->ReadString(pSection, "DropshipLoadout.InitialUnits", "", Phobos::readBuffer) > 0)
	{
		char* ctx = nullptr;

		for (char* cur = strtok_s(Phobos::readBuffer, Phobos::readDelims, &ctx); cur; cur = strtok_s(nullptr, Phobos::readDelims, &ctx))
		{
			if (auto pType = TechnoTypeClass::Find(cur))
				this->DropshipLoadout_InitialUnits.push_back(pType);
		}
	}

	this->DropshipLoadout_BuyClickSound.Read(exINI, pSection, "DropshipLoadout.BuyClickSound");
	this->DropshipLoadout_SellClickSound.Read(exINI, pSection, "DropshipLoadout.SellClickSound");
	this->DropshipLoadout_ArrowsClickSound.Read(exINI, pSection, "DropshipLoadout.ArrowsClickSound");
	this->DropshipLoadout_StartingDragDropSound.Read(exINI, pSection, "DropshipLoadout.StartingDragDropSound");
	this->DropshipLoadout_EndingDragDropSound.Read(exINI, pSection, "DropshipLoadout.EndingDragDropSound");
}

void SWTypeExt::LoadFromStream(PhobosStreamReader& Stm)
{
	AbstractTypeExt::LoadFromStream(Stm);
	this->Serialize(Stm);
}

void SWTypeExt::SaveToStream(PhobosStreamWriter& Stm)
{
	AbstractTypeExt::SaveToStream(Stm);
	this->Serialize(Stm);
}

bool SWTypeExt::LoadGlobals(PhobosStreamReader& Stm)
{
	return Stm
		.Success();
}

bool SWTypeExt::SaveGlobals(PhobosStreamWriter& Stm)
{
	return Stm
		.Success();
}

bool SWTypeExt::Activate(SuperClass* pSuper, CellStruct cell, bool isPlayer)
{
	const auto pSWTypeExt = SWTypeExt::Fetch(pSuper->Type);
	const int newIdx = NewSWType::GetNewSWTypeIdx(pSWTypeExt->TypeID.data());

	Debug::Log("[Phobos::SW::Active] %s\n", pSWTypeExt->TypeID.data());

	if (newIdx != -1)
		return NewSWType::GetNthItem(newIdx)->Activate(pSuper, cell, isPlayer);

	return false;
}

// =============================
// container

SWTypeExt::ExtContainer::ExtContainer() : Container("SuperWeaponTypeClass")
{ }

SWTypeExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

DEFINE_HOOK(0x6CE6F6, SuperWeaponTypeClass_CTOR, 0x5)
{
	GET(SuperWeaponTypeClass*, pItem, EAX);

	SWTypeExt::ExtMap.TryAllocate(pItem);

	return 0;
}

DEFINE_HOOK(0x6CEFE0, SuperWeaponTypeClass_SDDTOR, 0x8)
{
	GET(SuperWeaponTypeClass*, pItem, ECX);

	SWTypeExt::ExtMap.Remove(pItem);
	return 0;
}

//DEFINE_HOOK_AGAIN(0x6CEE50, SuperWeaponTypeClass_LoadFromINI, 0xA)// Section dont exist!
DEFINE_HOOK(0x6CEE43, SuperWeaponTypeClass_LoadFromINI, 0xA)
{
	GET(SuperWeaponTypeClass*, pItem, EBP);
	GET_STACK(CCINIClass*, pINI, 0x3FC);

	SWTypeExt::ExtMap.LoadFromINI(pItem, pINI);
	return 0;
}
