#pragma once

#include "TSQueue.hpp"
#include "UDPNetworking.hpp"
#include "InternalNetworkingMsgIds.hpp"

namespace Pit::Networking {
	class Client {
	public:
		virtual ~Client() = default;

		Client(const std::string& serverIp, unsigned short port)
			: m_ServerIp(serverIp), m_Port(port) {}

		void Connect() {
			Message connectMsg((size_t)InternalClientToServerMsgId::Connect);
			Send(connectMsg);
		}

		void Disconnect() {
			Message disconnectMsg((size_t)InternalClientToServerMsgId::Disconnect);
			Send(disconnectMsg);
			m_ServerIp = "empty";
			m_Port = 0;
			m_Socket.Close();
			m_Id = 0;
		}

		void Send(const Message& msg) {
			m_Socket.SendMsg(m_ServerIp, m_Port, m_Id, msg);
		}

		bool GetNextMessage(RecievedMessage* outMsg) {
			auto& recievedMsgs = m_Socket.GetRecievedMessages();
			if (!recievedMsgs.empty()) {
				*outMsg = recievedMsgs.pop_front();
				if (outMsg->msg.Id == (size_t)InternalServerToClientMsgId::ConnectResponse) {
					outMsg->msg >> m_Id;
					return false;
				}
				if (outMsg->msg.Id == (size_t)InternalServerToClientMsgId::Disconnect) {
					OnServerDisconnected();
					m_Socket.Close();
					return false;
				}
				return true;
			}
			return false;
		}

	protected:
		virtual void OnServerDisconnected() {
			std::cout << "Server disconnected!\n";
		}

	protected:
		std::string m_ServerIp;
		unsigned short m_Port = 0;
		size_t m_Id = 0;
		UDPSocket m_Socket;
	};
}