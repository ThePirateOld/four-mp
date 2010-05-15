#include <vector>

#include "log.h"
#include "Hook\types.h"
#include "Hook\classes.h"
//#include "Hook\hook.h"
#include "Hook\scripting.h"
#include "functions.h"
#include "structs.h"
#include "main.h"
#include "hook\classes.h"

scrThread* GetActiveThread();
void SetActiveThread(scrThread* thread);

extern FMPHook HOOK;
extern short MyID;

void FMPHook::PlayerConnect(char *name, short index, unsigned int model, float x, float y, float z)
{
	Debug("PlayerConnect: %s", "Start");
	Log("ConnectInfo: %s %d 0x%x %f %f %f", name, index, model, x, y, z);
	if(MyID == index) // My connect
	{
		//gPlayer[index].model = model;
		/*Log("Local player %d", IsThisModelAPed((eModel)model));
		RequestModel((eModel)model);
		Debug("PlayerConnect: %s", "RequestModel");
		while(!HasModelLoaded((eModel)model)) Sleep(1);
		Debug("PlayerConnect: %s", "ModelLoaded");
		ChangePlayerModel(_GetPlayer(), (eModel)model);
		Debug("PlayerConnect: %s", "ChangeModel");*/
		//GetPlayerChar(_GetPlayer(), &gPlayer[index].PedID);
		/*SetCharDefaultComponentVariation(gPlayer[index].PedID());
		SetCharCoordinates(gPlayer[index].PedID(), x, y, z);
		Debug("PlayerConnect: %s", "SetCoords");*/
		clientstate.game = GameStateInGame;
		clientstate.input = InputStateGame;
		HOOK.InputFreeze(0);
	}
	else // Other player connect
	{
		/*RequestModel((eModel)model);
		Debug("PlayerConnect: %s", "RequestModel");
		while(!HasModelLoaded((eModel)model)) wait(1);
		Debug("PlayerConnect: %s", "ModelLoaded");
		CreateChar(1, (eModel)model, x, y, z, &gPlayer[index].iPedID, 1);
		while(DoesCharExist(gPlayer[index].PedID())) wait(1);
		SetCharDefaultComponentVariation(gPlayer[index].PedID());
		Log("MovePlayer %d(%d) = %d", index,DoesCharExist(gPlayer[index].PedID()),gPlayer[index].PedID());
		Debug("PlayerConnect: %s", "CreateChar");
		GivePedFakeNetworkName(gPlayer[index].PedID(), name, gPlayer[index].color[1],gPlayer[index].color[2],gPlayer[index].color[3],gPlayer[index].color[0]);
		Log("Player NAME: %s 0x%x%x%x alpha:%x",name, gPlayer[index].color[1],gPlayer[index].color[2],gPlayer[index].color[3],gPlayer[index].color[0]);*/
	}
	//AddArmourToChar(gPlayer[index].PedID(), gPlayer[index].armour);
	/*gPlayer[index].model = model;
	gPlayer[index].position[0] = x;
	gPlayer[index].position[1] = y;
	gPlayer[index].position[2] = z;

	if(gPlayer[index].vehicleindex != -1)
	{
		if(gPlayer[index].seat_id == -1) TaskEnterCarAsDriver(gPlayer[index].PedID(), gCar[gPlayer[index].vehicleindex].CarID, 0);
		else TaskEnterCarAsPassenger(gPlayer[index].PedID(), gCar[gPlayer[index].vehicleindex].CarID, 0, gPlayer[index].seat_id);
	}

	for(int i = 0; i < 8; i++)
	{
		if(gPlayer[index].weapons[i] != 0 && gPlayer[index].ammo[i] > 0)
			GiveWeaponToChar(gPlayer[index].PedID(), (eWeapon)gPlayer[index].weapons[i], gPlayer[index].ammo[i], 0);
	}*/

	strcpy(gPlayer[index].name, name);
	gPlayer[index].last_active = GetTickCount();
	gPlayer[index].connected = 1;
	Debug("PlayerConnect: %s", "End");
}

