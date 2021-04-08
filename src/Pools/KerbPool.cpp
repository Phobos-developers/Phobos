#include "KerbPool.h"

std::vector<KerbPool*> KerbPool::Array;

PoolIdentifier KerbPool::WhatPoolAmI()
{
	return PoolIdentifier::KerbPool;
}