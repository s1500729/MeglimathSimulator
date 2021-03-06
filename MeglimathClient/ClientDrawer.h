﻿#pragma once
#include "../MeglimathCore/Drawer.h"
#include "../MeglimathClient/Client.h"

class ClientDrawer
	: public Drawer
{
public:
	void DrawInputState(Client& client);
	void DrawInstraction(Client& client);
public:
	ClientDrawer();
	~ClientDrawer();
};

