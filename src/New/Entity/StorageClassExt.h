#pragma once

#include <Utilities/TemplateDef.h>


class StorageClassExt
{
public:
	StorageClassExt(std::vector<float> & vector) :
		Tiberiums(&vector) {}

	int GetTotalValue() const;
	float GetTotalAmount() const;
	float GetAmount(int index) const;
	float IncreaseAmount(float amount, int index);
	float DecreaseAmount(float amount, int index);
	int FirstUsedSlot() const;

	StorageClassExt operator+=(StorageClassExt& that);
	StorageClassExt operator-=(StorageClassExt& that);

private:
	std::vector<float>* Tiberiums;
};
