#include <SFML/Graphics.hpp>
#include <optional>
#include <random>
#include <string>
#include <vector>
#include <cstring>
#include <iostream>
#include <algorithm>
#include <cmath>
#include <windows.h>
#include "NetworkManager.h"
#include "SingleGame.h"
#include "Render.hpp"
#include "MultiGame.h"

enum class GameState
{
    NicknameInput,
    MainMenu,
    CreateRoom,
    JoinRoom,
    SinglePlayer,
    Lobby,
    MultiGame
};

std::string generateRoomCode()
{
    static std::random_device rd;
    static std::mt19937 gen(rd());

    const std::string chars =
        "ABCDEFGHJKLMNPQRSTUVWXYZ123456789";

    std::uniform_int_distribution<int> dist(
        0,
        static_cast<int>(chars.size()) - 1
    );

    std::string code;

    for (int i = 0; i < 6; i++)
    {
        code += chars[dist(gen)];
    }

    return code;
}

bool copyToClipboard(const std::string& text)
{
    if (!OpenClipboard(nullptr))
    {
        return false;
    }

    EmptyClipboard();

    HGLOBAL hMemory =
        GlobalAlloc(
            GMEM_MOVEABLE,
            text.size() + 1
        );

    if (hMemory == nullptr)
    {
        CloseClipboard();
        return false;
    }

    char* memory =
        static_cast<char*>(
            GlobalLock(hMemory)
            );

    if (memory == nullptr)
    {
        GlobalFree(hMemory);
        CloseClipboard();
        return false;
    }

    std::memcpy(
        memory,
        text.c_str(),
        text.size() + 1
    );

    GlobalUnlock(hMemory);

    if (SetClipboardData(
        CF_TEXT,
        hMemory
    ) == nullptr)
    {
        GlobalFree(hMemory);
        CloseClipboard();
        return false;
    }

    CloseClipboard();

    return true;
}

std::string getRoomCodeFromClipboard()
{
    std::string result;

    if (!OpenClipboard(nullptr))
    {
        return result;
    }

    HANDLE hUnicode =
        GetClipboardData(CF_UNICODETEXT);

    if (hUnicode != nullptr)
    {
        const wchar_t* text =
            static_cast<const wchar_t*>(
                GlobalLock(hUnicode)
                );

        if (text != nullptr)
        {
            for (
                std::size_t i = 0;
                text[i] != L'\0' &&
                result.size() < 6;
                ++i
                )
            {
                wchar_t ch = text[i];

                if (ch >= L'a' &&
                    ch <= L'z')
                {
                    ch =
                        ch - L'a' + L'A';
                }

                if (
                    (ch >= L'A' &&
                        ch <= L'Z') ||
                    (ch >= L'0' &&
                        ch <= L'9')
                    )
                {
                    result +=
                        static_cast<char>(ch);
                }
            }

            GlobalUnlock(hUnicode);
        }
    }

    if (result.empty())
    {
        HANDLE hText =
            GetClipboardData(CF_TEXT);

        if (hText != nullptr)
        {
            const char* text =
                static_cast<const char*>(
                    GlobalLock(hText)
                    );

            if (text != nullptr)
            {
                for (
                    std::size_t i = 0;
                    text[i] != '\0' &&
                    result.size() < 6;
                    ++i
                    )
                {
                    char ch = text[i];

                    if (ch >= 'a' &&
                        ch <= 'z')
                    {
                        ch =
                            static_cast<char>(
                                ch - 'a' + 'A'
                                );
                    }

                    if (
                        (ch >= 'A' &&
                            ch <= 'Z') ||
                        (ch >= '0' &&
                            ch <= '9')
                        )
                    {
                        result += ch;
                    }
                }

                GlobalUnlock(hText);
            }
        }
    }

    CloseClipboard();

    return result;
}

void drawHover(
    sf::RenderWindow& window,
    sf::RectangleShape& hover,
    const sf::RectangleShape& button,
    const sf::Vector2f& mousePos)
{
    if (button.getGlobalBounds().contains(mousePos))
    {
        hover.setSize(button.getSize());
        hover.setPosition(button.getPosition());

        window.draw(hover);
    }
}

void centerText(
    sf::Text& text,
    float x,
    float y,
    float width,
    float height)
{
    sf::FloatRect bounds =
        text.getLocalBounds();

    text.setPosition({
        x +
        (width - bounds.size.x) / 2.f -
        bounds.position.x,

        y +
        (height - bounds.size.y) / 2.f -
        bounds.position.y
        });
}

void leftCenterText(
    sf::Text& text,
    float x,
    float y,
    float height)
{
    sf::FloatRect bounds =
        text.getLocalBounds();

    text.setPosition({
        x - bounds.position.x,

        y +
        (height - bounds.size.y) / 2.f -
        bounds.position.y
        });
}

std::string toUtf8String(const sf::String& value)
{
    sf::U8String utf8 = value.toUtf8();
    return std::string(utf8.begin(), utf8.end());
}

sf::Vector2f windowToGamePoint(
    const sf::RenderWindow& window,
    sf::Vector2i pixel,
    bool integerScale)
{
    const auto size = window.getSize();

    float scale = std::min(
        size.x / 640.f,
        size.y / 360.f
    );

    if (integerScale && scale >= 1.f)
    {
        scale = std::floor(scale);
    }

    if (scale <= 0.f)
    {
        scale = 1.f;
    }

    const float offsetX =
        (size.x - 640.f * scale) / 2.f;
    const float offsetY =
        (size.y - 360.f * scale) / 2.f;

    return {
        (static_cast<float>(pixel.x) - offsetX) / scale,
        (static_cast<float>(pixel.y) - offsetY) / scale
    };
}

int shopItemAt(sf::Vector2f point)
{
    for (int i = 0; i < dw::ShopItemCount; ++i)
    {
        const int col = i % 5;
        const int row = i / 5;

        const float x = 34.f + col * 116.f;
        const float y = 108.f + row * 74.f;

        if (point.x >= x && point.x <= x + 108.f &&
            point.y >= y && point.y <= y + 64.f)
        {
            return i;
        }
    }

    return -1;
}

int ammoItemAt(sf::Vector2f point)
{
    for (int i = 0; i < dw::ShellCount; ++i)
    {
        const int col = i % 3;
        const int row = i / 3;
        const float x = 220.f + col * 104.f;
        const float y = 138.f + row * 58.f;

        if (point.x >= x && point.x <= x + 94.f &&
            point.y >= y && point.y <= y + 48.f)
        {
            return i;
        }
    }

    return -1;
}

int shopVoteChoiceAt(sf::Vector2f point)
{
    if (point.x >= 330.f && point.x <= 418.f &&
        point.y >= 255.f && point.y <= 281.f)
    {
        return 1;
    }

    if (point.x >= 426.f && point.x <= 514.f &&
        point.y >= 255.f && point.y <= 281.f)
    {
        return 0;
    }

    return -1;
}

bool shopReadyAt(sf::Vector2f point)
{
    return
        point.x >= 500.f && point.x <= 606.f &&
        point.y >= 286.f && point.y <= 314.f;
}

bool playerNearAmmoRack(const dw::Game& game, int playerId)
{
    if (playerId < 0 || playerId >= dw::MaxPlayers)
        return false;

    const auto& player = game.players[playerId];

    return
        player.active &&
        player.held < 0 &&
        dw::dist(player.p, dw::AmmoPoint) < 32.f;
}

