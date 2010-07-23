#pragma once
#include <vector>
#include "RakNet/RakPeerInterface.h"
#include "RakNet/RakNetworkFactory.h"
#include "RakNet/RakNetTypes.h"
#include "RakNet/MessageIdentifiers.h"

#define FMP_PACKET_SIGNATURE 0xFF

class CNetwork
{
public:
	CNetwork();
	~CNetwork();

	bool Load(const unsigned short iPort, const unsigned short iMaxPlayers);
	bool Unload();

	void Tick();
	bool IsReady();

	template <typename DATATYPE>
	void Send(const DATATYPE * pData, const short iType, const char PackPriority = 2);

	void CloseConnection(const SystemAddress addr);
	void AddToBanList(const SystemAddress addr, wchar_t * pszNick);
	void ClearBanList();
	void ReloadBanList();
	void LoadBanList();

private:
	RakPeerInterface * m_pNet;
	std::vector<wchar_t *> m_vBadNick;
};