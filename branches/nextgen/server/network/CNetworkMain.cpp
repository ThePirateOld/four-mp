#include <stdio.h>
#include "CNetwork.h"
#include "../main.h"
#include "log/log.h"

CNetwork::CNetwork()
{
	m_pNet = NULL;
}

CNetwork::~CNetwork()
{
	this->Unload();
}

bool CNetwork::Load(const unsigned short iPort, const unsigned short iMaxPlayers)
{
	m_pNet = RakNetworkFactory::GetRakPeerInterface();
	SocketDescriptor s(iPort, 0);
	m_pNet->SetMaximumIncomingConnections(iMaxPlayers);
	m_pNet->Startup(iMaxPlayers, 1, &s, 1);

	this->LoadBanList();
	return true;
}

bool CNetwork::Unload()
{
	if (m_pNet)
	{
		m_pNet->Shutdown(100,0);
		RakNetworkFactory::DestroyRakPeerInterface(m_pNet);
	}
	return true;
}

bool CNetwork::IsReady()
{
	return this->m_pNet != NULL;
}

template <typename DATATYPE>
void CNetwork::Send(const DATATYPE * pData, const short iType, const char PackPriority)
{
	int iSize = sizeof(DATATYPE);
	if(iSize == 4) Log::Warning("Send %d: sizeof(pData) == 4", iType);

    char * pszData = new char[iSize + 3];
    pszData[0] = (char)FMP_PACKET_SIGNATURE;
    *(short*)(pszData + 1) = iType;
    memcpy(pszData + 3, pData, iSize);

	m_pNet->Send(pszData, iSize + 3, (PacketPriority)PackPriority, RELIABLE_ORDERED, NULL, m_ServerAddress, false);

	delete pszData;
}

void CNetwork::Tick()
{
	static Packet * pPack;
	for (pPack = m_pNet->Receive(); pPack; m_pNet->DeallocatePacket(pPack), pPack = m_pNet->Receive())
	{
		switch (pPack->data[0])
		{
		case FMP_PACKET_SIGNATURE:
			{
				//this->HandlePacket(pack->data + 1, pack->length - 1, pack->systemAddress);
			} break;
		case ID_CONNECTION_REQUEST:
			{
				Log::Debug(L"New connection request");
			} break;
		case ID_NEW_INCOMING_CONNECTION:
			{
				Log::Debug(L"New connection from %S", pPack->systemAddress.ToString(1));
			} break;
		case ID_DISCONNECTION_NOTIFICATION:
			{
				//this->HandleClientDisconnection(pack->systemAddress);
			} break;
		case ID_CONNECTION_LOST:
			{
				//this->HandleClientDisconnection(pack->systemAddress);
			} break;
		}
	}
}