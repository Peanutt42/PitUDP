#pragma once

#include "TSQueue.hpp"
#include "UDPNetworking.hpp"
#include "NetworkingMessageIds.hpp"
#include "InternalNetworkingMsgIds.hpp"

#include <unordered_map>
#include <memory>

namespace Pit::Networking {
#define SERVER_SEND_ID ((size_t)99999999999)

	struct ClientInfo {
		std::string Ip;
		unsigned short Port;
	};

	class Server {
	public:
		virtual ~Server() = default;

		Server(unsigned short port) : m_Port(port) {}

		void Bind() {
			m_Socket.Bind(m_Port);
		}

		void Send(size_t clientId, const Message& msg) {
			auto& clientInfo = m_Clients.at(clientId);
			m_Socket.SendMsg(clientInfo.Ip, clientInfo.Port, SERVER_SEND_ID, msg);
		}

		void SendAll(const Message& msg, const std::vector<size_t> excludeIds = {}) {
			for (auto& [clientId, clientInfo] : m_Clients) {
				bool excluded = false;
				for (auto& excludeId : excludeIds) {
					if (clientId == excludeId) {
						excluded = true;
						break;
					}
				}
				if (!excluded)
					Send(clientId, msg);
			}
		}

		bool GetNextMessage(RecievedMessage* outMsg) {
			auto& recievedMsgs = m_Socket.GetRecievedMessages();
			if (!recievedMsgs.empty()) {
				*outMsg = recievedMsgs.pop_front();
				if (outMsg->msg.Sender == SERVER_SEND_ID) return false;

				if (outMsg->msg.Id == (size_t)InternalClientToServerMsgId::ConnectRequest) {
					bool acceptClient = OnClientConnectionRequest(outMsg->ipAddress);
					size_t clientId = acceptClient ? m_ClientIdCounter++ : 0;
					Message connectRequestResponse((size_t)InternalServerToClientMsgId::ConnectionResponse);
					connectRequestResponse << acceptClient;
					connectRequestResponse << clientId;
					m_Socket.SendMsg(outMsg->ipAddress, outMsg->port, SERVER_SEND_ID, connectRequestResponse);
					if (acceptClient) {
						m_Clients[clientId] = { outMsg->ipAddress, outMsg->port };
						std::cout << "Accepted client " << clientId << '\n';
					}
					return false;
				}
				return true;
			}
			return false;
		}

	protected:
		virtual bool OnClientConnectionRequest(const std::string& ip) {
			return true; // do some kind of blacklist checking...
		}
		virtual void OnClientConnected(const std::string& ip) {}
		virtual void OnClientDisconnected(const std::string& ip) {}

	protected:
		unsigned short m_Port;
		size_t m_ClientIdCounter = 1;
		std::unordered_map<size_t, ClientInfo> m_Clients;
		UDPSocket m_Socket;
	};
}