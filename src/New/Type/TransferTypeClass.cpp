#include "TransferTypeClass.h"

#include <Utilities/TemplateDef.h>
#include <HouseClass.h>

template <typename A>
void Absolutize(Valueable<A>& value)
{
	if (!std::is_arithmetic<A>::value)
		return;
	if (value < 0.0)
		value = -value;
}

Enumerable<TransferTypeClass>::container_t Enumerable<TransferTypeClass>::Array;


const char* Enumerable<TransferTypeClass>::GetMainSection()
{
	return "TransferTypes";
}

void TransferTypeClass::LoadFromINI(CCINIClass* pINI)
{
	const char* section = this->Name;

	INI_EX exINI(pINI);

	this->Direction.Read(exINI, section, "Direction");
	this->Target_ConsiderArmor.Read(exINI, section, "Target.ConsiderArmor");
	this->Target_VersusWarhead.Read(exINI, section, "Target.VersusWarhead");
	this->Target_AffectHouses.Read(exINI, section, "Target.AffectHouses");
	this->Target_Spread_IgnoreSelf.Read(exINI, section, "Target.Spread.IgnoreSelf");
	this->Extra_Warhead.Read(exINI, section, "Extra.Warhead");
	this->Extra_ConsiderArmor.Read(exINI, section, "Extra.ConsiderArmor");
	this->Extra_AffectHouses.Read(exINI, section, "Extra.AffectHouses");
	this->Extra_Spread_EpicenterIsSource.Read(exINI, section, "Extra.Spread.EpicenterIsSource");
	this->Extra_Spread_IgnoreEpicenter.Read(exINI, section, "Extra.Spread.IgnoreEpicenter");
	this->VeterancyMultiplier_SourceOverSender.Read(exINI, section, "VeterancyMultiplier.SourceOverSender");
	this->VeterancyMultiplier_SourceOverReceiver.Read(exINI, section, "VeterancyMultiplier.SourceOverReceiver");
	this->VeterancyMultiplier_TargetOverTarget.Read(exINI, section, "VeterancyMultiplier.TargetOverTarget");
	this->VeterancyMultiplier_ExtraOverExtra.Read(exINI, section, "VeterancyMultiplier.ExtraOverExtra");
	this->Send_Resource.Read(exINI, section, "Send.Resource");
	this->Send_Value.Read(exINI, section, "Send.Value");
	this->Send_Type.Read(exINI, section, "Send.Value.Type");
	this->Send_FlatLimits.Read(exINI, section, "Send.Value.FlatLimits");
	this->Send_PreventUnderflow.Read(exINI, section, "Send.PreventUnderflow");
	this->Send_PreventOverflow.Read(exINI, section, "Send.PreventOverflow");
	this->Receive_Resource.Read(exINI, section, "Receive.Resource");
	this->Receive_Value.Read(exINI, section, "Receive.Value");
	this->Receive_Type.Read(exINI, section, "Receive.Value.Type");
	this->Receive_FlatLimits.Read(exINI, section, "Receive.Value.FlatLimits");
	this->Receive_ReturnUnderflow.Read(exINI, section, "Receive.ReturnUnderflow");
	this->Receive_ReturnOverflow.Read(exINI, section, "Receive.ReturnOverflow");
	this->Receive_SentFactor.Read(exINI, section, "Receive.SentFactor");
	this->Receive_Split.Read(exINI, section, "Receive.Split");
	this->Experience_PreventDemote.Read(exINI, section, "Experience.PreventDemote");
	this->Health_PreventKill.Read(exINI, section, "Health.PreventKill");
	this->GatlingRate_LimitStageChange.Read(exINI, section, "GatlingRate.LimitStageChange");
	this->Money_Display_Sender.Read(exINI, section, "Money.Display.Sender");
	this->Money_Display_Sender_Houses.Read(exINI, section, "Money.Display.Sender.Houses");
	this->Money_Display_Sender_Offset.Read(exINI, section, "Money.Display.Sender.Offset");
	this->Money_Display_Receiver.Read(exINI, section, "Money.Display.Receiver");
	this->Money_Display_Receiver_Houses.Read(exINI, section, "Money.Display.Receiver.Houses");
	this->Money_Display_Receiver_Offset.Read(exINI, section, "Money.Display.Receiver.Offset");
}

template <typename T>
void TransferTypeClass::Serialize(T& Stm)
{
	Stm
		.Process(this->Direction)
		.Process(this->Target_ConsiderArmor)
		.Process(this->Target_VersusWarhead)
		.Process(this->Target_AffectHouses)
		.Process(this->Target_Spread_IgnoreSelf)
		.Process(this->Extra_Warhead)
		.Process(this->Extra_ConsiderArmor)
		.Process(this->Extra_AffectHouses)
		.Process(this->Extra_Spread_EpicenterIsSource)
		.Process(this->Extra_Spread_IgnoreEpicenter)
		.Process(this->VeterancyMultiplier_SourceOverSender)
		.Process(this->VeterancyMultiplier_SourceOverReceiver)
		.Process(this->VeterancyMultiplier_TargetOverTarget)
		.Process(this->VeterancyMultiplier_ExtraOverExtra)
		.Process(this->Send_Resource)
		.Process(this->Send_Value)
		.Process(this->Send_Type)
		.Process(this->Send_FlatLimits)
		.Process(this->Send_PreventUnderflow)
		.Process(this->Send_PreventOverflow)
		.Process(this->Receive_Resource)
		.Process(this->Receive_Value)
		.Process(this->Receive_Type)
		.Process(this->Receive_FlatLimits)
		.Process(this->Receive_ReturnUnderflow)
		.Process(this->Receive_ReturnOverflow)
		.Process(this->Receive_SentFactor)
		.Process(this->Receive_Split)
		.Process(this->Experience_PreventDemote)
		.Process(this->Health_PreventKill)
		.Process(this->GatlingRate_LimitStageChange)
		.Process(this->Money_Display_Sender)
		.Process(this->Money_Display_Sender_Houses)
		.Process(this->Money_Display_Sender_Offset)
		.Process(this->Money_Display_Receiver)
		.Process(this->Money_Display_Receiver_Houses)
		.Process(this->Money_Display_Receiver_Offset)
		;
}

void TransferTypeClass::LoadFromStream(PhobosStreamReader& Stm)
{
	this->Serialize(Stm);
}

void TransferTypeClass::SaveToStream(PhobosStreamWriter& Stm)
{
	this->Serialize(Stm);
}
