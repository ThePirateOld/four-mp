/// \file
/// \brief Source file that contains implementation of the VirtualMachineManager class.
/// \details See class description.
/// \author FaTony. Wrapped around initial WNeZRoS' code.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>

#include "sq/squirrel.h"
#include "sq/sqstdaux.h"
#include "sq/sqstdblob.h"
#include "sq/sqstdio.h"
#include "sq/sqstdmath.h"
#include "sq/sqstdstring.h"
#include "sq/sqstdsystem.h"

#include "VirtualMachineManager.h"
#include "../../Shared/Console/common.h"
#include "logging.h"
#include "HandleManager.h"
#include "CoreHandleTypesManager.h"

#include "sq.h"
#include "sq_vmfunc.h"
#include "sq_consolenatives.h"
#include "sq_playernatives.h"
#include "sq_callbacks.h"

extern HandleManager hm;
extern CoreHandleTypesManager chtm;

VirtualMachineManager::VirtualMachineManager(void)
{
	maxvmbuffersize = 17;
	vmbuffer = (VirtualMachine **)calloc(1, sizeof(VirtualMachine *));
	vmbuffer[0] = NULL;
	vmbuffersize = 0;
}

VirtualMachineManager::~VirtualMachineManager(void)
{
	this->UnloadGameMode();
	this->UnloadFilterScripts();
	free(vmbuffer);
}

bool VirtualMachineManager::IsGameModeLoaded(void)
{
	if (vmbuffer[0] == NULL)
	{
		return false;
	}
	return true;
}

bool VirtualMachineManager::LoadGameMode(const wchar_t *string)
{
	if (vmbuffer[0] != NULL)
	{
		return false;
	}
	if (_waccess(L"gamemodes", 0) == -1)
	{
		_wmkdir(L"gamemodes");
		PrintToServer(L"Unable to load game mode. Directory \"gamemodes\" does not exist");
		return false;
	}
	size_t length = wcslen(string);
	wchar_t *gamemode = (wchar_t *)calloc(length + 11, sizeof(wchar_t));
	swprintf(gamemode, length + 11, L"gamemodes/%s", string);
	if (!this->LoadVirtualMachine(0, gamemode))
	{
		return false;
	}
	vmbuffer[0]->filename = (wchar_t *)calloc(length + 1, sizeof(wchar_t));
	wcscpy(vmbuffer[0]->filename, string);
	this->OnGameModeInit();
	return true;
}

bool VirtualMachineManager::UnloadGameMode(void)
{
	if (!this->IsGameModeLoaded())
	{
		return false;
	}
	this->OnGameModeExit();
	if (!this->UnloadVirtualMachine(0))
	{
		return false;
	}
	free(vmbuffer[0]->filename);
	delete vmbuffer[0];
	vmbuffer[0] = NULL;
	return true;
}

wchar_t *VirtualMachineManager::GetGameModeName(void)
{
	if (!this->IsGameModeLoaded())
	{
		return NULL;
	}
	wchar_t *name = (wchar_t *)calloc(wcslen(vmbuffer[0]->name) + 1, sizeof(wchar_t));
	wcscpy(name, vmbuffer[0]->name);
	return name;
}

unsigned char VirtualMachineManager::GetMaxVirtualMachineBufferSize(void)
{
	return maxvmbuffersize;
}

unsigned char VirtualMachineManager::GetVirtualMachineBufferSize(void)
{
	return vmbuffersize;
}

void VirtualMachineManager::LoadFilterScripts(void)
{
	if (_waccess(L"filterscripts", 0) == -1)
	{
		_wmkdir(L"filterscripts");
		return;
	}
	intptr_t ptr;
	_wfinddata64i32_t data;
	ptr = _wfindfirst(L"filterscripts\\*.*", &data);
	if (ptr == -1)
	{
		return;
	}
	_wfindnext(ptr, &data);
	int continuesearch = _wfindnext(ptr, &data);
	unsigned char slots = this->GetNumberOfFreeFilterScriptSlots();
	unsigned char i = 0;
	while ((continuesearch == 0) && (i < slots))
	{
		if ((!this->IsFilterScriptLoaded(data.name)) && (this->LoadFilterScript(data.name)))
		{
			i++;
		}
		continuesearch = _wfindnext(ptr, &data);
	}
	_findclose(ptr);
}

