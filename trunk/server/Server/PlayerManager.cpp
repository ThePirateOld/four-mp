#include <stdlib.h>

#include "PlayerManager.h"
#include "../../Shared/Console/common.h"
#include "logging.h"
#include "NetworkManager.h"
#include "VirtualMachineManager.h"
#include "VehicleManager.h"

extern NetworkManager nm;
extern VirtualMachineManager vmm;
extern VehicleManager vm;

PlayerManager::PlayerManager(void)
{
	maxplayerbuffersize = 32;
	maxclassbuffersize = 64;
	playerbuffer = NULL;
	classbuffer = NULL;
	playerbuffersize = 0;
	classbuffersize = 0;
}

PlayerManager::~PlayerManager(void)
{
}

short PlayerManager::GetMaxPlayers(void)
{
	return maxplayerbuffersize;
}

short PlayerManager::GetNumberOfPlayers(void)
{
	return numplayers;
}

bool PlayerManager::IsServerFull(void)
{
	if (playerbuffersize < maxplayerbuffersize)
	{
		return false;
	}
	for (unsigned char i = 0; i < playerbuffersize; i++)
	{
		if (playerbuffer[i] == NULL)
		{
			return false;
		}
	}
	return true;
}

bool PlayerManager::IsPlayerConnected(const short index)
{
	if ((index < 0) || (index >= playerbuffersize))
	{
		return false;
	}
	if (playerbuffer[index] == NULL)
	{
		return false;
	}
	return true;
}

char *PlayerManager::GetPlayerName(const short index)
{
	if ((index < 0) || (index >= playerbuffersize))
	{
		return NULL;
	}
	if (playerbuffer[index] == NULL)
	{
		return NULL;
	}
	return playerbuffer[index]->name;
}

unsigned int PlayerManager::GetPlayerModel(const short index)
{
	if ((index < 0) || (index >= playerbuffersize))
	{
		return INVALID_PLAYER_MODEL;
	}
	if (playerbuffer[index] == NULL)
	{
		return INVALID_PLAYER_MODEL;
	}
	return playerbuffer[index]->model;
}

bool PlayerManager::GetPlayerPosition(const short index, float (&position)[3])
{
	if ((index < 0) || (index >= playerbuffersize))
	{
		return false;
	}
	if (playerbuffer[index] == NULL)
	{
		return false;
	}
	memcpy(position, playerbuffer[index]->position, sizeof(float) * 3);
	return true;
}

bool PlayerManager::GetPlayerAngle(const short index, float &angle)
{
	if ((index < 0) || (index >= playerbuffersize))
	{
		return false;
	}
	if (playerbuffer[index] == NULL)
	{
		return false;
	}
	angle = playerbuffer[index]->angle;
	return true;
}

short PlayerManager::GetPlayerVehicle(const short index)
{
	if ((index < 0) || (index >= playerbuffersize))
	{
		return INVALID_VEHICLE_INDEX;
	}
	if (playerbuffer[index] == NULL)
	{
		return INVALID_VEHICLE_INDEX;
	}
	return playerbuffer[index]->vehicleindex;
}

bool PlayerManager::GetPlayerScore(const short index, int &score)
{
	if ((index < 0) || (index >= playerbuffersize))
	{
		return false;
	}
	if (playerbuffer[index] == NULL)
	{
		return false;
	}
	score = playerbuffer[index]->score;
	return true;
}

bool PlayerManager::GetPlayerHealth(const short index, int &health)
{
	if ((index < 0) || (index >= playerbuffersize))
	{
		return false;
	}
	if (playerbuffer[index] == NULL)
	{
		return false;
	}
	health = playerbuffer[index]->health;
	return true;
}

bool PlayerManager::GetPlayerArmor(const short index, int &armor)
{
	if ((index < 0) || (index >= playerbuffersize))
	{
		return false;
	}
	if (playerbuffer[index] == NULL)
	{
		return false;
	}
	armor = playerbuffer[index]->armor;
	return true;
}

