#pragma once

namespace Pit::Networking {
#define INTERNAL_ID(x) ((size_t)-1 - x)

	enum class InternalClientToServerMsgId : size_t {
		ConnectRequest = INTERNAL_ID(1),
		ConnectRequestAnswer = INTERNAL_ID(2),
		Disconnect = INTERNAL_ID(3)
	};

	enum class InternalServerToClientMsgId : size_t {
		ConnectQuestion = INTERNAL_ID(1),
		ConnectionResponse = INTERNAL_ID(2),
		Disconnect = INTERNAL_ID(3),
		Kick = INTERNAL_ID(4)
	};
#undef INTERNAL_ID
}