void VirtualMachineManager::UnloadFilterScripts(void)
{
	for (unsigned char i = 1; i < vmbuffersize; i++)
	{
		if (vmbuffer[i] != NULL)
		{
			this->UnloadFilterScript(i);
		}
	}
}

void VirtualMachineManager::ReloadFilterScripts(void)
{
	for (unsigned char i = 1; i < vmbuffersize; i++)
	{
		if (vmbuffer[i] != NULL)
		{
			this->ReloadFilterScript(i);
		}
	}
}

void VirtualMachineManager::PauseVirtualMachines(void)
{
	for (unsigned char i = 0; i < vmbuffersize; i++)
	{
		if (vmbuffer[i] != NULL)
		{
			this->PauseVirtualMachine(i);
		}
	}
}

void VirtualMachineManager::UnpauseVirtualMachines(void)
{
	for (unsigned char i = 0; i < vmbuffersize; i++)
	{
		if (vmbuffer[i] != NULL)
		{
			this->UnpauseVirtualMachine(i);
		}
	}
}

bool VirtualMachineManager::IsFilterScriptLoaded(const wchar_t *string)
{
	for (unsigned char i = 1; i < vmbuffersize; i++)
	{
		if ((vmbuffer[i] != NULL) && (wcscmp(vmbuffer[i]->filename, string) == 0))
		{
			return true;
		}
	}
	return false;
}

bool VirtualMachineManager::LoadFilterScript(const wchar_t *string)
{
	unsigned char index;
	if (!this->GetFilterScriptFreeSlot(index))
	{
		return false;
	}
	if (!this->LoadFilterScriptInternal(index, string))
	{
		return false;
	}
	return true;
}

bool VirtualMachineManager::UnloadFilterScript(const unsigned char index)
{
	if (index >= vmbuffersize)
	{
		return false;
	}
	if (index == 0)
	{
		return false;
	}
	if (vmbuffer[index] == NULL)
	{
		return false;
	}
	this->OnFilterScriptExit(index);
	if (!this->UnloadVirtualMachine(index))
	{
		return false;
	}
	free(vmbuffer[index]->filename);
	delete vmbuffer[index];
	vmbuffer[index] = NULL;
	return true;
}

bool VirtualMachineManager::ReloadFilterScript(const unsigned char index)
{
	if (index >= vmbuffersize)
	{
		return false;
	}
	if (vmbuffer[index] == NULL)
	{
		return false;
	}
	wchar_t *filename = (wchar_t *)calloc(wcslen(vmbuffer[index]->filename) + 1, sizeof(wchar_t));
	wcscpy(filename, vmbuffer[index]->filename);
	if (!this->UnloadFilterScript(index))
	{
		return false;
	}
	if (!this->LoadFilterScriptInternal(index, filename))
	{
		return false;
	}
	return true;
}

bool VirtualMachineManager::PauseVirtualMachine(const unsigned char index)
{
	if (index >= vmbuffersize)
	{
		return false;
	}
	if (vmbuffer[index] == NULL)
	{
		return false;
	}
	if (vmbuffer[index]->paused)
	{
		return false;
	}
	vmbuffer[index]->paused = true;
	return true;
}

bool VirtualMachineManager::UnpauseVirtualMachine(const unsigned char index)
{
	if (index >= vmbuffersize)
	{
		return false;
	}
	if (vmbuffer[index] == NULL)
	{
		return false;
	}
	if (!vmbuffer[index]->paused)
	{
		return false;
	}
	vmbuffer[index]->paused = false;
	return true;
}

