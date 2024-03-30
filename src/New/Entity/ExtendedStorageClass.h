#pragma once

#include <Utilities/TemplateDef.h>
#include <TiberiumClass.h>


class ExtendedStorageClass
{
public:
	ExtendedStorageClass() : Tiberiums(TiberiumClass::Array->Count) {}
	~ExtendedStorageClass() = default;

	int GetTotalValue() const;
	float GetTotalAmount() const;
	float GetAmount(int index) const;
	float IncreaseAmount(float amount, int index);
	float DecreaseAmount(float amount, int index);
	int FirstUsedSlot() const;

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