int main()
{
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    sf::RenderWindow window(
        sf::VideoMode({ 1280, 720 }),
        "Pirate Defense"
    );

    window.setFramerateLimit(60);

    GameState currentState =
        GameState::NicknameInput;

    NetworkManager networkManager;

    dw::Game singleGame;
    dw::SingleGame singleGameMode;
    dw::Renderer singleRenderer;
    dw::Screen singleScreen;
    dw::Input singleInput;

    sf::RenderTexture singleCanvas({
        640,
        360
        });

    singleCanvas.setSmooth(false);

    sf::Clock singleGameClock;

    dw::Game multiGame;
    dw::MultiGame multiGameMode(networkManager);
    dw::Screen multiScreen;
    dw::Input multiInput;
    dw::Renderer multiRenderer;

    sf::RenderTexture multiCanvas({
        640,
        360
        });

    multiCanvas.setSmooth(false);

    sf::Clock multiGameClock;

    bool multiChatVisible = false;
    sf::String multiChatInput;

    sf::Font font;

    if (!font.openFromFile(
        "C:/Windows/Fonts/malgun.ttf"))
    {
        return -1;
    }

    sf::Texture nicknameTexture;

    if (!nicknameTexture.loadFromFile(
        "assets/images/nickname_background.png"))
    {
        return -1;
    }

    sf::Sprite nicknameBackground(
        nicknameTexture
    );

    nicknameBackground.setScale({
        1280.f /
        static_cast<float>(
            nicknameTexture.getSize().x
        ),

        720.f /
        static_cast<float>(
            nicknameTexture.getSize().y
        )
        });

    sf::Texture mainMenuTexture;

    if (!mainMenuTexture.loadFromFile(
        "assets/images/mainmenu_background.png"))
    {
        return -1;
    }

    sf::Sprite mainMenuBackground(
        mainMenuTexture
    );

    mainMenuBackground.setScale({
        1280.f /
        static_cast<float>(
            mainMenuTexture.getSize().x
        ),

        720.f /
        static_cast<float>(
            mainMenuTexture.getSize().y
        )
        });

    sf::Texture createRoomTexture;

    if (!createRoomTexture.loadFromFile(
        "assets/images/createroom_background.png"))
    {
        return -1;
    }

    sf::Sprite createRoomBackground(
        createRoomTexture
    );

    createRoomBackground.setScale({
        1280.f /
        static_cast<float>(
            createRoomTexture.getSize().x
        ),

        720.f /
        static_cast<float>(
            createRoomTexture.getSize().y
        )
        });

    sf::Texture joinRoomTexture;

    if (!joinRoomTexture.loadFromFile(
        "assets/images/joinroom_background.png"))
    {
        return -1;
    }

    sf::Sprite joinRoomBackground(
        joinRoomTexture
    );

    joinRoomBackground.setScale({
        1280.f /
        static_cast<float>(
            joinRoomTexture.getSize().x
        ),

        720.f /
        static_cast<float>(
            joinRoomTexture.getSize().y
        )
        });

    sf::Texture lobbyBackgroundTexture;

    if (!lobbyBackgroundTexture.loadFromFile(
        "assets/images/lobby_background.png"))
    {
        return -1;
    }

    sf::Sprite lobbyBackground(
        lobbyBackgroundTexture
    );

    lobbyBackground.setScale({
        1280.f /
        static_cast<float>(
            lobbyBackgroundTexture.getSize().x
        ),

        720.f /
        static_cast<float>(
            lobbyBackgroundTexture.getSize().y
        )
        });

    sf::RectangleShape buttonHover;

    buttonHover.setFillColor(
        sf::Color(
            255,
            225,
            130,
            55
        )
    );

    sf::String nickname;

    bool nicknameInputActive =
        true;

    sf::RectangleShape nicknameInputArea({
        338.f,
        68.f
        });

    nicknameInputArea.setPosition({
        438.f,
        354.f
        });

    nicknameInputArea.setFillColor(
        sf::Color::Transparent
    );

    sf::Text nicknameText(font);

    nicknameText.setCharacterSize(22);

    nicknameText.setFillColor(
        sf::Color::White
    );

    nicknameText.setPosition({
        468.f,
        374.f
        });

    sf::RectangleShape nicknameCursor({
        2.f,
        25.f
        });

    nicknameCursor.setFillColor(
        sf::Color::White
    );

    sf::Clock nicknameCursorClock;

    sf::RectangleShape confirmButton({
        260.f,
        72.f
        });

    confirmButton.setPosition({
        510.f,
        442.f
        });

    confirmButton.setFillColor(
        sf::Color::Transparent
    );

    sf::Text welcomeText(font);

    welcomeText.setCharacterSize(22);

    welcomeText.setFillColor(
        sf::Color(
            245,
            232,
            200
        )
    );

    sf::RectangleShape createRoomButton({
        345.f,
        67.f
        });

    createRoomButton.setPosition({
        468.f,
        300.f
        });

    createRoomButton.setFillColor(
        sf::Color::Transparent
    );

    sf::RectangleShape joinRoomButton({
        345.f,
        67.f
        });

    joinRoomButton.setPosition({
        468.f,
        374.f
        });

    joinRoomButton.setFillColor(
        sf::Color::Transparent
    );

    sf::RectangleShape singleButton({
        345.f,
        67.f
        });

    singleButton.setPosition({
        468.f,
        448.f
        });

    singleButton.setFillColor(
        sf::Color::Transparent
    );

    sf::RectangleShape exitButton({
        345.f,
        67.f
        });

    exitButton.setPosition({
        468.f,
        522.f
        });

    exitButton.setFillColor(
        sf::Color::Transparent
    );

    sf::RectangleShape mainBackButton({
        245.f,
        60.f
        });

    mainBackButton.setPosition({
        36.f,
        620.f
        });

    mainBackButton.setFillColor(
        sf::Color::Transparent
    );

    // 메인 메뉴에서 바로 열 수 있는 별도 조작법 버튼
    bool mainHelpVisible = false;

    sf::RectangleShape mainHelpButton({
        190.f,
        54.f
        });

    mainHelpButton.setPosition({
        1045.f,
        622.f
        });

    mainHelpButton.setFillColor(
        sf::Color(18, 38, 52, 225)
    );
    mainHelpButton.setOutlineThickness(2.f);
    mainHelpButton.setOutlineColor(
        sf::Color(205, 177, 118)
    );

    sf::Text mainHelpButtonText(font);
    mainHelpButtonText.setString(sf::String(L"조작법"));
    mainHelpButtonText.setCharacterSize(20);
    mainHelpButtonText.setFillColor(
        sf::Color(245, 232, 200)
    );

    int selectedPlayers = 0;

    sf::String roomName;

    bool roomNameInputActive =
        false;

    sf::RectangleShape roomNameInputArea({
        340.f,
        55.f
        });

    roomNameInputArea.setPosition({
        560.f,
        278.f
        });

    roomNameInputArea.setFillColor(
        sf::Color::Transparent
    );

    sf::Text roomNameText(font);

    roomNameText.setCharacterSize(22);

    roomNameText.setFillColor(
        sf::Color::White
    );

    std::string roomCode =
        generateRoomCode();

    std::string currentRoomCode;

    sf::Text roomCodeText(font);

    roomCodeText.setString(
        roomCode
    );

    roomCodeText.setCharacterSize(27);

    roomCodeText.setFillColor(
        sf::Color(
            245,
            210,
            120
        )
    );

    sf::RectangleShape roomCodeCopyArea({
        350.f,
        56.f
        });

    roomCodeCopyArea.setPosition({
        371.f,
        505.f
        });

    roomCodeCopyArea.setFillColor(
        sf::Color::Transparent
    );

    sf::RectangleShape player2Button({
        135.f,
        70.f
        });

    player2Button.setPosition({
        363.f,
        388.f
        });

    player2Button.setFillColor(
        sf::Color::Transparent
    );

    sf::RectangleShape player3Button({
        135.f,
        70.f
        });

    player3Button.setPosition({
        505.f,
        388.f
        });

    player3Button.setFillColor(
        sf::Color::Transparent
    );

    sf::RectangleShape player4Button({
        135.f,
        70.f
        });

    player4Button.setPosition({
        644.f,
        388.f
        });

    player4Button.setFillColor(
        sf::Color::Transparent
    );

    sf::RectangleShape player5Button({
        140.f,
        70.f
        });

    player5Button.setPosition({
        782.f,
        388.f
        });

    player5Button.setFillColor(
        sf::Color::Transparent
    );

    sf::RectangleShape regenerateButton({
        175.f,
        67.f
        });

    regenerateButton.setPosition({
        735.f,
        504.f
        });

    regenerateButton.setFillColor(
        sf::Color::Transparent
    );

    sf::RectangleShape finalCreateRoomButton({
        330.f,
        75.f
        });

    finalCreateRoomButton.setPosition({
        474.f,
        575.f
        });

    finalCreateRoomButton.setFillColor(
        sf::Color::Transparent
    );

    sf::RectangleShape createBackButton({
        240.f,
        60.f
        });

    createBackButton.setPosition({
        36.f,
        623.f
        });

    createBackButton.setFillColor(
        sf::Color::Transparent
    );

    sf::RectangleShape selectedHighlight;

    selectedHighlight.setFillColor(
        sf::Color(
            255,
            190,
            40,
            75
        )
    );

    sf::String joinCode;

    bool joinInputActive =
        false;

    sf::Text joinCodeText(font);

    joinCodeText.setCharacterSize(24);

    joinCodeText.setFillColor(
        sf::Color::White
    );

    sf::RectangleShape joinInputArea({
        380.f,
        70.f
        });

    joinInputArea.setPosition({
        450.f,
        388.f
        });

    joinInputArea.setFillColor(
        sf::Color::Transparent
    );

    sf::RectangleShape joinConfirmButton({
        300.f,
        73.f
        });

    joinConfirmButton.setPosition({
        491.f,
        477.f
        });

    joinConfirmButton.setFillColor(
        sf::Color::Transparent
    );

    sf::RectangleShape joinBackButton({
        250.f,
        58.f
        });

    joinBackButton.setPosition({
        35.f,
        594.f
        });

    joinBackButton.setFillColor(
        sf::Color::Transparent
    );

    sf::RectangleShape lobbyStartButton({
        300.f,
        63.f
        });

    lobbyStartButton.setPosition({
        490.f,
        565.f
        });

    lobbyStartButton.setFillColor(
        sf::Color::Transparent
    );

    sf::RectangleShape lobbyBackButton({
        240.f,
        58.f
        });

    lobbyBackButton.setPosition({
        38.f,
        623.f
        });

    lobbyBackButton.setFillColor(
        sf::Color::Transparent
    );

    sf::Text lobbyRoomCodeText(font);

    lobbyRoomCodeText.setCharacterSize(15);

    lobbyRoomCodeText.setFillColor(
        sf::Color(
            55,
            30,
            15
        )
    );

    sf::RectangleShape lobbyRoomCodeCopyArea({
        190.f,
        30.f
        });

    lobbyRoomCodeCopyArea.setPosition({
        665.f,
        207.f
        });

    lobbyRoomCodeCopyArea.setFillColor(
        sf::Color::Transparent
    );

    std::vector<sf::RectangleShape>
        lobbyKickButtons;

    const sf::Vector2f
        kickPositions[4] =
    {
        { 758.f, 318.f },
        { 758.f, 380.f },
        { 758.f, 442.f },
        { 758.f, 504.f }
    };

    for (int i = 0; i < 4; ++i)
    {
        sf::RectangleShape kickButton({
            100.f,
            48.f
            });

        kickButton.setPosition(
            kickPositions[i]
        );

        kickButton.setFillColor(
            sf::Color::Transparent
        );

        lobbyKickButtons.push_back(
            kickButton
        );
    }

    std::vector<sf::Text>
        lobbyPlayerTexts;

    lobbyPlayerTexts.reserve(5);

    for (int i = 0; i < 5; ++i)
    {
        sf::Text playerText(font);

        playerText.setCharacterSize(22);

        playerText.setFillColor(
            sf::Color(
                55,
                30,
                15
            )
        );

        lobbyPlayerTexts.push_back(
            playerText
        );
    }

    std::size_t lobbyVisiblePlayerCount =
        0;

    sf::Text lobbyRoomNameText(font);

    lobbyRoomNameText.setCharacterSize(20);

    lobbyRoomNameText.setFillColor(
        sf::Color(
            55,
            30,
            15
        )
    );

    sf::Text tempText(font);

    tempText.setCharacterSize(32);

    tempText.setFillColor(
        sf::Color::White
    );

    tempText.setPosition({
        470.f,
        330.f
        });

    sf::Text multiGameText(font);

    multiGameText.setString(
        U"멀티플레이 게임 시작"
    );

    multiGameText.setCharacterSize(42);

    multiGameText.setFillColor(
        sf::Color::White
    );

    centerText(
        multiGameText,
        0.f,
        0.f,
        1280.f,
        720.f
    );

    while (window.isOpen())
    {
        while (const std::optional event =
            window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
            {
                if (
                    networkManager
                    .isServerRunning()
                    )
                {
                    networkManager
                        .stopServer();
                }

                window.close();
            }

            if (
                currentState ==
                GameState::NicknameInput
                )
            {
                nicknameInputActive =
                    true;

                if (const auto* mousePressed =
                    event->getIf<
                    sf::Event::MouseButtonPressed>())
                {
                    if (
                        mousePressed->button ==
                        sf::Mouse::Button::Left
                        )
                    {
                        sf::Vector2f mousePos(
                            static_cast<float>(
                                mousePressed
                                ->position.x
                                ),
                            static_cast<float>(
                                mousePressed
                                ->position.y
                                )
                        );

                        if (
                            nicknameInputArea
                            .getGlobalBounds()
                            .contains(mousePos)
                            )
                        {
                            nicknameInputActive =
                                true;
                        }

                        if (
                            confirmButton
                            .getGlobalBounds()
                            .contains(mousePos)
                            )
                        {
                            if (
                                !nickname.isEmpty()
                                )
                            {
                                sf::String welcome =
                                    nickname;

                                welcome +=
                                    U"님, 환영합니다!";

                                welcomeText
                                    .setString(
                                        welcome
                                    );

                                centerText(
                                    welcomeText,
                                    438.f,
                                    211.f,
                                    338.f,
                                    77.f
                                );

                                nicknameInputActive =
                                    false;

                                currentState =
                                    GameState::MainMenu;
                            }
                        }
                    }
                }

                if (const auto* keyPressed =
                    event->getIf<
                    sf::Event::KeyPressed>())
                {
                    if (
                        keyPressed->code ==
                        sf::Keyboard::Key::Enter
                        )
                    {
                        if (
                            !nickname.isEmpty()
                            )
                        {
                            sf::String welcome =
                                nickname;

                            welcome +=
                                U"님, 환영합니다!";

                            welcomeText.setString(
                                welcome
                            );

                            centerText(
                                welcomeText,
                                438.f,
                                211.f,
                                338.f,
                                77.f
                            );

                            nicknameInputActive =
                                false;

                            currentState =
                                GameState::MainMenu;
                        }
                    }
                }

                if (const auto* textEntered =
                    event->getIf<
                    sf::Event::TextEntered>())
                {
                    char32_t unicode =
                        textEntered->unicode;

                    if (unicode == 8)
                    {
                        if (
                            !nickname.isEmpty()
                            )
                        {
                            nickname.erase(
                                nickname.getSize() -
                                1,
                                1
                            );
                        }
                    }
                    else if (unicode >= 32)
                    {
                        if (
                            nickname.getSize() <
                            12
                            )
                        {
                            nickname += unicode;
                        }
                    }

                    nicknameText.setString(
                        nickname
                    );
                }
            }

            else if (
                currentState ==
                GameState::MainMenu
                )
            {
                if (const auto* mousePressed =
                    event->getIf<
                    sf::Event::MouseButtonPressed>())
                {
                    if (
                        mousePressed->button ==
                        sf::Mouse::Button::Left
                        )
                    {
                        sf::Vector2f mousePos(
                            static_cast<float>(
                                mousePressed
                                ->position.x
                                ),
                            static_cast<float>(
                                mousePressed
                                ->position.y
                                )
                        );

                        if (
                            mainHelpButton
                            .getGlobalBounds()
                            .contains(mousePos)
                            )
                        {
                            mainHelpVisible =
                                !mainHelpVisible;
                            continue;
                        }

                        if (mainHelpVisible)
                        {
                            // 도움말이 열린 동안 뒤쪽 메뉴 버튼은 눌리지 않게 합니다.
                            continue;
                        }

                        if (
                            createRoomButton
                            .getGlobalBounds()
                            .contains(mousePos)
                            )
                        {
                            selectedPlayers = 0;

                            roomName.clear();

                            roomNameText.setString(
                                roomName
                            );

                            roomNameInputActive =
                                true;

                            roomCode =
                                generateRoomCode();

                            roomCodeText.setString(
                                roomCode
                            );

                            currentRoomCode.clear();

                            currentState =
                                GameState::CreateRoom;
                        }

                        else if (
                            joinRoomButton
                            .getGlobalBounds()
                            .contains(mousePos)
                            )
                        {
                            joinCode.clear();

                            joinCodeText.setString(
                                joinCode
                            );

                            joinInputActive = true;

                            currentRoomCode.clear();

                            currentState =
                                GameState::JoinRoom;
                        }

                        else if (
                            singleButton
                            .getGlobalBounds()
                            .contains(mousePos)
                            )
                        {
                            singleGame =
                                dw::Game{};

                            singleGameMode.reset();

                            singleGameMode.start(
                                singleGame
                            );

                            singleScreen =
                                dw::Screen{};

                            singleScreen.menu =
                                false;

                            singleScreen.help =
                                false;

                            singleScreen.paused =
                                false;

                            singleScreen.online =
                                false;

                            singleScreen.client =
                                false;

                            singleScreen.local =
                                0;

                            singleInput = {};

                            singleGameClock.restart();

                            window.create(
                                sf::VideoMode::
                                getFullscreenModes()
                                .front(),

                                "Pirate Defense",

                                sf::State::Fullscreen
                            );

                            window.setFramerateLimit(
                                60
                            );

                            currentState =
                                GameState::
                                SinglePlayer;
                        }

                        else if (
                            exitButton
                            .getGlobalBounds()
                            .contains(mousePos)
                            )
                        {
                            window.close();
                        }

                        else if (
                            mainBackButton
                            .getGlobalBounds()
                            .contains(mousePos)
                            )
                        {
                            currentState =
                                GameState::
                                NicknameInput;

                            nicknameInputActive =
                                true;
                        }
                    }
                }

                if (
                    const auto* keyPressed =
                    event->getIf<sf::Event::KeyPressed>()
                    )
                {
                    if (
                        keyPressed->code == sf::Keyboard::Key::F1 ||
                        keyPressed->scancode == sf::Keyboard::Scancode::F1
                        )
                    {
                        mainHelpVisible = !mainHelpVisible;
                    }
                    else if (
                        mainHelpVisible &&
                        (keyPressed->code == sf::Keyboard::Key::Escape ||
                            keyPressed->scancode == sf::Keyboard::Scancode::Escape)
                        )
                    {
                        mainHelpVisible = false;
                    }
                }
            }

            else if (
                currentState ==
                GameState::CreateRoom
                )
            {
                if (const auto* mousePressed =
                    event->getIf<
                    sf::Event::MouseButtonPressed>())
                {
                    if (
                        mousePressed->button ==
                        sf::Mouse::Button::Left
                        )
                    {
                        sf::Vector2f mousePos(
                            static_cast<float>(
                                mousePressed
                                ->position.x
                                ),
                            static_cast<float>(
                                mousePressed
                                ->position.y
                                )
                        );

                        if (
                            roomNameInputArea
                            .getGlobalBounds()
                            .contains(mousePos)
                            )
                        {
                            roomNameInputActive =
                                true;
                        }
                        else
                        {
                            roomNameInputActive =
                                false;
                        }

                        if (
                            player2Button
                            .getGlobalBounds()
                            .contains(mousePos)
                            )
                        {
                            selectedPlayers = 2;
                        }

                        else if (
                            player3Button
                            .getGlobalBounds()
                            .contains(mousePos)
                            )
                        {
                            selectedPlayers = 3;
                        }

                        else if (
                            player4Button
                            .getGlobalBounds()
                            .contains(mousePos)
                            )
                        {
                            selectedPlayers = 4;
                        }

                        else if (
                            player5Button
                            .getGlobalBounds()
                            .contains(mousePos)
                            )
                        {
                            selectedPlayers = 5;
                        }

                        else if (
                            roomCodeCopyArea
                            .getGlobalBounds()
                            .contains(mousePos)
                            )
                        {
                            if (
                                copyToClipboard(
                                    roomCode
                                )
                                )
                            {
                                MessageBoxW(
                                    nullptr,
                                    L"방 코드가 복사되었습니다.",
                                    L"방 코드 복사",
                                    MB_OK |
                                    MB_ICONINFORMATION
                                );
                            }
                            else
                            {
                                MessageBoxW(
                                    nullptr,
                                    L"방 코드 복사에 실패했습니다.",
                                    L"복사 실패",
                                    MB_OK |
                                    MB_ICONERROR
                                );
                            }
                        }

                        else if (
                            regenerateButton
                            .getGlobalBounds()
                            .contains(mousePos)
                            )
                        {
                            roomCode =
                                generateRoomCode();

                            roomCodeText.setString(
                                roomCode
                            );
                        }

                        else if (
                            createBackButton
                            .getGlobalBounds()
                            .contains(mousePos)
                            )
                        {
                            currentState =
                                GameState::
                                MainMenu;
                        }

                        else if (
                            finalCreateRoomButton
                            .getGlobalBounds()
                            .contains(mousePos)
                            )
                        {
                            if (
                                roomName.isEmpty()
                                )
                            {
                                MessageBoxW(
                                    nullptr,
                                    L"방 이름을 입력해주세요.",
                                    L"방 만들기",
                                    MB_OK |
                                    MB_ICONWARNING
                                );
                            }
                            else if (
                                selectedPlayers ==
                                0
                                )
                            {
                                MessageBoxW(
                                    nullptr,
                                    L"방 인원을 먼저 선택해주세요.",
                                    L"방 만들기",
                                    MB_OK |
                                    MB_ICONWARNING
                                );
                            }
                            else
                            {
                                networkManager
                                    .setRoomName(
                                        roomName
                                    );

                                networkManager
                                    .setRoomCode(
                                        roomCode
                                    );

                                networkManager
                                    .setMaxPlayers(
                                        selectedPlayers
                                    );

                                bool serverStarted =
                                    networkManager
                                    .startServer(
                                        54000
                                    );

                                if (serverStarted)
                                {
                                    currentRoomCode =
                                        roomCode;

                                    currentState =
                                        GameState::
                                        Lobby;
                                }
                                else
                                {
                                    MessageBoxW(
                                        nullptr,
                                        L"방을 만들 수 없습니다.\n서버 상태를 확인해주세요.",
                                        L"서버 오류",
                                        MB_OK |
                                        MB_ICONERROR
                                    );
                                }
                            }
                        }
                    }
                }

                if (roomNameInputActive)
                {
                    if (
                        const auto*
                        textEntered =
                        event->getIf<
                        sf::Event::
                        TextEntered>()
                        )
                    {
                        char32_t unicode =
                            textEntered
                            ->unicode;

                        if (unicode == 8)
                        {
                            if (
                                !roomName
                                .isEmpty()
                                )
                            {
                                roomName.erase(
                                    roomName
                                    .getSize() -
                                    1,
                                    1
                                );
                            }
                        }

                        else if (
                            unicode >= 32
                            )
                        {
                            if (
                                roomName
                                .getSize() <
                                12
                                )
                            {
                                roomName +=
                                    unicode;
                            }
                        }

                        roomNameText.setString(
                            roomName
                        );
                    }
                }
            }

            else if (
                currentState ==
                GameState::JoinRoom
                )
            {
                if (
                    const auto*
                    keyPressed =
                    event->getIf<
                    sf::Event::
                    KeyPressed>()
                    )
                {
                    using K =
                        sf::Keyboard::Key;

                    if (
                        keyPressed->control &&
                        keyPressed->code ==
                        K::V
                        )
                    {
                        std::string pastedCode =
                            getRoomCodeFromClipboard();

                        if (
                            !pastedCode.empty()
                            )
                        {
                            joinCode =
                                sf::String::
                                fromUtf8(
                                    pastedCode
                                    .begin(),
                                    pastedCode
                                    .end()
                                );

                            joinCodeText
                                .setString(
                                    joinCode
                                );

                            joinInputActive =
                                true;
                        }

                        continue;
                    }
                }

                if (
                    const auto*
                    mousePressed =
                    event->getIf<
                    sf::Event::
                    MouseButtonPressed>()
                    )
                {
                    if (
                        mousePressed->button ==
                        sf::Mouse::Button::Left
                        )
                    {
                        sf::Vector2f mousePos(
                            static_cast<float>(
                                mousePressed
                                ->position.x
                                ),
                            static_cast<float>(
                                mousePressed
                                ->position.y
                                )
                        );

                        if (
                            joinInputArea
                            .getGlobalBounds()
                            .contains(mousePos)
                            )
                        {
                            joinInputActive =
                                true;
                        }

                        if (
                            joinBackButton
                            .getGlobalBounds()
                            .contains(mousePos)
                            )
                        {
                            currentState =
                                GameState::
                                MainMenu;
                        }

                        else if (
                            joinConfirmButton
                            .getGlobalBounds()
                            .contains(mousePos)
                            )
                        {
                            if (
                                joinCode
                                .getSize() !=
                                6
                                )
                            {
                                MessageBoxW(
                                    nullptr,
                                    L"방 코드는 6자리입니다.",
                                    L"방 참가",
                                    MB_OK |
                                    MB_ICONWARNING
                                );
                            }
                            else
                            {
                                sf::U8String
                                    utf8Code =
                                    joinCode
                                    .toUtf8();

                                std::string
                                    enteredRoomCode(
                                        utf8Code
                                        .begin(),
                                        utf8Code
                                        .end()
                                    );

                                auto roomServer =
                                    networkManager
                                    .findRoomServer(
                                        enteredRoomCode
                                    );

                                if (
                                    !roomServer
                                    .has_value()
                                    )
                                {
                                    MessageBoxW(
                                        nullptr,
                                        L"해당 방 코드를 가진 방을 찾을 수 없습니다.\n같은 네트워크인지 확인해주세요.",
                                        L"방 찾기 실패",
                                        MB_OK |
                                        MB_ICONWARNING
                                    );
                                }
                                else
                                {
                                    bool connected =
                                        networkManager
                                        .connectToServer(
                                            roomServer
                                            ->address,
                                            roomServer
                                            ->port
                                        );

                                    if (connected)
                                    {
                                        bool sent =
                                            networkManager
                                            .sendJoinRequest(
                                                nickname,
                                                enteredRoomCode
                                            );

                                        if (!sent)
                                        {
                                            MessageBoxW(
                                                nullptr,
                                                L"참가 요청을 전송하지 못했습니다.",
                                                L"접속 오류",
                                                MB_OK |
                                                MB_ICONERROR
                                            );

                                            networkManager
                                                .disconnect();
                                        }
                                        else
                                        {
                                            currentRoomCode =
                                                enteredRoomCode;
                                        }
                                    }
                                    else
                                    {
                                        MessageBoxW(
                                            nullptr,
                                            L"찾은 방의 서버에 연결할 수 없습니다.",
                                            L"접속 실패",
                                            MB_OK |
                                            MB_ICONERROR
                                        );
                                    }
                                }
                            }
                        }
                    }
                }

                if (
                    const auto*
                    textEntered =
                    event->getIf<
                    sf::Event::
                    TextEntered>()
                    )
                {
                    char32_t unicode =
                        textEntered
                        ->unicode;

                    if (unicode == 8)
                    {
                        if (
                            !joinCode
                            .isEmpty()
                            )
                        {
                            joinCode.erase(
                                joinCode
                                .getSize() -
                                1,
                                1
                            );
                        }
                    }

                    else if (
                        joinCode.getSize() <
                        6
                        )
                    {
                        if (
                            (
                                unicode >= U'A' &&
                                unicode <= U'Z'
                                )
                            ||
                            (
                                unicode >= U'a' &&
                                unicode <= U'z'
                                )
                            ||
                            (
                                unicode >= U'0' &&
                                unicode <= U'9'
                                )
                            )
                        {
                            if (
                                unicode >= U'a' &&
                                unicode <= U'z'
                                )
                            {
                                unicode =
                                    unicode -
                                    U'a' +
                                    U'A';
                            }

                            joinCode += unicode;
                        }
                    }

                    joinCodeText.setString(
                        joinCode
                    );
                }
            }

            else if (
                currentState ==
                GameState::Lobby
                )
            {
                if (
                    const auto*
                    mousePressed =
                    event->getIf<
                    sf::Event::
                    MouseButtonPressed>()
                    )
                {
                    if (
                        mousePressed->button ==
                        sf::Mouse::Button::Left
                        )
                    {
                        sf::Vector2f mousePos(
                            static_cast<float>(
                                mousePressed
                                ->position.x
                                ),
                            static_cast<float>(
                                mousePressed
                                ->position.y
                                )
                        );

                        if (
                            lobbyBackButton
                            .getGlobalBounds()
                            .contains(mousePos)
                            )
                        {
                            if (
                                networkManager
                                .isServerRunning()
                                )
                            {
                                networkManager
                                    .stopServer();

                                window.close();

                                continue;
                            }

                            if (
                                networkManager
                                .isConnected()
                                )
                            {
                                networkManager
                                    .disconnect();
                            }

                            currentRoomCode.clear();

                            currentState =
                                GameState::
                                MainMenu;
                        }

                        else if (
                            lobbyRoomCodeCopyArea
                            .getGlobalBounds()
                            .contains(mousePos)
                            )
                        {
                            if (
                                !currentRoomCode
                                .empty()
                                )
                            {
                                if (
                                    copyToClipboard(
                                        currentRoomCode
                                    )
                                    )
                                {
                                    MessageBoxW(
                                        nullptr,
                                        L"방 코드가 복사되었습니다.",
                                        L"방 코드 복사",
                                        MB_OK |
                                        MB_ICONINFORMATION
                                    );
                                }
                            }
                        }
                        else
                        {
                            bool kickClicked =
                                false;

                            for (
                                std::size_t i =
                                0;

                                i <
                                lobbyKickButtons
                                .size();

                                ++i
                                )
                            {
                                if (
                                    lobbyKickButtons[i]
                                    .getGlobalBounds()
                                    .contains(mousePos)
                                    )
                                {
                                    kickClicked =
                                        true;

                                    if (
                                        !networkManager
                                        .isServerRunning()
                                        )
                                    {
                                        MessageBoxW(
                                            nullptr,
                                            L"방장 권한입니다.",
                                            L"내보내기",
                                            MB_OK |
                                            MB_ICONINFORMATION
                                        );
                                    }

                                    else if (
                                        i + 1 <
                                        lobbyVisiblePlayerCount
                                        )
                                    {
                                        networkManager
                                            .kickPlayer(i);
                                    }

                                    break;
                                }
                            }

                            if (
                                !kickClicked &&
                                lobbyStartButton
                                .getGlobalBounds()
                                .contains(mousePos)
                                )
                            {
                                if (
                                    !networkManager
                                    .isServerRunning()
                                    )
                                {
                                    MessageBoxW(
                                        nullptr,
                                        L"방장만 게임을 시작할 수 있습니다.",
                                        L"게임 시작",
                                        MB_OK |
                                        MB_ICONINFORMATION
                                    );
                                }
                                else
                                {
                                    int currentPlayers =
                                        1;

                                    for (
                                        const auto& name :
                                        networkManager
                                        .getPlayerNicknames()
                                        )
                                    {
                                        if (
                                            !name.isEmpty()
                                            )
                                        {
                                            ++currentPlayers;
                                        }
                                    }

                                    int requiredPlayers =
                                        networkManager
                                        .getMaxPlayers();

                                    if (
                                        currentPlayers <
                                        requiredPlayers
                                        )
                                    {
                                        std::wstring message =
                                            L"인원이 부족합니다.\n현재 인원 : "
                                            +
                                            std::to_wstring(
                                                currentPlayers
                                            )
                                            +
                                            L" / "
                                            +
                                            std::to_wstring(
                                                requiredPlayers
                                            );

                                        MessageBoxW(
                                            nullptr,
                                            message.c_str(),
                                            L"게임 시작",
                                            MB_OK |
                                            MB_ICONWARNING
                                        );
                                    }
                                    else
                                    {
                                        bool started =
                                            networkManager
                                            .startGame();

                                        if (started)
                                        {
                                            multiGame =
                                                dw::Game{};

                                            multiInput = {};

                                            multiScreen =
                                                dw::Screen{};

                                            multiScreen.menu =
                                                false;

                                            multiScreen.help =
                                                false;

                                            multiScreen.paused =
                                                false;

                                            multiGameMode
                                                .start(
                                                    multiGame
                                                );

                                            multiGameMode
                                                .updateScreen(
                                                    multiScreen
                                                );

                                            multiGameClock
                                                .restart();

                                            window.create(
                                                sf::VideoMode::
                                                getFullscreenModes()
                                                .front(),

                                                "Pirate Defense",

                                                sf::State::
                                                Fullscreen
                                            );

                                            window
                                                .setFramerateLimit(
                                                    60
                                                );

                                            currentState =
                                                GameState::
                                                MultiGame;
                                        }
                                        else
                                        {
                                            MessageBoxW(
                                                nullptr,
                                                L"게임을 시작할 수 없습니다.",
                                                L"게임 시작",
                                                MB_OK |
                                                MB_ICONERROR
                                            );
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }

            else if (
                currentState ==
                GameState::SinglePlayer
                )
            {
                if (
                    event->is<
                    sf::Event::FocusLost>() &&
                    singleGame.phase ==
                    dw::Phase::Play
                    )
                {
                    singleScreen.paused =
                        true;
                }

                if (
                    const auto* mousePressed =
                    event->getIf<
                    sf::Event::MouseButtonPressed>()
                    )
                {
                    if (
                        mousePressed->button ==
                        sf::Mouse::Button::Left
                        )
                    {
                        const sf::Vector2f gamePoint =
                            windowToGamePoint(
                                window,
                                mousePressed->position,
                                true
                            );

                        if (
                            singleGame.phase == dw::Phase::Play &&
                            singleScreen.ammoMenuVisible
                            )
                        {
                            const int shell =
                                ammoItemAt(gamePoint);

                            if (shell >= 0)
                            {
                                singleInput.select = shell;
                                singleInput.tap = true;
                            }

                            singleScreen.ammoMenuVisible = false;
                            continue;
                        }

                        if (singleGame.phase == dw::Phase::Shop)
                        {
                            singleInput.shopVisit = singleGame.shopVisit;
                            singleInput.purchaseVoteId = singleGame.purchaseVoteId;

                            if (singleGame.purchaseVoteActive)
                            {
                                const int vote = shopVoteChoiceAt(gamePoint);
                                if (vote >= 0)
                                    singleInput.purchaseVote = vote;
                                continue;
                            }

                            if (shopReadyAt(gamePoint))
                            {
                                singleInput.ready = true;
                                continue;
                            }

                            const int item = shopItemAt(gamePoint);
                            if (item >= 0)
                                singleInput.buy = item;
                        }
                    }
                }

                if (
                    const auto*
                    keyPressed =
                    event->getIf<
                    sf::Event::
                    KeyPressed>()
                    )
                {
                    const auto key =
                        keyPressed->code;

                    const auto scan =
                        keyPressed->scancode;

                    using K =
                        sf::Keyboard::Key;

                    using S =
                        sf::Keyboard::Scancode;

                    if (key == K::F10)
                    {
                        singleGameMode
                            .reset();

                        singleInput = {};

                        singleScreen =
                            dw::Screen{};

                        window.create(
                            sf::VideoMode({
                                1280,
                                720
                                }),

                            "Pirate Defense",

                            sf::State::Windowed
                        );

                        window.setFramerateLimit(
                            60
                        );

                        currentState =
                            GameState::
                            MainMenu;

                        continue;
                    }

                    if (key == K::F1)
                    {
                        singleScreen.help =
                            !singleScreen.help;

                        continue;
                    }

                    if (
                        singleScreen.ammoMenuVisible &&
                        (key == K::Escape || scan == S::Escape)
                        )
                    {
                        singleScreen.ammoMenuVisible = false;
                        continue;
                    }

                    // ESC: 싱글 게임 종료 후 메인 메뉴로 복귀
                    if (key == K::Escape || scan == S::Escape)
                    {
                        singleGameMode
                            .reset();

                        singleGame =
                            dw::Game{};

                        singleInput = {};

                        singleScreen =
                            dw::Screen{};

                        window.create(
                            sf::VideoMode({
                                1280,
                                720
                                }),

                            "Pirate Defense",

                            sf::State::Windowed
                        );

                        window.setFramerateLimit(
                            60
                        );

                        currentState =
                            GameState::MainMenu;

                        continue;
                    }

                    if (
                        key == K::Enter &&
                        (
                            singleGame.phase ==
                            dw::Phase::Lobby ||

                            singleGame.phase ==
                            dw::Phase::Won ||

                            singleGame.phase ==
                            dw::Phase::Lost
                            )
                        )
                    {
                        singleGameMode.start(
                            singleGame
                        );

                        singleInput = {};

                        singleScreen.paused =
                            false;

                        singleScreen.help =
                            false;

                        continue;
                    }

                    if (
                        singleScreen.help ||
                        singleScreen.paused
                        )
                    {
                        continue;
                    }

                    if (
                        singleGame.phase ==
                        dw::Phase::Shop
                        )
                    {
                        singleInput.shopVisit =
                            singleGame.shopVisit;

                        singleInput.purchaseVoteId =
                            singleGame
                            .purchaseVoteId;

                        // 상품/찬반 투표는 마우스로 진행합니다.
                        if (key == K::F || scan == S::F)
                        {
                            singleInput.sell = 1;
                        }
                        else if (
                            key == K::Enter ||
                            scan == S::Enter
                            )
                        {
                            singleInput.ready = true;
                        }

                        continue;
                    }

                    if (
                        key >= K::Num1 &&
                        key <= K::Num6
                        )
                    {
                        int number =
                            static_cast<int>(
                                key
                                )
                            -
                            static_cast<int>(
                                K::Num1
                                );

                        if (
                            singleGame.phase ==
                            dw::Phase::Reward
                            )
                        {
                            if (number < 3)
                            {
                                singleInput.vote =
                                    number;
                            }
                        }
                    }

                    if (key == K::E || scan == S::E)
                    {
                        if (
                            singleGame.phase == dw::Phase::Play &&
                            playerNearAmmoRack(singleGame, 0)
                            )
                        {
                            singleScreen.ammoMenuVisible =
                                !singleScreen.ammoMenuVisible;
                            singleInput.tap = false;
                            continue;
                        }

                        if (singleScreen.ammoMenuVisible)
                        {
                            singleScreen.ammoMenuVisible = false;
                            continue;
                        }

                        singleInput.tap = true;
                    }

                    if (key == K::Q || scan == S::Q)
                    {
                        singleInput.drop =
                            true;
                    }

                    if (key == K::Space)
                    {
                        singleInput.fire =
                            true;
                    }

                    if (key == K::C || scan == S::C)
                    {
                        singleInput.melee =
                            true;
                    }

                    // 유동호 병합: 물고기 1마리를 사용해 개인 체력 회복
                    if (key == K::H || scan == S::H)
                    {
                        singleInput.useFish =
                            true;
                    }
                }
            }

            else if (
                currentState ==
                GameState::MultiGame
                )
            {
                if (
                    const auto* mousePressed =
                    event->getIf<
                    sf::Event::MouseButtonPressed>()
                    )
                {
                    if (
                        mousePressed->button == sf::Mouse::Button::Left &&
                        !multiChatVisible
                        )
                    {
                        const sf::Vector2f gamePoint =
                            windowToGamePoint(
                                window,
                                mousePressed->position,
                                false
                            );

                        if (
                            multiGame.phase == dw::Phase::Play &&
                            multiScreen.ammoMenuVisible
                            )
                        {
                            const int shell = ammoItemAt(gamePoint);

                            if (shell >= 0)
                            {
                                multiInput.select = shell;
                                multiInput.tap = true;
                            }

                            multiScreen.ammoMenuVisible = false;
                            continue;
                        }

                        if (multiGame.phase == dw::Phase::Shop)
                        {
                            multiInput.shopVisit = multiGame.shopVisit;
                            multiInput.purchaseVoteId = multiGame.purchaseVoteId;

                            if (multiGame.purchaseVoteActive)
                            {
                                const int vote = shopVoteChoiceAt(gamePoint);
                                if (vote >= 0)
                                    multiInput.purchaseVote = vote;
                                continue;
                            }

                            if (shopReadyAt(gamePoint))
                            {
                                multiInput.ready = true;
                                continue;
                            }

                            const int item = shopItemAt(gamePoint);
                            if (item >= 0)
                                multiInput.buy = item;
                        }
                    }
                }

                if (multiChatVisible)
                {
                    if (
                        const auto* textEntered =
                        event->getIf<
                        sf::Event::TextEntered>()
                        )
                    {
                        const char32_t unicode =
                            textEntered->unicode;

                        if (unicode == 8)
                        {
                            if (!multiChatInput.isEmpty())
                            {
                                multiChatInput.erase(
                                    multiChatInput.getSize() - 1,
                                    1
                                );
                            }
                        }
                        else if (
                            unicode >= 32 &&
                            unicode != 127 &&
                            multiChatInput.getSize() < 120
                            )
                        {
                            multiChatInput += unicode;
                        }
                    }
                }

                if (
                    const auto*
                    keyPressed =
                    event->getIf<
                    sf::Event::
                    KeyPressed>()
                    )
                {
                    const auto key =
                        keyPressed->code;

                    const auto scan =
                        keyPressed->scancode;

                    using K =
                        sf::Keyboard::Key;

                    using S =
                        sf::Keyboard::Scancode;

                    if (key == K::Tab || scan == S::Tab)
                    {
                        multiChatVisible =
                            !multiChatVisible;

                        if (multiChatVisible)
                        {
                            multiScreen.ammoMenuVisible = false;
                        }

                        if (!multiChatVisible)
                        {
                            multiChatInput.clear();
                        }

                        multiInput.x = 0.f;
                        multiInput.y = 0.f;
                        multiInput.hold = false;
                        multiInput.brace = false;
                        multiInput.edgesOff();
                        continue;
                    }

                    if (multiChatVisible)
                    {
                        if ((key == K::Enter || scan == S::Enter) &&
                            !multiChatInput.isEmpty())
                        {
                            networkManager.sendChatMessage(
                                nickname,
                                multiChatInput
                            );

                            multiChatInput.clear();
                        }

                        // 채팅 입력 중에는 게임/상점 단축키가 동작하지 않습니다.
                        continue;
                    }

                    if (key == K::F10 || scan == S::F10)
                    {
                        multiGameMode.stop();

                        if (
                            networkManager
                            .isServerRunning()
                            )
                        {
                            networkManager
                                .stopServer();
                        }
                        else if (
                            networkManager
                            .isConnected()
                            )
                        {
                            networkManager
                                .disconnect();
                        }

                        multiGame =
                            dw::Game{};

                        multiInput = {};

                        multiScreen =
                            dw::Screen{};

                        currentRoomCode.clear();
                        multiChatVisible = false;
                        multiChatInput.clear();

                        window.create(
                            sf::VideoMode({
                                1280,
                                720
                                }),

                            "Pirate Defense",

                            sf::State::Windowed
                        );

                        window.setFramerateLimit(
                            60
                        );

                        currentState =
                            GameState::
                            MainMenu;

                        continue;
                    }

                    if (
                        multiScreen.ammoMenuVisible &&
                        (key == K::Escape || scan == S::Escape)
                        )
                    {
                        multiScreen.ammoMenuVisible = false;
                        continue;
                    }

                    // ESC: 멀티 게임 종료 후 메인 메뉴로 복귀
                    // 방장은 stopServer()에서 참가자에게 HOST_CLOSED를 전송
                    if (key == K::Escape || scan == S::Escape)
                    {
                        multiGameMode.stop();

                        if (
                            networkManager
                            .isServerRunning()
                            )
                        {
                            networkManager
                                .stopServer();
                        }
                        else if (
                            networkManager
                            .isConnected()
                            )
                        {
                            networkManager
                                .disconnect();
                        }

                        multiGame =
                            dw::Game{};

                        multiInput = {};

                        multiScreen =
                            dw::Screen{};

                        currentRoomCode.clear();
                        multiChatVisible = false;
                        multiChatInput.clear();

                        window.create(
                            sf::VideoMode({
                                1280,
                                720
                                }),

                            "Pirate Defense",

                            sf::State::Windowed
                        );

                        window.setFramerateLimit(
                            60
                        );

                        currentState =
                            GameState::MainMenu;

                        continue;
                    }

                    if (key == K::F1 || scan == S::F1)
                    {
                        multiScreen.help =
                            !multiScreen.help;

                        continue;
                    }

                    if (multiScreen.help)
                    {
                        continue;
                    }

                    if (
                        (key == K::Enter || scan == S::Enter) &&
                        (
                            multiGame.phase ==
                            dw::Phase::Won ||

                            multiGame.phase ==
                            dw::Phase::Lost
                            )
                        )
                    {
                        multiGameMode.restart(
                            multiGame
                        );

                        multiInput = {};

                        continue;
                    }

                    if (
                        multiGame.phase ==
                        dw::Phase::Shop
                        )
                    {
                        multiInput.shopVisit =
                            multiGame.shopVisit;

                        multiInput.purchaseVoteId =
                            multiGame
                            .purchaseVoteId;

                        // 상품/찬반 투표는 마우스로 진행합니다.
                        if (key == K::F || scan == S::F)
                        {
                            multiInput.sell = 1;
                        }
                        else if (
                            key == K::Enter ||
                            scan == S::Enter
                            )
                        {
                            multiInput.ready = true;
                        }

                        continue;
                    }

                    if (
                        key >= K::Num1 &&
                        key <= K::Num6
                        )
                    {
                        int number =
                            static_cast<int>(
                                key
                                )
                            -
                            static_cast<int>(
                                K::Num1
                                );

                        if (
                            multiGame.phase ==
                            dw::Phase::Reward
                            )
                        {
                            if (number < 3)
                            {
                                multiInput.vote =
                                    number;
                            }
                        }
                    }

                    if (key == K::E || scan == S::E)
                    {
                        const int localId =
                            std::clamp(
                                multiScreen.local,
                                0,
                                dw::MaxPlayers - 1
                            );

                        if (
                            multiGame.phase == dw::Phase::Play &&
                            playerNearAmmoRack(multiGame, localId)
                            )
                        {
                            multiScreen.ammoMenuVisible =
                                !multiScreen.ammoMenuVisible;
                            multiInput.tap = false;
                            continue;
                        }

                        if (multiScreen.ammoMenuVisible)
                        {
                            multiScreen.ammoMenuVisible = false;
                            continue;
                        }

                        multiInput.tap = true;
                    }

                    if (key == K::Q || scan == S::Q)
                    {
                        multiInput.drop =
                            true;
                    }

                    if (key == K::Space)
                    {
                        multiInput.fire =
                            true;
                    }

                    if (key == K::C || scan == S::C)
                    {
                        multiInput.melee =
                            true;
                    }

                    // 유동호 병합: 물고기 1마리를 사용해 개인 체력 회복
                    if (key == K::H || scan == S::H)
                    {
                        multiInput.useFish =
                            true;
                    }
                }
            }
        }

        if (
            networkManager
            .isServerRunning()
            )
        {
            networkManager
                .updateServer();

            networkManager
                .broadcastPlayerList(
                    nickname
                );
        }

        if (
            networkManager
            .isConnected()
            )
        {
            networkManager
                .updateClient();

            if (
                networkManager
                .isHostClosed()
                )
            {
                // 방장이 나가도 클라이언트 창은 닫지 않고 메인 메뉴로 복귀
                multiGameMode.stop();
                networkManager.disconnect();

                multiGame = dw::Game{};
                multiInput = {};
                multiScreen = dw::Screen{};
                currentRoomCode.clear();

                window.create(
                    sf::VideoMode({ 1280, 720 }),
                    "Pirate Defense",
                    sf::State::Windowed
                );
                window.setFramerateLimit(60);
                currentState = GameState::MainMenu;

                MessageBoxW(
                    nullptr,
                    L"방장이 나갔습니다. 메인화면으로 돌아갑니다.",
                    L"방장 연결 종료",
                    MB_OK |
                    MB_ICONINFORMATION
                );

                continue;
            }

            if (
                networkManager
                .isKicked()
                )
            {
                MessageBoxW(
                    nullptr,
                    L"방장에 의해 방에서 내보내졌습니다.",
                    L"내보내기",
                    MB_OK |
                    MB_ICONWARNING
                );

                networkManager
                    .disconnect();

                currentRoomCode.clear();

                currentState =
                    GameState::MainMenu;
            }

            else if (
                networkManager
                .isJoinRejected()
                )
            {
                MessageBoxW(
                    nullptr,
                    L"방 코드가 올바르지 않습니다.",
                    L"입장 실패",
                    MB_OK |
                    MB_ICONWARNING
                );

                networkManager
                    .disconnect();

                currentRoomCode.clear();
            }

            else if (
                networkManager
                .isRoomFull()
                )
            {
                MessageBoxW(
                    nullptr,
                    L"방 인원이 가득 찼습니다.",
                    L"입장 실패",
                    MB_OK |
                    MB_ICONWARNING
                );

                networkManager
                    .disconnect();

                currentRoomCode.clear();
            }

            else if (
                networkManager
                .isGameStarted()
                )
            {
                multiGame =
                    dw::Game{};

                multiInput = {};

                multiScreen =
                    dw::Screen{};

                multiScreen.menu =
                    false;

                multiScreen.help =
                    false;

                multiScreen.paused =
                    false;

                multiGameMode.start(
                    multiGame
                );

                multiGameMode.updateScreen(
                    multiScreen
                );

                multiGameClock.restart();

                window.create(
                    sf::VideoMode::
                    getFullscreenModes()
                    .front(),

                    "Pirate Defense",

                    sf::State::Fullscreen
                );

                window.setFramerateLimit(
                    60
                );

                currentState =
                    GameState::
                    MultiGame;
            }

            else if (
                networkManager
                .isJoinAccepted()
                )
            {
                currentState =
                    GameState::Lobby;
            }
        }

        if (
            networkManager
            .isHostClosed()
            )
        {
            // 비정상 연결 종료/방장 강제 종료도 동일하게 메인 메뉴로 복귀
            multiGameMode.stop();
            networkManager.disconnect();

            multiGame = dw::Game{};
            multiInput = {};
            multiScreen = dw::Screen{};
            currentRoomCode.clear();

            window.create(
                sf::VideoMode({ 1280, 720 }),
                "Pirate Defense",
                sf::State::Windowed
            );
            window.setFramerateLimit(60);
            currentState = GameState::MainMenu;

            MessageBoxW(
                nullptr,
                L"방장이 나갔습니다. 메인화면으로 돌아갑니다.",
                L"방장 연결 종료",
                MB_OK |
                MB_ICONINFORMATION
            );

            continue;
        }

        if (
            currentState ==
            GameState::Lobby
            )
        {
            std::vector<sf::String>
                displayNames;

            if (
                networkManager
                .isServerRunning()
                )
            {
                displayNames.push_back(
                    nickname
                );

                const auto&
                    receivedNames =
                    networkManager
                    .getPlayerNicknames();

                for (
                    const auto&
                    playerName :
                    receivedNames
                    )
                {
                    if (
                        !playerName
                        .isEmpty()
                        )
                    {
                        displayNames
                            .push_back(
                                playerName
                            );
                    }
                }
            }
            else
            {
                const auto&
                    syncedNames =
                    networkManager
                    .getSyncedPlayerNicknames();

                if (
                    !syncedNames.empty()
                    )
                {
                    for (
                        const auto&
                        name :
                        syncedNames
                        )
                    {
                        displayNames
                            .push_back(
                                name
                            );
                    }
                }
                else
                {
                    displayNames
                        .push_back(
                            nickname
                        );
                }
            }

            lobbyVisiblePlayerCount =
                displayNames.size();

            sf::String roomNameDisplay =
                U"방 이름 : ";

            if (
                networkManager
                .isServerRunning()
                )
            {
                roomNameDisplay +=
                    networkManager
                    .getRoomName();
            }
            else
            {
                roomNameDisplay +=
                    networkManager
                    .getSyncedRoomName();
            }

            lobbyRoomNameText.setString(
                roomNameDisplay
            );

            lobbyRoomNameText
                .setCharacterSize(
                    16
                );

            centerText(
                lobbyRoomNameText,
                425.f,
                207.f,
                190.f,
                30.f
            );

            sf::String roomCodeDisplay =
                U"방 코드 : ";

            roomCodeDisplay +=
                sf::String::fromUtf8(
                    currentRoomCode.begin(),
                    currentRoomCode.end()
                );

            lobbyRoomCodeText.setString(
                roomCodeDisplay
            );

            lobbyRoomCodeText
                .setCharacterSize(
                    16
                );

            centerText(
                lobbyRoomCodeText,
                665.f,
                207.f,
                190.f,
                30.f
            );

            const sf::Vector2f
                lobbyNamePositions[5] =
            {
                { 505.f, 266.f },
                { 505.f, 328.f },
                { 505.f, 390.f },
                { 505.f, 452.f },
                { 505.f, 514.f }
            };

            for (
                std::size_t i = 0;
                i <
                lobbyPlayerTexts.size();
                ++i
                )
            {
                if (
                    i <
                    displayNames.size()
                    )
                {
                    lobbyPlayerTexts[i]
                        .setString(
                            displayNames[i]
                        );

                    lobbyPlayerTexts[i]
                        .setPosition(
                            lobbyNamePositions[i]
                        );
                }
                else
                {
                    lobbyPlayerTexts[i]
                        .setString("");
                }
            }
        }

        if (
            currentState ==
            GameState::SinglePlayer
            )
        {
            float elapsed =
                std::min(
                    singleGameClock
                    .restart()
                    .asSeconds(),

                    0.25f
                );

            // 물리 키 위치 기준 입력: 한/영이 한글 상태여도 조작 가능
            auto keyDown =
                [](
                    sf::Keyboard::Scancode key
                    )
                {
                    return
                        sf::Keyboard::
                        isKeyPressed(key);
                };

            bool control =
                window.hasFocus() &&
                !singleScreen.menu &&
                !singleScreen.help &&
                !singleScreen.paused &&
                !singleScreen.ammoMenuVisible;

            singleInput.shopVisit =
                singleGame.shopVisit;

            singleInput.purchaseVoteId =
                singleGame
                .purchaseVoteId;

            sf::Vector2i gameMouse =
                sf::Mouse::
                getPosition(window);

            auto windowSize =
                window.getSize();

            float gameScale =
                std::min(
                    windowSize.x /
                    640.f,

                    windowSize.y /
                    360.f
                );

            if (gameScale >= 1.f)
            {
                gameScale =
                    std::floor(
                        gameScale
                    );
            }

            float gameOffsetX =
                (
                    windowSize.x -
                    640.f *
                    gameScale
                    )
                / 2.f;

            float gameOffsetY =
                (
                    windowSize.y -
                    360.f *
                    gameScale
                    )
                / 2.f;

            singleInput.aim =
            {
                (
                    static_cast<float>(
                        gameMouse.x
                    )
                    -
                    gameOffsetX
                )
                /
                gameScale,

                (
                    static_cast<float>(
                        gameMouse.y
                    )
                    -
                    gameOffsetY
                )
                /
                gameScale
            };

            singleInput.x =
                control
                ?
                static_cast<float>(
                    keyDown(
                        sf::Keyboard::
                        Scancode::D
                    )
                    )
                -
                static_cast<float>(
                    keyDown(
                        sf::Keyboard::
                        Scancode::A
                    )
                    )
                :
                0.f;

            singleInput.y =
                control
                ?
                static_cast<float>(
                    keyDown(
                        sf::Keyboard::
                        Scancode::S
                    )
                    )
                -
                static_cast<float>(
                    keyDown(
                        sf::Keyboard::
                        Scancode::W
                    )
                    )
                :
                0.f;

            singleInput.hold =
                control &&
                keyDown(
                    sf::Keyboard::
                    Scancode::E
                );

            singleInput.brace =
                control &&
                keyDown(
                    sf::Keyboard::
                    Scancode::C
                );

            if (!control)
            {
                singleInput.brace =
                    false;

                singleInput
                    .edgesOff();
            }

            singleGameMode.update(
                singleGame,
                singleInput,
                elapsed,
                control
            );
        }
        else
        {
            singleGameClock.restart();
        }

        if (
            currentState ==
            GameState::MultiGame
            )
        {
            float elapsed =
                std::min(
                    multiGameClock
                    .restart()
                    .asSeconds(),

                    0.25f
                );

            // 물리 키 위치 기준 입력: 한/영이 한글 상태여도 조작 가능
            auto keyDown =
                [](
                    sf::Keyboard::Scancode key
                    )
                {
                    return
                        sf::Keyboard::
                        isKeyPressed(key);
                };

            bool control =
                window.hasFocus() &&
                !multiScreen.help &&
                !multiScreen.paused &&
                !multiChatVisible &&
                !multiScreen.ammoMenuVisible;

            multiInput.shopVisit =
                multiGame.shopVisit;

            multiInput.purchaseVoteId =
                multiGame
                .purchaseVoteId;

            sf::Vector2i gameMouse =
                sf::Mouse::
                getPosition(window);

            auto windowSize =
                window.getSize();

            float gameScale =
                std::min(
                    windowSize.x /
                    640.f,

                    windowSize.y /
                    360.f
                );

            if (gameScale <= 0.f)
            {
                gameScale = 1.f;
            }

            float gameOffsetX =
                (
                    windowSize.x -
                    640.f *
                    gameScale
                    )
                / 2.f;

            float gameOffsetY =
                (
                    windowSize.y -
                    360.f *
                    gameScale
                    )
                / 2.f;

            multiInput.aim =
            {
                (
                    static_cast<float>(
                        gameMouse.x
                    )
                    -
                    gameOffsetX
                )
                /
                gameScale,

                (
                    static_cast<float>(
                        gameMouse.y
                    )
                    -
                    gameOffsetY
                )
                /
                gameScale
            };

            multiInput.x =
                control
                ?
                static_cast<float>(
                    keyDown(
                        sf::Keyboard::
                        Scancode::D
                    )
                    )
                -
                static_cast<float>(
                    keyDown(
                        sf::Keyboard::
                        Scancode::A
                    )
                    )
                :
                0.f;

            multiInput.y =
                control
                ?
                static_cast<float>(
                    keyDown(
                        sf::Keyboard::
                        Scancode::S
                    )
                    )
                -
                static_cast<float>(
                    keyDown(
                        sf::Keyboard::
                        Scancode::W
                    )
                    )
                :
                0.f;

            multiInput.hold =
                control &&
                keyDown(
                    sf::Keyboard::
                    Scancode::E
                );

            multiInput.brace =
                control &&
                keyDown(
                    sf::Keyboard::
                    Scancode::C
                );

            if (!control)
            {
                multiInput.x = 0.f;
                multiInput.y = 0.f;

                multiInput.hold =
                    false;

                multiInput.brace =
                    false;

                multiInput
                    .edgesOff();
            }

            multiGameMode.update(
                multiGame,
                multiInput,
                elapsed
            );

            multiGameMode.updateScreen(
                multiScreen
            );

            // 플레이어 번호 대신 실제 닉네임을 캐릭터 머리 위/HUD에 표시합니다.
            multiScreen.playerNames.fill("");

            if (networkManager.isServerRunning())
            {
                multiScreen.playerNames[0] =
                    toUtf8String(nickname);

                int id = 1;
                for (const auto& name :
                    networkManager.getPlayerNicknames())
                {
                    if (!name.isEmpty() &&
                        id < dw::MaxPlayers)
                    {
                        multiScreen.playerNames[id++] =
                            toUtf8String(name);
                    }
                }
            }
            else
            {
                const auto& names =
                    networkManager.getSyncedPlayerNicknames();

                for (std::size_t i = 0;
                    i < names.size() &&
                    i < static_cast<std::size_t>(dw::MaxPlayers);
                    ++i)
                {
                    multiScreen.playerNames[i] =
                        toUtf8String(names[i]);
                }
            }

            multiScreen.chatVisible =
                multiChatVisible;
            multiScreen.chatInput =
                toUtf8String(multiChatInput);
            multiScreen.chatLines.clear();

            for (const auto& chat :
                networkManager.getChatMessages())
            {
                multiScreen.chatLines.push_back(
                    toUtf8String(chat.sender) +
                    ": " +
                    toUtf8String(chat.message)
                );
            }
        }
        else
        {
            multiGameClock.restart();
        }

        singleScreen.playerNames.fill("");
        singleScreen.playerNames[0] =
            toUtf8String(nickname);

        sf::Vector2i mousePixel =
            sf::Mouse::getPosition(
                window
            );

        sf::Vector2f mousePos(
            static_cast<float>(
                mousePixel.x
                ),
            static_cast<float>(
                mousePixel.y
                )
        );

        window.clear();

        if (
            currentState ==
            GameState::NicknameInput
            )
        {
            window.draw(
                nicknameBackground
            );

            leftCenterText(
                nicknameText,
                468.f,
                354.f,
                68.f
            );

            window.draw(
                nicknameText
            );

            if (nicknameInputActive)
            {
                sf::FloatRect textBounds =
                    nicknameText
                    .getLocalBounds();

                nicknameCursor.setPosition({
                    nicknameText
                    .getPosition().x
                    +
                    textBounds.position.x
                    +
                    textBounds.size.x
                    +
                    3.f,

                    375.f
                    });

                int blink =
                    static_cast<int>(
                        nicknameCursorClock
                        .getElapsedTime()
                        .asSeconds()
                        *
                        2
                        )
                    %
                    2;

                if (blink == 0)
                {
                    window.draw(
                        nicknameCursor
                    );
                }
            }

            drawHover(
                window,
                buttonHover,
                confirmButton,
                mousePos
            );

            window.draw(
                nicknameInputArea
            );

            window.draw(
                confirmButton
            );
        }

        else if (
            currentState ==
            GameState::MainMenu
            )
        {
            window.draw(
                mainMenuBackground
            );

            window.draw(
                welcomeText
            );

            drawHover(
                window,
                buttonHover,
                createRoomButton,
                mousePos
            );

            drawHover(
                window,
                buttonHover,
                joinRoomButton,
                mousePos
            );

            drawHover(
                window,
                buttonHover,
                singleButton,
                mousePos
            );

            drawHover(
                window,
                buttonHover,
                exitButton,
                mousePos
            );

            drawHover(
                window,
                buttonHover,
                mainBackButton,
                mousePos
            );

            window.draw(
                createRoomButton
            );

            window.draw(
                joinRoomButton
            );

            window.draw(
                singleButton
            );

            window.draw(
                exitButton
            );

            window.draw(
                mainBackButton
            );

            drawHover(
                window,
                buttonHover,
                mainHelpButton,
                mousePos
            );

            window.draw(mainHelpButton);
            centerText(
                mainHelpButtonText,
                1045.f,
                622.f,
                190.f,
                54.f
            );
            window.draw(mainHelpButtonText);

            if (mainHelpVisible)
            {
                sf::RectangleShape shade({ 1280.f, 720.f });
                shade.setFillColor(sf::Color(0, 0, 0, 150));
                window.draw(shade);

                sf::RectangleShape helpPanel({ 720.f, 470.f });
                helpPanel.setPosition({ 280.f, 120.f });
                helpPanel.setFillColor(sf::Color(15, 31, 43, 245));
                helpPanel.setOutlineThickness(2.f);
                helpPanel.setOutlineColor(sf::Color(205, 177, 118));
                window.draw(helpPanel);

                auto drawHelpLine = [&](const wchar_t* line, float y, unsigned size = 20)
                    {
                        sf::Text t(font);
                        t.setString(sf::String(line));
                        t.setCharacterSize(size);
                        t.setFillColor(sf::Color(235, 238, 232));
                        t.setPosition({ 325.f, y });
                        window.draw(t);
                    };

                drawHelpLine(L"조작법", 145.f, 30);
                drawHelpLine(L"WASD : 이동", 200.f);
                drawHelpLine(L"E : 상호작용 / 탄약고 열기 / 낚시 시작", 238.f);
                drawHelpLine(L"탄약고 : E → 마우스로 원하는 포탄 클릭", 276.f);
                drawHelpLine(L"대포 : E 길게 장전 → 마우스 조준 → Space 발사", 314.f);
                drawHelpLine(L"Q : 들고 있는 포탄 내려놓기", 352.f);
                drawHelpLine(L"C : 근접 공격 / 큰 파도 방어", 390.f);
                drawHelpLine(L"H : 물고기 사용(체력 회복)", 428.f);
                drawHelpLine(L"Tab : 실시간 채팅 열기/ 숨기기", 466.f);
                drawHelpLine(L"F1 : 게임 중 조작법 / Esc : 도움말 종료", 520.f, 17);
            }
        }

        else if (
            currentState ==
            GameState::CreateRoom
            )
        {
            window.draw(
                createRoomBackground
            );

            leftCenterText(
                roomNameText,
                580.f,
                278.f,
                55.f
            );

            window.draw(
                roomNameText
            );

            window.draw(
                roomNameInputArea
            );

            drawHover(
                window,
                buttonHover,
                roomCodeCopyArea,
                mousePos
            );

            centerText(
                roomCodeText,
                371.f,
                505.f,
                350.f,
                56.f
            );

            window.draw(
                roomCodeText
            );

            window.draw(
                roomCodeCopyArea
            );

            drawHover(
                window,
                buttonHover,
                player2Button,
                mousePos
            );

            drawHover(
                window,
                buttonHover,
                player3Button,
                mousePos
            );

            drawHover(
                window,
                buttonHover,
                player4Button,
                mousePos
            );

            drawHover(
                window,
                buttonHover,
                player5Button,
                mousePos
            );

            drawHover(
                window,
                buttonHover,
                regenerateButton,
                mousePos
            );

            drawHover(
                window,
                buttonHover,
                finalCreateRoomButton,
                mousePos
            );

            drawHover(
                window,
                buttonHover,
                createBackButton,
                mousePos
            );

            if (selectedPlayers == 2)
            {
                selectedHighlight.setSize(
                    player2Button.getSize()
                );

                selectedHighlight.setPosition(
                    player2Button.getPosition()
                );

                window.draw(
                    selectedHighlight
                );
            }

            else if (
                selectedPlayers == 3
                )
            {
                selectedHighlight.setSize(
                    player3Button.getSize()
                );

                selectedHighlight.setPosition(
                    player3Button.getPosition()
                );

                window.draw(
                    selectedHighlight
                );
            }

            else if (
                selectedPlayers == 4
                )
            {
                selectedHighlight.setSize(
                    player4Button.getSize()
                );

                selectedHighlight.setPosition(
                    player4Button.getPosition()
                );

                window.draw(
                    selectedHighlight
                );
            }

            else if (
                selectedPlayers == 5
                )
            {
                selectedHighlight.setSize(
                    player5Button.getSize()
                );

                selectedHighlight.setPosition(
                    player5Button.getPosition()
                );

                window.draw(
                    selectedHighlight
                );
            }

            window.draw(
                player2Button
            );

            window.draw(
                player3Button
            );

            window.draw(
                player4Button
            );

            window.draw(
                player5Button
            );

            window.draw(
                regenerateButton
            );

            window.draw(
                finalCreateRoomButton
            );

            window.draw(
                createBackButton
            );
        }

        else if (
            currentState ==
            GameState::JoinRoom
            )
        {
            window.draw(
                joinRoomBackground
            );

            leftCenterText(
                joinCodeText,
                472.f,
                389.f,
                68.f
            );

            window.draw(
                joinCodeText
            );

            drawHover(
                window,
                buttonHover,
                joinConfirmButton,
                mousePos
            );

            drawHover(
                window,
                buttonHover,
                joinBackButton,
                mousePos
            );

            window.draw(
                joinInputArea
            );

            window.draw(
                joinConfirmButton
            );

            window.draw(
                joinBackButton
            );
        }

        else if (
            currentState ==
            GameState::Lobby
            )
        {
            window.draw(
                lobbyBackground
            );

            window.draw(
                lobbyRoomNameText
            );

            drawHover(
                window,
                buttonHover,
                lobbyRoomCodeCopyArea,
                mousePos
            );

            window.draw(
                lobbyRoomCodeText
            );

            window.draw(
                lobbyRoomCodeCopyArea
            );

            drawHover(
                window,
                buttonHover,
                lobbyStartButton,
                mousePos
            );

            drawHover(
                window,
                buttonHover,
                lobbyBackButton,
                mousePos
            );

            if (
                networkManager
                .isServerRunning()
                )
            {
                for (
                    std::size_t i = 0;
                    i <
                    lobbyKickButtons.size();
                    ++i
                    )
                {
                    if (
                        i + 1 <
                        lobbyVisiblePlayerCount
                        )
                    {
                        drawHover(
                            window,
                            buttonHover,
                            lobbyKickButtons[i],
                            mousePos
                        );
                    }
                }
            }

            window.draw(
                lobbyStartButton
            );

            window.draw(
                lobbyBackButton
            );

            for (
                const auto& kickButton :
                lobbyKickButtons
                )
            {
                window.draw(
                    kickButton
                );
            }

            for (
                auto& playerText :
                lobbyPlayerTexts
                )
            {
                window.draw(
                    playerText
                );
            }
        }

        else if (
            currentState ==
            GameState::MultiGame
            )
        {
            multiRenderer.draw(
                multiCanvas,
                multiGame,
                multiScreen
            );

            multiCanvas.display();

            sf::Sprite multiGameSprite(
                multiCanvas.getTexture()
            );

            auto windowSize =
                window.getSize();

            float scale =
                std::min(
                    windowSize.x /
                    640.f,

                    windowSize.y /
                    360.f
                );

            float offsetX =
                (
                    windowSize.x -
                    640.f *
                    scale
                    )
                / 2.f;

            float offsetY =
                (
                    windowSize.y -
                    360.f *
                    scale
                    )
                / 2.f;

            multiGameSprite.setScale({
                scale,
                scale
                });

            multiGameSprite.setPosition({
                offsetX,
                offsetY
                });

            window.draw(
                multiGameSprite
            );
        }

        else if (
            currentState ==
            GameState::SinglePlayer
            )
        {
            singleRenderer.draw(
                singleCanvas,
                singleGame,
                singleScreen
            );

            singleCanvas.display();

            sf::Sprite singleGameSprite(
                singleCanvas.getTexture()
            );

            auto windowSize =
                window.getSize();

            float scale =
                std::min(
                    windowSize.x /
                    640.f,

                    windowSize.y /
                    360.f
                );

            if (scale >= 1.f)
            {
                scale =
                    std::floor(scale);
            }

            float offsetX =
                (
                    windowSize.x -
                    640.f *
                    scale
                    )
                / 2.f;

            float offsetY =
                (
                    windowSize.y -
                    360.f *
                    scale
                    )
                / 2.f;

            singleGameSprite.setScale({
                scale,
                scale
                });

            singleGameSprite.setPosition({
                offsetX,
                offsetY
                });

            window.draw(
                singleGameSprite
            );
        }

        window.display();
    }

    return 0;
}