wchar_t *VirtualMachineManager::GetVirtualMachineInfoString(const unsigned char index)
{
	if (index >= vmbuffersize)
	{
		return NULL;
	}
	if (vmbuffer[index] == NULL)
	{
		return NULL;
	}
	wchar_t *string;
	if (!vmbuffer[index]->paused)
	{
		int length = _scwprintf(L"%d \"%s\" (%s) by %s", index, vmbuffer[index]->name, vmbuffer[index]->version, vmbuffer[index]->author) + 1;
		string = (wchar_t *)calloc(length, sizeof(wchar_t));
		swprintf(string, length, L"%d \"%s\" (%s) by %s", index, vmbuffer[index]->name, vmbuffer[index]->version, vmbuffer[index]->author);
	}
	else
	{
		int length = _scwprintf(L"%d (Paused) \"%s\" (%s) by %s", index, vmbuffer[index]->name, vmbuffer[index]->version, vmbuffer[index]->author) + 1;
		string = (wchar_t *)calloc(length, sizeof(wchar_t));
		swprintf(string, length, L"%d (Paused) \"%s\" (%s) by %s", index, vmbuffer[index]->name, vmbuffer[index]->version, vmbuffer[index]->author);
	}
	return string;
}

bool VirtualMachineManager::FindVirtualMachine(const HSQUIRRELVM *v, unsigned char &index)
{
	for (index = 0; index < vmbuffersize; index++)
	{
		if ((vmbuffer[index] != NULL) && (vmbuffer[index]->lang == VMLanguageSquirrel) && (*vmbuffer[index]->ptr.squirrel == *v))
		{
			return true;
		}
	}
	return false;
}

void VirtualMachineManager::SetVirtualMachineName(const unsigned char index, const wchar_t *string)
{
	if (index >= vmbuffersize)
	{
		return;
	}
	if (vmbuffer[index] == NULL)
	{
		return;
	}
	ResizeBuffer<wchar_t *>(vmbuffer[index]->name, wcslen(string) + 1);
	wcscpy(vmbuffer[index]->name, string);
}

void VirtualMachineManager::SetVirtualMachineVersion(const unsigned char index, const wchar_t *string)
{
	if (index >= vmbuffersize)
	{
		return;
	}
	if (vmbuffer[index] == NULL)
	{
		return;
	}
	ResizeBuffer<wchar_t *>(vmbuffer[index]->version, wcslen(string) + 1);
	wcscpy(vmbuffer[index]->version, string);
}

void VirtualMachineManager::SetVirtualMachineAuthor(const unsigned char index, const wchar_t *string)
{
	if (index >= vmbuffersize)
	{
		return;
	}
	if (vmbuffer[index] == NULL)
	{
		return;
	}
	ResizeBuffer<wchar_t *>(vmbuffer[index]->author, wcslen(string) + 1);
	wcscpy(vmbuffer[index]->author, string);
}

bool VirtualMachineManager::OnPlayerConnect(const short index, const wchar_t *name)
{
	for (unsigned char i = 0; i < vmbuffersize; i++)
	{
		if ((vmbuffer[i] != NULL) && (!vmbuffer[i]->paused))
		{
			switch (vmbuffer[i]->lang)
			{
			case VMLanguageSquirrel:
				{
					if (!sc_OnPlayerConnect(*vmbuffer[i]->ptr.squirrel, index, name))
					{
						return false;
					}
					break;
				}
			}
		}
	}
	return true;
}

void VirtualMachineManager::OnPlayerDisconnect(const short index)
{
	for (unsigned char i = 0; i < vmbuffersize; i++)
	{
		if ((vmbuffer[i] != NULL) && (!vmbuffer[i]->paused))
		{
			switch (vmbuffer[i]->lang)
			{
			case VMLanguageSquirrel:
				{
					sc_OnPlayerDisconnect(*vmbuffer[i]->ptr.squirrel, index);
					break;
				}
			}
		}
	}
}

