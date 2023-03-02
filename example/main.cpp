#define PIT_NETWORKING_LINK_WS2 true
#include "include/NetworkingClient.hpp"
#include "include/NetworkingServer.hpp"

using namespace Pit;
using namespace Networking;

enum class ServerToClientMsgId : size_t {
	None = 0,
	NewChatMsg = 1
};

enum class ClientToServerMsgId : size_t {
	None = 0,
	ChatPrivate = 1,
	ChatPublic = 2
};

static size_t MySecureFunction(size_t inputA, size_t inputB) {
	size_t output = 8129812;

	output ^= std::hash<size_t>{}(inputA)+0x9e3779b9 + (output << 6) + (output >> 2);
	output ^= std::hash<size_t>{}(inputB)+0x9e3779b9 + (output << 6) + (output >> 2);

	return output;
}

int main(int argc, char** argv) {
	Networking::Init();

	if (argc < 3) {
		std::cout << "Usage: [client/server] [port] [client: serverIp]\n";
		return 1;
	}

	bool isClient = strcmp(argv[1], "client") == 0;
	if (isClient && argc < 4) {
		std::cout << "Don't forget to add serverIp as 3rd arg!\n";
		return 1;
	}
	unsigned short port = (unsigned short)atoi(argv[2]);

	if (isClient) {
		std::string serverIp = argv[3];
		Client client(serverIp, port, MySecureFunction);
		client.Connect();
		while (true) {
			std::string message;
			std::getline(std::cin, message);
			if (message == "exit") break;
			static bool publicChat = false;
			if (message == "public") {
				publicChat = true;
				continue;
			}
			if (message == "private") {
				publicChat = false;
				continue;
			}

			Message chatMsg(publicChat ? (size_t)ClientToServerMsgId::ChatPublic : (size_t)ClientToServerMsgId::ChatPrivate);
			chatMsg.Write(message.c_str(), message.size() + 1);
			client.Send(chatMsg);

			RecievedMessage recievedMsg((size_t)ServerToClientMsgId::None);
			while (client.GetNextMessage(&recievedMsg)) {
				switch ((ServerToClientMsgId)recievedMsg.msg.Id) {
				default:
				case ServerToClientMsgId::None: break;
				case ServerToClientMsgId::NewChatMsg:
					std::string message = (const char*)recievedMsg.msg.Body.data();
					std::cout << message << '\n';
				}
			}
		}
		client.Disconnect();
	}
	else {
		Server server(port, MySecureFunction);
		server.Bind();
		while (true) {
			if (GetAsyncKeyState(VK_ESCAPE) & 0x01) break;

			RecievedMessage recievedMsg((size_t)ClientToServerMsgId::None);
			while (server.GetNextMessage(&recievedMsg)) {
				switch ((ClientToServerMsgId)recievedMsg.msg.Id) {
				default:
				case ClientToServerMsgId::None: break;
				case ClientToServerMsgId::ChatPrivate: {
					std::string message = (const char*)recievedMsg.msg.Body.data();
					std::cout << "Msg only for me: " << message << '\n';
				} break;
				case ClientToServerMsgId::ChatPublic: {
					std::string message = (const char*)recievedMsg.msg.Body.data();
					std::cout << "Pass msg to everyone\n";
					Message passChatMsg((size_t)ServerToClientMsgId::NewChatMsg);
					passChatMsg.Write(message.c_str(), message.size() + 1);
					server.SendAll(passChatMsg, std::vector<size_t>{ recievedMsg.msg.Sender });
				} break;
				}
			}
		}
	}

	Networking::Shutdown();
}