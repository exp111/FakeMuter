#pragma once

class Config
{
public:
	Config();
	~Config();

	bool enabled = false;
	bool changeName = true;
};

extern Config* config;
