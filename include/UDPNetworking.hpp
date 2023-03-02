#pragma once

#include "TSQueue.hpp"
#include "NetworkingMessage.hpp"

#include <iostream>
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <WinSock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <thread>
#include <string>
#include <array>
#include <functional>

#ifdef PIT_NETWORKING_LINK_WS2
#pragma comment(lib, "Ws2_32.lib")
#endif
#include <cassert>

namespace Pit::Networking {
    using SecureFunctionSignature = std::function<size_t(size_t, size_t)>;

    struct RecievedMessage {
        std::string ipAddress;
        unsigned short port;
        Message msg;

        RecievedMessage(MessageId id, size_t capacity = 0)
            : msg(id, capacity), ipAddress("empty"), port(0) {}
    };

    class UDPSocket {
    public:
        UDPSocket() {
            Open();
        }
        ~UDPSocket() {
            Close();
        }

        void Open() {
            if (IsOpen()) Close();

            sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
            if (sock == INVALID_SOCKET)
                throw std::system_error(WSAGetLastError(), std::system_category(), "Error opening socket");

            int m_MaxMsgSizeLen = sizeof(m_MaxMsgSize);
            getsockopt(sock, SOL_SOCKET, SO_MAX_MSG_SIZE, (char*)&m_MaxMsgSize, &m_MaxMsgSizeLen);
            assert(m_MaxMsgSizeLen == sizeof(m_MaxMsgSize) && "MaxMsgSize must be a int!");

            m_ListenThread = std::thread([this]() {
                std::vector<uint8_t> buffer(m_MaxMsgSize);
                while (m_Listen) {
                    sockaddr_in from;
                    int size = sizeof(from);
                    int returnSize = recvfrom(sock, (char*)buffer.data(), (int)buffer.size(), 0, reinterpret_cast<SOCKADDR*>(&from), &size);

                    if (!m_Listen) break;

                    if (returnSize >= 0) {
                        m_RecievedMsgs.emplace_back(0);
                        auto& recievedMsg = m_RecievedMsgs.back();
                        recievedMsg.ipAddress = inet_ntoa(from.sin_addr);
                        recievedMsg.port = htons(from.sin_port);
                        recievedMsg.msg.Deserialize(buffer.data(), (size_t)returnSize);
                    }

                    std::memset(buffer.data(), 0, buffer.size());
                }
                });
        }

        void Close() {
            if (sock)
                closesocket(sock);
            m_Listen = false;
            if (m_ListenThread.joinable())
                m_ListenThread.join();
        }

        bool IsOpen() const {
            return m_Listen && sock;
        }

        bool Send(const std::string& address, unsigned short port, const void* buffer, size_t bufferSize) {
            sockaddr_in add;
            add.sin_family = AF_INET;
            add.sin_addr.s_addr = inet_addr(address.c_str());
            add.sin_port = htons(port);
            int ret = sendto(sock, (const char*)buffer, (int)bufferSize, 0, reinterpret_cast<SOCKADDR*>(&add), sizeof(add));
            return ret > 0;
        }
        void SendMsg(const std::string& address, unsigned short port, size_t sender, const Message& msg) {
            std::vector<uint8_t> data(msg.size());
            msg.Serialize(data, sender);
            Send(address, port, data.data(), data.size());
        }
        void Bind(unsigned short port) {
            sockaddr_in add;
            add.sin_family = AF_INET;
            add.sin_addr.s_addr = htonl(INADDR_ANY);
            add.sin_port = htons(port);

            int ret = bind(sock, reinterpret_cast<SOCKADDR*>(&add), sizeof(add));
            if (ret < 0)
                throw std::system_error(WSAGetLastError(), std::system_category(), "Bind failed");
        }

        TSQueue<RecievedMessage>& GetRecievedMessages() { return m_RecievedMsgs; }

    private:
        SOCKET sock;
        int m_MaxMsgSize = 0;
        TSQueue<RecievedMessage> m_RecievedMsgs;
        std::thread m_ListenThread;
        bool m_Listen = true;
    };

    [[maybe_unused]] static void Init() {
        WSAData data;
        int ret = WSAStartup(MAKEWORD(2, 2), &data);
        if (ret != 0)
            throw std::system_error(WSAGetLastError(), std::system_category(), "WSAStartup Failed");
    }

    [[maybe_unused]] static void Shutdown() {
        WSACleanup();
    }
}