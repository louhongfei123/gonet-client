#include "Account.h"
#include "Base/bitStream.h"
#include "WinTcp/ClientSocket.h"
#include "Base/MemGuard.h"
#include "message/client.pb.h"

using namespace WinTcp;
using namespace message;
#if defined(ANDROID)
#include<netinet/in.h>
#endif

WinTcp::Account::Account() : m_AccountId(0), m_PlayerId(0)
{
	REGISTER_PACKET(new W_C_SelectPlayerResponse(), std::bind(&Account::_W_C_SelectPlayerResponse, this, std::placeholders::_1));
	REGISTER_PACKET(new W_C_CreatePlayerResponse(), std::bind(&Account::_W_C_CreatePlayerResponse, this, std::placeholders::_1));
	REGISTER_PACKET(new A_C_LoginRequest(), std::bind(&Account::_A_C_LoginRequest, this, std::placeholders::_1));
	REGISTER_PACKET(new A_C_RegisterResponse(), std::bind(&Account::_A_C_RegisterResponse, this, std::placeholders::_1));
}

WinTcp::Account* WinTcp::Account::Instance()
{
	static Account s_Instace;
	return &s_Instace;
}


bool Account::_W_C_SelectPlayerResponse(::google::protobuf::Message* _packet) {
	auto packet = (W_C_SelectPlayerResponse*)(_packet);
	if (!packet) {
		return false;
	}

	m_AccountId = packet->accountid();
	auto nLen = packet->playerdata_size();
	if (nLen == 0) {
		auto packet = new C_W_CreatePlayerRequest();
		auto packetHead = packet->mutable_packethead();
		Packet::BuildPacketHead(packetHead, m_AccountId);
		packet->set_playername("1111");
		packet->set_sex(0);
		CLIENT_TCP->Send(packet);
	}
	else {
		m_PlayerId = packet->playerdata(0).playerid();
		LoginGame();
	}

	return true;
}

bool Account::_W_C_CreatePlayerResponse(::google::protobuf::Message* _packet) {
	auto packet = (W_C_CreatePlayerResponse*)(_packet);
	if (!packet) {
		return false;
	}
	if (packet->error() == 0) {
		m_PlayerId = packet->playerid();
		LoginGame();
	}
	else {//����ʧ��
	}

	return true;
}

bool Account::_A_C_LoginRequest(::google::protobuf::Message* _packet) {
	auto packet = (A_C_LoginRequest*)(_packet);
	if (!packet) {
		return false;
	}
	if (packet->error() == ACCOUNT_NOEXIST) {
		auto packet = new C_A_RegisterRequest();
		auto packetHead = packet->mutable_packethead();
		Packet::BuildPacketHead(packetHead, m_AccountId, ACCOUNTSERVER);
		packet->set_accountname("test112");
		packet->set_socketid(0);
		CLIENT_TCP->Send(packet);;
	}

	return true;
}

bool Account::_A_C_RegisterResponse(::google::protobuf::Message* _packet) {
	auto packet = (A_C_RegisterResponse*)(_packet);
	if (!packet) {
		return false;
	}
	if (packet->error() != 0) {
		//ע��ʧ��
	}
	return true;
};


bool Account::LoginGame()
{
	auto packet = new C_W_Game_LoginRequset();
	auto packetHead = packet->mutable_packethead();
	Packet::BuildPacketHead(packetHead, this->m_AccountId);
	packet->set_playerid(this->m_PlayerId);
	return ClientSocket::Instance()->Send(packet);
}

bool Account::LoginAccount() {
	auto packet = new C_A_LoginRequest();
	auto packetHead = packet->mutable_packethead();
	Packet::BuildPacketHead(packetHead, 0, ACCOUNTSERVER);
	packet->set_accountname("test112");
	packet->set_buildno(BUILD_NO);
	packet->set_socketid(0);
	return ClientSocket::Instance()->Send(packet);
}