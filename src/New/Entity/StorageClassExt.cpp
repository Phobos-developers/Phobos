#include "StorageClassExt.h"
#include <TiberiumClass.h>

int StorageClassExt::GetTotalValue() const
{
	float total = 0;

	for (size_t i = 0; i < Tiberiums->size(); i++)
		total += static_cast<int>((*Tiberiums)[i] * TiberiumClass::Array[i]->Value);

	return static_cast<int>(total);
}

float StorageClassExt::GetTotalAmount() const
{
	float total = 0;

	for (size_t i = 0; i < Tiberiums->size(); i++)
		total += (*Tiberiums)[i];

	return total;
}

float StorageClassExt::GetAmount(int index) const
{
	return (*Tiberiums)[index];
}

float StorageClassExt::IncreaseAmount(float amount, int index)
{
	(*Tiberiums)[index] += amount;
	return (*Tiberiums)[index];
}

float StorageClassExt::DecreaseAmount(float amount, int index)
{
	if (amount < (*Tiberiums)[index])
		amount = (*Tiberiums)[index];

	(*Tiberiums)[index] -= amount;
	return amount;
}

int StorageClassExt::FirstUsedSlot() const
{
	for (size_t i = 0; i < Tiberiums->size(); i++)
	{
		if ((*Tiberiums)[i] > 0.0)
			return i;
	}

	return -1;
}

StorageClassExt StorageClassExt::operator+=(StorageClassExt& that)
{
	for (size_t i = 0; i < Tiberiums->size(); i++)
		(*Tiberiums)[i] += (*that.Tiberiums)[i];

	return *this;
}

StorageClassExt StorageClassExt::operator-=(StorageClassExt& that)
{
	for (size_t i = 0; i < Tiberiums->size(); i++)
		(*Tiberiums)[i] -= (*that.Tiberiums)[i];

	return *this;
}
