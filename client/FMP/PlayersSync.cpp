#include <vector>

#include "log.h"
#include "Hook/types.h"
#include "Hook/classes.h"
//#include "Hook\hook.h"
#include "Hook/scripting.h"
#include "functions.h"
#include "structs.h"
#include "main.h"
#include "../Shared/ClientCore.h"
#include "../Shared/NetworkManager.h"

scrThread* GetActiveThread();
void SetActiveThread(scrThread* thread);

extern ClientCore client;
extern FMPHook HOOK;
extern NetworkManager nm;

void FMPHook::PlayerConnect(char *name, short index, unsigned int model, float position[3], bool start)
{
	Log::Debug("PlayerConnect: %s", "Start");
	if(name == 0)
	{
		Log::Error("Can't connecting player (Name is bad)");
		return;
	}
	if(index < 0 || index > MAX_PLAYERS)
	{
		Log::Error("Can't connecting player (bad index)");
		return;
	}

	Log::Info("ConnectInfo: %s %d 0x%x %f %f %f", name, index, model, position[0], position[1], position[2]);
	if(client.GetIndex() == index) // My connect
	{
		Log::Debug("Our PlayerConnect");
		if(start)
		{
			//Natives::SetCharHealth(_GetPlayerPed(), 0);
			nm.SendPlayerSpawnRequest();
			gPlayer[index].want_spawn = 1;
			gPlayer[index].sync_state = 1;

			strcpy(gPlayer[index].name, name);
			gPlayer[index].last_active = GetTickCount();
			gPlayer[index].connected = 1;

			client.SetGameState(GameStateInGame);
			client.SetInputState(InputStateGame);
			
			InputFreeze(0);
			//SetFreeCam(0);

			Log::Debug("PlayerConnect: %s", "fastEnd");
			return;
		}
		else
		{
			Log::Info("Local player %d", Natives::IsThisModelAPed((eModel)model));
			Natives::RequestModel((eModel)model);
			Log::Debug("PlayerConnect: %s", "RequestModel");
			while(!Natives::HasModelLoaded((eModel)model)) wait(1);
			Log::Debug("PlayerConnect: %s", "ModelLoaded");
			Natives::ChangePlayerModel(_GetPlayer(), (eModel)model);
			Log::Debug("PlayerConnect: %s", "ChangeModel");
			Natives::GetPlayerChar(_GetPlayer(), &gPlayer[index].PedID);
			Natives::SetCharCoordinates(gPlayer[index].PedID, position[0], position[1], position[2]);
			Log::Debug("PlayerConnect: %s", "SetCoords");
		}

		client.SetGameState(GameStateInGame);
		client.SetInputState(InputStateGame);
		
		InputFreeze(0);
		SetFreeCam(0);
	}
	else // Other player connect
	{
		Log::Debug("Other player connect");
		Natives::RequestModel((eModel)model);
		Log::Debug("PlayerConnect: %s", "RequestModel");
		while(!Natives::HasModelLoaded((eModel)model)) wait(1);
		Log::Debug("PlayerConnect: %s", "ModelLoaded");
		Natives::CreateChar(1, (eModel)model, position[0], position[1], position[2], &gPlayer[index].PedID, 1);
		Log::Debug("PlayerConnect: %s", "CreateChar");
		while(!Natives::DoesCharExist(gPlayer[index].PedID)) wait(1);
		Log::Info("MovePlayer %d(%d) = %d", index, Natives::DoesCharExist(gPlayer[index].PedID),gPlayer[index].PedID);
		Natives::GivePedFakeNetworkName(gPlayer[index].PedID, name, gPlayer[index].color[1],gPlayer[index].color[2],gPlayer[index].color[3],gPlayer[index].color[0]);
		Log::Info("Player NAME: %s 0x%x%x%x alpha:%x",name, gPlayer[index].color[1],gPlayer[index].color[2],gPlayer[index].color[3],gPlayer[index].color[0]);
		Natives::SetBlockingOfNonTemporaryEvents(gPlayer[index].PedID, 1);
	}

	Natives::SetCharDefaultComponentVariation( gPlayer[index].PedID );
	Natives::AddArmourToChar(gPlayer[index].PedID, gPlayer[index].armor);
	gPlayer[index].model = model;
	memcpy(gPlayer[index].position, position, sizeof(float) * 3);

	if(gPlayer[index].vehicleindex != -1)
	{
		if(gPlayer[index].seatindex == -1) Natives::TaskEnterCarAsDriver(gPlayer[index].PedID, gCar[gPlayer[index].vehicleindex].CarID, 0);
		else Natives::TaskEnterCarAsPassenger(gPlayer[index].PedID, gCar[gPlayer[index].vehicleindex].CarID, 0, gPlayer[index].seatindex);
	}

	for(int i = 0; i < 8; i++)
	{
		if(gPlayer[index].weapons[i] != 0 && gPlayer[index].ammo[i] > 0)
			Natives::GiveWeaponToChar(gPlayer[index].PedID, (eWeapon)gPlayer[index].weapons[i], gPlayer[index].ammo[i], 0);
	}

	strcpy(gPlayer[index].name, name);
	gPlayer[index].last_active = GetTickCount();
	gPlayer[index].connected = 1;
	Log::Debug("PlayerConnect: %s", "End");
}