void FMPHook::ReCreatePlayer(int player)
{
	Natives::RequestModel((eModel)gPlayer[player].model);
	Log("REPlayerConnect: %s", "RequestModel");
	while(!Natives::HasModelLoaded((eModel)gPlayer[player].model)) wait(1);
	Log("REPlayerConnect: %s", "ModelLoaded");
	Natives::CreateChar(1, (eModel)gPlayer[player].model, gPlayer[player].position[0], gPlayer[player].position[1], gPlayer[player].position[2], &gPlayer[player].PedID, 1);
	wait(1);
	Log("REMovePlayer %d(%d) = %d", player, Natives::DoesCharExist(gPlayer[player].PedID),gPlayer[player].PedID);
	Log("REPlayerConnect: %s", "CreateChar");
}

void FMPHook::Jump(int player)
{
	if(gPlayer[player].connected == 0) return;
	Log("Jump %d(%d) = %d", player,Natives::DoesCharExist(gPlayer[player].PedID),gPlayer[player].PedID);
	Natives::TaskJump(gPlayer[player].PedID, 1);
	gPlayer[player].last_active = GetTickCount();
}

void FMPHook::Duck(int player, bool duck)
{
	if(gPlayer[player].connected == 0) return;
	Log("Duck %d(%d) = %d", player,Natives::DoesCharExist(gPlayer[player].PedID),gPlayer[player].PedID);
	Natives::TaskToggleDuck(gPlayer[player].PedID, duck);
	gPlayer[player].last_active = GetTickCount();
	gPlayer[player].isducking = duck;
}

void FMPHook::PlayerMove(int player, float x, float y, float z, float speed)
{
	Log("PlayerMOVE HOOK");
	if(gPlayer[player].connected == 0) return;
	float lx,ly,lz;
	Natives::GetCharCoordinates(gPlayer[player].PedID, &lx, &ly, &lz);
	Log("GETCOORD");
	float d = GetDist(lx, ly, lz, x, y, z);
	if(gPlayer[player].vehicleindex == -1) // ���� ������
	{
		Log("MovePlayer (HOOK) %s", "Start");
		int ms = 4;
		Log("Before Move");
		if(speed < 3)
		{
			ms = 2;
			if(d > 10)
			{
				Natives::SetCharCoordinates(gPlayer[player].PedID, x, y, z);
				Natives::SetCharHeading(gPlayer[player].PedID, gPlayer[player].angle);
				Log("PORTAL or TIMESHIFT");
			}
		}
		else if(speed < 5)
		{
			ms = 3;
			if(d > 20)
			{
				Natives::SetCharCoordinates(gPlayer[player].PedID, x, y, z);
				Natives::SetCharHeading(gPlayer[player].PedID, gPlayer[player].angle);
				Log("PORTAL or TIMESHIFT");
			}
		}
		else
		{
			ms = 4;
			if(d > 30)
			{
				Natives::SetCharCoordinates(gPlayer[player].PedID, x, y, z);
				Natives::SetCharHeading(gPlayer[player].PedID, gPlayer[player].angle);
				Log("PORTAL or TIMESHIFT");
			}
		}
		Log("MOVE %f=(%f) %d", speed, d, ms);
		Natives::TaskGoStraightToCoord(gPlayer[player].PedID, x, y, z, ms, 45000);
		wait(1);
		gPlayer[player].position[0] = x;
		gPlayer[player].position[1] = y;
		gPlayer[player].position[2] = z;
	}
	else // ���� �� ������
	{
		int vect = 1;
		if(speed < 0) { vect = 2; speed = speed*-1; }
		if(speed * 3 < d)
		{
			Natives::SetCharCoordinates(gPlayer[player].PedID, x, y, z);
			Natives::SetCarHeading(gCar[gPlayer[player].vehicleindex].CarID, gCar[gPlayer[player].vehicleindex].angle);
		}

		Natives::TaskCarDriveToCoord(gPlayer[player].PedID, gCar[gPlayer[player].vehicleindex].CarID, x, y, z, speed, vect, 2, 3, 2, 45000000);
	}
	gPlayer[player].last_active = GetTickCount();
}

