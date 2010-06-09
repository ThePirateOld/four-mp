#include <time.h>
#include "ServerCore.h"
#include "logging.h"
#include "HandleManager.h"
#include "CoreHandleTypesManager.h"
#include "../../Shared/Console/ConsoleCore.h"
#include "../../Shared/Console/ConsoleScreen.h"
#include "../../Shared/Console/coreconcommands.h"
#include "con_fmpcvarhooks.h"
#include "con_fmpcommands.h"
#include "MasterServerManager.h"
#include "NetworkManager.h"
#include "PluginManager.h"
#include "VirtualMachineManager.h"
#include "PlayerManager.h"

#define SERVER_TICKRATE 10

extern HandleManager hm;
extern ConsoleCore concore;
extern ConsoleScreen conscreen;
extern MasterServerManager msm;
extern NetworkManager nm;
extern PluginManager plugm;
extern VirtualMachineManager vmm;
extern PlayerManager playm;

ServerCore::ServerCore(void)
{
	isrunning = false;
	sleepcount = 100;
	lastcheck = 0;
	hostname = NULL;
	port = 7777;
	lan = 1;
	gamemode = NULL;
	password = NULL;
	rconpassword = NULL;
	componentselect = false;
	componentselectcvar = NULL;
	gametime.ticksperminute = 20;
	gametime.tickcount = 0;
	gametime.time[0] = 0;
	gametime.time[1] = 0;
}

ServerCore::~ServerCore(void)
{
	if (hostname)
	{
		free(hostname);
	}
	if (gamemode)
	{
		free(gamemode);
	}
	if (password)
	{
		free(password);
	}
	if (rconpassword)
	{
		free(rconpassword);
	}
}