void VirtualMachineManager::OnPlayerSpawn(const short playerindex)
{
	for (unsigned char i = 0; i < vmbuffersize; i++)
	{
		if ((vmbuffer[i] != NULL) && (!vmbuffer[i]->paused))
		{
			switch (vmbuffer[i]->lang)
			{
			case VMLanguageSquirrel:
				{
					sc_OnPlayerSpawn(*vmbuffer[i]->ptr.squirrel, playerindex);
					break;
				}
			}
		}
	}
}

bool VirtualMachineManager::OnPlayerText(const short playerindex, const wchar_t *data)
{
	for (unsigned char i = 0; i < vmbuffersize; i++)
	{
		if ((vmbuffer[i] != NULL) && (!vmbuffer[i]->paused))
		{
			switch (vmbuffer[i]->lang)
			{
			case VMLanguageSquirrel:
				{
					if (!sc_OnPlayerText(*vmbuffer[i]->ptr.squirrel, playerindex, data))
					{
						return false;
					}
					break;
				}
			}
		}
	}
	return true;
}

void VirtualMachineManager::FireCommandCallback(const unsigned char index, const wchar_t *callback, const unsigned char numargs)
{
	if (index >= vmbuffersize)
	{
		return;
	}
	if (vmbuffer[index] == NULL)
	{
		return;
	}
	switch (vmbuffer[index]->lang)
	{
	case VMLanguageSquirrel:
		{
			sc_CommandCallback(*vmbuffer[index]->ptr.squirrel, callback, numargs);
			break;
		}
	}
}

bool VirtualMachineManager::LoadFilterScriptInternal(const unsigned char index, const wchar_t *string)
{
	size_t length = wcslen(string);
	wchar_t *filterscript = (wchar_t *)calloc(length + 15, sizeof(wchar_t));
	swprintf(filterscript, length + 15, L"filterscripts/%s", string);
	if (!this->LoadVirtualMachine(index, filterscript))
	{
		return false;
	}
	vmbuffer[index]->filename = (wchar_t *)calloc(length + 1, sizeof(wchar_t));
	wcscpy(vmbuffer[index]->filename, string);
	this->OnFilterScriptInit(index);
	return true;
}

