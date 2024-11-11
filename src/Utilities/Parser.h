#pragma region Ares Copyrights
/*
 *Copyright (c) 2008+, All Ares Contributors
 *All rights reserved.
 *
 *Redistribution and use in source and binary forms, with or without
 *modification, are permitted provided that the following conditions are met:
 *1. Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 *2. Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 *3. All advertising materials mentioning features or use of this software
 *   must display the following acknowledgement:
 *   This product includes software developed by the Ares Contributors.
 *4. Neither the name of Ares nor the
 *   names of its contributors may be used to endorse or promote products
 *   derived from this software without specific prior written permission.
 *
 *THIS SOFTWARE IS PROVIDED BY ITS CONTRIBUTORS ''AS IS'' AND ANY
 *EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 *WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *DISCLAIMED. IN NO EVENT SHALL THE ARES CONTRIBUTORS BE LIABLE FOR ANY
 *DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 *(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#pragma endregion

#pragma once

#include <type_traits>
#include <Windows.h>
#include <stdio.h>
#include <CRT.h>

//! Parses strings into one or more elements of another type.
/*!
	\tparam T The type to convert to.
	\tparam count The maximum number of elements.
*/
template<typename T, size_t count = 1>
class Parser
{
public:
	using OutType = T;
	using BaseType = std::remove_pointer_t<T>;
	static const size_t Count = count;

	//! Parses at most Count values and returns the number of items parsed.
	/*!
		Splits the pValue string into its parts (using comma as separator) and
		parses each element separately.

		Stops on the first element that cannot be parsed and returns the number
		of successfully parsed elements.

		\param pValue The string value to be parsed.
		\param outValue Optional pointer to array of at least Count.

		\returns Number of items parsed.

		\author AlexB
		\date 2013-03-10
	*/
	static size_t Parse(const char* pValue, OutType* outValue)
	{
		char buffer[0x80];
		for (size_t i = 0; i < Count; ++i)
		{
			// skip the leading spaces
			while (isspace(static_cast<unsigned char>(*pValue)))
			{
				++pValue;
			}

			// read the next part
			int n = 0;
			if (sscanf_s(pValue, "%[^,]%n", buffer, sizeof(buffer), &n) != 1)
			{
				return i;
			}

			// skip all read chars and the comma
			pValue += n;
			if (*pValue)
			{
				++pValue;
			}

			// trim the trailing spaces
			while (n && isspace(static_cast<unsigned char>(buffer[n - 1])))
			{
				buffer[n-- - 1] = '\0';
			}

			// interpret the value
			if (!Parser<OutType>::TryParse(buffer, &outValue[i]))
			{
				return i;
			}
		}

		return Count;
	}

	//! Parses either Count values or none.
	/*!
		Parses using a temporary buffer. Only if all elements could be parsed
		they are written to outValue (if it is set).

		This function keeps outValue consistent. Either all elements are
		changed or all elements are unchanged. It is not possible that only
		the first n elements are replaced.

		\param pValue The string value to be parsed.
		\param outValue Optional pointer to array of at least Count.

		\returns true, if all elements could be parsed, false otherwise.

		\author AlexB
		\date 2013-03-11
	*/
	static bool TryParse(const char* pValue, OutType* outValue)
	{
		OutType buffer[Count] = {};

		if (Parse(pValue, buffer) != Count)
		{
			return false;
		}

		if (outValue)
		{
			for (size_t i = 0; i < Count; ++i)
			{
				outValue[i] = buffer[i];
			}
		}

		return true;
	}
};

template<typename T>
class Parser<T, 1>
{
public:
	using OutType = T;
	using BaseType = std::remove_pointer_t<T>;
	static const size_t Count = 1;

	//! Parses a single element.
	/*!
		If the function returns 1 and outValue is set, outValue contains the
		result of the parse operation. Otherwise, outValue is unchanged.

		\param pValue The string value to be parsed.
		\param outValue Optional pointer to the target memory.

		\returns 1, if the element could be parsed, 0 otherwise.

		\author AlexB
		\date 2013-03-11
	*/
	static int Parse(const char* pValue, OutType* outValue)
	{
		return TryParse(pValue, outValue) ? 1 : 0;
	}

	//! Tries to parse a single element.
	/*!
		If the function returns true and outValue is set, outValue contains the
		result of the parse operation. Otherwise, outValue is unchanged.

		\param pValue The string value to be parsed.
		\param outValue Optional pointer to the target memory.

		\returns true, if the element could be parsed, false otherwise.

		\author AlexB
		\date 2013-03-11
	*/
	static bool TryParse(const char* pValue, OutType* outValue)
	{
		// non-specialized: read AbstractTypes
		if (auto pType = BaseType::Find(pValue))
		{
			if (outValue)
			{
				*outValue = pType;
			}
			return true;
		}
		return false;
	}
};

// Specializations
// Usually, it's enough to specialize Parser<T,1>::TryParse, because all other
// functions will eventually call them.

