#pragma once

#include "TSQueue.hpp"
#include "UDPNetworking.hpp"
#include "NetworkingMessageIds.hpp"
#include "InternalNetworkingMsgIds.hpp"

namespace Pit::Networking {
	class Client {
	public:
		virtual ~Client() = default;

		Client(const std::string& serverIp, unsigned short port)
			: m_ServerIp(serverIp), m_Port(port) {
			
			Message connectRequestMsg((size_t)InternalClientToServerMsgId::ConnectRequest);
			m_Socket.SendMsg(m_ServerIp, m_Port, m_Id, connectRequestMsg);
		}

		void Disconnect() {
			m_ServerIp = "empty";
			m_Port = 0;
			m_Socket.Close();
		}

		void Send(const Message& msg) {
			m_Socket.SendMsg(m_ServerIp, m_Port, m_Id, msg);
		}

		bool GetNextMessage(RecievedMessage* outMsg) {
			auto& recievedMsgs = m_Socket.GetRecievedMessages();
			if (!recievedMsgs.empty()) {
				*outMsg = recievedMsgs.pop_front();
				if (outMsg->msg.Id == (size_t)InternalServerToClientMsgId::ConnectionResponse) {
					bool accepted = false;
					outMsg->msg >> m_Id >> accepted;
					if (accepted)
						std::cout << "Got accepted by server!\n";
					else
						std::cout << "Got rejected by server!\n";

					return false;
				}
				return true;
			}
			return false;
		}

	protected:
		std::string m_ServerIp;
		unsigned short m_Port = 0;
		size_t m_Id = 0;
		UDPSocket m_Socket;
	};
}