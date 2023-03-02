#pragma once

namespace Pit::Networking {
#define INTERNAL_ID(x) ((size_t)-1 - x)

	enum class InternalClientToServerMsgId : size_t {
		Connect = INTERNAL_ID(1),
		Disconnect = INTERNAL_ID(2)
	};

	enum class InternalServerToClientMsgId : size_t {
		ConnectResponse = INTERNAL_ID(1),
		Disconnect = INTERNAL_ID(2)
	};
#undef INTERNAL_ID
}