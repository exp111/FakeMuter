#pragma once

class Config
{
public:
	Config();
	~Config();

	bool enabled = false;
	bool changeName = true;
	bool muteMicrophone = true;
};

extern Config* config;