char PlayerManager::GetPlayerWantedLevel(const short index)
{
	if ((index < 0) || (index >= playerbuffersize))
	{
		return INVALID_PLAYER_WANTED_LEVEL;
	}
	if (playerbuffer[index] == NULL)
	{
		return INVALID_PLAYER_WANTED_LEVEL;
	}
	return playerbuffer[index]->wanted_level;
}

bool PlayerManager::GetPlayerColor(const short index, unsigned char (&color)[4])
{
	if ((index < 0) || (index >= playerbuffersize))
	{
		return false;
	}
	if (playerbuffer[index] == NULL)
	{
		return false;
	}
	memcpy(color, playerbuffer[index]->color, sizeof(unsigned char) * 4);
	return true;
}

unsigned char PlayerManager::GetNumberOfPlayerClasses(void)
{
	unsigned char numclasses = 0;
	for (unsigned char i = 0; i < classbuffersize; i++)
	{
		if (classbuffer[i] != NULL)
		{
			numclasses++;
		}
	}
	return numclasses;
}

bool PlayerManager::AddPlayerClass(const unsigned int model, const float position[3], const float angle, const char weapons[8], const short ammo[8], unsigned char &index)
{
	PrintToServer("Player manager::AddPlayerClass");
	if (!this->GetClassFreeSlot(index))
	{
		PrintToServer("Unable to add player class. No free slots.");
		return false;
	}
	if (index >= classbuffersize)
	{
		if (classbuffersize == maxclassbuffersize)
		{
			return false;
		}
		debug("resize, index = %d", index);
		if (!ResizeBuffer<PlayerClass **, PlayerClass *, unsigned char>(classbuffer, index + 1))
		{
			debug("finish resize");
			return false;
		}
		classbuffer[index] = NULL;
		classbuffersize = index + 1;
	}
	PrintToServer("111");
	classbuffer[index] = new PlayerClass;
	classbuffer[index]->model = model;
	classbuffer[index]->position[0] = position[0];
	classbuffer[index]->position[1] = position[1];
	classbuffer[index]->position[2] = position[2];
	classbuffer[index]->angle = angle;
	classbuffer[index]->weapons[0] = weapons[0];
	classbuffer[index]->weapons[1] = weapons[1];
	classbuffer[index]->weapons[2] = weapons[2];
	classbuffer[index]->weapons[3] = weapons[3];
	classbuffer[index]->weapons[4] = weapons[4];
	classbuffer[index]->weapons[5] = weapons[5];
	classbuffer[index]->weapons[6] = weapons[6];
	classbuffer[index]->weapons[7] = weapons[7];
	classbuffer[index]->ammo[0] = ammo[0];
	classbuffer[index]->ammo[1] = ammo[1];
	classbuffer[index]->ammo[2] = ammo[2];
	classbuffer[index]->ammo[3] = ammo[3];
	classbuffer[index]->ammo[4] = ammo[4];
	classbuffer[index]->ammo[5] = ammo[5];
	classbuffer[index]->ammo[6] = ammo[6];
	classbuffer[index]->ammo[7] = ammo[7];
	//TODO: sync it? Does classes even sync?
	return true;
}

