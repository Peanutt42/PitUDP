#pragma once

#include "TSQueue.hpp"
#include "UDPNetworking.hpp"
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
		Server(unsigned short port)
			: m_Port(port) {}

		virtual ~Server() {
			Close();
		}

		void Bind() {
			m_Socket.Bind(m_Port);
		}

		void Close() {
			Message disconnectMsg((size_t)InternalServerToClientMsgId::Disconnect);
			SendAll(disconnectMsg);
			m_Socket.Close();
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

				if (outMsg->msg.Id == (size_t)InternalClientToServerMsgId::Connect) {
					bool acceptClient = OnClientConnectionRequest(outMsg->ipAddress);
					if (acceptClient) {
						Message connectResponse((size_t)InternalServerToClientMsgId::ConnectResponse);
						size_t clientId = m_ClientIdCounter++;
						connectResponse << clientId;
						m_Clients[clientId] = { outMsg->ipAddress, outMsg->port };
						Send(clientId, connectResponse);
						OnClientConnected(clientId);
					}
					return false;
				}
				if (outMsg->msg.Id == (size_t)InternalClientToServerMsgId::Disconnect) {
					auto findClient = m_Clients.find(outMsg->msg.Sender);
					if (findClient != m_Clients.end()) {
						OnClientDisconnected(outMsg->msg.Sender);
						m_Clients.erase(outMsg->msg.Sender);
					}
				}

				return true;
			}
			return false;
		}

	protected:
		virtual bool OnClientConnectionRequest([[maybe_unused]] const std::string& ip) {
			return true; // do some kind of blacklist checking...
		}
		virtual void OnClientConnected(size_t clientId) { std::cout << "Client connected: " << clientId << '\n'; }
		virtual void OnClientDisconnected(size_t clientId) { std::cout << "Client disconnected: " << clientId << '\n'; }

	protected:
		unsigned short m_Port;
		size_t m_ClientIdCounter = 1;
		std::unordered_map<size_t, ClientInfo> m_Clients;
		UDPSocket m_Socket;
	};
}