#pragma once

#include <Utilities/TemplateDef.h>
#include <TiberiumClass.h>


class PhobosStorageClass
{
public:
	PhobosStorageClass(std::vector<float>* vector) :
		Tiberiums(vector) {}
	~PhobosStorageClass() = default;

	int GetTotalValue() const;
	float GetTotalAmount() const;
	float GetAmount(int index) const;
	float IncreaseAmount(float amount, int index);
	float DecreaseAmount(float amount, int index);
	int FirstUsedSlot() const;

	PhobosStorageClass operator+=(PhobosStorageClass& that);
	PhobosStorageClass operator-=(PhobosStorageClass& that);

	bool Load(PhobosStreamReader& Stm, bool RegisterForChange);
	bool Save(PhobosStreamWriter& Stm) const;

private:
	std::vector<float>* Tiberiums;
};
