#pragma once

#include "pch.h"

class ObjectUtils
{
public:
	static PlayerRef CreatePlayer(GameSessionRef session);

private:
	static atomic<int64> s_idGenerator;
};

