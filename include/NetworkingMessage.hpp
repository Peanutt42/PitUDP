#pragma once

#include <cstdint>
#include <vector>
#include <iostream>

namespace Pit::Networking {
	using MessageId = size_t;
	
	struct Message {
		MessageId Id;
		size_t Sender;
		std::vector<uint8_t> Body;

		Message(MessageId id = 0, size_t capacity = 0)
			: Id(id), Sender(0) {
			Body.reserve(capacity);
		}

		void Serialize(std::vector<uint8_t>& outData, size_t sender) const {
			std::memcpy(outData.data(), &Id, sizeof(Id));
			std::memcpy(outData.data() + sizeof(Id), &sender, sizeof(sender));
			std::memcpy(outData.data() + sizeof(Id) + sizeof(sender), Body.data(), Body.size());
		}

		void Deserialize(const void* data, size_t size) {
			std::memcpy(&Id, data, sizeof(Id));
			std::memcpy(&Sender, (const uint8_t*)data + sizeof(Id), sizeof(Sender));
			Body.resize(size - (sizeof(Id) + sizeof(Sender)));
			std::memcpy(Body.data(), (const uint8_t*)data + sizeof(Id) + sizeof(Sender), Body.size());
		}

		size_t size() const {
			return sizeof(Id) + sizeof(Sender) + Body.size();
		}

		// TODO: Support custom complex types with pointers...
		template<typename T>
		friend Message& operator << (Message& msg, const T& data) {
			static_assert(std::is_standard_layout_v<T>, "Data is too complex, implement custom reading/writing types from messages!");

			size_t oldSize = msg.Body.size();
			msg.Body.resize(msg.Body.size() + sizeof(T));
			std::memcpy(msg.Body.data() + oldSize, &data, sizeof(T));
			return msg;
		}

		void Write(const void* data, size_t size) {
			size_t oldSize = Body.size();
			Body.resize(oldSize + size);
			std::memcpy(Body.data() + oldSize, data, size);
		}

		template<typename T>
		friend Message& operator >> (Message& msg, T& outData) {
			static_assert(std::is_standard_layout_v<T>, "Data is too complex, implement custom reading/writing types from messages!");

			size_t dataIndex = msg.Body.size() - sizeof(T);
			std::memcpy(&outData, msg.Body.data() + dataIndex, sizeof(T));
			msg.Body.resize(dataIndex);
			
			return msg;
		}
	};
}