bool VirtualMachineManager::LoadVirtualMachine(const unsigned char index, const wchar_t *string)
{
	if (index >= maxvmbuffersize)
	{
		return false;
	}
	if (index >= vmbuffersize)
	{
		if (vmbuffersize == maxvmbuffersize)
		{
			return false;
		}
		if (!ResizeBuffer<VirtualMachine **>(vmbuffer, index + 1))
		{
			return false;
		}
		vmbuffer[index] = NULL;
		vmbuffersize = index + 1;
	}
	if (vmbuffer[index] != NULL)
	{
		return false;
	}
	VirtualMachineLanguage lang;
	if (wcscmp(wcsrchr(string, L'.') + 1, L"nut") == 0)
	{
		lang = VMLanguageSquirrel;
	}
	else
	{
		return false;
	}
	vmbuffer[index] = new VirtualMachine;
	switch (lang)
	{
	case VMLanguageSquirrel:
		{
			vmbuffer[index]->ptr.squirrel = new HSQUIRRELVM;
			// Init Squirrel
			*vmbuffer[index]->ptr.squirrel = sq_open(1024);
			sqstd_seterrorhandlers(*vmbuffer[index]->ptr.squirrel);
			sq_setprintfunc(*vmbuffer[index]->ptr.squirrel, sq_PrintToServer, sq_PrintToServer);
			// Register Script Funcions
			// Script identity functions
			register_global_func(*vmbuffer[index]->ptr.squirrel, (SQFUNCTION)sq_SetScriptName, L"SetScriptName");
			register_global_func(*vmbuffer[index]->ptr.squirrel, (SQFUNCTION)sq_SetScriptVersion, L"SetScriptVersion");
			register_global_func(*vmbuffer[index]->ptr.squirrel, (SQFUNCTION)sq_SetScriptAuthor, L"SetScriptAuthor");
			// Console functions
			register_global_func(*vmbuffer[index]->ptr.squirrel, (SQFUNCTION)sq_CreateConVar, L"CreateConVar");
			register_global_func(*vmbuffer[index]->ptr.squirrel, (SQFUNCTION)sq_FindConVar, L"FindConVar");
			register_global_func(*vmbuffer[index]->ptr.squirrel, (SQFUNCTION)sq_ResetConVar, L"ResetConVar");
			register_global_func(*vmbuffer[index]->ptr.squirrel, (SQFUNCTION)sq_GetConVarName, L"GetConVarName");
			register_global_func(*vmbuffer[index]->ptr.squirrel, (SQFUNCTION)sq_GetConVarFloat, L"GetConVarFloat");
			register_global_func(*vmbuffer[index]->ptr.squirrel, (SQFUNCTION)sq_GetConVarInt, L"GetConVarInt");
			register_global_func(*vmbuffer[index]->ptr.squirrel, (SQFUNCTION)sq_GetConVarString, L"GetConVarString");
			register_global_func(*vmbuffer[index]->ptr.squirrel, (SQFUNCTION)sq_GetConVarFlags, L"GetConVarFlags");
			register_global_func(*vmbuffer[index]->ptr.squirrel, (SQFUNCTION)sq_GetConVarBoundFloat, L"GetConVarBoundFloat");
			register_global_func(*vmbuffer[index]->ptr.squirrel, (SQFUNCTION)sq_GetConVarBoundInt, L"GetConVarBoundInt");
			register_global_func(*vmbuffer[index]->ptr.squirrel, (SQFUNCTION)sq_SetConVarFloat, L"SetConVarFloat");
			register_global_func(*vmbuffer[index]->ptr.squirrel, (SQFUNCTION)sq_SetConVarInt, L"SetConVarInt");
			register_global_func(*vmbuffer[index]->ptr.squirrel, (SQFUNCTION)sq_SetConVarString, L"SetConVarString");
			register_global_func(*vmbuffer[index]->ptr.squirrel, (SQFUNCTION)sq_SetConVarFlags, L"SetConVarFlags");
			register_global_func(*vmbuffer[index]->ptr.squirrel, (SQFUNCTION)sq_SetConVarBoundFloat, L"SetConVarBoundFloat");
			register_global_func(*vmbuffer[index]->ptr.squirrel, (SQFUNCTION)sq_SetConVarBoundInt, L"SetConVarBoundInt");
			register_global_func(*vmbuffer[index]->ptr.squirrel, (SQFUNCTION)sq_RegServerCmd, L"RegServerCmd");
			register_global_func(*vmbuffer[index]->ptr.squirrel, (SQFUNCTION)sq_GetCmdArgs, L"GetCmdArgs");
			register_global_func(*vmbuffer[index]->ptr.squirrel, (SQFUNCTION)sq_GetCmdArgsAsString, L"GetCmdArgsAsString");
			register_global_func(*vmbuffer[index]->ptr.squirrel, (SQFUNCTION)sq_GetCmdArgType, L"GetCmdArgType");
			register_global_func(*vmbuffer[index]->ptr.squirrel, (SQFUNCTION)sq_GetCmdArgString, L"GetCmdArgString");
			register_global_func(*vmbuffer[index]->ptr.squirrel, (SQFUNCTION)sq_GetCmdArgInt, L"GetCmdArgInt");
			register_global_func(*vmbuffer[index]->ptr.squirrel, (SQFUNCTION)sq_GetCmdArgFloat, L"GetCmdArgFloat");
			register_global_func(*vmbuffer[index]->ptr.squirrel, (SQFUNCTION)sq_ServerCommand, L"ServerCommand");
			// Player functions
			register_global_func(*vmbuffer[index]->ptr.squirrel, (SQFUNCTION)sq_GetPlayerName, L"GetPlayerName");
			register_global_func(*vmbuffer[index]->ptr.squirrel, (SQFUNCTION)sq_GetPlayerModel, L"GetPlayerModel");
			register_global_func(*vmbuffer[index]->ptr.squirrel, (SQFUNCTION)sq_GetPlayerPosition, L"GetPlayerPosition");
			register_global_func(*vmbuffer[index]->ptr.squirrel, (SQFUNCTION)sq_GetPlayerAngle, L"GetPlayerAngle");
			register_global_func(*vmbuffer[index]->ptr.squirrel, (SQFUNCTION)sq_GetPlayerScore, L"GetPlayerScore");
			register_global_func(*vmbuffer[index]->ptr.squirrel, (SQFUNCTION)sq_GetPlayerHealth, L"GetPlayerHealth");
			register_global_func(*vmbuffer[index]->ptr.squirrel, (SQFUNCTION)sq_GetPlayerArmor, L"GetPlayerArmor");
			register_global_func(*vmbuffer[index]->ptr.squirrel, (SQFUNCTION)sq_GetPlayerWantedLevel, L"GetPlayerWantedLevel");
			register_global_func(*vmbuffer[index]->ptr.squirrel, (SQFUNCTION)sq_GiveWeapon, L"GiveWeapon");
			register_global_func(*vmbuffer[index]->ptr.squirrel, (SQFUNCTION)sq_addPlayerClass, L"addPlayerClass");
			register_global_func(*vmbuffer[index]->ptr.squirrel, (SQFUNCTION)sq_enableComponentSelect, L"enableComponentSelect");
			register_global_func(*vmbuffer[index]->ptr.squirrel, (SQFUNCTION)sq_SetPlayerSpawnPos, L"SetPlayerSpawnPos");
			register_global_func(*vmbuffer[index]->ptr.squirrel, (SQFUNCTION)sq_SetPlayerModel, L"SetPlayerModel");
			// Time func
			register_global_func(*vmbuffer[index]->ptr.squirrel, (SQFUNCTION)sq_GetGameHour, L"GetGameHour");
			register_global_func(*vmbuffer[index]->ptr.squirrel, (SQFUNCTION)sq_GetGameMinutes, L"GetGameMinutes");

			// Car functions
			register_global_func(*vmbuffer[index]->ptr.squirrel, (SQFUNCTION)sq_CreateCar, L"CreateCar");
			sq_pushroottable(*vmbuffer[index]->ptr.squirrel);
			// Register Standard Script Functions
			sqstd_register_stringlib(*vmbuffer[index]->ptr.squirrel);
			sqstd_register_mathlib(*vmbuffer[index]->ptr.squirrel);
			sqstd_register_systemlib(*vmbuffer[index]->ptr.squirrel);
			sqstd_seterrorhandlers(*vmbuffer[index]->ptr.squirrel);
			if(!SQ_SUCCEEDED(sqstd_dofile(*vmbuffer[index]->ptr.squirrel, string, 0, SQTrue))) 
			{
				sq_close(*vmbuffer[index]->ptr.squirrel);
				delete vmbuffer[index]->ptr.squirrel;
				delete vmbuffer[index];
				vmbuffer[index] = NULL;
				return false;
			}
			vmbuffer[index]->lang = VMLanguageSquirrel;
			break;
		}
	//case VMLanguagePawn:
	//	{
			//Init Pawn
			//int err = aux_LoadProgram(&amx, "test.amx", NULL);
			//if (err != AMX_ERR_NONE)
			//{
			//	//ErrorExit(&amx, err);
			//}
			//pawn_Init(amx);
			//if (err)
			//{
			//	ErrorExit(&amx, err);
			//}
			//err = amx_Exec(&amx, &ret, AMX_EXEC_MAIN);
	//		break;
	//	}
	}
	vmbuffer[index]->paused = false;
	vmbuffer[index]->name = (wchar_t *)calloc(16, sizeof(wchar_t));
	wcscpy(vmbuffer[index]->name, L"Untitled script");
	vmbuffer[index]->version = (wchar_t *)calloc(8, sizeof(wchar_t));
	wcscpy(vmbuffer[index]->version, L"0.0.0.0");
	vmbuffer[index]->author = (wchar_t *)calloc(15, sizeof(wchar_t));
	wcscpy(vmbuffer[index]->author, L"Unnamed author");
	return true;
}

