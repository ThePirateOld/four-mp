/// \file
/// \brief Header file that describes the NetworkManager class.
/// \details See class description.
/// \author WNeZRoS, FaTony

#pragma once

#include "NetworkIDObject.h"

#include "../../Shared/Network/GtaEnums.h"
#include "../../Shared/Network/NetworkProtocol.h"

#ifdef WIN32
#include <windows.h>
#endif

namespace RakNet
{
	class RPC3;
}

/// \brief A network manager. It handles all network traffic.
/// \details TODO:
/// \author WNeZRoS, FaTony

class NetworkManager : public NetworkIDObject
{
public:
	NetworkManager(void);
	~NetworkManager(void);
	void Load(short maxclients, unsigned short port);
	void Tick(void);
	void Unload(void);
	void CheckClients(void);
	void UpdateServerInfo(void);

	void RecieveClientConnectionRequest(NetworkPlayerConnectionRequestData data, RakNet::RPC3 *clientrpc3);
	void RecieveClientConnectionConfirmation(NetworkPlayerConnectionConfirmationData data, RakNet::RPC3 *clientrpc3);
	void RecievePlayerFootSync(NetworkPlayerFootData data, RakNet::RPC3 *serverrpc3);
	void RecievePlayerVehicleSync(NetworkPlayerVehicleData data, RakNet::RPC3 *serverrpc3);
	void RecievePlayerStartEntranceInVehicle(NetworkPlayerStartEntranceInVehicleData data, RakNet::RPC3 *clientrpc3);
	void RecievePlayerCancelEntranceInVehicle(NetworkPlayerCancelEntranceInVehicleData data, RakNet::RPC3 *clientrpc3);
	void RecievePlayerFinishEntranceInVehicle(NetworkPlayerFinishEntranceInVehicleData data, RakNet::RPC3 *clientrpc3);
	void RecievePlayerStartExitFromVehicle(NetworkPlayerStartExitFromVehicleData data, RakNet::RPC3 *clientrpc3);
	void RecievePlayerFinishExitFromVehicle(NetworkPlayerFinishExitFromVehicleData data, RakNet::RPC3 *clientrpc3);
	void RecievePlayerSpawnRequest(NetworkPlayerSpawnRequestData data, RakNet::RPC3 *clientrpc3);
	void RecievePlayerModelChange(NetworkPlayerModelChangeData data, RakNet::RPC3 *clientrpc3);
	void RecievePlayerComponentsChange(NetworkPlayerComponentsChangeData data, RakNet::RPC3 *clientrpc3);
	void RecievePlayerChat(NetworkPlayerChatData data, RakNet::RPC3 *clientrpc3);

