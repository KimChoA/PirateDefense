#pragma once

#include <SFML/Network.hpp>
#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "Game.hpp"

class NetworkManager
{
public:
    struct ChatMessage
    {
        sf::String sender;
        sf::String message;
    };

    struct RoomServerInfo
    {
        sf::IpAddress address;
        unsigned short port;
    };

private:
    sf::TcpListener listener;
    sf::TcpSocket serverSocket;

    std::vector<std::unique_ptr<sf::TcpSocket>> clients;
    std::vector<sf::String> playerNicknames;
    std::vector<int> clientPlayerIds;

    std::vector<sf::String> syncedPlayerNicknames;
    std::vector<ChatMessage> chatMessages;

    std::string serverRoomCode;
    sf::String serverRoomName;
    sf::String syncedRoomName;

    bool serverRunning = false;
    bool connected = false;

    bool joinAccepted = false;
    bool joinRejected = false;
    bool roomFull = false;
    bool kicked = false;
    bool gameStarted = false;
    bool hostClosed = false;

    int maxPlayers = 2;
    int syncedMaxPlayers = 2;

    unsigned short serverPort = 54000;

    // 실제 게임 통신용 상태
    std::array<dw::Input, dw::MaxPlayers> remoteGameInputs{};
    dw::Game receivedGameState{};
    bool receivedGameStateReady = false;
    int localPlayerId = 0;

public:
    NetworkManager();

    // 서버
    bool startServer(unsigned short port);
    void updateServer();
    void stopServer();

    // 방 설정
    void setRoomCode(const std::string& roomCode);
    void setRoomName(const sf::String& roomName);
    void setMaxPlayers(int count);

    // 같은 LAN에서 TCP로 방 코드 검색
    std::optional<RoomServerInfo> findRoomServer(
        const std::string& roomCode
    );

    // 참가자
    bool connectToServer(
        const sf::IpAddress& ip,
        unsigned short port
    );

    bool sendJoinRequest(
        const sf::String& nickname,
        const std::string& roomCode
    );

    void updateClient();
    void disconnect();

    // 방장 기능
    bool kickPlayer(std::size_t participantIndex);
    bool startGame();

    // 상태
    bool isServerRunning() const;
    bool isConnected() const;
    bool isJoinAccepted() const;
    bool isJoinRejected() const;
    bool isRoomFull() const;
    bool isKicked() const;
    bool isGameStarted() const;
    bool isHostClosed() const;

    int getMaxPlayers() const;
    int getSyncedMaxPlayers() const;

    std::size_t getClientCount() const;

    const std::vector<sf::String>&
        getPlayerNicknames() const;

    const std::vector<sf::String>&
        getSyncedPlayerNicknames() const;

    const sf::String& getRoomName() const;
    const sf::String& getSyncedRoomName() const;

    // 실시간 채팅: 방장/참가자 모두 같은 API를 사용합니다.
    bool sendChatMessage(
        const sf::String& sender,
        const sf::String& message
    );

    const std::vector<ChatMessage>&
        getChatMessages() const;

    void clearChatMessages();

    void broadcastPlayerList(
        const sf::String& hostNickname
    );

    // 실제 멀티 게임 통신

    // 참가자 -> 방장 : 자신의 입력 전송
    bool sendGameInput(const dw::Input& input);

    // 방장 : 현재까지 받은 참가자 입력을 복사
    // tap/drop/fire/select/vote 같은 1회성 입력은 복사 후 서버 저장본에서 해제
    void copyGameInputs(
        std::array<dw::Input, dw::MaxPlayers>& inputs
    );

    // 방장 -> 참가자 : 권위 있는 전체 게임 상태 전송
    void broadcastGameState(const dw::Game& game);

    // 참가자 : 가장 최근에 받은 게임 상태 적용
    bool consumeGameState(dw::Game& game);

    // 참가자에게 GAME_START 때 부여된 번호 (방장=0, 참가자=1~4)
    int getLocalPlayerId() const;

    // 참가자가 GAME_START를 한 번 처리한 뒤 플래그 해제
    void clearGameStarted();
};
