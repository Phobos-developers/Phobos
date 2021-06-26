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

	bool Read(INI_EX& parser, const char* pSection, const char* pKey, bool Allocate = false)
	{
		int buffer;
		if (parser.ReadArmor(pSection, pKey, &buffer))
		{
			this->Value = buffer;
			return true;
		}
		else if (!parser.empty())
		{
			Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a valid ArmorType");
		}
		return false;
	}
};