void FMPHook::Jump(short index)
{
	if(!SafeCheckPlayer(index)) return;

	Log::Info("Jump %d(%d) = %d", index, Natives::DoesCharExist(gPlayer[index].PedID), gPlayer[index].PedID);
	Natives::TaskJump(gPlayer[index].PedID, 1);
	gPlayer[index].last_active = GetTickCount();
}

void FMPHook::Duck(short index, bool duck)
{
	if(!SafeCheckPlayer(index)) return;

	Log::Info("Duck %d(%d) = %d", index, Natives::DoesCharExist(gPlayer[index].PedID) ,gPlayer[index].PedID);
	Natives::TaskToggleDuck(gPlayer[index].PedID, duck);
	gPlayer[index].last_active = GetTickCount();
	gPlayer[index].isducking = duck;
}

void FMPHook::PlayerMove(short index, float position[3], float speed)
{
	Log::Info("PlayerMOVE HOOK");
	Log::Info("Index: %d, position: %f %f %f, speed: %f", index, position[0], position[1], position[2], speed);
	
	if(!SafeCheckPlayer(index)) return;

	float lx,ly,lz;
	Natives::GetCharCoordinates(gPlayer[index].PedID, &lx, &ly, &lz);

	float d = GetDist(lx, ly, lz, position[0], position[1], position[2]);
	if(gPlayer[index].vehicleindex == -1) // ���� ������
	{
		Log::Info("MovePlayer (HOOK) %s", "Start");
		int ms = 4;
		Log::Info("Before Move");
		if(speed < 3)
		{
			ms = 2;
			if(d > 10)
			{
				Natives::SetCharCoordinates(gPlayer[index].PedID, position[0], position[1], position[2]);
				Natives::SetCharHeading(gPlayer[index].PedID, gPlayer[index].angle);
				Log::Info("PORTAL or TIMESHIFT");
			}
		}
		else if(speed < 5)
		{
			ms = 3;
			if(d > 20)
			{
				Natives::SetCharCoordinates(gPlayer[index].PedID, position[0], position[1], position[2]);
				Natives::SetCharHeading(gPlayer[index].PedID, gPlayer[index].angle);
				Log::Info("PORTAL or TIMESHIFT");
			}
		}
		else
		{
			ms = 4;
			if(d > 30)
			{
				Natives::SetCharCoordinates(gPlayer[index].PedID, position[0], position[1], position[2]);
				Natives::SetCharHeading(gPlayer[index].PedID, gPlayer[index].angle);
				Log::Info("PORTAL or TIMESHIFT");
			}
		}

		Natives::TaskGoStraightToCoord(gPlayer[index].PedID, position[0], position[1], position[2], ms, 45000);
		wait(1);
		memcpy(gPlayer[index].position, position, sizeof(float) * 3);
	}
	else // ���� �� ������
	{
		if(!SafeCheckVehicle(gPlayer[index].vehicleindex)) return;

		int vect = 1;
		if(speed < 0) { vect = 2; speed = speed*-1; }
		if(speed * 3 < d)
		{
			Natives::SetCharCoordinates(gPlayer[index].PedID, position[0], position[1], position[2]);
			Natives::SetCarHeading(gCar[gPlayer[index].vehicleindex].CarID, gCar[gPlayer[index].vehicleindex].angle);
		}

		Natives::TaskCarDriveToCoord(gPlayer[index].PedID, gCar[gPlayer[index].vehicleindex].CarID, position[0], position[1], position[2], speed, vect, 2, 3, 2, 45000000);
	}
	gPlayer[index].last_active = GetTickCount();
}

void FMPHook::PlayerDisconnect(short index)
{
	if(index == client.GetIndex())
	{
		SafeCleanPlayers();
		SafeCleanVehicles();

		client.SetIndex(-1);

		client.SetGameState(GameStateOffline);
		client.SetInputState(InputStateGui);
		return;
	}

	if(!SafeCheckPlayer(index)) return;

	Log::Info("DELETE CHAR %d", index);
	Natives::DeleteChar(&gPlayer[index].PedID);
	gPlayer[index].connected = 0;
}

void FMPHook::CancelEnterInVehicle(short index)
{
	if(!SafeCheckPlayer(index)) return;

	Log::Info("CancelEnterInVehicle %d", index);
	Natives::ClearCharTasks(gPlayer[index].PedID);
	gPlayer[index].vehicleindex = -1;
}

