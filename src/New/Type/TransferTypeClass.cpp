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

	this->TargetToSource.Read(exINI, section, "TargetToSource");
	this->Send_Resource.Read(exINI, section, "Send.Resource");
	this->Send_Value.Read(exINI, section, "Send.Value");
	this->Send_Value_Type.Read(exINI, section, "Send.Value.Type");
	this->Send_Value_FlatLimits.Read(exINI, section, "Send.Value.FlatLimits");
	this->Send_Value_SourceVeterancyMultiplier.Read(exINI, section, "Send.Value.SourceVeterancyMultiplier");
	this->Send_PreventUnderflow.Read(exINI, section, "Send.PreventUnderflow");
	this->Send_PreventOverflow.Read(exINI, section, "Send.PreventOverflow");
	this->Send_Text.Read(exINI, section, "Send.Text");
	this->Send_Text_ShowSign.Read(exINI, section, "Send.Text.ShowSign");
	this->Send_Text_Houses.Read(exINI, section, "Send.Text.Houses");
	this->Send_Text_Color.Read(exINI, section, "Send.Text.Color");
	this->Send_Text_Color_Positive.Read(exINI, section, "Send.Text.Color.Positive");
	this->Send_Text_Offset.Read(exINI, section, "Send.Text.Offset");
	this->Receive_Resource.Read(exINI, section, "Receive.Resource");
	this->Receive_Value.Read(exINI, section, "Receive.Value");
	this->Receive_Value_Type.Read(exINI, section, "Receive.Value.Type");
	this->Receive_Value_FlatLimits.Read(exINI, section, "Receive.Value.FlatLimits");
	this->Receive_Value_SourceVeterancyMultiplier.Read(exINI, section, "Receive.Value.SourceVeterancyMultiplier");
	this->Receive_Multiplier.Read(exINI, section, "Receive.Multiplier");
	this->Receive_ReturnOverflow.Read(exINI, section, "Receive.ReturnOverflow");
	this->Receive_Text.Read(exINI, section, "Receive.Text");
	this->Receive_Text_ShowSign.Read(exINI, section, "Receive.Text.ShowSign");
	this->Receive_Text_Houses.Read(exINI, section, "Receive.Text.Houses");
	this->Receive_Text_Color.Read(exINI, section, "Receive.Text.Color");
	this->Receive_Text_Color_Negative.Read(exINI, section, "Receive.Text.Color.Negative");
	this->Receive_Text_Offset.Read(exINI, section, "Receive.Text.Offset");
	this->Decrease_Experience_AllowDemote.Read(exINI, section, "Decrease.Experience.AllowDemote");
	this->Decrease_Health_AllowKill.Read(exINI, section, "Decrease.Health.AllowKill");
	this->Target_ConsiderArmor.Read(exINI, section, "Target.ConsiderArmor");
	this->Target_Warhead.Read(exINI, section, "Target.Warhead");
	this->Target_AffectHouses.Read(exINI, section, "Target.AffectHouses");
	this->Target_Spread_CountUnaffected.Read(exINI, section, "Target.Spread.CountUnaffected");
	this->Target_Spread_Distribution.Read(exINI, section, "Target.Spread.Distribution");
	this->Target_Spread_IgnoreSelf.Read(exINI, section, "Target.Spread.IgnoreSelf");
}

template <typename T>
void TransferTypeClass::Serialize(T& Stm)
{
	Stm
		.Process(this->TargetToSource)
		.Process(this->Send_Resource)
		.Process(this->Send_Value)
		.Process(this->Send_Value_Type)
		.Process(this->Send_Value_FlatLimits)
		.Process(this->Send_Value_SourceVeterancyMultiplier)
		.Process(this->Send_PreventUnderflow)
		.Process(this->Send_PreventOverflow)
		.Process(this->Send_Text)
		.Process(this->Send_Text_ShowSign)
		.Process(this->Send_Text_Houses)
		.Process(this->Send_Text_Color)
		.Process(this->Send_Text_Color_Positive)
		.Process(this->Send_Text_Offset)
		.Process(this->Receive_Resource)
		.Process(this->Receive_Value)
		.Process(this->Receive_Value_Type)
		.Process(this->Receive_Value_FlatLimits)
		.Process(this->Receive_Value_SourceVeterancyMultiplier)
		.Process(this->Receive_Multiplier)
		.Process(this->Receive_ReturnOverflow)
		.Process(this->Receive_Text)
		.Process(this->Receive_Text_ShowSign)
		.Process(this->Receive_Text_Houses)
		.Process(this->Receive_Text_Color)
		.Process(this->Receive_Text_Color_Negative)
		.Process(this->Receive_Text_Offset)
		.Process(this->Decrease_Experience_AllowDemote)
		.Process(this->Decrease_Health_AllowKill)
		.Process(this->Target_ConsiderArmor)
		.Process(this->Target_Warhead)
		.Process(this->Target_AffectHouses)
		.Process(this->Target_Spread_CountUnaffected)
		.Process(this->Target_Spread_Distribution)
		.Process(this->Target_Spread_IgnoreSelf)
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