template<>
inline bool Parser<bool>::TryParse(const char* pValue, OutType* outValue)
{
	switch (toupper(static_cast<unsigned char>(*pValue)))
	{
	case '1':
	case 'T':
	case 'Y':
		if (outValue)
		{
			*outValue = true;
		}
		return true;
	case '0':
	case 'F':
	case 'N':
		if (outValue)
		{
			*outValue = false;
		}
		return true;
	default:
		return false;
	}
};

template<>
inline bool Parser<int>::TryParse(const char* pValue, OutType* outValue)
{
	const char* pFmt = nullptr;
	if (*pValue == '$')
	{
		pFmt = "$%d";
	}
	else if (tolower(static_cast<unsigned char>(pValue[strlen(pValue) - 1])) == 'h')
	{
		pFmt = "%xh";
	}
	else
	{
		pFmt = "%d";
	}

	int buffer = 0;
	if (sscanf_s(pValue, pFmt, &buffer) == 1)
	{
		if (outValue)
		{
			*outValue = buffer;
		}
		return true;
	}
	return false;
}

template<>
inline bool Parser<double>::TryParse(const char* pValue, OutType* outValue)
{

	// Game doesn't use double precision when parsing, using double here would create inconsistency.
	float buffer = 0.0;

	// Use game's sscanf function, the C library one has different precision/rounding.
	if (CRT::sscanf(pValue, "%f", &buffer) == 1)
	{
		if (strchr(pValue, '%'))
		{
			buffer *= 0.01f;
		}
		if (outValue)
		{
			*outValue = buffer;
		}
		return true;
	}
	return false;
};

template<>
inline bool Parser<float>::TryParse(const char* pValue, OutType* outValue)
{
	double buffer = 0.0;
	if (Parser<double>::TryParse(pValue, &buffer))
	{
		if (outValue)
		{
			*outValue = static_cast<float>(buffer);
		}
		return true;
	}
	return false;
}

template<>
inline bool Parser<BYTE>::TryParse(const char* pValue, OutType* outValue)
{
	// no way to read unsigned char, use short instead.
	const char* pFmt = nullptr;
	if (*pValue == '$')
	{
		pFmt = "$%hu";
	}
	else if (tolower(static_cast<unsigned char>(pValue[strlen(pValue) - 1])) == 'h')
	{
		pFmt = "%hxh";
	}
	else
	{
		pFmt = "%hu";
	}

	WORD buffer;
	if (sscanf_s(pValue, pFmt, &buffer) == 1)
	{
		if (buffer <= UCHAR_MAX)
		{
			if (outValue)
			{
				*outValue = static_cast<BYTE>(buffer);
			}
			return true;
		}
	}
	return false;
};

//! Parses strings into one or more elements of another type, with an
//! arbitrary maximum count that can be determined at moment of parsing.
/*!
	\tparam T The type to convert to.
*/
template<typename T>
class MultiParser
{
public:
	using OutType = T;
	using BaseType = std::remove_pointer_t<T>;

	//! Parses at most count values and returns the number of items parsed.
	/*!
		Splits the pValue string into its parts (using comma as separator) and
		parses each element separately.

		Stops on the first element that cannot be parsed and returns the number
		of successfully parsed elements.

		\param pValue The string value to be parsed.
		\param outValue Optional pointer to array of at least count.
		\param count Maximum number of elements to parse.

		\returns Number of items parsed.

		\author AlexB & Starkku
		\date 2022-10-20
	*/
	static size_t Parse(const char* pValue, OutType* outValue, size_t count)
	{
		char buffer[0x80];
		for (size_t i = 0; i < count; ++i)
		{
			// skip the leading spaces
			while (isspace(static_cast<unsigned char>(*pValue)))
			{
				++pValue;
			}

			// read the next part
			int n = 0;
			if (sscanf_s(pValue, "%[^,]%n", buffer, sizeof(buffer), &n) != 1)
			{
				return i;
			}

			// skip all read chars and the comma
			pValue += n;
			if (*pValue)
			{
				++pValue;
			}

			// trim the trailing spaces
			while (n && isspace(static_cast<unsigned char>(buffer[n - 1])))
			{
				buffer[n-- - 1] = '\0';
			}

			// interprete the value
			if (!Parser<OutType>::TryParse(buffer, &outValue[i]))
			{
				return i;
			}
		}

		return count;
	}

	//! Parses either count values or none.
	/*!
		Parses using a temporary buffer. Only if all elements could be parsed
		they are written to outValue (if it is set).

		This function keeps outValue consistent. Either all elements are
		changed or all elements are unchanged. It is not possible that only
		the first n elements are replaced.

		\param pValue The string value to be parsed.
		\param outValue Optional pointer to array of at least Count.
		\param count Number of elements to parse.

		\returns true, if all elements could be parsed, false otherwise.

		\author AlexB & Starkku
		\date 2022-10-20
	*/
	static bool TryParse(const char* pValue, OutType* outValue, size_t count)
	{
		OutType buffer[count] = {};

		if (Parse(pValue, buffer) != count)
		{
			return false;
		}

		if (outValue)
		{
			for (size_t i = 0; i < count; ++i)
			{
				outValue[i] = buffer[i];
			}
		}

		return true;
	}
};
