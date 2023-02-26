#pragma once

#include "TSQueue.hpp"
#include "UDPNetworking.hpp"
#include "InternalNetworkingMsgIds.hpp"

namespace Pit::Networking {
	class Client {
	public:
		virtual ~Client() = default;

		Client(const std::string& serverIp, unsigned short port, SecureFunctionSignature secureFunction)
			: m_ServerIp(serverIp), m_Port(port), m_SecureFunction(secureFunction) {}

		void Connect() {
			Message connectRequestMsg((size_t)InternalClientToServerMsgId::ConnectRequest);
			m_Socket.SendMsg(m_ServerIp, m_Port, m_Id, connectRequestMsg);
		}

		void Disconnect() {
			Message disconnectMsg((size_t)InternalClientToServerMsgId::Disconnect);
			Send(disconnectMsg);
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
				if (outMsg->msg.Id == (size_t)InternalServerToClientMsgId::ConnectQuestion) {
					size_t questionInputA = 0, questionInputB = 0;
					outMsg->msg >> questionInputB >> questionInputA;
					outMsg->msg >> m_Id;
					size_t answer = m_SecureFunction(questionInputA, questionInputB);
					Message answerMsg((size_t)InternalClientToServerMsgId::ConnectRequestAnswer);
					answerMsg << answer;
					Send(answerMsg);
					return false;
				}
				if (outMsg->msg.Id == (size_t)InternalServerToClientMsgId::ConnectionResponse) {
					bool accepted = false;
					outMsg->msg >> accepted;
					if (accepted)
						OnServerAccept();
					else
						OnServerRejected();

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
		virtual void OnServerAccept() {
			std::cout << "Got accepted by server!\n";
		}
		virtual void OnServerRejected() {
			std::cout << "Got rejected by server!\n";
		}
		virtual void OnServerDisconnected() {
			std::cout << "Server disconnected!\n";
		}

	protected:
		std::string m_ServerIp;
		unsigned short m_Port = 0;
		size_t m_Id = 0;
		UDPSocket m_Socket;
		SecureFunctionSignature m_SecureFunction;
	};
}