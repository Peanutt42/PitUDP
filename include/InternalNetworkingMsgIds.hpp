#pragma once

namespace Pit::Networking {
	enum class InternalClientToServerMsgId : size_t {
		ConnectRequest = (size_t)-1 - 1
	};

	enum class InternalServerToClientMsgId : size_t {
		ConnectionResponse = (size_t)-1 - 1
	};
}