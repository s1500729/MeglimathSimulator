﻿#pragma once
#include "Client.h"
class RandomClient :
	public Client
{
public:
	String Name() override
	{
		return U"Random";
	}

	void Update(const GameInfo& info) override
	{
		if (IsReady())
		{
			return;
		}

		_think.steps[0] = { Action(Random(0,1)),Direction(Random(0,7)) };
		_think.steps[1] = { Action(Random(0,1)),Direction(Random(0,7)) };

		_is_ready = true;
	}

public:
	RandomClient(TeamType type);
	~RandomClient();
};