bool VirtualMachineManager::UnloadVirtualMachine(const unsigned char index)
{
	if (index >= vmbuffersize)
	{
		return false;
	}
	if (vmbuffer[index] == NULL)
	{
		return false;
	}
	switch (vmbuffer[index]->lang)
	{
	case VMLanguageSquirrel:
		{
			sq_close(*vmbuffer[index]->ptr.squirrel);
			delete vmbuffer[index]->ptr.squirrel;
			break;
		}
	}
	vmbuffer[index]->paused = false;
	free(vmbuffer[index]->name);
	free(vmbuffer[index]->version);
	free(vmbuffer[index]->author);
	hm.CloseAllHandles(index + 1);
	return true;
}

unsigned char VirtualMachineManager::GetNumberOfFreeFilterScriptSlots(void)
{
	unsigned char slots = 0;
	for (unsigned char i = 1; i < vmbuffersize; i++)
	{
		if (vmbuffer[i] == NULL)
		{
			slots++;
		}
	}
	slots = maxvmbuffersize - vmbuffersize + slots - 1;
	return slots;
}

bool VirtualMachineManager::GetFilterScriptFreeSlot(unsigned char &index)
{
	for (index = 1; index < vmbuffersize; index++)
	{
		if (vmbuffer[index] == NULL)
		{
			return true;
		}
	}
	if (vmbuffersize == maxvmbuffersize)
	{
		return false;
	}
	return true;
}

