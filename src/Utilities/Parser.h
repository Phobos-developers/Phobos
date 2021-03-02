#pragma once

#include <cstdio>
#include <Windows.h>

//! Parses strings into one or more elements of another type.
/*!
	\tparam T The type to convert to.
	\tparam count The maximum number of elements.
*/
template<typename T, size_t count = 1>
class Parser {
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
	static size_t Parse(const char* pValue, OutType* outValue) {
		char buffer[0x80];
		for (size_t i = 0; i < Count; ++i) {
			// skip the leading spaces
			while (isspace(static_cast<unsigned char>(*pValue))) {
				++pValue;
			}

			// read the next part
			int n = 0;
			if (sscanf_s(pValue, "%[^,]%n", buffer, sizeof(buffer), &n) != 1) {
				return i;
			}

			// skip all read chars and the comma
			pValue += n;
			if (*pValue) {
				++pValue;
			}

			// trim the trailing spaces
			while (n && isspace(static_cast<unsigned char>(buffer[n - 1]))) {
				buffer[n-- - 1] = '\0';
			}

			// interprete the value
			if (!Parser<OutType>::TryParse(buffer, &outValue[i])) {
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
	static bool TryParse(const char* pValue, OutType* outValue) {
		OutType buffer[Count] = {};

		if (Parse(pValue, buffer) != Count) {
			return false;
		}

		if (outValue) {
			for (size_t i = 0; i < Count; ++i) {
				outValue[i] = buffer[i];
			}
		}

		return true;
	}
};

template<typename T>
class Parser<T, 1> {
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
	static int Parse(const char* pValue, OutType* outValue) {
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
	static bool TryParse(const char* pValue, OutType* outValue) {
		// non-specialized: read AbstractTypes
		if (auto pType = BaseType::Find(pValue)) {
			if (outValue) {
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
static bool Parser<bool>::TryParse(const char* pValue, OutType* outValue) {
	switch (toupper(static_cast<unsigned char>(*pValue))) {
	case '1':
	case 'T':
	case 'Y':
		if (outValue) {
			*outValue = true;
		}
		return true;
	case '0':
	case 'F':
	case 'N':
		if (outValue) {
			*outValue = false;
		}
		return true;
	default:
		return false;
	}
};

template<>
static bool Parser<int>::TryParse(const char* pValue, OutType* outValue) {
	const char* pFmt = nullptr;
	if (*pValue == '$') {
		pFmt = "$%d";
	}
	else if (tolower(static_cast<unsigned char>(pValue[strlen(pValue) - 1])) == 'h') {
		pFmt = "%xh";
	}
	else {
		pFmt = "%d";
	}

	int buffer = 0;
	if (sscanf_s(pValue, pFmt, &buffer) == 1) {
		if (outValue) {
			*outValue = buffer;
		}
		return true;
	}
	return false;
}

template<>
static bool Parser<double>::TryParse(const char* pValue, OutType* outValue) {
	double buffer = 0.0;
	if (sscanf_s(pValue, "%lf", &buffer) == 1) {
		if (strchr(pValue, '%')) {
			buffer *= 0.01;
		}
		if (outValue) {
			*outValue = buffer;
		}
		return true;
	}
	return false;
};

template<>
static bool Parser<float>::TryParse(const char* pValue, OutType* outValue) {
	double buffer = 0.0;
	if (Parser<double>::TryParse(pValue, &buffer)) {
		if (outValue) {
			*outValue = static_cast<float>(buffer);
		}
		return true;
	}
	return false;
}

template<>
static bool Parser<BYTE>::TryParse(const char* pValue, OutType* outValue) {
	// no way to read unsigned char, use short instead.
	const char* pFmt = nullptr;
	if (*pValue == '$') {
		pFmt = "$%hu";
	}
	else if (tolower(static_cast<unsigned char>(pValue[strlen(pValue) - 1])) == 'h') {
		pFmt = "%hxh";
	}
	else {
		pFmt = "%hu";
	}

	WORD buffer;
	if (sscanf_s(pValue, pFmt, &buffer) == 1) {
		if (buffer <= UCHAR_MAX) {
			if (outValue) {
				*outValue = static_cast<BYTE>(buffer);
			}
			return true;
		}
	}
	return false;
};