void FMPHook::ExitFromVehicle(short index)
{
	if(!SafeCheckPlayer(index)) return;

	Log::Info("ExitFromVehicle %d", index);
	Natives::TaskLeaveAnyCar(gPlayer[index].PedID);
	gPlayer[index].vehicleindex = -1;
}

void FMPHook::EnterInVehicle(short index, int car, int seat)
{
	if(!SafeCheckPlayer(index)) return;

	Log::Info("EnterInVehicle %d, %d, %d", index, car, seat);
	if(seat == 0) Natives::TaskEnterCarAsDriver(gPlayer[index].PedID, gCar[car].CarID, -1);
	else if(seat > 0) Natives::TaskEnterCarAsPassenger(gPlayer[index].PedID, gCar[car].CarID, -1, seat-1);
	gPlayer[index].vehicleindex = car;
	gPlayer[index].seatindex = seat;
}

void FMPHook::PlayerFireAim(short index, int gun, int time, float x, float y, float z, bool fire)
{
	if(!SafeCheckPlayer(index)) return;

	if(fire)
	{
		Natives::SetCurrentCharWeapon(gPlayer[index].PedID, (eWeapon)gun, 1);
		Natives::TaskShootAtCoord(gPlayer[index].PedID, x, y, z, time, 5);
	}
	else
	{
		Natives::SetCurrentCharWeapon(gPlayer[index].PedID, (eWeapon)gun, 1);
		Natives::TaskAimGunAtCoord(gPlayer[index].PedID, x, y, z, time);
	}
}

void FMPHook::PlayerSwapGun(short index, int gun)
{
	if(!SafeCheckPlayer(index)) return;

	Natives::TaskSwapWeapon(gPlayer[index].PedID, (eWeapon)gun);
}

void FMPHook::SetPosition(short index, float pos[3], float angle)
{
	if(!SafeCheckPlayer(index)) return;

	memcpy(gPlayer[index].position, pos, 3 * sizeof(float));
	gPlayer[index].angle = angle;

	Natives::SetCharCoordinates(gPlayer[index].PedID, pos[0], pos[1], pos[2]);
	Natives::SetCharHeading(gPlayer[index].PedID, angle);
}

void FMPHook::PlayerSyncSkin(short index, unsigned int skin)
{
	if(!SafeCheckPlayer(index)) return;

	Log::Info("Player Sync Skin START");
	if(index == client.GetIndex())
	{
		SafeChangeModel(skin);
	}
	else
	{
		Natives::DeleteChar(&gPlayer[index].PedID);
		gPlayer[index].model = skin;
		SafeCreatePlayer(index);
	}
	Log::Info("Player Sync Skin END");
}

void FMPHook::PlayerSyncSkinVariation(short index, int sm[11], int st[11])
{
	if(!SafeCheckPlayer(index)) return;

	Log::Info("Player Sync Skin Variation START");
	for(int i = 0; i < 11; i++)
		Natives::SetCharComponentVariation(gPlayer[index].PedID, (ePedComponent)i, sm[i], st[i]); 
	Log::Info("Player Sync Skin Variantion END");
}

void FMPHook::xPlayerSpawn(NetworkPlayerSpawnData data)
{
	Log::Info("Spawn %d (%d)", data.client, client.GetIndex());
	if(data.client == client.GetIndex()) 
	{
		Log::Info("Its me");
		if(gPlayer[client.GetIndex()].sync_state == 1) SetFreeCam(0);
		gPlayer[client.GetIndex()].sync_state = 0;
		gPlayer[data.client].want_spawn = 1;
		return;
	}
	if(!SafeCheckPlayer(data.client)) 
	{
		gPlayer[data.client].want_spawn = 0;
		return;
	}
	Log::Info("Player Spawn START");

	Ped old = gPlayer[data.client].PedID;
	
	SafeRequestModel(data.model);
	Natives::CreateChar(1, (eModel)data.model, data.position[0], data.position[1], data.position[2], &gPlayer[data.client].PedID, 1);

	while(!Natives::DoesCharExist(gPlayer[data.client].PedID)) wait(0);

	Natives::SetCharDefaultComponentVariation(gPlayer[data.client].PedID);
	PlayerSyncSkinVariation(data.client, data.compD, data.compT);

	Log::Info("Set UP");
	Natives::SetCharHeading(gPlayer[data.client].PedID, data.angle);
	Natives::SetCharHealth(gPlayer[data.client].PedID, data.health);
	Natives::AddArmourToChar(gPlayer[data.client].PedID, data.armor);
	if(data.room != 0) Natives::SetRoomForCharByKey(gPlayer[data.client].PedID, (eInteriorRoomKey)data.room);
	Log::Info("Player SPAWN END");
	if(Natives::DoesCharExist(old)) Natives::DeleteChar(&old);
	gPlayer[data.client].want_spawn = 0;
}