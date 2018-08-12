/*
* TeamSpeak 3 demo plugin
*
* Copyright (c) 2008-2017 TeamSpeak Systems GmbH
*/

#ifdef _WIN32
#pragma warning (disable : 4100)  /* Disable Unreferenced parameter warning */
#include <Windows.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "teamspeak/public_errors.h"
#include "teamspeak/public_errors_rare.h"
#include "teamspeak/public_definitions.h"
#include "teamspeak/public_rare_definitions.h"
#include "teamspeak/clientlib_publicdefinitions.h"
#include "ts3_functions.h"
#include "plugin.h"
#include <iostream>
#include <string>
#include "plugin.h"
#include "config.h"

static struct TS3Functions ts3Functions;

#ifdef _WIN32
#define _strcpy(dest, destSize, src) strcpy_s(dest, destSize, src)
#define snprintf sprintf_s
#else
#define _strcpy(dest, destSize, src) { strncpy(dest, src, destSize-1); (dest)[destSize-1] = '\0'; }
#endif

#define PLUGIN_API_VERSION 22

#define PATH_BUFSIZE 512
#define COMMAND_BUFSIZE 128
#define INFODATA_BUFSIZE 128
#define SERVERINFO_BUFSIZE 256
#define CHANNELINFO_BUFSIZE 512
#define RETURNCODE_BUFSIZE 128

#define PLUGIN_NAME "FakeMuter"
//Global Variables
static char* pluginID = NULL;

#ifdef _WIN32
/* Helper function to convert wchar_T to Utf-8 encoded strings on Windows */
static int wcharToUtf8(const wchar_t* str, char** result) {
	int outlen = WideCharToMultiByte(CP_UTF8, 0, str, -1, 0, 0, 0, 0);
	*result = (char*)malloc(outlen);
	if (WideCharToMultiByte(CP_UTF8, 0, str, -1, *result, outlen, 0, 0) == 0) {
		*result = NULL;
		return -1;
	}
	return 0;
}
#endif

/*********************************** Required functions ************************************/


const char* ts3plugin_name() {
#ifdef _WIN32
	static char* result = NULL;  /* Static variable so it's allocated only once */
	if (!result) {
		const wchar_t* name = L"FakeMuter";
		if (wcharToUtf8(name, &result) == -1) {  /* Convert name into UTF-8 encoded result */
			result = _strdup(PLUGIN_NAME);  /* Conversion failed, fallback here */
		}
	}
	return result;
#else
	return PLUGIN_NAME;
#endif
}

const char* ts3plugin_version() {
	return "1.0";
}

int ts3plugin_apiVersion() {
	return PLUGIN_API_VERSION;
}

const char* ts3plugin_author() {
	return "Exp";
}

const char* ts3plugin_description() {
	return "Mute yourself while being able to see who's talking.";
}

void ts3plugin_setFunctionPointers(const struct TS3Functions funcs) {
	ts3Functions = funcs;
}

int ts3plugin_init() {
	char appPath[PATH_BUFSIZE];
	char resourcesPath[PATH_BUFSIZE];
	char configPath[PATH_BUFSIZE];
	char pluginPath[PATH_BUFSIZE];

	ts3Functions.getAppPath(appPath, PATH_BUFSIZE);
	ts3Functions.getResourcesPath(resourcesPath, PATH_BUFSIZE);
	ts3Functions.getConfigPath(configPath, PATH_BUFSIZE);
	ts3Functions.getPluginPath(pluginPath, PATH_BUFSIZE, pluginID);

	return 0;
}

void ts3plugin_shutdown() {
	if (pluginID) {
		free(pluginID);
		pluginID = NULL;
	}
}

int ts3plugin_offersConfigure() {
	return PLUGIN_OFFERS_NO_CONFIGURE;
}

void ts3plugin_registerPluginID(const char* id) {
	const size_t sz = strlen(id) + 1;
	pluginID = (char*)malloc(sz * sizeof(char));
	_strcpy(pluginID, sz, id);
}

const char* ts3plugin_commandKeyword() {
	return ""; //Command Suffix for chat; example: /test
}

const char* ts3plugin_infoTitle() {
	return PLUGIN_NAME;
}

void ts3plugin_freeMemory(void* data) {
	free(data);
}
int ts3plugin_requestAutoload() {
	return 0;
}

//Helper Functions

/* Helper function to create a menu item */
static struct PluginMenuItem* createMenuItem(enum PluginMenuType type, int id, const char* text, const char* icon) {
	struct PluginMenuItem* menuItem = (struct PluginMenuItem*)malloc(sizeof(struct PluginMenuItem));
	menuItem->type = type;
	menuItem->id = id;
	_strcpy(menuItem->text, PLUGIN_MENU_BUFSZ, text);
	_strcpy(menuItem->icon, PLUGIN_MENU_BUFSZ, icon);
	return menuItem;
}

