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
			m_Id = 0;
		}

		void Send(const Message& msg) {
			if (m_Accepted)
				m_Socket.SendMsg(m_ServerIp, m_Port, m_Id, msg);
			else {
				static size_t tryCount = 0;
				tryCount++;
				if (tryCount > 10) {
					Connect();
					tryCount = 0;
				}
			}
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
					m_Socket.SendMsg(m_ServerIp, m_Port, m_Id, answerMsg);
					return false;
				}
				if (outMsg->msg.Id == (size_t)InternalServerToClientMsgId::ConnectionResponse) {
					outMsg->msg >> m_Accepted;
					if (m_Accepted)
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
		bool m_Accepted = false;
		UDPSocket m_Socket;
		SecureFunctionSignature m_SecureFunction;
	};
}