#pragma once

#include <utility>

enum class ePoolIdentifier : int
{
	None, Event
};

class PoolIdentifier
{
	ePoolIdentifier _identifier;
public:
	PoolIdentifier(ePoolIdentifier identifier)
		: _identifier{ identifier }
	{}

	PoolIdentifier(const PoolIdentifier& poolIdentifier)
		: _identifier{ poolIdentifier._identifier }
	{}

	PoolIdentifier(const PoolIdentifier&& poolIdentifier)
		: _identifier{ poolIdentifier._identifier }
	{}

	~PoolIdentifier() = default;

	std::weak_ordering operator <=>(const PoolIdentifier& another)
	{
		auto result = (int)_identifier - (int)another._identifier;
		if (result < 0) return std::weak_ordering::less;
		if (result > 0) return std::weak_ordering::greater;
		return std::weak_ordering::equivalent;
	}

	const ePoolIdentifier getIdentifier() const
		{ return _identifier; }
};

class PoolIdentifierHashFunc
{
	int operator() (const PoolIdentifier& pool) const
	{
		return static_cast<int>(pool.getIdentifier());
	}
};