	bool SendGameTimeChangeToAll(const unsigned char time[2]);
	bool SendPlayerModelChangeToAll(const short index);
	bool SendPlayerWeaponGiftToAll(const short index, const eWeaponSlot slot);
	bool SendPlayerSpawnPositionChange(const short index);
	bool SendNewVehicleInfoToAll(const short index);
	bool SendPlayerPosition(const short index, const float pos[3], const float angle);
private:
	unsigned short serverport;
	NetworkIDManager *manager;
	NetworkID serverid;
	NetworkID defaultclientid;
	RakNet::RPC3 *rpc3;
	RakPeerInterface *net;
	struct Client
	{
		SystemAddress address;
		NetworkID id;
	};
	short maxclientbuffersize;
	short clientbuffersize;
	Client **clientbuffer;
	enum NetworkRPCType
	{
		NetworkRPCPlayerConnectionRequest,
		NetworkRPCPlayerConnectionConfirmation,
		NetworkRPCPlayerFootSync,
		NetworkRPCPlayerVehicleSync,
		NetworkRPCPlayerStartEntranceInVehicle,
		NetworkRPCPlayerCancelEntranceInVehicle,
		NetworkRPCPlayerFinishEntranceInVehicle,
		NetworkRPCPlayerStartExitFromVehicle,
		NetworkRPCPlayerFinishExitFromVehicle,
		NetworkRPCPlayerSpawnRequest,
		NetworkRPCPlayerModelChange,
		NetworkRPCPlayerComponentsChange,
		NetworkRPCPlayerChat
	};
	struct NetworkPlayerConnectionRequestDataInternal
	{
		SystemAddress address;
		wchar_t name[MAX_CLIENT_NAME_LENGTH];
		unsigned int sessionkey;
	};
	union NetworkRPCUnion
	{
		NetworkPlayerConnectionRequestDataInternal *playerconnectionrequest;
		NetworkPlayerConnectionConfirmationData *playerconnectionconfirmation;
		NetworkPlayerFootData *playerfoot;
		NetworkPlayerVehicleData *playervehicle;
		NetworkPlayerStartEntranceInVehicleData *playerstartentranceinvehicle;
		NetworkPlayerCancelEntranceInVehicleData *playercancelentranceinvehicle;
		NetworkPlayerFinishEntranceInVehicleData *playerfinishentranceinvehicle;
		NetworkPlayerStartExitFromVehicleData *playerstartexitfromvehicle;
		NetworkPlayerFinishExitFromVehicleData *playerfinishexitfromvehicle;
		NetworkPlayerSpawnRequestData *playerspawnrequest;
		NetworkPlayerModelChangeData *playermodelchange;
		NetworkPlayerComponentsChangeData *playercomponentschange;
		NetworkPlayerChatData *playerchat;
	};
	struct NetworkRPCData
	{
		NetworkRPCType type;
		NetworkRPCUnion data;
	};
	int maxrpcbuffersize;
	int rpcbuffersize;
	NetworkRPCData *rpcbuffer;
#ifdef WIN32
	CRITICAL_SECTION rpcbuffersection;
#endif
	template <typename DATATYPE>
	void WriteToRPCBuffer(const NetworkRPCType type, const DATATYPE *data);
	void HandleRPCData(const NetworkRPCType type, const NetworkRPCUnion *data);
	void FreeRPCBuffer(void);
	//TODO: Make this handlers work on raw arguments instead of structs.
	void HandleClientConnectionRequest(NetworkPlayerConnectionRequestDataInternal *data); //TODO: Should be const.
	void HandleClientConnectionConfirmation(const NetworkPlayerConnectionConfirmationData *data);
	void HandlePlayerFootSync(const NetworkPlayerFootData *data);
	void HandlePlayerVehicleSync(const NetworkPlayerVehicleData *data);
	void HandlePlayerStartEntranceInVehicle(const NetworkPlayerStartEntranceInVehicleData *data);
	void HandlePlayerCancelEntranceInVehicle(const NetworkPlayerCancelEntranceInVehicleData *data);
	void HandlePlayerFinishEntranceInVehicle(const NetworkPlayerFinishEntranceInVehicleData *data);
	void HandlePlayerStartExitFromVehicle(const NetworkPlayerStartExitFromVehicleData *data);
	void HandlePlayerFinishExitFromVehicle(const NetworkPlayerFinishExitFromVehicleData *data);
	void HandlePlayerSpawnRequest(const NetworkPlayerSpawnRequestData *data);
	void HandlePlayerModelChange(const NetworkPlayerModelChangeData *data);
	void HandlePlayerComponentsChange(const NetworkPlayerComponentsChangeData *data);
	void HandlePlayerChat(NetworkPlayerChatData *data); //TODO: Should be const.
	short RegisterNewClient(const SystemAddress address);
	void HandleClientDisconnection(const SystemAddress address);
	short GetClientIndex(const SystemAddress address);
	short GetClientFreeSlot(void);
	void SendConnectionError(const SystemAddress address, const NetworkPlayerConnectionError error);
	NetworkPlayerFullUpdateData *GetPlayerFullUpdateData(const short index);
	NetworkVehicleFullUpdateData *GetVehicleFullUpdateData(const short index);
	template <typename DATATYPE>
	void SendDataToAll(const char *RPC, const DATATYPE *data);
	template <typename DATATYPE>
	bool SendDataToOne(const char *RPC, const short index, const DATATYPE *data);
	template <typename DATATYPE>
	void SendDataToAllExceptOne(const char *RPC, const short index, const DATATYPE *data);
	//void SendClassInfo(const short client);
};
