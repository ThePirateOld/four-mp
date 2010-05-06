/// \file
/// \brief Source file that contains implementation of the four-mp console variable hooks.
/// \details See con_fmpcvarhooks.h.
/// \author FaTony

#include <string.h>
#include <stdlib.h>

#include "con_fmpcvarhooks.h"

#include "ServerCore.h"
#include "../../Shared/Console/common.h"

extern ServerCore server;

void ConVarHookHostGamemode(ConVar *convar, const ConVarType oldtype, void *oldvalue, const ConVarType newtype, void *newvalue)
{
	if (newtype == ConVarTypeString)
	{
		ResizeBuffer<char *, char, unsigned int>(server.gamemode, strlen((char *)newvalue) + 1);
		strcpy(server.gamemode, (char *)newvalue);
		return;
	}
	char *value;
	convar->GetValue(value);
	ResizeBuffer<char *, char, unsigned int>(server.gamemode, strlen(value) + 1);
	strcpy(server.gamemode, value);
	free(value);
}

void ConVarHookHostname(ConVar *convar, const ConVarType oldtype, void *oldvalue, const ConVarType newtype, void *newvalue)
{
	if (newtype == ConVarTypeString)
	{
		ResizeBuffer<char *, char, unsigned int>(server.hostname, strlen((char *)newvalue) + 1);
		strcpy(server.hostname, (char *)newvalue);
		return;
	}
	char *value;
	convar->GetValue(value);
	ResizeBuffer<char *, char, unsigned int>(server.hostname, strlen(value) + 1);
	strcpy(server.hostname, value);
	free(value);
}

void ConVarHookRconPassword(ConVar *convar, const ConVarType oldtype, void *oldvalue, const ConVarType newtype, void *newvalue)
{
	if (newtype == ConVarTypeString)
	{
		ResizeBuffer<char *, char, unsigned int>(server.rconpassword, strlen((char *)newvalue) + 1);
		strcpy(server.rconpassword, (char *)newvalue);
		return;
	}
	char *value;
	convar->GetValue(value);
	ResizeBuffer<char *, char, unsigned int>(server.rconpassword, strlen(value) + 1);
	strcpy(server.rconpassword, value);
	free(value);
}

void ConVarHookSvComponentselect(ConVar *convar, const ConVarType oldtype, void *oldvalue, const ConVarType newtype, void *newvalue)
{
	if (newtype != ConVarTypeInt)
	{
		return;
	}
	server.componentselect = *(int *)newvalue;
}
void ConVarHookSvLan(ConVar *convar, const ConVarType oldtype, void *oldvalue, const ConVarType newtype, void *newvalue)
{
	if (newtype != ConVarTypeInt)
	{
		return;
	}
	server.lan = *(int *)newvalue;
}

void ConVarHookSvPassword(ConVar *convar, const ConVarType oldtype, void *oldvalue, const ConVarType newtype, void *newvalue)
{
	if (newtype == ConVarTypeString)
	{
		ResizeBuffer<char *, char, unsigned int>(server.password, strlen((char *)newvalue) + 1);
		strcpy(server.password, (char *)newvalue);
		return;
	}
	char *value;
	convar->GetValue(value);
	ResizeBuffer<char *, char, unsigned int>(server.password, strlen(value) + 1);
	strcpy(server.password, value);
	free(value);
}

void ConVarHookSvPort(ConVar *convar, const ConVarType oldtype, void *oldvalue, const ConVarType newtype, void *newvalue)
{
	if (newtype != ConVarTypeInt)
	{
		return;
	}
	server.port = *(int *)newvalue;
}