bool ServerCore::Load(void)
{
	concore.SetOutputFunction(PrintToServer);
	concore.SetExecPath(L"cfg/");
	conscreen.SetCaption(L"FOUR-MP");
	//Core console functions
	hm.AddNewHandle(0, HandleTypeConCmd, concore.AddConCmd(L"alias", ConCmdAlias, L"Alias a command.", 0));
	hm.AddNewHandle(0, HandleTypeConCmd, concore.AddConCmd(L"cvarlist", ConCmdCvarlist, L"Show the list of convars/concommands.", 0));
	hm.AddNewHandle(0, HandleTypeConVar, concore.AddConVar(L"developer", 0, L"Show developer messages.", 0, true, 0, true, 2));
	hm.AddNewHandle(0, HandleTypeConCmd, concore.AddConCmd(L"echo", ConCmdEcho, L"Echo text to console.", 0));
	hm.AddNewHandle(0, HandleTypeConCmd, concore.AddConCmd(L"exec", ConCmdExec, L"Execute script file.", 0));
	hm.AddNewHandle(0, HandleTypeConCmd, concore.AddConCmd(L"find", ConCmdFind, L"Find concommands with the specified string in their name/help text.", 0));
	hm.AddNewHandle(0, HandleTypeConCmd, concore.AddConCmd(L"help", ConCmdHelp, L"Find help about a convar/concommand.", 0));
	// FMP console functions
	hm.AddNewHandle(0, HandleTypeConCmd, concore.AddConCmd(L"exit", ConCmdQuit, L"Exit the engine.", 0));
	hm.AddNewHandle(0, HandleTypeConCmd, concore.AddConCmd(L"fs_list", ConCmdFsList, L"Prints details about loaded gamemode/filterscripts.", 0));
	hm.AddNewHandle(0, HandleTypeConCmd, concore.AddConCmd(L"fs_load", ConCmdFsLoad, L"fs_load <filename> : loads a filterscript", 0));
	hm.AddNewHandle(0, HandleTypeConCmd, concore.AddConCmd(L"fs_load_all", ConCmdFsLoadAll, L"Loads all filterscripts", 0));
	hm.AddNewHandle(0, HandleTypeConCmd, concore.AddConCmd(L"fs_pause", ConCmdFsPause, L"fs_pause <index> : pauses a loaded filterscript", 0));
	hm.AddNewHandle(0, HandleTypeConCmd, concore.AddConCmd(L"fs_pause_all", ConCmdFsPauseAll, L"Pauses all loaded filterscripts", 0));
	hm.AddNewHandle(0, HandleTypeConCmd, concore.AddConCmd(L"fs_reload", ConCmdFsReload, L"fs_reload <index> : reloads a filterscript", 0));
	hm.AddNewHandle(0, HandleTypeConCmd, concore.AddConCmd(L"fs_reload_all", ConCmdFsReloadAll, L"Reloads all filterscripts", 0));
	hm.AddNewHandle(0, HandleTypeConCmd, concore.AddConCmd(L"fs_unload", ConCmdFsUnload, L"fs_unload <index> : unloads a filterscript", 0));
	hm.AddNewHandle(0, HandleTypeConCmd, concore.AddConCmd(L"fs_unload_all", ConCmdFsUnloadAll, L"Unloads all filterscripts", 0));
	hm.AddNewHandle(0, HandleTypeConCmd, concore.AddConCmd(L"fs_unpause", ConCmdFsUnpause, L"fs_unpause <index> : unpauses a loaded filterscript", 0));
	hm.AddNewHandle(0, HandleTypeConCmd, concore.AddConCmd(L"fs_unpause_all", ConCmdFsUnpauseAll, L"Unpauses all disabled filterscripts", 0));
	ConVar *gamemodecvar = concore.AddConVar(L"host_gamemode", L"", L"Current gamemode name.", 0);
	gamemodecvar->HookChange(ConVarHookHostGamemode);
	hm.AddNewHandle(0, HandleTypeConVar, gamemodecvar);
	ConVar *hostnamecvar = concore.AddConVar(L"hostname", L"", L"Hostname for server.", 0);
	hostnamecvar->HookChange(ConVarHookHostname);
	hm.AddNewHandle(0, HandleTypeConVar, hostnamecvar);
	hm.AddNewHandle(0, HandleTypeConCmd, concore.AddConCmd(L"plugin_list", ConCmdPluginList, L"Prints details about loaded plugins.", 0));
	hm.AddNewHandle(0, HandleTypeConCmd, concore.AddConCmd(L"plugin_load", ConCmdPluginLoad, L"plugin_load <filename> : loads a plugin", 0));
	hm.AddNewHandle(0, HandleTypeConCmd, concore.AddConCmd(L"plugin_load_all", ConCmdPluginLoadAll, L"Loads all plugins", 0));
	hm.AddNewHandle(0, HandleTypeConCmd, concore.AddConCmd(L"plugin_pause", ConCmdPluginPause, L"plugin_pause <index> : pauses a loaded plugin", 0));
	hm.AddNewHandle(0, HandleTypeConCmd, concore.AddConCmd(L"plugin_pause_all", ConCmdPluginPauseAll, L"Pauses all loaded plugins", 0));
	hm.AddNewHandle(0, HandleTypeConCmd, concore.AddConCmd(L"plugin_reload", ConCmdPluginReload, L"fs_reload <index> : reloads a plugin", 0));
	hm.AddNewHandle(0, HandleTypeConCmd, concore.AddConCmd(L"plugin_reload_all", ConCmdPluginReloadAll, L"Reloads all plugins", 0));
	hm.AddNewHandle(0, HandleTypeConCmd, concore.AddConCmd(L"plugin_unload", ConCmdPluginUnload, L"fs_unload <index> : unloads a plugin", 0));
	hm.AddNewHandle(0, HandleTypeConCmd, concore.AddConCmd(L"plugin_unload_all", ConCmdPluginUnloadAll, L"Unloads all plugins", 0));
	hm.AddNewHandle(0, HandleTypeConCmd, concore.AddConCmd(L"plugin_unpause", ConCmdPluginUnpause, L"fs_unpause <index> : unpauses a loaded plugin", 0));
	hm.AddNewHandle(0, HandleTypeConCmd, concore.AddConCmd(L"plugin_unpause_all", ConCmdPluginUnpauseAll, L"Unpauses all disabled plugins", 0));
	hm.AddNewHandle(0, HandleTypeConCmd, concore.AddConCmd(L"quit", ConCmdQuit, L"Exit the engine.", 0));
	ConVar *rconpasswordcvar = concore.AddConVar(L"rcon_password", L"", L"Remote console password.", 0);
	rconpasswordcvar->HookChange(ConVarHookRconPassword);
	hm.AddNewHandle(0, HandleTypeConVar, rconpasswordcvar);
	componentselectcvar = concore.AddConVar(L"sv_componentselect", 0, L"Enables component select", 0, true, 0, true, 1);
	componentselectcvar->HookChange(ConVarHookSvComponentselect);
	hm.AddNewHandle(0, HandleTypeConVar, componentselectcvar);
	ConVar *lancvar = concore.AddConVar(L"sv_lan", 1, L"Server is a lan server (no heartbeat, no authentication, no non-class C addresses).", 0, true, 0, true, 1);
	lancvar->HookChange(ConVarHookSvLan);
	hm.AddNewHandle(0, HandleTypeConVar, lancvar);
	ConVar *passwordcvar = concore.AddConVar(L"sv_password", L"", L"Server password for entry into multiplayer games", 0);
	passwordcvar->HookChange(ConVarHookSvPassword);
	hm.AddNewHandle(0, HandleTypeConVar, passwordcvar);
	ConVar *portcvar = concore.AddConVar(L"sv_port", 7777, L"Server port.", 0, true, 0, true, 65535);
	portcvar->HookChange(ConVarHookSvPort);
	hm.AddNewHandle(0, HandleTypeConVar, portcvar);
	PrintToServer(L"FOUR-MP. Copyright 2009-2010 Four-mp team.");
	concore.InterpretLine(L"exec server.cfg");
	sleepcount = 1000 / SERVER_TICKRATE;
	gametime.ticksperminute = SERVER_TICKRATE * 2;
	nm.Load(playm.GetMaxPlayers(), port);
	maxplayers = playm.GetMaxPlayers();
	plugm.LoadPlugins();
	vmm.LoadFilterScripts();
	if (!vmm.LoadGameMode(gamemode))
	{
		PrintToServer(L"Can't load gamemode.");
		return false;
	}
	gamemodename = vmm.GetGameModeName();
	if (!lan)
	{
		msm.Init();
		if (!msm.RegisterServer(port, hostname, gamemodename, L"World", maxplayers, password))
		{
			PrintToServer(L"Unable to register server.");
		}
		lastmasterservercheck = time(0);
	}
	nm.UpdateServerInfo();
	isrunning = true;
	debug(L"Started");
	return true;
}

