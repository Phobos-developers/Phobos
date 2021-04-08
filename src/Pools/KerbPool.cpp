#include "KerbPool.h"

std::vector<KerbPool*> KerbPool::Array;

PoolIdentifier KerbPool::WhatAmI()
{
	return PoolIdentifier::KerbPool;
}