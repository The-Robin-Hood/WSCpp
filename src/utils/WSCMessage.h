#include <vector>
#include <chrono>
#include <iomanip>
#include <sstream>

enum WSCMessageType {
    // Opcode mask (bits 0-3)
    OPCODE_MASK = 0x0F,

    // Frame types (opcodes)
    CONTINUATION = 0x0,
    TEXT = 0x1,
    BINARY = 0x2,
    CLOSE = 0x8,
    PING = 0x9,
    PONG = 0xA,

    // Control bits (bits 4-7)
    FIN = 0x80,   // Final fragment
    RSV1 = 0x40,  // Reserved 1
    RSV2 = 0x20,  // Reserved 2
    RSV3 = 0x10,  // Reserved 3

    UNINITIALIZED = -1,
    SENT = 100,
    RECEIVED = 101,
};

struct WSCMessage {
    WSCMessageType type = WSCMessageType::UNINITIALIZED;
    std::vector<uint8_t> payload;
    std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::now();
    uint16_t closeCode{0};
    std::string closeReason = "";
    std::string getPayload() const { return std::string(payload.begin(), payload.end()); }
    std::string getFormattedTimestamp() const {
        auto time = std::chrono::system_clock::to_time_t(timestamp);
        std::tm localTimeBuffer;
#ifdef _WIN32
        localtime_s(&localTimeBuffer, &time);
        std::tm *localTime = &localTimeBuffer;
#else
        std::tm *localTime = localtime_r(&time, &localTimeBuffer);
#endif
        std::stringstream ss;
        ss << std::put_time(localTime, "%Y-%m-%d %H:%M:%S");
        return ss.str();
    }
};