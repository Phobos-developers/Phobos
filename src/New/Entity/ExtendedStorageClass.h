#pragma once

#include <Utilities/TemplateDef.h>
#include <TiberiumClass.h>


class ExtendedStorageClass
{
public:
	ExtendedStorageClass() : Tiberiums(TiberiumClass::Array->Count) {}
	~ExtendedStorageClass() = default;

	int Get_Total_Value() const;
	float Get_Total_Amount() const;
	float Get_Amount(int tiberium) const;
	float Increase_Amount(float amount, int tiberium);
	float Decrease_Amount(float amount, int tiberium);
	int First_Used_Slot() const;

	ExtendedStorageClass operator+(ExtendedStorageClass& that) const;
	ExtendedStorageClass* operator+=(ExtendedStorageClass& that);
	ExtendedStorageClass operator-(ExtendedStorageClass& that) const;
	ExtendedStorageClass* operator-=(ExtendedStorageClass& that);

	bool Load(PhobosStreamReader& Stm, bool RegisterForChange);
	bool Save(PhobosStreamWriter& Stm) const;

private:
	template <typename T>
	bool Serialize(T& Stm);

	std::vector<float> Tiberiums;
};