void VirtualMachineManager::OnGameModeInit(void)
{
	for (unsigned char i = 0; i < vmbuffersize; i++)
	{
		if (vmbuffer[i] != NULL)
		{
			switch (vmbuffer[i]->lang)
			{
			case VMLanguageSquirrel:
				{
					sc_OnGameModeInit(*vmbuffer[i]->ptr.squirrel);
					break;
				}
			}
		}
	}
}

void VirtualMachineManager::OnGameModeExit(void)
{
	for (unsigned char i = 0; i < vmbuffersize; i++)
	{
		if (vmbuffer[i] != NULL)
		{
			switch (vmbuffer[i]->lang)
			{
			case VMLanguageSquirrel:
				{
					sc_OnGameModeExit(*vmbuffer[i]->ptr.squirrel);
					break;
				}
			}
		}
	}
}

void VirtualMachineManager::OnFilterScriptInit(const unsigned char index)
{
	if (index >= vmbuffersize)
	{
		return;
	}
	if (vmbuffer[index] == NULL)
	{
		return;
	}
	switch (vmbuffer[index]->lang)
	{
	case VMLanguageSquirrel:
		{
			sc_OnFilterScriptInit(*vmbuffer[index]->ptr.squirrel);
			break;
		}
	}
}

void VirtualMachineManager::OnFilterScriptExit(const unsigned char index)
{
	if (index >= vmbuffersize)
	{
		return;
	}
	if (vmbuffer[index] == NULL)
	{
		return;
	}
	switch (vmbuffer[index]->lang)
	{
	case VMLanguageSquirrel:
		{
			sc_OnFilterScriptExit(*vmbuffer[index]->ptr.squirrel);
			break;
		}
	}
}