void FMPHook::PlayerDisconnect(int id)
{
	Log("DELETE CHAR %d", id);
	Natives::DeleteChar(&gPlayer[id].PedID);
	gPlayer[id].connected = 0;
}

void FMPHook::CancelEnterInVehicle(int id)
{
	Log("CancelEnterInVehicle %d", id);
	Natives::ClearCharTasks(gPlayer[id].PedID);
	gPlayer[id].vehicleindex = -1;
}

void FMPHook::ExitFromVehicle(int id)
{
	Log("ExitFromVehicle %d", id);
	Natives::TaskLeaveAnyCar(gPlayer[id].PedID);
	gPlayer[id].vehicleindex = -1;
}

void FMPHook::EnterInVehicle(int id, int car, int seat)
{
	Log("EnterInVehicle %d, %d, %d", id, car, seat);
	if(seat == 0) Natives::TaskEnterCarAsDriver(gPlayer[id].PedID, gCar[car].CarID, -1);
	else Natives::TaskEnterCarAsPassenger(gPlayer[id].PedID, gCar[car].CarID, -1, seat);
	gPlayer[id].vehicleindex = car;
	gPlayer[id].seatindex = seat;
}

void FMPHook::PlayerFireAim(int playerid, int gun, int time, float x, float y, float z, bool fire)
{
	if(fire)
	{
		Natives::SetCurrentCharWeapon(gPlayer[playerid].PedID, (eWeapon)gun, 1);
		Natives::TaskShootAtCoord(gPlayer[playerid].PedID, x, y, z, time, 5);
	}
	else
	{
		Natives::SetCurrentCharWeapon(gPlayer[playerid].PedID, (eWeapon)gun, 1);
		Natives::TaskAimGunAtCoord(gPlayer[playerid].PedID, x, y, z, time);
	}
}

void FMPHook::PlayerSwapGun(int player, int gun)
{
	Natives::TaskSwapWeapon(gPlayer[player].PedID, (eWeapon)gun);
}

void FMPHook::PlayerSyncSkin(int player, int skin)
{
	Log("Player Sync Skin START");
	//Totally bug here.
	if(player == MyID)
	{
		Natives::RequestModel((eModel)skin);
		while(!Natives::HasModelLoaded((eModel)skin)) wait(0);
		Natives::ChangePlayerModel(_GetPlayer(), (eModel)skin);
	}
	else
	{
		Natives::RequestModel((eModel)skin);
		while(!Natives::HasModelLoaded((eModel)skin)) wait(0);
		Natives::ChangePlayerModel(_GetPlayer(), (eModel)skin);
	}
	Log("Player Sync Skin END");
}

void FMPHook::PlayerSyncSkinVariation(int player, int sm[11], int st[11])
{
	Log("Player Sync Skin Variation START");
	for(int i = 0; i < 11; i++)
		Natives::SetCharComponentVariation(gPlayer[player].PedID, (ePedComponent)i, sm[i], st[i]); 
	Log("Player Sync Skin Variantion END");
}

void FMPHook::xPlayerSpawn(NetworkPlayerSpawnData data)
{
	Log("Player Spawn START");
	unsigned int model;
	Natives::GetCharModel(gPlayer[data.client].PedID, (eModel*)&model);
	Log("Get Model");
	if(data.model != model) PlayerSyncSkin(data.client, data.model);
	Log("IF SPAWN MODEL");

	PlayerSyncSkinVariation(data.client, data.compD, data.compT);

	Log("Set UP");
	Natives::SetCharCoordinates(gPlayer[data.client].PedID, data.position[0], data.position[1], data.position[2]);
	Natives::SetCharHeading(gPlayer[data.client].PedID, data.angle);
	Natives::SetCharHealth(gPlayer[data.client].PedID, data.health);
	Natives::AddArmourToChar(gPlayer[data.client].PedID, -1000);
	Natives::AddArmourToChar(gPlayer[data.client].PedID, data.armor);
	Natives::SetRoomForCharByKey(gPlayer[data.client].PedID, (eInteriorRoomKey)data.room);
	Log("Player SPAWN END");
}