/* Some makros to make the code to create menu items a bit more readable */
#define BEGIN_CREATE_MENUS(x) const size_t sz = x + 1; size_t n = 0; *menuItems = (struct PluginMenuItem**)malloc(sizeof(struct PluginMenuItem*) * sz);
#define CREATE_MENU_ITEM(a, b, c, d) (*menuItems)[n++] = createMenuItem(a, b, c, d);
#define END_CREATE_MENUS (*menuItems)[n++] = NULL; assert(n == sz);


enum {
	MENU_ID_GLOBAL_1 = 1,
	MENU_ID_GLOBAL_2,
	MENU_ID_MAX
};

void ts3plugin_initMenus(struct PluginMenuItem*** menuItems, char** menuIcon) {

	BEGIN_CREATE_MENUS(MENU_ID_MAX - 1); //Needs to be correct
	CREATE_MENU_ITEM(PLUGIN_MENU_TYPE_GLOBAL, MENU_ID_GLOBAL_1, "Enable/Disable", "");
	CREATE_MENU_ITEM(PLUGIN_MENU_TYPE_GLOBAL, MENU_ID_GLOBAL_1, "Enable/Disable Name Changing", "");
	END_CREATE_MENUS;

	*menuIcon = (char*)malloc(PLUGIN_MENU_BUFSZ * sizeof(char));
	_strcpy(*menuIcon, PLUGIN_MENU_BUFSZ, ""); //PLUGIN MENU IMAGE
}

static struct PluginHotkey* createHotkey(const char* keyword, const char* description) {
	struct PluginHotkey* hotkey = (struct PluginHotkey*)malloc(sizeof(struct PluginHotkey));
	_strcpy(hotkey->keyword, PLUGIN_HOTKEY_BUFSZ, keyword);
	_strcpy(hotkey->description, PLUGIN_HOTKEY_BUFSZ, description);
	return hotkey;
}

#define BEGIN_CREATE_HOTKEYS(x) const size_t sz = x + 1; size_t n = 0; *hotkeys = (struct PluginHotkey**)malloc(sizeof(struct PluginHotkey*) * sz);
#define CREATE_HOTKEY(a, b) (*hotkeys)[n++] = createHotkey(a, b);
#define END_CREATE_HOTKEYS (*hotkeys)[n++] = NULL; assert(n == sz);


void ts3plugin_initHotkeys(struct PluginHotkey*** hotkeys) {
	BEGIN_CREATE_HOTKEYS(1);
	CREATE_HOTKEY("toggleFakeMuter", "Toggle the Plugin");
	END_CREATE_HOTKEYS;
}

void ToggleFakeMuter()
{
	config->enabled = !config->enabled;
	std::string message = config->enabled ? "Enabled" : "Disabled";
	message += " Fake Muter.";
	ts3Functions.printMessageToCurrentTab(message.c_str());

	if (!config->changeName)
		return;

	uint64 schid = ts3Functions.getCurrentServerConnectionHandlerID();
	anyID clientID = 0;
	ts3Functions.getClientID(schid, &clientID);
	char* name = new char[256];
	ts3Functions.getClientSelfVariableAsString(schid, CLIENT_NICKNAME, &name);

	std::string sName(name);
	if (config->enabled)
	{
		ts3Functions.setClientSelfVariableAsString(schid, CLIENT_NICKNAME, (sName + " [MUTED]").c_str());
	}
	else
	{
		std::string newName = sName.substr(0, sName.size() - 8);
		ts3Functions.setClientSelfVariableAsString(schid, CLIENT_NICKNAME, newName.c_str());
	}
	ts3Functions.flushClientSelfUpdates(schid, NULL);
}
/************************** TeamSpeak callbacks ***************************/
void ts3plugin_onMenuItemEvent(uint64 serverConnectionHandlerID, enum PluginMenuType type, int menuItemID, uint64 selectedItemID) {
	switch (type) {
	case PLUGIN_MENU_TYPE_GLOBAL:
		/* Channel contextmenu item was triggered. selectedItemID is the channelID of the selected channel */
		switch (menuItemID) {
		case MENU_ID_GLOBAL_1:
			ToggleFakeMuter();
			break;
		case MENU_ID_GLOBAL_2:
			config->changeName = !config->changeName;
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}
}

const char* ts3plugin_keyDeviceName(const char* keyIdentifier) {
	return NULL;
}

const char* ts3plugin_displayKeyText(const char* keyIdentifier) {
	return NULL;
}

const char* ts3plugin_keyPrefix() {
	return NULL;
}

void ts3plugin_onHotkeyEvent(const char* keyword)
{
	if (strcmp(keyword, "toggleFakeMuter") == 0)
		ToggleFakeMuter();
}

void ts3plugin_onEditPlaybackVoiceDataEvent(uint64 serverConnectionHandlerID, anyID clientID, short* samples, int sampleCount, int channels)
{
	if (!config->enabled)
		return;

	for (int i = 0; i < sampleCount * channels; i++)
	{
		samples[i] = 0;
	}
}