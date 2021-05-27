#pragma once

#include <Utilities/Template.h>


class ArmorType : public Valueable<int>
{
public:
	template <typename T>
	ArmorType(T value)
	{
		this->Value = (int)std::move(value);
	}

	template <typename T>
	ArmorType& operator = (T value)
	{
		this->Value = std::move(value);
		return *this;
	}
};