bool ServerCore::IsRunning(void)
{
	return isrunning;
}
void ServerCore::Tick(void)
{
	conscreen.CheckUserInput();
	nm.Tick();
	gametime.tickcount++;
	if (gametime.tickcount == gametime.ticksperminute)
	{
		gametime.tickcount = 0;
		this->IncrementGameTime();
	}
	if(GetTickCount() - lastcheck >= 30000)
	{
		lastcheck = GetTickCount();
		nm.CheckClients();
	}
	if (!lan)
	{
		if (time(0) - lastmasterservercheck >= 3600000)
		{
			lastmasterservercheck = time(0);
			if (!msm.RegisterServer(port, hostname, gamemodename, L"World", maxplayers, password))
			{
				PrintToServer(L"Unable to register server.");
			}
		}
	}
	Sleep(sleepcount);
}

void ServerCore::Unload(void)
{
	//TODO:
	//1. Politely kick all players.
	//2. Destroy all vehicles.
	vmm.UnloadGameMode(); //3. Unload all virtual machines.
	vmm.UnloadFilterScripts(); //3. Unload all virtual machines.
	plugm.UnloadPlugins(); //4. Unload all plugins.
	nm.Unload();
	//5. Clean up all remaining data - destructors.
}

void ServerCore::Shutdown(void)
{
	isrunning = false;
}

bool ServerCore::IsLAN(void)
{
	return lan;
}

bool ServerCore::IsPasswordProtected(void)
{
	if (password == NULL)
	{
		return false;
	}
	return true;
}

wchar_t *ServerCore::GetHostname(void)
{
	if (hostname == NULL)
	{
		return NULL;
	}
	wchar_t *tempstring = (wchar_t *)calloc(wcslen(hostname) + 1, sizeof(wchar_t));
	wcscpy(tempstring, hostname);
	return tempstring;
}

bool ServerCore::GetComponentSelectStatus(void)
{
	return componentselect;
}

void ServerCore::EnableComponentSelect(bool enable)
{
	componentselect = enable;
	componentselectcvar->SetValue(enable);
}

void ServerCore::GetTime(unsigned char (&time)[2])
{
	memcpy(time, &gametime.time, sizeof(unsigned char) * 2);
}

void ServerCore::SetTime(const unsigned char time[2])
{
	if ((time[0] >= 24) || (time[1] >= 60))
	{
		return;
	}
	memcpy(gametime.time, &time, sizeof(unsigned char) * 2);
	nm.SendTime(gametime.time);
}

void ServerCore::IncrementGameTime(void)
{
	gametime.time[1]++;
	if (gametime.time[1] == 60)
	{
		gametime.time[1] = 0;
		gametime.time[0]++;
		if (gametime.time[0] == 24)
		{
			gametime.time[0] = 0;
		}
	}
}