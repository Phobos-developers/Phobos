#pragma once

#include <WarheadTypeClass.h>
#include <Utilities/Enumerable.h>
#include <Utilities/Template.h>
#include <Utilities/EnumFunctions.h>

class TransferTypeClass final : public Enumerable<TransferTypeClass>
{
public:
	Nullable<TransferTypeAttribute> Attribute;
	Valueable<bool> TargetToSource;

	Valueable<double> Receive_Value;
	Valueable<bool> Receive_Value_IsPercentReceived;
	Valueable<bool> Receive_Value_Minimum;
	Valueable<bool> Receive_Value_Maximum;

	Valueable<bool> Receive_Text;
	Valueable<AffectedHouse> Receive_Text_Houses;
	Valueable<ColorStruct> Receive_Text_Color;
	Valueable<Point2D> Receive_Text_Offset;

	Valueable<double> Send_Value;
	Valueable<bool> Send_Value_IsPercent;
	Valueable<bool> Send_Value_Minimum;
	Valueable<bool> Send_Value_Maximum;

	Valueable<bool> Send_Text;
	Valueable<AffectedHouse> Send_Text_Houses;
	Valueable<ColorStruct> Send_Text_Color;
	Valueable<Point2D> Send_Text_Offset;

	Valueable<bool> Send_ConsiderArmor;
	Nullable<WarheadTypeClass*> Send_Warhead;
	Valueable<SpreadDistribution> Send_SpreadDistribution;

	Valueable<bool> Send_RequireRealResource;
	Valueable<bool> Send_PercentOfTotal;
	Valueable<bool> Send_Experience_AllowDemote;

	TransferTypeClass(const char* pTitle = NONE_STR) : Enumerable<TransferTypeClass>(pTitle)
		, Attribute {}
		, TargetToSource { false }
		, Receive_Value { 0.0 }
		, Receive_Value_IsPercentReceived { false }
		, Receive_Value_Minimum { false }
		, Receive_Value_Maximum { false }
		, Receive_Text { false }
		, Receive_Text_Houses { AffectedHouse::All }
		, Receive_Text_Color { { 0, 0, 0 } }
		, Receive_Text_Offset { { 0, 0 } }
		, Send_Value { 0.0 }
		, Send_Value_IsPercent { false }
		, Send_Value_Minimum { false }
		, Send_Value_Maximum { false }
		, Send_Text { false }
		, Send_Text_Houses { AffectedHouse::All }
		, Send_Text_Color { { 0, 0, 0 } }
		, Send_Text_Offset { { 0, 0 } }
		, Send_ConsiderArmor { false }
		, Send_Warhead {}
		, Send_SpreadDistribution { }
		, Send_RequireRealResource { false }
		, Send_PercentOfTotal { false }
		, Send_Experience_AllowDemote { false }
	{ }

	virtual ~TransferTypeClass() override = default;

	virtual void LoadFromINI(CCINIClass* pINI) override;
	virtual void LoadFromStream(PhobosStreamReader& Stm);
	virtual void SaveToStream(PhobosStreamWriter& Stm);

private:
	template <typename T>
	void Serialize(T& Stm);
};
