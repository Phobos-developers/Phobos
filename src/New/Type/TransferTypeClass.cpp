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
	this->Send_Value_IsPercent.Read(exINI, section, "Send.Value.IsPercent");
	this->Send_Value_PercentOfTotal.Read(exINI, section, "Send.Value.PercentOfTotal");
	this->Send_Value_FlatMinimum.Read(exINI, section, "Send.Value.FlatMinimum");
	this->Send_Value_FlatMaximum.Read(exINI, section, "Send.Value.FlatMaximum");
	this->Send_ConsiderSourceVeterancy.Read(exINI, section, "Send.ConsiderSourceVeterancy");
	this->Receive_Resource.Read(exINI, section, "Receive.Resource");
	this->Receive_Value.Read(exINI, section, "Receive.Value");
	this->Receive_Value_IsPercent.Read(exINI, section, "Receive.Value.IsPercent");
	this->Receive_Value_PercentOfTotal.Read(exINI, section, "Receive.Value.PercentOfTotal");
	this->Receive_Value_ProportionalToSent.Read(exINI, section, "Receive.Value.ProportionalToSent");
	this->Receive_Value_FlatMinimum.Read(exINI, section, "Receive.Value.FlatMinimum");
	this->Receive_Value_FlatMaximum.Read(exINI, section, "Receive.Value.FlatMaximum");
	this->Receive_ConsiderSourceVeterancy.Read(exINI, section, "Receive.ConsiderSourceVeterancy");
	this->Send_RequireRealResource.Read(exINI, section, "Send.RequireRealResource");
	this->Send_Experience_AllowDemote.Read(exINI, section, "Send.Experience.AllowDemote");
	this->Receive_RefuseOverflow.Read(exINI, section, "Receive.RefuseOverflow");
	this->Target_ConsiderArmor.Read(exINI, section, "Target.ConsiderArmor");
	this->Target_Warhead.Read(exINI, section, "Target.Warhead");
	this->Target_AffectedHouses.Read(exINI, section, "Target.AffectedHouses");
	this->Target_Spread_Distribution.Read(exINI, section, "Target.Spread.Distribution");
	this->Target_Spread_CountUnaffected.Read(exINI, section, "Target.Spread.CountUnaffected");
	this->Send_Text.Read(exINI, section, "Send.Text");
	this->Send_Text_ShowSign.Read(exINI, section, "Send.Text.ShowSign");
	this->Send_Text_Houses.Read(exINI, section, "Send.Text.Houses");
	this->Send_Text_Color.Read(exINI, section, "Send.Text.Color");
	this->Send_Text_Offset.Read(exINI, section, "Send.Text.Offset");
	this->Receive_Text.Read(exINI, section, "Receive.Text");
	this->Receive_Text_ShowSign.Read(exINI, section, "Receive.Text.ShowSign");
	this->Receive_Text_Houses.Read(exINI, section, "Receive.Text.Houses");
	this->Receive_Text_Color.Read(exINI, section, "Receive.Text.Color");
	this->Receive_Text_Offset.Read(exINI, section, "Receive.Text.Offset");

	Absolutize(this->Send_Value);
	Absolutize(this->Send_Value_FlatMinimum);
	Absolutize(this->Send_Value_FlatMaximum);
	Absolutize(this->Receive_Value);
	Absolutize(this->Receive_Value_FlatMinimum);
	Absolutize(this->Receive_Value_FlatMaximum);
}

template <typename T>
void TransferTypeClass::Serialize(T& Stm)
{
	Stm
		.Process(this->TargetToSource)
		.Process(this->Send_Resource)
		.Process(this->Send_Value)
		.Process(this->Send_Value_IsPercent)
		.Process(this->Send_Value_PercentOfTotal)
		.Process(this->Send_Value_FlatMinimum)
		.Process(this->Send_Value_FlatMaximum)
		.Process(this->Send_ConsiderSourceVeterancy)
		.Process(this->Receive_Resource)
		.Process(this->Receive_Value)
		.Process(this->Receive_Value_IsPercent)
		.Process(this->Receive_Value_PercentOfTotal)
		.Process(this->Receive_Value_ProportionalToSent)
		.Process(this->Receive_Value_FlatMinimum)
		.Process(this->Receive_Value_FlatMaximum)
		.Process(this->Receive_ConsiderSourceVeterancy)
		.Process(this->Send_RequireRealResource)
		.Process(this->Send_Experience_AllowDemote)
		.Process(this->Receive_RefuseOverflow)
		.Process(this->Target_ConsiderArmor)
		.Process(this->Target_Warhead)
		.Process(this->Target_AffectedHouses)
		.Process(this->Target_Spread_Distribution)
		.Process(this->Target_Spread_CountUnaffected)
		.Process(this->Send_Text)
		.Process(this->Send_Text_ShowSign)
		.Process(this->Send_Text_Houses)
		.Process(this->Send_Text_Color)
		.Process(this->Send_Text_Offset)
		.Process(this->Receive_Text)
		.Process(this->Receive_Text_ShowSign)
		.Process(this->Receive_Text_Houses)
		.Process(this->Receive_Text_Color)
		.Process(this->Receive_Text_Offset)
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
