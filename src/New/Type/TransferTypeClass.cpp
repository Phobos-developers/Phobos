#include "TransferTypeClass.h"

#include <Utilities/TemplateDef.h>
#include <HouseClass.h>

Enumerable<TransferTypeClass>::container_t Enumerable<TransferTypeClass>::Array;

const char* Enumerable<TransferTypeClass>::GetMainSection()
{
	return "TransferTypes";
}

void TransferTypeClass::LoadFromINI(CCINIClass* pINI)
{
	const char* section = this->Name;

	INI_EX exINI(pINI);

	this->Attribute.Read(exINI, section, "Attribute");

	this->Attribute.Read(exINI, section, "Attribute");
	this->TargetToSource.Read(exINI, section, "TargetToSource");

	this->Receive_Value.Read(exINI, section, "Receive.Value");
	this->Receive_Value_IsPercentReceived.Read(exINI, section, "Receive.Value.IsPercentReceived");
	this->Receive_Value_Minimum.Read(exINI, section, "Receive.Value.Minimum");
	this->Receive_Value_Maximum.Read(exINI, section, "Receive.Value.Maximum");

	this->Receive_Text.Read(exINI, section, "Receive.Text");
	this->Receive_Text_Houses.Read(exINI, section, "Receive.Text.Houses");
	this->Receive_Text_Color.Read(exINI, section, "Receive.Text.Color");
	this->Receive_Text_Offset.Read(exINI, section, "Receive.Text.Offset");

	this->Send_Value.Read(exINI, section, "Send.Value");
	this->Send_Value_IsPercent.Read(exINI, section, "Send.Value.IsPercent");
	this->Send_Value_Minimum.Read(exINI, section, "Send.Value.Minimum");
	this->Send_Value_Maximum.Read(exINI, section, "Send.Value.Maximum");

	this->Send_Text.Read(exINI, section, "Send.Text");
	this->Send_Text_Houses.Read(exINI, section, "Send.Text.Houses");
	this->Send_Text_Color.Read(exINI, section, "Send.Text.Color");
	this->Send_Text_Offset.Read(exINI, section, "Send.Text.Offset");

	this->Send_ConsiderArmor.Read(exINI, section, "Send.ConsiderArmor");
	this->Send_Warhead.Read(exINI, section, "Send.Warhead");
	this->Send_SpreadDistribution.Read(exINI, section, "Send.SpreadDistribution");

	this->Send_RequireRealResource.Read(exINI, section, "Send.RequireRealResource");
	this->Send_PercentOfTotal.Read(exINI, section, "Send.PercentOfTotal");
	this->Send_Experience_AllowDemote.Read(exINI, section, "Send.Experience.AllowDemote");
}

template <typename T>
void TransferTypeClass::Serialize(T& Stm)
{
	Stm
		.Process(this->Attribute)
		.Process(this->TargetToSource)
		.Process(this->Receive_Value)
		.Process(this->Receive_Value_IsPercentReceived)
		.Process(this->Receive_Value_Minimum)
		.Process(this->Receive_Value_Maximum)
		.Process(this->Receive_Text)
		.Process(this->Receive_Text_Houses)
		.Process(this->Receive_Text_Color)
		.Process(this->Receive_Text_Offset)
		.Process(this->Send_Value)
		.Process(this->Send_Value_IsPercent)
		.Process(this->Send_Value_Minimum)
		.Process(this->Send_Value_Maximum)
		.Process(this->Send_Text)
		.Process(this->Send_Text_Houses)
		.Process(this->Send_Text_Color)
		.Process(this->Send_Text_Offset)
		.Process(this->Send_ConsiderArmor)
		.Process(this->Send_Warhead)
		.Process(this->Send_SpreadDistribution)
		.Process(this->Send_RequireRealResource)
		.Process(this->Send_PercentOfTotal)
		.Process(this->Send_Experience_AllowDemote)
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