bool PlayerManager::GetPlayerClassData(const unsigned char index, unsigned int &model, float (&position)[3], float &angle, char (&weapons)[8], short (&ammo)[8])
{
	if (index >= classbuffersize)
	{
		return false;
	}
	if (classbuffer[index] == NULL)
	{
		return false;
	}
	model = classbuffer[index]->model;
	position[0] = classbuffer[index]->position[0];
	position[1] = classbuffer[index]->position[1];
	position[2] = classbuffer[index]->position[2];
	angle = classbuffer[index]->angle;
	weapons[0] = classbuffer[index]->weapons[0];
	weapons[1] = classbuffer[index]->weapons[1];
	weapons[2] = classbuffer[index]->weapons[2];
	weapons[3] = classbuffer[index]->weapons[3];
	weapons[4] = classbuffer[index]->weapons[4];
	weapons[5] = classbuffer[index]->weapons[5];
	weapons[6] = classbuffer[index]->weapons[6];
	weapons[7] = classbuffer[index]->weapons[7];
	ammo[0] = classbuffer[index]->ammo[0];
	ammo[1] = classbuffer[index]->ammo[1];
	ammo[2] = classbuffer[index]->ammo[2];
	ammo[3] = classbuffer[index]->ammo[3];
	ammo[4] = classbuffer[index]->ammo[4];
	ammo[5] = classbuffer[index]->ammo[5];
	ammo[6] = classbuffer[index]->ammo[6];
	ammo[7] = classbuffer[index]->ammo[7];
	return true;
}

bool PlayerManager::RegisterNewPlayer(const short index, const char *name)
{
	if ((index < 0) || (index >= maxplayerbuffersize))
	{
		return false;
	}
	if (index >= playerbuffersize)
	{
		if (playerbuffersize == maxplayerbuffersize)
		{
			return false;
		}
		if (!ResizeBuffer<Player **, Player *, short>(playerbuffer, index + 1))
		{
			return false;
		}
		playerbuffer[index] = NULL;
		playerbuffersize = index + 1;
	}
	if (playerbuffer[index] != NULL)
	{
		return false;
	}
	playerbuffer[index] = new Player;
	//playerbuffer[index]->name = (char *)calloc(strlen(name) + 1, sizeof(char));
	strcpy(playerbuffer[index]->name, name);
	playerbuffer[index]->model = 0x98E29920;
	playerbuffer[index]->position[0] = 0;
	playerbuffer[index]->position[1] = 0;
	playerbuffer[index]->position[2] = 0;
	playerbuffer[index]->angle = 0;
	playerbuffer[index]->last_active = 0;
	playerbuffer[index]->sync_state = 0;
	for(unsigned char i = 0; i < 8; i++)
	{
		playerbuffer[index]->weapons[i] = 0;
		playerbuffer[index]->ammo[i] = 0;
	}
	playerbuffer[index]->animation[0] = 0;
	playerbuffer[index]->vehicleindex = -1;
	playerbuffer[index]->seatindex = -1;
	playerbuffer[index]->score = 0;
	playerbuffer[index]->health = 200;
	playerbuffer[index]->armor = 0;
	playerbuffer[index]->wanted_level = 0;
	playerbuffer[index]->edSprint = 1;
	playerbuffer[index]->edLockon = 1;
	playerbuffer[index]->edDoDriveBy = 1;
	playerbuffer[index]->edUseCover = 1;
	playerbuffer[index]->edConrol = 1;
	playerbuffer[index]->edFreeze = 0;
	playerbuffer[index]->isducking = 0;
	playerbuffer[index]->room = 0;
	playerbuffer[index]->car_enter = 0;
	playerbuffer[index]->color[0] = 0xFF;
	playerbuffer[index]->color[1] = 0xFF;
	playerbuffer[index]->color[2] = 0x00;
	playerbuffer[index]->color[3] = 0x00;
	playerbuffer[index]->currentweapon = 0;
	numplayers++;
	return true;
}

short PlayerManager::GetPlayerFreeSlot(void)
{
	short index;
	for (index = 0; index < playerbuffersize; index++)
	{
		if (playerbuffer[index] == NULL)
		{
			return index;
		}
	}
	if (playerbuffersize == maxplayerbuffersize)
	{
		return INVALID_PLAYER_INDEX;
	}
	return index;
}

bool PlayerManager::GetClassFreeSlot(unsigned char &index)
{
	for (index = 0; index < classbuffersize; index++)
	{
		if (classbuffer[index] == NULL)
		{
			return true;
		}
	}
	if (classbuffersize == maxclassbuffersize)
	{
		return false;
	}
	return true;
}