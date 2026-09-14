#pragma once
#include "Game.hpp"
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <memory>
#include <cmath>
#include <cstdint>
#include <stdexcept>
#include <unordered_map>
#include <algorithm>
#include <array>
#include <string>
#include <vector>

namespace dw
{
    inline sf::Color shellColor(int n)
    {
        static const sf::Color c[] = {
            {222, 221, 190},
            {255, 169, 76},
            {198, 161, 236},
            {251, 100, 60},
            {117, 217, 237}
        };

        return n >= 0 && n < 5
            ? c[n]
            : sf::Color(93, 112, 124);
    }

    inline sf::Color playerColor(int n)
    {
        static const sf::Color c[] = {
            {70, 193, 194},
            {247, 151, 99},
            {167, 147, 234},
            {237, 202, 91},
            {218, 119, 170}
        };

        return c[std::clamp(n, 0, 4)];
    }

    struct Screen
    {
        bool menu = true;
        bool help = false;
        bool paused = false;
        bool editing = false;
        bool muted = false;
        bool client = false;
        bool online = false;

        int local = 0;

        // 메인에서 네트워크 닉네임/채팅 상태를 채워 렌더러에 전달합니다.
        std::array<std::string, MaxPlayers> playerNames{};
        bool chatVisible = false;
        std::string chatInput;
        std::vector<std::string> chatLines;
        bool ammoMenuVisible = false;

        std::string endpoint =
            "127.0.0.1:53000";

        std::string status = "";
        std::string address = "";
    };

    class Renderer
    {
        sf::Font font;

        std::vector<sf::Vertex>
            rectangles;

        struct TextSlot
        {
            std::string original;
            std::unique_ptr<sf::Text> text;
        };

        std::vector<TextSlot> labels;

        std::size_t labelIndex = 0;

        void flush(
            sf::RenderTarget& r)
        {
            if (!rectangles.empty())
            {
                r.draw(
                    rectangles.data(),
                    rectangles.size(),
                    sf::PrimitiveType::Triangles
                );

                rectangles.clear();
            }
        }

    public:
        Renderer()
        {
            if (!font.openFromFile(
                "C:/Windows/Fonts/malgun.ttf"))
            {
                throw std::runtime_error(
                    "Korean font unavailable"
                );
            }

            font.setSmooth(true);
        }

        void box(
            sf::RenderTarget& r,
            float x,
            float y,
            float w,
            float h,
            sf::Color c)
        {
            x = std::floor(x);
            y = std::floor(y);

            const sf::Vertex a{
                {x, y},
                c
            };

            const sf::Vertex b{
                {x + w, y},
                c
            };

            const sf::Vertex d{
                {x, y + h},
                c
            };

            const sf::Vertex e{
                {x + w, y + h},
                c
            };

            rectangles.insert(
                rectangles.end(),
                {
                    a,
                    b,
                    d,
                    d,
                    b,
                    e
                }
            );
        }

        std::string koreanText(
            std::string value)
        {
            static std::unordered_map<
                std::string,
                std::string>
                cache;

            const auto found =
                cache.find(value);

            if (found != cache.end())
            {
                return found->second;
            }

            const std::string
                originalText = value;

            static const char*
                translations[][2] =
            {
                {
                    "PAUSED",
                    "일시정지"
                },

                {
                    "CAPTAIN'S HANDBOOK",
                    "조작 방법"
                },

                {
                    "WASD move  /  E interact  /  Q put down shell",
                    "WASD 이동 / E 상호작용 / Q 포탄 내려놓기"
                },

                {
                    "At the ammo rack press E, then click the shell you want",
                    "탄약고에서 E → 마우스로 원하는 포탄 클릭"
                },

                {
                    "Hold E beside a cannon for 1 second to load",
                    "함포 옆에서 E를 1초 누르면 장전"
                },

                {
                    "After loading, aim with MOUSE and press SPACE to fire.",
                    "장전 후 마우스로 조준하고 Space로 발사"
                },

                {
                    "Hold E at damage sites: repair / extinguish",
                    "파손·화재 위치에서 E를 길게 눌러 수리·소화"
                },

                {
                    "Survive 90 sec each stage; kill the final sea monster.",
                    "각 스테이지 90초 생존 후 마지막 바다 괴물 처치"
                },

                {
                    "Online match keeps running while help is open.",
                    "멀티플레이는 도움말을 열어도 계속 진행됩니다."
                },

                {
                    "Solo play pauses when unfocused or help is open.",
                    "혼자 할 때는 도움말·다른 창 전환 시 일시정지됩니다."
                },

                {
                    "C melee/brace   H eat fish   TAB chat",
                    "C 근접공격/방어 / H 물고기 사용 / Tab 채팅"
                },

                {
                    "F1 close help   ESC main menu   M mute",
                    "F1 도움말 닫기 / Esc 메인 메뉴 / M 효과음"
                },

                {
                    "CREW ASSEMBLY",
                    "출항 대기실"
                },

                {
                    "Connected: ",
                    "접속 인원: "
                },

                {
                    "Waiting for host to start...",
                    "방장의 시작을 기다리는 중..."
                },

                {
                    "ENTER: depart with current crew",
                    "Enter: 현재 인원으로 시작"
                },

                {
                    "ESC: return to main menu",
                    "Esc: 메인 메뉴로 돌아가기"
                },

                {
                    "STAGE CLEAR - CHOOSE A CREW RELIC",
                    "스테이지 완료! 팀 보상을 선택하세요"
                },

                {
                    "Each player votes. Majority wins; ties favor the left card.",
                    "모두 투표하면 다수결로 결정됩니다. 동률이면 왼쪽 보상!"
                },

                {
                    "1  REINFORCED HULL",
                    "1  강화 선체"
                },

                {
                    "2  REPAIR KIT",
                    "2  수리 도구"
                },

                {
                    "3  WATER SEAL",
                    "3  방수 처리"
                },

                {
                    "+150 max HP / heal 250",
                    "최대 체력 +150 / 회복 250"
                },

                {
                    "+25 repair heal / heal 160",
                    "수리 회복 +25 / 회복 160"
                },

                {
                    "35% less leaks / heal 160",
                    "지속 피해 -35% / 회복 160"
                },

                {
                    "VOTES ",
                    "투표 "
                },

                {
                    "Press 1, 2 or 3. No shop. Cannon damage / load time stay fixed.",
                    "1~3으로 선택하세요. 함포 공격력과 장전 시간은 유지됩니다."
                },

                {
                    "THE SEA IS YOURS",
                    "최종 승리!"
                },

                {
                    "SHIP LOST",
                    "배가 침몰했습니다"
                },

                {
                    "All three stages complete. The monster is defeated.",
                    "모든 스테이지 완료! 바다 괴물을 처치했습니다."
                },

                {
                    "The hull reached zero. Your crew can try again.",
                    "배의 체력이 바닥났습니다. 다시 도전해 보세요."
                },

                {
                    "Enemies sunk: ",
                    "격침한 적: "
                },

                {
                    "Waiting for host to restart",
                    "방장의 재시작을 기다리는 중"
                },

                {
                    "ENTER: restart from stage 1",
                    "Enter: 1스테이지부터 재시작"
                },

                {
                    "ESC: main menu",
                    "Esc: 메인 메뉴"
                },

                {
                    "HOST DISCONNECTED",
                    "방장과 연결이 끊겼습니다"
                },

                {
                    "Room closed. ESC returns to the menu.",
                    "방이 닫혔습니다. Esc를 눌러 메뉴로 돌아가세요."
                },

                {
                    "Host disconnected / room closed",
                    "연결 종료 / 방이 닫혔습니다"
                },

                {
                    "Port is in use or blocked",
                    "포트가 사용 중이거나 차단되어 있습니다"
                },

                {
                    "Enter an IPv4 address, e.g. 192.168.0.10",
                    "IPv4 주소를 입력하세요. 예: 192.168.0.10"
                },

                {
                    "Connection failed. Check IP, port and host.",
                    "접속 실패: 방장의 IP와 포트, 방 생성 여부를 확인하세요"
                },

                {
                    "Connecting...",
                    "접속 중..."
                },

                {
                    "CONNECTED / P",
                    "접속 완료 / P"
                },

                {
                    "HOST TCP :",
                    "방장 포트: "
                },

                {
                    "Invalid port (1-65535)",
                    "포트는 1~65535 사이로 입력하세요"
                },

                {
                    "LAN IP: ",
                    "내부 IP: "
                },

                {
                    "HOST: ",
                    "방장 주소: "
                },

                {
                    "unknown",
                    "확인 불가"
                },

                {
                    "MOUSE aim / SPACE fire",
                    "마우스 조준 / Space 발사"
                },

                {
                    "Hold E to fish",
                    "E 길게 누르기: 낚시"
                },

                {
                    "Press E to start fishing",
                    "E 한 번 누르기: 낚시 시작"
                },

                {
                    "Hold E to repair / extinguish",
                    "E 길게 누르기: 수리·소화"
                },

                {
                    "Hold E: load for 1 second",
                    "E 1초 누르기: 장전"
                },

                {
                    "E: open ammo rack / click shell",
                    "E: 탄약고 열기 / 마우스로 포탄 선택"
                },

                {
                    "WASD move / E interact / Q drop",
                    "WASD 이동 / E 상호작용 / Q 내려놓기"
                },

                {
                    "E hold: repair / extinguish",
                    "E 길게: 수리·소화"
                },

                {
                    "F1 help   M sound   F11 screen",
                    "F1 도움말 / M 효과음 / F11 화면"
                },

                {
                    "! ATTACK !",
                    "! 공격 예고 !"
                },

                {
                    "TENTACLE SLAM",
                    "촉수 강타"
                },

                {
                    "FIRE SURGE",
                    "화염 분출"
                },

                {
                    "THE ABYSS",
                    "심해 괴물"
                },

                {
                    "BOSS FIGHT",
                    "보스 전투"
                },

                {
                    " READY",
                    " 장전"
                },

                {
                    " EMPTY",
                    " 비었음"
                },

                {
                    "AMMO",
                    "탄약고"
                },

                {
                    "FISHING",
                    "낚시"
                },

                {
                    "DECKWATCH",
                    "해상 방어전"
                },

                {
                    "STAGE ",
                    "스테이지 "
                },

                {
                    "SHIP ",
                    "배 체력 "
                },

                {
                    "TIME ",
                    "시간 "
                },

                {
                    "WAVE ",
                    "공세 "
                },

                {
                    "KILLS ",
                    "격침 "
                },

                {
                    "CREW ",
                    "인원 "
                },

                {
                    "SHELL ",
                    "포탄 "
                },

                {
                    "SELECTED: ",
                    "선택: "
                },

                {
                    "TCP CLIENT",
                    "참가자"
                },

                {
                    "TCP HOST",
                    "방장"
                },

                {
                    "SOLO",
                    "혼자"
                },

                {
                    "IRON",
                    "일반탄"
                },

                {
                    "SPREAD",
                    "확산탄"
                },

                {
                    "PIERCE",
                    "관통탄"
                },

                {
                    "BLAST",
                    "폭발탄"
                },

                {
                    "FLAME",
                    "화염탄"
                },

                {
                    "HEAVY",
                    "중포탄"
                },

                {
                    "NONE",
                    "없음"
                },

                // 유동호 병합: 플레이어 상태 / 낚시 안내
                {
                    "DOWNED - WAITING FOR NEXT STAGE",
                    "행동 불능 - 다음 스테이지를 기다리는 중"
                },

                {
                    "FISHING... (MOVE OR E TO CANCEL)",
                    "낚시 중... (이동 또는 E로 취소)"
                },

                {
                    "E: start fishing / H: eat fish",
                    "E: 낚시 시작 / H: 물고기 먹기(회복)"
                },

                {
                    "WASD move / E interact / Q drop / H eat fish",
                    "WASD 이동 / E 상호작용 / Q 내려놓기 / H 회복"
                },

                {
                    "FISH ",
                    "물고기 "
                }
            };

            for (const auto& entry :
                translations)
            {
                const std::string original =
                    entry[0];

                const std::string translated =
                    entry[1];

                std::size_t position = 0;

                while (
                    (
                        position =
                        value.find(
                            original,
                            position
                        )
                        )
                    != std::string::npos
                    )
                {
                    value.replace(
                        position,
                        original.size(),
                        translated
                    );

                    position +=
                        translated.size();
                }
            }

            if (cache.size() < 4096)
            {
                cache.emplace(
                    originalText,
                    value
                );
            }

            return value;
        }

        void text(
            sf::RenderTarget& r,
            const std::string& s,
            float x,
            float y,
            unsigned size = 10,
            sf::Color c =
            { 222, 232, 222 },
            bool centered = false)
        {
            flush(r);

            if (labelIndex ==
                labels.size())
            {
                labels.push_back({
                    "",
                    std::make_unique<
                        sf::Text>(font)
                    });
            }

            auto& slot =
                labels[labelIndex++];

            if (slot.original != s)
            {
                const auto translated =
                    koreanText(s);

                slot.text->setString(
                    sf::String::fromUtf8(
                        translated.begin(),
                        translated.end()
                    )
                );

                slot.original = s;
            }

            auto& t = *slot.text;

            if (
                t.getCharacterSize() !=
                size
                )
            {
                t.setCharacterSize(size);
            }

            if (centered)
            {
                const sf::FloatRect bounds =
                    t.getLocalBounds();

                t.setOrigin({
                    bounds.position.x +
                    bounds.size.x / 2.f,
                    0.f
                    });
            }
            else
            {
                t.setOrigin({ 0.f, 0.f });
            }

            t.setPosition({
                std::floor(x),
                std::floor(y)
                });

            if (t.getFillColor() != c)
            {
                t.setFillColor(c);
            }

            r.draw(t);
        }

        void panel(
            sf::RenderTarget& r,
            float x,
            float y,
            float w,
            float h)
        {
            box(
                r,
                x + 3,
                y + 3,
                w,
                h,
                { 4, 15, 24, 170 }
            );

            box(
                r,
                x,
                y,
                w,
                h,
                { 14, 30, 43, 246 }
            );

            box(
                r,
                x,
                y,
                w,
                2,
                { 130, 149, 143 }
            );
        }

        void bar(
            sf::RenderTarget& r,
            float x,
            float y,
            float w,
            float ratio,
            sf::Color c)
        {
            box(
                r,
                x - 1,
                y - 1,
                w + 2,
                7,
                { 10, 22, 33 }
            );

            box(
                r,
                x,
                y,
                w,
                5,
                { 52, 60, 60 }
            );

            box(
                r,
                x,
                y,
                w *
                std::clamp(
                    ratio,
                    0.f,
                    1.f
                ),
                5,
                c
            );
        }

        void ball(
            sf::RenderTarget& r,
            V p,
            int type)
        {
            box(
                r,
                p.x - 3,
                p.y - 3,
                6,
                6,
                { 25, 35, 43 }
            );

            box(
                r,
                p.x - 2,
                p.y - 2,
                4,
                4,
                shellColor(type)
            );

            box(
                r,
                p.x - 2,
                p.y - 2,
                2,
                1,
                { 255, 245, 205 }
            );
        }

        void fishIcon(
            sf::RenderTarget& r,
            float x,
            float y,
            float s,
            sf::Color body,
            sf::Color fin,
            sf::Color eye = sf::Color(247, 250, 252))
        {
            box(r, x + 0.f * s, y + 2.f * s, 1.0f * s, 1.0f * s, body);
            box(r, x + 1.f * s, y + 1.f * s, 1.0f * s, 1.0f * s, body);
            box(r, x + 1.f * s, y + 3.f * s, 1.0f * s, 1.0f * s, body);
            box(r, x + 2.f * s, y + 1.f * s, 4.0f * s, 3.0f * s, body);
            box(r, x + 6.f * s, y + 2.f * s, 1.0f * s, 1.0f * s, body);
            box(r, x + 4.f * s, y + 0.f * s, 1.0f * s, 1.0f * s, fin);
            box(r, x + 4.f * s, y + 4.f * s, 1.0f * s, 1.0f * s, fin);
            box(r, x + 5.5f * s, y + 1.6f * s, 0.8f * s, 0.8f * s, eye);
        }

        void drawNightVision(
            sf::RenderTarget& r,
            const Game& g,
            const Screen& ui)
        {
            if (g.wave <= 1)
                return;

            const int localId =
                std::clamp(ui.local, 0, MaxPlayers - 1);

            if (!g.players[localId].active)
                return;

            const V center = g.players[localId].p;

            // 난이도 완화: 밤에도 주변 상황을 읽을 수 있도록
            // 시야 반경과 가장자리 페이드를 넉넉하게 유지한다.
            float radius = 195.f;
            float fade = 90.f;
            float maxAlpha = 155.f;

            if (g.wave == 2)
            {
                const float eveningProgress =
                    std::clamp(
                        1.f - g.time / WaveSeconds,
                        0.f,
                        1.f
                    );

                // 저녁 초반은 거의 전체가 보이고, 후반에도
                // 전투가 답답하지 않도록 255 -> 195 정도만 줄인다.
                radius =
                    255.f -
                    60.f * eveningProgress;

                maxAlpha =
                    45.f +
                    110.f * eveningProgress;

                fade =
                    100.f -
                    10.f * eveningProgress;
            }
            else
            {
                // 밤 / 보스전: 이전보다 훨씬 넓고 부드러운 시야
                radius = 180.f;
                fade = 90.f;
                maxAlpha = 185.f;
            }

            constexpr float Cell = 10.f;
            constexpr float WorldTop = 43.f;
            constexpr float WorldBottom = 338.f;

            for (float y = WorldTop; y < WorldBottom; y += Cell)
            {
                for (float x = 0.f; x < 640.f; x += Cell)
                {
                    const float cx = x + Cell * 0.5f;
                    const float cy = y + Cell * 0.5f;
                    const float dx = cx - center.x;
                    const float dy = cy - center.y;
                    const float d = std::sqrt(dx * dx + dy * dy);

                    if (d <= radius)
                        continue;

                    const float darkness =
                        std::clamp(
                            (d - radius) / fade,
                            0.f,
                            1.f
                        );

                    const std::uint8_t alpha =
                        static_cast<std::uint8_t>(
                            std::clamp(
                                maxAlpha * darkness,
                                0.f,
                                245.f
                            )
                            );

                    if (alpha == 0)
                        continue;

                    box(
                        r,
                        x,
                        y,
                        Cell + 0.5f,
                        Cell + 0.5f,
                        sf::Color(3, 7, 18, alpha)
                    );
                }
            }
        }

        void draw(
            sf::RenderTarget& r,
            const Game& g,
            const Screen& ui)
        {
            rectangles.clear();
            labelIndex = 0;

            sf::Color seaColor;
            sf::Color waveColor;

            if (g.wave == 1)
            {
                seaColor =
                    sf::Color(
                        35,
                        125,
                        160
                    );

                waveColor =
                    sf::Color(
                        95,
                        190,
                        210
                    );
            }
            else if (g.wave == 2)
            {
                seaColor =
                    sf::Color(
                        85,
                        72,
                        115
                    );

                waveColor =
                    sf::Color(
                        145,
                        115,
                        155
                    );
            }
            else if (g.wave == 3)
            {
                seaColor =
                    sf::Color(
                        10,
                        25,
                        50
                    );

                waveColor =
                    sf::Color(
                        35,
                        65,
                        90
                    );
            }
            else
            {
                seaColor =
                    sf::Color(
                        35,
                        125,
                        160
                    );

                waveColor =
                    sf::Color(
                        95,
                        190,
                        210
                    );
            }

            r.clear(seaColor);

            for (
                int row = 0;
                row < 18;
                ++row
                )
            {
                for (
                    int col = 0;
                    col < 17;
                    ++col
                    )
                {
                    float x =
                        static_cast<float>(
                            (
                                col * 43 +
                                row * 19 +
                                static_cast<int>(
                                    g.age * 5
                                    )
                                )
                            % 660
                            )
                        - 10.f;

                    float y =
                        row * 21.f + 8.f;

                    box(
                        r,
                        x,
                        y,
                        13.f,
                        2.f,
                        waveColor
                    );

                    box(
                        r,
                        x + 13.f,
                        y - 2.f,
                        6.f,
                        2.f,
                        waveColor
                    );
                }
            }

            for (
                int i = 0;
                i < 12;
                ++i
                )
            {
                float y =
                    54.f +
                    i * 24.f +
                    std::sin(
                        g.age * 2.f + i
                    ) * 2.f;

                box(
                    r,
                    DeckLeft - 10.f,
                    y,
                    6.f,
                    7.f,
                    { 70, 139, 149 }
                );

                box(
                    r,
                    DeckRight + 4.f,
                    y,
                    6.f,
                    7.f,
                    { 70, 139, 149 }
                );
            }

            box(
                r,
                DeckLeft - 9.f,
                49.f,
                DeckRight -
                DeckLeft +
                18.f,
                285.f,
                { 12, 30, 41 }
            );

            box(
                r,
                DeckLeft - 4.f,
                46.f,
                DeckRight -
                DeckLeft +
                8.f,
                286.f,
                { 53, 36, 31 }
            );

            box(
                r,
                DeckLeft + 2.f,
                50.f,
                DeckRight -
                DeckLeft -
                4.f,
                278.f,
                { 110, 69, 43 }
            );

            for (
                int y = 52;
                y < 327;
                y += 13
                )
            {
                box(
                    r,
                    DeckLeft + 6.f,
                    static_cast<float>(y),
                    DeckRight -
                    DeckLeft -
                    12.f,
                    11.f,
                    { 151, 99, 57 }
                );

                for (
                    int x =
                    static_cast<int>(
                        DeckLeft + 10.f
                        ) +
                    (y % 3) * 11;

                    x <
                    static_cast<int>(
                        DeckRight - 8.f
                        );

                    x += 43
                    )
                {
                    box(
                        r,
                        static_cast<float>(x),
                        static_cast<float>(y),
                        1.f,
                        11.f,
                        { 119, 75, 47 }
                    );

                    box(
                        r,
                        x + 3.f,
                        y + 3.f,
                        2.f,
                        1.f,
                        { 186, 130, 70 }
                    );
                }
            }

            box(
                r,
                DeckLeft - 1.f,
                51.f,
                5.f,
                276.f,
                { 206, 154, 85 }
            );

            box(
                r,
                DeckRight - 4.f,
                51.f,
                5.f,
                276.f,
                { 206, 154, 85 }
            );

            for (
                int y = 59;
                y < 329;
                y += 26
                )
            {
                box(
                    r,
                    DeckLeft - 3.f,
                    static_cast<float>(y),
                    9.f,
                    5.f,
                    { 85, 61, 43 }
                );

                box(
                    r,
                    DeckRight - 6.f,
                    static_cast<float>(y),
                    9.f,
                    5.f,
                    { 85, 61, 43 }
                );
            }

            box(
                r,
                34.f,
                158.f,
                52.f,
                44.f,
                { 50, 37, 31 }
            );

            box(
                r,
                38.f,
                162.f,
                48.f,
                35.f,
                { 135, 88, 50 }
            );

            for (
                int x = 44;
                x < 82;
                x += 13
                )
            {
                box(
                    r,
                    static_cast<float>(x),
                    164.f,
                    2.f,
                    31.f,
                    { 91, 59, 40 }
                );
            }

            box(
                r,
                FishingPoint.x - 2.f,
                FishingPoint.y - 15.f,
                4.f,
                18.f,
                { 229, 218, 183 }
            );

            box(
                r,
                FishingPoint.x,
                FishingPoint.y - 14.f,
                16.f,
                2.f,
                { 229, 218, 183 }
            );

            text(
                r,
                "FISHING",
                26.f,
                208.f,
                8,
                { 249, 220, 151 }
            );

            // 낚시터 장식 물고기: 왼쪽 바다에 서로 겹치지 않게 분산 배치
            // 각 물고기의 움직임 위상을 다르게 해서 한 덩어리처럼 보이지 않게 한다.
            const float fishBobA = std::sin(g.age * 2.2f) * 1.4f;
            const float fishBobB = std::sin(g.age * 1.8f + 1.1f) * 1.3f;
            const float fishBobC = std::sin(g.age * 2.5f + 2.0f) * 1.1f;
            const float fishSwimA = std::sin(g.age * 1.25f) * 2.8f;
            const float fishSwimB = std::sin(g.age * 1.05f + 1.7f) * 2.2f;

            fishIcon(
                r, 6.f + fishSwimA, 128.f + fishBobA, 1.5f,
                sf::Color(104, 205, 224), sf::Color(134, 233, 245)
            );

            fishIcon(
                r, 27.f - fishSwimB, 146.f + fishBobB, 1.3f,
                sf::Color(244, 185, 96), sf::Color(251, 216, 144)
            );

            fishIcon(
                r, 5.f + fishSwimB * 0.5f, 169.f + fishBobC, 1.6f,
                sf::Color(173, 160, 236), sf::Color(200, 191, 246)
            );

            fishIcon(
                r, 18.f - fishSwimA * 0.35f, 191.f - fishBobA, 1.25f,
                sf::Color(112, 224, 181), sf::Color(157, 240, 204)
            );

            fishIcon(
                r, 4.f + fishSwimA * 0.6f, 216.f + fishBobB, 1.45f,
                sf::Color(238, 132, 156), sf::Color(250, 172, 190)
            );

            fishIcon(
                r, 28.f - fishSwimB * 0.45f, 238.f - fishBobC, 1.25f,
                sf::Color(111, 186, 238), sf::Color(157, 215, 249)
            );

            fishIcon(
                r, 10.f + fishSwimA * 0.35f, 261.f + fishBobC, 1.35f,
                sf::Color(245, 202, 105), sf::Color(255, 225, 146)
            );

            box(
                r,
                AmmoPoint.x - 24.f,
                AmmoPoint.y - 17.f,
                48.f,
                31.f,
                { 64, 43, 36 }
            );

            box(
                r,
                AmmoPoint.x - 21.f,
                AmmoPoint.y - 19.f,
                42.f,
                27.f,
                { 91, 67, 47 }
            );

            box(
                r,
                AmmoPoint.x - 21.f,
                AmmoPoint.y - 15.f,
                42.f,
                3.f,
                { 188, 141, 76 }
            );

            box(
                r,
                AmmoPoint.x - 21.f,
                AmmoPoint.y + 1.f,
                42.f,
                3.f,
                { 188, 141, 76 }
            );

            for (
                int i = 0;
                i < 3;
                ++i
                )
            {
                ball(
                    r,
                    {
                        AmmoPoint.x -
                        10.f +
                        i * 10.f,

                        AmmoPoint.y -
                        5.f
                    },
                    Normal
                );
            }

            text(
                r,
                "AMMO",
                AmmoPoint.x - 14.f,
                AmmoPoint.y + 14.f,
                9,
                { 249, 220, 151 }
            );

            for (const auto& e :
                g.enemies)
            {
                V p = e.p;

                if (e.type == 0)
                {
                    float w = 30.f;

                    box(
                        r,
                        p.x - w / 2.f - 3.f,
                        p.y + 11.f,
                        w + 6.f,
                        4.f,
                        { 10, 43, 60 }
                    );

                    box(
                        r,
                        p.x - w / 2.f,
                        p.y - 9.f,
                        w,
                        19.f,
                        { 61, 40, 35 }
                    );

                    box(
                        r,
                        p.x - w / 2.f - 4.f,
                        p.y - 3.f,
                        w + 8.f,
                        8.f,
                        { 104, 60, 43 }
                    );

                    box(
                        r,
                        p.x - w / 2.f + 3.f,
                        p.y - 7.f,
                        w - 6.f,
                        12.f,
                        { 180, 121, 60 }
                    );

                    box(
                        r,
                        p.x - 1.f,
                        p.y - 24.f,
                        2.f,
                        29.f,
                        { 50, 36, 30 }
                    );

                    box(
                        r,
                        p.x - 11.f,
                        p.y - 23.f,
                        20.f,
                        15.f,
                        { 213, 201, 156 }
                    );

                    box(
                        r,
                        p.x - 4.f,
                        p.y - 20.f,
                        7.f,
                        5.f,
                        { 40, 39, 40 }
                    );

                    box(
                        r,
                        p.x - 2.f,
                        p.y - 15.f,
                        3.f,
                        2.f,
                        { 40, 39, 40 }
                    );
                }
                else if (e.type == 1)
                {
                    box(
                        r,
                        p.x - 15.f,
                        p.y + 8.f,
                        30.f,
                        3.f,
                        { 10, 43, 60 }
                    );

                    box(
                        r,
                        p.x - 12.f,
                        p.y - 5.f,
                        24.f,
                        12.f,
                        { 112, 42, 38 }
                    );

                    box(
                        r,
                        p.x - 16.f,
                        p.y,
                        32.f,
                        5.f,
                        { 164, 62, 48 }
                    );

                    box(
                        r,
                        p.x - 8.f,
                        p.y - 8.f,
                        16.f,
                        8.f,
                        { 203, 93, 59 }
                    );

                    box(
                        r,
                        p.x,
                        p.y - 25.f,
                        1.f,
                        23.f,
                        { 52, 34, 30 }
                    );

                    box(
                        r,
                        p.x - 8.f,
                        p.y - 23.f,
                        15.f,
                        11.f,
                        { 190, 65, 60 }
                    );

                    box(
                        r,
                        p.x + 14.f,
                        p.y - 2.f,
                        7.f,
                        2.f,
                        { 180, 225, 225, 150 }
                    );

                    box(
                        r,
                        p.x + 17.f,
                        p.y + 3.f,
                        9.f,
                        2.f,
                        { 180, 225, 225, 100 }
                    );
                }
                else
                {
                    box(
                        r,
                        p.x - 25.f,
                        p.y + 14.f,
                        50.f,
                        5.f,
                        { 8, 35, 48 }
                    );

                    box(
                        r,
                        p.x - 21.f,
                        p.y - 11.f,
                        42.f,
                        24.f,
                        { 47, 52, 57 }
                    );

                    box(
                        r,
                        p.x - 24.f,
                        p.y - 4.f,
                        48.f,
                        11.f,
                        { 74, 83, 88 }
                    );

                    box(
                        r,
                        p.x - 17.f,
                        p.y - 10.f,
                        34.f,
                        13.f,
                        { 107, 117, 121 }
                    );

                    for (
                        int k = -15;
                        k <= 15;
                        k += 10
                        )
                    {
                        box(
                            r,
                            p.x +
                            static_cast<float>(k),
                            p.y + 1.f,
                            2.f,
                            2.f,
                            { 165, 170, 165 }
                        );
                    }

                    box(
                        r,
                        p.x - 2.f,
                        p.y - 29.f,
                        4.f,
                        24.f,
                        { 46, 45, 44 }
                    );

                    box(
                        r,
                        p.x - 13.f,
                        p.y - 27.f,
                        25.f,
                        14.f,
                        { 92, 99, 101 }
                    );

                    box(
                        r,
                        p.x - 6.f,
                        p.y - 22.f,
                        12.f,
                        5.f,
                        { 153, 160, 160 }
                    );
                }

                if (e.freeze > 0.f)
                {
                    float w =
                        e.type == 2
                        ? 42.f
                        : e.type == 1
                        ? 24.f
                        : 30.f;

                    box(
                        r,
                        p.x - w / 2.f,
                        p.y + 8.f,
                        w,
                        3.f,
                        { 117, 222, 242 }
                    );
                }

                if (e.burn > 0.f)
                {
                    box(
                        r,
                        p.x + 6.f,
                        p.y - 12.f,
                        5.f,
                        10.f,
                        { 245, 112, 50 }
                    );

                    box(
                        r,
                        p.x + 8.f,
                        p.y - 16.f,
                        3.f,
                        9.f,
                        { 255, 210, 90 }
                    );
                }

                float hpWidth =
                    e.type == 2
                    ? 40.f
                    : e.type == 1
                    ? 24.f
                    : 30.f;

                bar(
                    r,
                    p.x - hpWidth / 2.f,
                    p.y + 22.f,
                    hpWidth,
                    e.hp / e.maxHp,
                    e.type == 2
                    ? sf::Color(
                        155,
                        165,
                        170
                    )
                    : e.type == 1
                    ? sf::Color(
                        235,
                        90,
                        75
                    )
                    : sf::Color(
                        210,
                        90,
                        70
                    )
                );
            }

            for (const auto& s :
                g.sailors)
            {
                if (s.hp <= 0.f)
                    continue;

                float drawX = s.p.x;
                float drawY = s.p.y;

                if (s.boarding > 0.f)
                {
                    float t =
                        std::clamp(
                            s.boarding /
                            1.2f,
                            0.f,
                            1.f
                        );

                    drawX +=
                        35.f * t;
                }

                box(
                    r,
                    drawX - 8.f,
                    drawY + 9.f,
                    16.f,
                    4.f,
                    sf::Color(
                        0,
                        0,
                        0,
                        90
                    )
                );

                box(
                    r,
                    drawX - 6.f,
                    drawY - 7.f,
                    12.f,
                    15.f,
                    sf::Color(
                        105,
                        210,
                        205,
                        220
                    )
                );

                box(
                    r,
                    drawX - 4.f,
                    drawY - 13.f,
                    8.f,
                    7.f,
                    sf::Color(
                        170,
                        245,
                        225,
                        235
                    )
                );

                box(
                    r,
                    drawX - 3.f,
                    drawY - 10.f,
                    2.f,
                    2.f,
                    sf::Color(
                        20,
                        40,
                        45
                    )
                );

                box(
                    r,
                    drawX + 1.f,
                    drawY - 10.f,
                    2.f,
                    2.f,
                    sf::Color(
                        20,
                        40,
                        45
                    )
                );

                // 유령 선원이 C 근접공격으로 처치 가능한 대상임을
                // 바로 알 수 있도록 머리 위 HP바를 선명하게 표시한다.
                const float hpRatio =
                    std::clamp(
                        s.hp / 60.f,
                        0.f,
                        1.f
                    );

                box(
                    r,
                    drawX - 13.f,
                    drawY - 22.f,
                    26.f,
                    5.f,
                    sf::Color(18, 24, 29, 230)
                );

                const sf::Color ghostHpColor =
                    hpRatio > 0.55f
                    ? sf::Color(88, 224, 135)
                    : hpRatio > 0.25f
                    ? sf::Color(244, 197, 82)
                    : sf::Color(238, 91, 82);

                box(
                    r,
                    drawX - 12.f,
                    drawY - 21.f,
                    24.f * hpRatio,
                    3.f,
                    ghostHpColor
                );

                if (s.windup > 0.f)
                {
                    box(
                        r,
                        s.strike.x - 13.f,
                        s.strike.y - 2.f,
                        26.f,
                        4.f,
                        sf::Color(
                            255,
                            70,
                            70,
                            180
                        )
                    );

                    box(
                        r,
                        s.strike.x - 2.f,
                        s.strike.y - 13.f,
                        4.f,
                        26.f,
                        sf::Color(
                            255,
                            70,
                            70,
                            180
                        )
                    );

                    if (
                        static_cast<int>(
                            g.age * 10.f
                            )
                        % 2 == 0
                        )
                    {
                        box(
                            r,
                            drawX - 9.f,
                            drawY - 15.f,
                            18.f,
                            25.f,
                            sf::Color(
                                255,
                                60,
                                60,
                                55
                            )
                        );
                    }
                }
            }

            if (g.greatWave.active)
            {
                if (
                    g.greatWave.warning >
                    0.f
                    )
                {
                    box(
                        r,
                        DeckRight - 18.f,
                        DeckTop,
                        18.f,
                        DeckBottom -
                        DeckTop,
                        sf::Color(
                            80,
                            180,
                            255,
                            90
                        )
                    );

                    if (
                        static_cast<int>(
                            g.age * 8.f
                            )
                        % 2 == 0
                        )
                    {
                        box(
                            r,
                            DeckRight - 28.f,
                            DeckTop,
                            28.f,
                            DeckBottom -
                            DeckTop,
                            sf::Color(
                                120,
                                210,
                                255,
                                75
                            )
                        );
                    }

                    text(
                        r,
                        "BIG WAVE!",
                        DeckRight - 100.f,
                        DeckTop + 18.f,
                        10,
                        sf::Color(
                            190,
                            235,
                            255
                        )
                    );

                    text(
                        r,
                        "HOLD C",
                        DeckRight - 91.f,
                        DeckTop + 32.f,
                        9,
                        sf::Color(
                            230,
                            245,
                            255
                        )
                    );
                }

                if (
                    g.greatWave.impacting
                    )
                {
                    float x =
                        g.greatWave.front;

                    box(
                        r,
                        x - 13.f,
                        DeckTop - 10.f,
                        26.f,
                        DeckBottom -
                        DeckTop +
                        20.f,
                        sf::Color(
                            75,
                            170,
                            220,
                            210
                        )
                    );

                    box(
                        r,
                        x - 10.f,
                        DeckTop - 10.f,
                        6.f,
                        DeckBottom -
                        DeckTop +
                        20.f,
                        sf::Color(
                            185,
                            235,
                            250,
                            235
                        )
                    );

                    box(
                        r,
                        x + 13.f,
                        DeckTop,
                        16.f,
                        DeckBottom -
                        DeckTop,
                        sf::Color(
                            80,
                            145,
                            200,
                            120
                        )
                    );

                    box(
                        r,
                        x + 29.f,
                        DeckTop + 8.f,
                        10.f,
                        DeckBottom -
                        DeckTop -
                        16.f,
                        sf::Color(
                            90,
                            140,
                            190,
                            65
                        )
                    );

                    for (
                        float y =
                        DeckTop + 8.f;

                        y < DeckBottom;

                        y += 25.f
                        )
                    {
                        box(
                            r,
                            x - 8.f,
                            y,
                            14.f,
                            3.f,
                            sf::Color(
                                220,
                                245,
                                250,
                                210
                            )
                        );

                        box(
                            r,
                            x + 9.f,
                            y + 7.f,
                            9.f,
                            2.f,
                            sf::Color(
                                190,
                                230,
                                245,
                                160
                            )
                        );
                    }
                }
            }

            for (const auto& t :
                g.tentacles)
            {
                if (t.hp <= 0.f)
                    continue;

                if (t.warning > 0.f)
                {
                    box(
                        r,
                        t.p.x - 18.f,
                        t.p.y - 4.f,
                        36.f,
                        8.f,
                        sf::Color(
                            180,
                            70,
                            210,
                            80
                        )
                    );

                    box(
                        r,
                        t.p.x - 4.f,
                        t.p.y - 18.f,
                        8.f,
                        36.f,
                        sf::Color(
                            180,
                            70,
                            210,
                            80
                        )
                    );

                    if (
                        static_cast<int>(
                            g.age * 10.f
                            )
                        % 2 == 0
                        )
                    {
                        box(
                            r,
                            t.p.x - 13.f,
                            t.p.y - 13.f,
                            26.f,
                            26.f,
                            sf::Color(
                                220,
                                90,
                                230,
                                55
                            )
                        );
                    }

                    continue;
                }

                for (
                    int j = 0;
                    j < 8;
                    ++j
                    )
                {
                    V a =
                        t.segmentPoint(
                            j,
                            g.age
                        );

                    V b =
                        t.segmentPoint(
                            j + 1,
                            g.age
                        );

                    V middle{
                        (
                            a.x +
                            b.x
                        ) * 0.5f,

                        (
                            a.y +
                            b.y
                        ) * 0.5f
                    };

                    float width =
                        11.f -
                        j * 0.7f;

                    box(
                        r,
                        middle.x -
                        width * 0.5f,
                        middle.y - 5.f,
                        width,
                        10.f,
                        sf::Color(
                            112,
                            62,
                            145,
                            235
                        )
                    );

                    box(
                        r,
                        middle.x -
                        width * 0.25f,
                        middle.y - 3.f,
                        width * 0.5f,
                        5.f,
                        sf::Color(
                            166,
                            91,
                            183,
                            220
                        )
                    );
                }

                box(
                    r,
                    t.p.x - 10.f,
                    t.p.y - 5.f,
                    20.f,
                    10.f,
                    sf::Color(
                        72,
                        48,
                        100,
                        230
                    )
                );

                float tentacleHp =
                    std::clamp(
                        t.hp / 90.f,
                        0.f,
                        1.f
                    );

                box(
                    r,
                    t.p.x - 15.f,
                    t.p.y + 11.f,
                    30.f,
                    4.f,
                    sf::Color(
                        35,
                        30,
                        45,
                        220
                    )
                );

                box(
                    r,
                    t.p.x - 15.f,
                    t.p.y + 11.f,
                    30.f *
                    tentacleHp,
                    4.f,
                    sf::Color(
                        195,
                        90,
                        200,
                        230
                    )
                );

                if (
                    t.slamTimer <=
                    0.75f &&
                    t.slamAnimation <=
                    0.f
                    )
                {
                    V hit =
                        t.slamPoint();

                    box(
                        r,
                        hit.x - 18.f,
                        hit.y - 3.f,
                        36.f,
                        6.f,
                        sf::Color(
                            255,
                            70,
                            90,
                            150
                        )
                    );

                    box(
                        r,
                        hit.x - 3.f,
                        hit.y - 18.f,
                        6.f,
                        36.f,
                        sf::Color(
                            255,
                            70,
                            90,
                            150
                        )
                    );

                    if (
                        static_cast<int>(
                            g.age * 12.f
                            )
                        % 2 == 0
                        )
                    {
                        box(
                            r,
                            t.p.x - 12.f,
                            t.p.y - 55.f,
                            24.f,
                            60.f,
                            sf::Color(
                                255,
                                80,
                                110,
                                45
                            )
                        );
                    }
                }
            }

            if (g.boss.active)
            {
                V p = g.boss.p;

                if (g.boss.type == BossType::GhostShip) {
                    box(r, p.x - 55, p.y - 16, 110, 40, { 42,67,76 });
                    box(r, p.x - 45, p.y + 24, 90, 8, { 30,47,59 });
                    for (int j = 0; j < 3; ++j) {
                        float x = p.x - 34 + j * 30.f;
                        box(r, x, p.y - 65, 4, 52, { 72,110,111 });
                        box(r, x + 4, p.y - 60, 22, 32, { 139,223,204 });
                        box(r, x - 3, p.y - 4, 12, 10, { 87,248,195 });
                    }
                    box(r, p.x - 61, p.y - 10, 12, 24, { 89,213,194 });
                }
                else if (g.boss.type == BossType::Leviathan) {
                    for (int j = 8; j >= 0; --j) {
                        float x = p.x + j * 7.f, y = p.y + std::sin(g.age * 2 + j * .7f) * 24;
                        box(r, x - 16, y - 15, 28, 30, { 35,123,147 });
                        box(r, x - 10, y - 20, 12, 8, { 102,231,209 });
                    }
                    box(r, p.x - 34, p.y - 25, 48, 46, { 50,166,168 });
                    box(r, p.x - 32, p.y - 33, 9, 15, { 183,241,222 });
                    box(r, p.x - 29, p.y - 10, 10, 6, { 255,213,105 });
                    box(r, p.x - 39, p.y + 10, 26, 8, { 20,65,89 });
                }
                else {
                    for (int k = 0; k < 6; ++k)
                        for (int j = 0; j < 7; ++j)
                        {
                            float x = p.x + (k < 3 ? -1 : 1) * (18 + j * 4.f),
                                y = p.y + 14 + (k % 3) * 10 + std::sin(g.age * 3 + j * .7f + k) * 8;
                            box(r, x, y, 8, 8, { 77, 70, 136 });
                            box(r, x + 1, y + 5, 4, 2, { 166, 111, 174 });
                        }
                    box(r, p.x - 31, p.y - 25, 62, 40, { 75, 60, 120 });
                    box(r, p.x - 25, p.y - 34, 50, 54, { 96, 78, 151 });
                    box(r, p.x - 18, p.y - 39, 36, 12, { 116, 92, 172 });
                    box(r, p.x - 23, p.y - 12, 15, 10, { 20, 27, 46 });
                    box(r, p.x + 8, p.y - 12, 15, 10, { 20, 27, 46 });
                    box(r, p.x - 20, p.y - 10, 8, 5, { 250, 174, 91 });
                    box(r, p.x + 12, p.y - 10, 8, 5, { 250, 174, 91 });
                    box(r, p.x - 6, p.y + 6, 12, 7, { 33, 34, 50 });
                }
                if (g.boss.warning > 0.f)
                {
                    const char* cue = g.boss.type == BossType::GhostShip ? "BROADSIDE!" :
                        g.boss.type == BossType::Leviathan ? "WATER VOLLEY!" : "INK VOLLEY!";
                    text(r, cue, p.x - 45.f, p.y - 53.f, 10, { 255, 172, 90 });
                }
            }

            for (
                int i = 0;
                i <
                static_cast<int>(
                    g.cannons.size()
                    );
                ++i
                )
            {
                const auto& c =
                    g.cannons[i];

                float x = c.p.x;
                float y = c.p.y;

                box(
                    r,
                    x - 12.f,
                    y - 9.f,
                    24.f,
                    22.f,
                    { 55, 40, 32 }
                );

                box(
                    r,
                    x - 10.f,
                    y - 11.f,
                    20.f,
                    22.f,
                    { 61, 70, 70 }
                );

                box(
                    r,
                    x - 7.f,
                    y - 8.f,
                    14.f,
                    16.f,
                    { 96, 111, 113 }
                );

                for (
                    int j = 0;
                    j < 6;
                    ++j
                    )
                {
                    box(
                        r,

                        x +
                        std::cos(
                            c.angle
                        )
                        *
                        c.side *
                        (
                            j *
                            3.f
                            )
                        - 3.f,

                        y +
                        std::sin(
                            c.angle
                        )
                        *
                        (
                            j *
                            3.f
                            )
                        - 3.f,

                        6.f,
                        6.f,
                        { 36, 48, 57 }
                    );
                }

                box(
                    r,
                    x - 2.f,
                    y - 2.f,
                    4.f,
                    4.f,
                    shellColor(
                        c.ammo
                    )
                );

                text(
                    r,

                    std::to_string(
                        i + 1
                    )
                    +
                    (
                        c.ammo >= 0
                        ? " READY"
                        : " EMPTY"
                        ),

                    x - 22.f,
                    y + 15.f,
                    8,

                    c.ammo >= 0
                    ? sf::Color(
                        171,
                        241,
                        135
                    )
                    : sf::Color(
                        248,
                        197,
                        134
                    )
                );

                if (
                    c.progress >
                    0.f
                    )
                {
                    bar(
                        r,
                        x - 15.f,
                        y - 18.f,
                        30.f,
                        c.progress,
                        { 244, 212, 111 }
                    );
                }

                int localId =
                    std::clamp(
                        ui.local,
                        0,
                        MaxPlayers - 1
                    );

                if (
                    g.players[
                        localId
                    ].active
                    &&
                            g.nearCannon(
                                g.players[
                                    localId
                                ]
                            )
                            == i
                            &&
                            c.ammo >= 0
                            )
                {
                    for (
                        int k = 0;
                        k < 8;
                        ++k
                        )
                    {
                        box(
                            r,

                            x +
                            c.side *
                            std::cos(
                                c.angle
                            )
                            *
                            (
                                25.f +
                                k * 10.f
                                ),

                            y +
                            std::sin(
                                c.angle
                            )
                            *
                            (
                                25.f +
                                k * 10.f
                                ),

                            2.f,
                            2.f,
                            { 232, 218, 135 }
                        );
                    }
                }
            }

            for (const auto& h :
                g.hazards)
            {
                auto p = h.p;

                if (h.fire)
                {
                    box(
                        r,
                        p.x - 7.f,
                        p.y - 5.f,
                        14.f,
                        12.f,
                        { 190, 60, 35 }
                    );

                    box(
                        r,
                        p.x - 5.f,
                        p.y - 11.f,
                        9.f,
                        15.f,
                        { 246, 135, 40 }
                    );

                    box(
                        r,
                        p.x - 2.f,
                        p.y -
                        7.f -
                        static_cast<int>(
                            g.age * 8.f
                            ) % 3,
                        5.f,
                        12.f,
                        { 255, 220, 90 }
                    );
                }
                else
                {
                    box(
                        r,
                        p.x - 8.f,
                        p.y - 4.f,
                        16.f,
                        8.f,
                        { 38, 34, 34 }
                    );

                    box(
                        r,
                        p.x - 4.f,
                        p.y - 8.f,
                        5.f,
                        17.f,
                        { 38, 34, 34 }
                    );

                    box(
                        r,
                        p.x - 6.f,
                        p.y,
                        11.f,
                        3.f,
                        { 58, 128, 152 }
                    );
                }

                if (h.progress > 0.f)
                {
                    bar(
                        r,
                        p.x - 12.f,
                        p.y - 18.f,
                        24.f,
                        h.progress /
                        (
                            h.fire
                            ? 1.5f
                            : 2.f
                            ),
                        { 147, 219, 161 }
                    );
                }
            }

            for (const auto& d :
                g.drops)
            {
                ball(
                    r,
                    d.p,
                    d.type
                );
            }

            for (const auto& s :
                g.shots)
            {
                if (s.black)
                {
                    box(
                        r,
                        s.p.x - 5.f,
                        s.p.y - 5.f,
                        10.f,
                        10.f,
                        sf::Color(
                            115,
                            45,
                            125,
                            210
                        )
                    );

                    box(
                        r,
                        s.p.x - 4.f,
                        s.p.y - 4.f,
                        8.f,
                        8.f,
                        sf::Color(
                            8,
                            8,
                            12,
                            255
                        )
                    );

                    box(
                        r,
                        s.p.x - 2.f,
                        s.p.y - 2.f,
                        2.f,
                        2.f,
                        sf::Color(
                            180,
                            80,
                            190,
                            220
                        )
                    );
                }
                else
                {
                    if (s.hostile && g.boss.active && g.boss.type == BossType::Leviathan)
                        box(r, s.p.x - 3.f, s.p.y - 3.f, 6.f, 6.f, { 117, 217, 237 });
                    else
                        ball(r, s.p, s.type);

                    if (s.hostile)
                    {
                        box(
                            r,
                            s.p.x - 1.f,
                            s.p.y - 1.f,
                            3.f,
                            3.f,
                            sf::Color(
                                255,
                                85,
                                70
                            )
                        );
                    }
                }
            }

            for (
                int i = 0;
                i < MaxPlayers;
                ++i
                )
            {
                const auto& p =
                    g.players[i];

                if (!p.active)
                    continue;

                const auto& c =
                    g.crew[i];

                V v = p.p;

                // 유동호 병합: 쓰러진 플레이어는 서 있는 캐릭터 대신 누운 상태로 표시
                if (c.hp <= 0.f)
                {
                    box(
                        r,
                        v.x - 10.f,
                        v.y + 4.f,
                        20.f,
                        6.f,
                        { 55, 55, 60 }
                    );

                    box(
                        r,
                        v.x - 12.f,
                        v.y + 3.f,
                        6.f,
                        6.f,
                        { 180, 150, 120 }
                    );

                    box(
                        r,
                        v.x - 6.f,
                        v.y + 4.f,
                        10.f,
                        5.f,
                        playerColor(i)
                    );

                    text(
                        r,
                        "DOWN",
                        v.x - 12.f,
                        v.y - 14.f,
                        8,
                        { 255, 75, 75 }
                    );

                    const std::string downName =
                        !ui.playerNames[i].empty()
                        ? ui.playerNames[i]
                        : "플레이어";

                    text(
                        r,
                        downName,
                        v.x,
                        v.y - 24.f,
                        8,
                        { 150, 150, 150 },
                        true
                    );

                    continue;
                }

                // 유동호 병합: 낚시 진행 게이지
                if (
                    p.fishing &&
                    p.fishProgress > 0.f
                    )
                {
                    bar(
                        r,
                        v.x - 14.f,
                        v.y - 35.f,
                        28.f,
                        p.fishProgress,
                        { 115, 215, 235 }
                    );
                }

                if (p.fishCatchFx > 0.f)
                {
                    const float rise = (1.f - p.fishCatchFx) * 28.f;
                    const std::uint8_t alpha = static_cast<std::uint8_t>(70.f + 185.f * p.fishCatchFx);
                    const sf::Color body(110, 218, 238, alpha);
                    const sf::Color fin(170, 242, 250, alpha);
                    const sf::Color txt(240, 252, 255, alpha);

                    fishIcon(
                        r,
                        v.x - 12.f,
                        v.y - 54.f - rise,
                        1.6f,
                        body,
                        fin
                    );

                    fishIcon(
                        r,
                        v.x + 2.f,
                        v.y - 49.f - rise * 0.8f,
                        1.2f,
                        sf::Color(245, 191, 106, alpha),
                        sf::Color(255, 221, 140, alpha)
                    );

                    text(
                        r,
                        "+" + std::to_string(std::max(1, p.fishCatchAmount)) + " FISH",
                        v.x - 18.f,
                        v.y - 66.f - rise,
                        8,
                        txt
                    );
                }

                box(
                    r,
                    v.x - 6.f,
                    v.y + 8.f,
                    14.f,
                    3.f,
                    { 96, 65, 44 }
                );

                int step =
                    static_cast<int>(
                        g.age * 8.f + i
                        )
                    % 2;

                box(
                    r,
                    v.x - 5.f,
                    v.y + 3.f,
                    4.f,
                    7.f +
                    static_cast<float>(
                        step
                        ),
                    { 30, 42, 58 }
                );

                box(
                    r,
                    v.x + 2.f,
                    v.y + 3.f,
                    4.f,
                    8.f -
                    static_cast<float>(
                        step
                        ),
                    { 30, 42, 58 }
                );

                box(
                    r,
                    v.x - 6.f,
                    v.y - 5.f,
                    12.f,
                    10.f,
                    playerColor(i)
                );

                box(
                    r,
                    v.x - 4.f,
                    v.y - 11.f,
                    8.f,
                    7.f,
                    { 245, 196, 141 }
                );

                box(
                    r,
                    v.x - 6.f,
                    v.y - 13.f,
                    12.f,
                    4.f,
                    { 239, 233, 203 }
                );

                box(
                    r,
                    v.x - 1.f,
                    v.y - 9.f,
                    2.f,
                    2.f,
                    { 44, 50, 50 }
                );

                const std::string playerName =
                    !ui.playerNames[i].empty()
                    ? ui.playerNames[i]
                    : "플레이어";

                text(
                    r,
                    playerName,
                    v.x,
                    v.y - 24.f,
                    8,
                    playerColor(i),
                    true
                );

                if (p.held >= 0)
                {
                    ball(
                        r,
                        {
                            v.x + 7.f,
                            v.y + 1.f
                        },
                        p.held
                    );
                }

                if (i == ui.local)
                {
                    box(
                        r,
                        v.x - 5.f,
                        v.y + 12.f,
                        10.f,
                        1.f,
                        { 245, 226, 171 }
                    );
                }

                float hpRatio =
                    c.maxHp > 0.f
                    ? std::clamp(
                        c.hp / c.maxHp,
                        0.f,
                        1.f
                    )
                    : 0.f;

                box(
                    r,
                    v.x - 13.f,
                    v.y - 30.f,
                    26.f,
                    4.f,
                    sf::Color(
                        35,
                        35,
                        35,
                        220
                    )
                );

                box(
                    r,
                    v.x - 13.f,
                    v.y - 30.f,
                    26.f * hpRatio,
                    4.f,
                    hpRatio <= 0.30f
                    ? sf::Color(
                        230,
                        80,
                        70,
                        230
                    )
                    : sf::Color(
                        90,
                        220,
                        110,
                        230
                    )
                );

                if (c.swing > 0.f)
                {
                    box(
                        r,
                        v.x + 11.f,
                        v.y - 13.f,
                        3.f,
                        22.f,
                        sf::Color(
                            245,
                            245,
                            215,
                            220
                        )
                    );

                    box(
                        r,
                        v.x + 7.f,
                        v.y - 15.f,
                        11.f,
                        3.f,
                        sf::Color(
                            245,
                            245,
                            215,
                            220
                        )
                    );
                }

                if (
                    c.invulnerable >
                    0.f &&
                    c.revive <= 0.f &&
                    static_cast<int>(
                        g.age * 12.f
                        )
                    % 2 == 0
                    )
                {
                    box(
                        r,
                        v.x - 10.f,
                        v.y - 15.f,
                        20.f,
                        28.f,
                        sf::Color(
                            255,
                            255,
                            255,
                            65
                        )
                    );
                }

                if (c.revive > 0.f)
                {
                    box(
                        r,
                        v.x - 12.f,
                        v.y - 16.f,
                        24.f,
                        30.f,
                        sf::Color(
                            20,
                            20,
                            25,
                            150
                        )
                    );

                    text(
                        r,

                        "REVIVE " +
                        std::to_string(
                            static_cast<int>(
                                std::ceil(
                                    c.revive
                                )
                                )
                        ),

                        v.x - 21.f,
                        v.y - 40.f,
                        7
                    );
                }

                if (c.bracing)
                {
                    box(
                        r,
                        v.x - 14.f,
                        v.y - 18.f,
                        3.f,
                        35.f,
                        sf::Color(
                            100,
                            210,
                            255,
                            180
                        )
                    );

                    box(
                        r,
                        v.x + 11.f,
                        v.y - 18.f,
                        3.f,
                        35.f,
                        sf::Color(
                            100,
                            210,
                            255,
                            180
                        )
                    );
                }
            }

            for (const auto& e :
                g.effects)
            {
                float f =
                    1.f -
                    e.life /
                    e.maxLife;

                sf::Color color =
                    e.kind == 2
                    ? sf::Color(
                        141,
                        230,
                        165
                    )
                    : sf::Color(
                        250,
                        181,
                        80
                    );

                for (
                    int k = 0;
                    k < 8;
                    ++k
                    )
                {
                    float a =
                        k * Pi / 4.f;

                    box(
                        r,

                        e.p.x +
                        std::cos(a) *
                        f *
                        25.f,

                        e.p.y +
                        std::sin(a) *
                        f *
                        25.f,

                        3.f,
                        3.f,
                        color
                    );
                }
            }

            if (g.wave == 2)
            {
                const float eveningProgress =
                    std::clamp(
                        1.f - g.time / WaveSeconds,
                        0.f,
                        1.f
                    );

                const std::uint8_t tintAlpha =
                    static_cast<std::uint8_t>(
                        18.f +
                        42.f * eveningProgress
                        );

                box(
                    r,
                    0.f,
                    43.f,
                    640.f,
                    295.f,
                    sf::Color(
                        55,
                        20,
                        70,
                        tintAlpha
                    )
                );
            }
            else if (g.wave >= 3)
            {
                box(
                    r,
                    0.f,
                    43.f,
                    640.f,
                    295.f,
                    sf::Color(
                        5,
                        10,
                        35,
                        68
                    )
                );
            }

            // 저녁부터 플레이어 주변 시야가 점점 좁아지고
            // 밤에는 로컬 플레이어 주변만 밝게 보인다.
            drawNightVision(r, g, ui);

            box(
                r,
                0.f,
                0.f,
                640.f,
                43.f,
                { 13, 27, 39 }
            );

            box(
                r,
                0.f,
                42.f,
                640.f,
                1.f,
                { 70, 103, 111 }
            );

            text(
                r,
                "DECKWATCH",
                12.f,
                4.f,
                15,
                { 248, 218, 155 }
            );

            text(
                r,

                "STAGE 0" +
                std::to_string(
                    g.stage
                )
                +
                " / 03",

                13.f,
                25.f,
                9,
                { 132, 180, 189 }
            );

            text(
                r,

                "SHIP " +
                std::to_string(
                    static_cast<int>(
                        g.hp
                        )
                )
                +
                " / " +
                std::to_string(
                    static_cast<int>(
                        g.maxHp
                        )
                ),

                181.f,
                5.f,
                10
            );

            // 유동호 병합: 팀 공용 물고기 보유량 HUD
            text(
                r,

                "FISH " +
                std::to_string(
                    g.fishCount
                ),

                105.f,
                25.f,
                9,
                g.fishCount > 0
                ? sf::Color(
                    120,
                    220,
                    240
                )
                : sf::Color(
                    140,
                    150,
                    155
                )
            );

            for (int i = 0; i < 5; ++i)
            {
                const bool filled = i < std::min(g.fishCount, 5);

                fishIcon(
                    r,
                    106.f + i * 12.f,
                    38.f,
                    1.3f,
                    filled
                    ? sf::Color(108, 213, 233)
                    : sf::Color(76, 88, 96),
                    filled
                    ? sf::Color(160, 239, 250)
                    : sf::Color(98, 109, 118)
                );
            }

            bar(
                r,
                182.f,
                25.f,
                150.f,
                g.hp / g.maxHp,
                g.hp < 250.f
                ? sf::Color(
                    234,
                    103,
                    85
                )
                : sf::Color(
                    105,
                    199,
                    138
                )
            );

            int seconds =
                static_cast<int>(
                    std::ceil(
                        g.time
                    )
                    );

            text(
                r,

                g.boss.active
                ? "BOSS FIGHT"
                :
                "TIME " +
                std::to_string(
                    seconds / 60
                )
                +
                ":"
                +
                (
                    seconds % 60 < 10
                    ? "0"
                    : ""
                    )
                +
                std::to_string(
                    seconds % 60
                ),

                354.f,
                5.f,
                12,
                { 249, 218, 151 }
            );

            std::string timeName;

            if (g.wave == 1)
            {
                timeName = "DAY";
            }
            else if (g.wave == 2)
            {
                timeName = "EVENING";
            }
            else if (g.wave == 3)
            {
                timeName = "NIGHT";
            }
            else
            {
                timeName = "DAY";
            }

            text(
                r,

                "WAVE " +
                std::to_string(
                    g.wave
                )
                +
                "/3  " +
                timeName +
                "  KILLS " +
                std::to_string(
                    g.kills
                ),

                354.f,
                25.f,
                9
            );

            int id =
                std::clamp(
                    ui.local,
                    0,
                    4
                );

            const auto& p =
                g.players[id];

            text(
                r,

                (
                    !ui.playerNames[id].empty()
                    ? ui.playerNames[id]
                    : "플레이어"
                    )
                +
                "  CREW " +
                std::to_string(
                    g.count()
                )
                +
                "/5",

                500.f,
                4.f,
                9,
                playerColor(id)
            );

            text(
                r,

                "GOLD " +
                std::to_string(
                    g.gold.getGold()
                )
                +
                "G",

                500.f,
                16.f,
                9,
                sf::Color(
                    249,
                    218,
                    151
                )
            );

            text(
                r,

                "SHELL " +
                std::string(
                    shellName(
                        p.held
                    )
                ),

                500.f,
                28.f,
                8
            );

            if (g.boss.active)
            {
                panel(
                    r,
                    165.f,
                    45.f,
                    310.f,
                    19.f
                );

                text(
                    r,
                    (g.boss.type == BossType::GhostShip ? "GHOST SHIP" : g.boss.type == BossType::Leviathan ? "LEVIATHAN" : "KRAKEN"),
                    174.f,
                    49.f,
                    9,
                    { 210, 160, 232 }
                );

                bar(
                    r,
                    244.f,
                    51.f,
                    220.f,
                    g.boss.hp /
                    g.boss.maxHp,
                    { 186, 100, 181 }
                );
            }

            // 탄 종류/재고는 화면에 상시 나열하지 않습니다.
            // 탄약고에서 E를 누르면 마우스 선택창에서 확인합니다.

            panel(
                r,
                438.f,
                291.f,
                194.f,
                31.f
            );

            text(
                r,
                "E hold: repair / extinguish",
                445.f,
                296.f,
                9
            );

            text(
                r,
                "F1 help   M sound   F11 screen",
                445.f,
                308.f,
                8,
                { 159, 188, 196 }
            );

            box(
                r,
                0.f,
                338.f,
                640.f,
                22.f,
                { 13, 27, 39 }
            );

            text(
                r,
                g.hint(id),
                12.f,
                344.f,
                10
            );

            text(
                r,

                ui.online
                ? (
                    ui.client
                    ? "TCP CLIENT"
                    : "TCP HOST"
                    )
                : "SOLO",

                555.f,
                344.f,
                9,
                { 151, 188, 197 }
            );

            if (
                g.phase ==
                Phase::Lobby &&
                !ui.menu
                )
            {
                panel(
                    r,
                    112.f,
                    90.f,
                    416.f,
                    174.f
                );

                text(
                    r,
                    "CREW ASSEMBLY",
                    143.f,
                    108.f,
                    23,
                    { 249, 218, 155 }
                );

                text(
                    r,

                    "Connected: " +
                    std::to_string(
                        g.count()
                    )
                    +
                    " / 5",

                    144.f,
                    146.f,
                    12
                );

                text(
                    r,
                    ui.status,
                    144.f,
                    168.f,
                    10
                );

                text(
                    r,
                    ui.address,
                    144.f,
                    186.f,
                    10,
                    { 142, 192, 201 }
                );

                text(
                    r,

                    ui.client
                    ? "Waiting for host to start..."
                    : "ENTER: depart with current crew",

                    144.f,
                    219.f,
                    12,
                    { 248, 218, 155 }
                );

                text(
                    r,
                    "ESC: return to main menu",
                    144.f,
                    241.f,
                    10
                );
            }

            if (
                g.phase ==
                Phase::Shop &&
                !ui.menu
                )
            {
                auto shopGold =
                    g.gold;

                auto shopAmmo =
                    g.ammo;

                auto shopUpgrades =
                    g.upgrades;

                ShopManager shop(
                    shopGold,
                    shopAmmo,
                    shopUpgrades
                );

                box(
                    r,
                    0.f,
                    43.f,
                    640.f,
                    295.f,
                    sf::Color(
                        6,
                        16,
                        25,
                        225
                    )
                );

                panel(
                    r,
                    22.f,
                    52.f,
                    596.f,
                    286.f
                );

                text(
                    r,
                    "선상 상점",
                    34.f,
                    62.f,
                    20,
                    sf::Color(
                        249,
                        218,
                        155
                    )
                );

                text(
                    r,

                    "팀 골드 " +
                    std::to_string(
                        g.gold.getGold()
                    )
                    +
                    "G",

                    220.f,
                    68.f,
                    13,
                    sf::Color(
                        249,
                        218,
                        155
                    )
                );

                text(
                    r,

                    "물고기 " +
                    std::to_string(
                        g.fishCount
                    ),

                    365.f,
                    68.f,
                    11,
                    sf::Color(
                        180,
                        225,
                        245
                    )
                );

                text(
                    r,

                    std::to_string(
                        static_cast<int>(
                            std::ceil(
                                g.shopRemaining
                            )
                            )
                    )
                    +
                    "초 남음",

                    505.f,
                    68.f,
                    13,
                    sf::Color(
                        249,
                        218,
                        155
                    )
                );

                text(
                    r,
                    "특수탄 구매 / 팀 강화",
                    34.f,
                    91.f,
                    10,
                    sf::Color(
                        150,
                        185,
                        193
                    )
                );

                const char*
                    itemNames[
                        ShopItemCount] =
                        {
                            "확산탄 +5발",
                            "관통탄 +4발",
                            "폭발탄 +3발",
                            "화염탄 +3발",
                            "중포탄 +2발",
                            "일반탄 공격력",
                            "배 최대 체력",
                            "장전 속도",
                            "수리 속도",
                            "낚시 속도"
                        };

                        for (
                            int i = 0;
                            i < ShopItemCount;
                            ++i
                            )
                        {
                            int col =
                                i % 5;

                            int row =
                                i / 5;

                            float x =
                                34.f +
                                col * 116.f;

                            float y =
                                108.f +
                                row * 74.f;

                            const auto item =
                                ShopItems[i];

                            int price =
                                shop.getPrice(
                                    item
                                );

                            bool maxLevel =
                                shop.isMaxLevel(
                                    item
                                );

                            int level =
                                shop.getUpgradeLevel(
                                    item
                                );

                            box(
                                r,
                                x,
                                y,
                                108.f,
                                64.f,
                                sf::Color(
                                    28,
                                    47,
                                    60
                                )
                            );

                            text(
                                r,
                                itemNames[i],
                                x + 5.f,
                                y + 7.f,
                                10,
                                sf::Color(
                                    245,
                                    218,
                                    155
                                )
                            );

                            if (maxLevel)
                            {
                                text(
                                    r,
                                    "MAX",
                                    x + 5.f,
                                    y + 43.f,
                                    10,
                                    sf::Color(
                                        150,
                                        185,
                                        193
                                    )
                                );
                            }
                            else
                            {
                                text(
                                    r,

                                    std::to_string(
                                        price
                                    )
                                    +
                                    "G",

                                    x + 5.f,
                                    y + 43.f,
                                    10,
                                    sf::Color(
                                        245,
                                        218,
                                        155
                                    )
                                );
                            }

                            if (level >= 0)
                            {
                                text(
                                    r,

                                    "Lv." +
                                    std::to_string(
                                        level
                                    )
                                    +
                                    "/5",

                                    x + 58.f,
                                    y + 43.f,
                                    9,
                                    sf::Color(
                                        150,
                                        185,
                                        193
                                    )
                                );
                            }
                        }

                        if (
                            g.purchaseVoteActive &&
                            g.purchaseVoteItem >=
                            0 &&
                            g.purchaseVoteItem <
                            ShopItemCount
                            )
                        {
                            const char*
                                voteItemNames[
                                    ShopItemCount] =
                                    {
                                        "확산탄",
                                        "관통탄",
                                        "폭발탄",
                                        "화염탄",
                                        "중포탄",
                                        "일반탄 공격력",
                                        "배 최대 체력",
                                        "장전 속도",
                                        "수리 속도",
                                        "낚시 속도"
                                    };

                                    float yesWeight =
                                        g.purchaseVoteWeight(
                                            1
                                        );

                                    float noWeight =
                                        g.purchaseVoteWeight(
                                            0
                                        );

                                    auto voteWeightText = [](float value)
                                        {
                                            const int whole = static_cast<int>(value);
                                            return value - static_cast<float>(whole) > 0.25f
                                                ? std::to_string(whole) + ".5"
                                                : std::to_string(whole);
                                        };

                                    box(
                                        r,
                                        34.f,
                                        252.f,
                                        572.f,
                                        34.f,
                                        sf::Color(
                                            42,
                                            34,
                                            38
                                        )
                                    );

                                    text(
                                        r,
                                        std::string("구매 투표: ") +
                                        voteItemNames[g.purchaseVoteItem],
                                        42.f,
                                        258.f,
                                        10,
                                        sf::Color(249, 218, 155)
                                    );

                                    // 마우스로 직접 투표: 방장 1.5표 / 참가자 1표, 자동 찬성 없음
                                    box(r, 330.f, 255.f, 88.f, 26.f, sf::Color(44, 92, 62));
                                    box(r, 426.f, 255.f, 88.f, 26.f, sf::Color(102, 54, 54));

                                    text(
                                        r,
                                        "찬성 " + voteWeightText(yesWeight),
                                        374.f,
                                        261.f,
                                        10,
                                        sf::Color::White,
                                        true
                                    );

                                    text(
                                        r,
                                        "반대 " + voteWeightText(noWeight),
                                        470.f,
                                        261.f,
                                        10,
                                        sf::Color::White,
                                        true
                                    );

                                    text(
                                        r,
                                        std::to_string(
                                            static_cast<int>(
                                                std::ceil(g.purchaseVoteRemaining)
                                                )
                                        ) + "초",
                                        565.f,
                                        260.f,
                                        9,
                                        sf::Color(249, 218, 155)
                                    );
                        }

                        int localId =
                            ui.local;

                        if (
                            localId >= 0 &&
                            localId <
                            MaxPlayers &&
                            g.shopResultTime[
                                localId
                            ] > 0.f
                            )
                        {
                            std::string
                                resultMessage;

                            switch (
                                g.shopResult[
                                    localId
                                ]
                                )
                            {
                            case 1:
                                resultMessage =
                                    "구매 완료";
                                break;

                            case 2:
                                resultMessage =
                                    "골드 부족";
                                break;

                            case 3:
                                resultMessage =
                                    "이미 최대 강화";
                                break;

                            case 4:
                                resultMessage =
                                    "판매 완료";
                                break;

                            case 5:
                                resultMessage =
                                    "구매 투표 부결";
                                break;

                            default:
                                break;
                            }

                            if (
                                !resultMessage.empty()
                                )
                            {
                                text(
                                    r,
                                    resultMessage,
                                    430.f,
                                    91.f,
                                    10,
                                    sf::Color(
                                        249,
                                        218,
                                        155
                                    )
                                );
                            }
                        }

                        int readyCount = 0;

                        for (
                            int i = 0;
                            i < MaxPlayers;
                            ++i
                            )
                        {
                            if (
                                g.players[
                                    i
                                ].active &&
                                g.shopReady[
                                    i
                                ]
                                        )
                            {
                                ++readyCount;
                            }
                        }

                        text(
                            r,
                            "준비 " + std::to_string(readyCount) +
                            "/" + std::to_string(g.count()),
                            430.f,
                            291.f,
                            10,
                            sf::Color(245, 218, 155)
                        );

                        box(
                            r,
                            500.f,
                            286.f,
                            106.f,
                            28.f,
                            sf::Color(45, 82, 94)
                        );

                        text(
                            r,
                            "출항 준비",
                            553.f,
                            293.f,
                            10,
                            sf::Color::White,
                            true
                        );

                        text(
                            r,
                            "F: 물고기 판매  |  Enter: 출항 준비",
                            430.f,
                            319.f,
                            8,
                            sf::Color(150, 185, 193)
                        );
            }

            if (
                g.phase ==
                Phase::Reward &&
                !ui.menu
                )
            {
                panel(
                    r,
                    35.f,
                    90.f,
                    570.f,
                    188.f
                );

                text(
                    r,
                    "STAGE CLEAR - CHOOSE A CREW RELIC",
                    55.f,
                    104.f,
                    18,
                    { 249, 218, 155 }
                );

                text(
                    r,
                    "Each player votes. Majority wins; ties favor the left card.",
                    55.f,
                    131.f,
                    9,
                    { 151, 190, 197 }
                );

                const char*
                    names[] =
                {
                    "1  REINFORCED HULL",
                    "2  REPAIR KIT",
                    "3  WATER SEAL"
                };

                const char*
                    desc[] =
                {
                    "+150 max HP / heal 250",
                    "+25 repair heal / heal 160",
                    "35% less leaks / heal 160"
                };

                for (
                    int k = 0;
                    k < 3;
                    ++k
                    )
                {
                    box(
                        r,
                        50.f +
                        k * 180.f,
                        156.f,
                        170.f,
                        74.f,

                        p.vote == k
                        ? sf::Color(
                            50,
                            85,
                            80
                        )
                        : sf::Color(
                            28,
                            47,
                            60
                        )
                    );

                    text(
                        r,
                        names[k],
                        57.f +
                        k * 180.f,
                        168.f,
                        11,
                        { 246, 216, 148 }
                    );

                    text(
                        r,
                        desc[k],
                        57.f +
                        k * 180.f,
                        195.f,
                        9
                    );

                    int votes = 0;

                    for (
                        const auto& x :
                        g.players
                        )
                    {
                        if (
                            x.active &&
                            x.vote == k
                            )
                        {
                            ++votes;
                        }
                    }

                    text(
                        r,

                        "VOTES " +
                        std::to_string(
                            votes
                        ),

                        57.f +
                        k * 180.f,
                        213.f,
                        9
                    );
                }

                text(
                    r,
                    "Press 1, 2 or 3. No shop. Cannon damage / load time stay fixed.",
                    55.f,
                    247.f,
                    9
                );
            }

            if (
                (
                    g.phase ==
                    Phase::Won ||
                    g.phase ==
                    Phase::Lost
                    )
                &&
                !ui.menu
                )
            {
                panel(
                    r,
                    115.f,
                    90.f,
                    410.f,
                    183.f
                );

                text(
                    r,

                    g.phase ==
                    Phase::Won
                    ? "THE SEA IS YOURS"
                    : "SHIP LOST",

                    143.f,
                    111.f,
                    27,
                    { 249, 218, 155 }
                );

                text(
                    r,

                    g.phase ==
                    Phase::Won
                    ?
                    "All three stages complete. The monster is defeated."
                    :
                    "The hull reached zero. Your crew can try again.",

                    143.f,
                    156.f,
                    10
                );

                text(
                    r,

                    "Enemies sunk: " +
                    std::to_string(
                        g.kills
                    ),

                    143.f,
                    183.f,
                    12
                );

                text(
                    r,

                    ui.client
                    ? "Waiting for host to restart"
                    : "ENTER: restart from stage 1",

                    143.f,
                    217.f,
                    12,
                    { 244, 213, 151 }
                );

                text(
                    r,
                    "ESC: main menu",
                    143.f,
                    245.f,
                    10
                );
            }

            if (ui.menu)
            {
                panel(
                    r,
                    90.f,
                    60.f,
                    460.f,
                    250.f
                );

                text(
                    r,
                    "해상 방어전",
                    123.f,
                    78.f,
                    25,
                    { 249, 218, 155 }
                );

                text(
                    r,
                    "총 3개 스테이지 / 하나의 배 / 최대 5명 협동",
                    123.f,
                    115.f,
                    12,
                    { 151, 193, 202 }
                );

                text(
                    r,
                    "1   혼자 시작하기",
                    123.f,
                    143.f,
                    16
                );

                text(
                    r,
                    "2   방 만들기",
                    123.f,
                    167.f,
                    16
                );

                text(
                    r,
                    "3   다른 사람의 방에 접속",
                    123.f,
                    191.f,
                    16
                );

                box(
                    r,
                    121.f,
                    221.f,
                    396.f,
                    22.f,

                    ui.editing
                    ? sf::Color(
                        60,
                        80,
                        80
                    )
                    : sf::Color(
                        28,
                        45,
                        58
                    )
                );

                const std::string
                    addressText =
                    std::string(
                        ui.editing
                        ? "> "
                        : "접속 주소: "
                    )
                    +
                    ui.endpoint
                    +
                    (
                        ui.editing
                        ? "_"
                        : ""
                        );

                text(
                    r,
                    addressText,
                    128.f,
                    225.f,
                    12,
                    { 242, 216, 168 }
                );

                text(
                    r,
                    "Tab 주소 수정 / Enter 입력 완료 / F1 조작법",
                    123.f,
                    250.f,
                    11
                );

                text(
                    r,
                    ui.status,
                    123.f,
                    273.f,
                    10,
                    { 239, 164, 137 }
                );

                text(
                    r,
                    "F11 전체화면 / M 효과음 / Esc 메인 메뉴",
                    123.f,
                    292.f,
                    11,
                    { 151, 190, 197 }
                );
            }

            // 탄약고 선택창: 탄약고 근처에서 E를 누른 뒤 마우스로 선택합니다.
            if (
                !ui.menu &&
                ui.ammoMenuVisible &&
                g.phase == Phase::Play
                )
            {
                panel(r, 195.f, 82.f, 350.f, 196.f);

                text(
                    r,
                    "탄약고 - 사용할 포탄 선택",
                    370.f,
                    96.f,
                    15,
                    sf::Color(249, 218, 155),
                    true
                );

                text(
                    r,
                    "원하는 포탄을 클릭하면 바로 가져옵니다",
                    370.f,
                    116.f,
                    9,
                    sf::Color(170, 205, 213),
                    true
                );

                for (int i = 0; i < ShellCount; ++i)
                {
                    const int col = i % 3;
                    const int row = i / 3;
                    const float x = 220.f + col * 104.f;
                    const float y = 138.f + row * 58.f;

                    box(r, x, y, 94.f, 48.f, sf::Color(30, 50, 64));

                    text(
                        r,
                        shellName(i),
                        x + 47.f,
                        y + 8.f,
                        10,
                        shellColor(i),
                        true
                    );

                    const std::string stock =
                        i == Normal
                        ? "무제한"
                        : "보유 " + std::to_string(g.ammoCount(i)) + "발";

                    text(
                        r,
                        stock,
                        x + 47.f,
                        y + 27.f,
                        8,
                        sf::Color(210, 220, 220),
                        true
                    );
                }

                text(
                    r,
                    "ESC 또는 E: 닫기",
                    370.f,
                    258.f,
                    9,
                    sf::Color(160, 190, 198),
                    true
                );
            }

            if (
                ui.help ||
                ui.paused
                )
            {
                panel(
                    r,
                    70.f,
                    50.f,
                    500.f,
                    262.f
                );

                text(
                    r,

                    ui.paused
                    ? "PAUSED"
                    : "CAPTAIN'S HANDBOOK",

                    90.f,
                    66.f,
                    22,
                    { 249, 218, 155 }
                );

                text(
                    r,
                    "WASD move  /  E interact  /  Q put down shell",
                    90.f,
                    106.f,
                    12
                );

                text(
                    r,
                    "At the ammo rack press E, then click the shell you want",
                    90.f,
                    128.f,
                    11
                );

                text(
                    r,
                    "Hold E beside a cannon for 1 second to load",
                    90.f,
                    150.f,
                    11
                );

                text(
                    r,
                    "After loading, aim with MOUSE and press SPACE to fire.",
                    90.f,
                    172.f,
                    11
                );

                text(
                    r,
                    "Hold E at damage sites: repair / extinguish",
                    90.f,
                    194.f,
                    11
                );

                text(
                    r,
                    "Survive 90 sec each stage; kill the final sea monster.",
                    90.f,
                    218.f,
                    10
                );

                text(
                    r,

                    ui.online
                    ?
                    "Online match keeps running while help is open."
                    :
                    "Solo play pauses when unfocused or help is open.",

                    90.f,
                    240.f,
                    10,
                    { 151, 190, 197 }
                );

                text(
                    r,
                    "C melee/brace   H eat fish   TAB chat",
                    90.f,
                    264.f,
                    10,
                    { 151, 190, 197 }
                );

                text(
                    r,
                    "F1 / ESC close   F10 main menu   M mute",
                    90.f,
                    284.f,
                    11,
                    { 248, 218, 155 }
                );
            }

            // 게임 중 항상 보이는 간단 조작법 안내. 전체 설명은 F1.
            if (!ui.menu && !ui.help &&
                (g.phase == Phase::Play || g.phase == Phase::Shop))
            {
                box(
                    r,
                    492.f,
                    317.f,
                    140.f,
                    34.f,
                    sf::Color(4, 12, 18, 145)
                );

                text(
                    r,
                    "F1 조작법  |  TAB 채팅",
                    499.f,
                    322.f,
                    8,
                    { 220, 232, 222 }
                );

                text(
                    r,
                    "자동 난이도: " + std::to_string(std::clamp(g.count(), 1, MaxPlayers)) + "인",
                    499.f,
                    334.f,
                    8,
                    { 248, 218, 155 }
                );
            }

            // 실시간 채팅은 배경 패널 없이 텍스트만 표시해 게임 화면을 가리지 않습니다.
            if (!ui.menu && ui.chatVisible)
            {
                const std::size_t maxLines =
                    g.phase == Phase::Shop ? 3 : 5;
                const std::size_t begin =
                    ui.chatLines.size() > maxLines
                    ? ui.chatLines.size() - maxLines
                    : 0;

                float y =
                    g.phase == Phase::Shop ? 292.f : 279.f;
                for (std::size_t i = begin;
                    i < ui.chatLines.size(); ++i)
                {
                    text(
                        r,
                        ui.chatLines[i],
                        13.f,
                        y + 1.f,
                        9,
                        sf::Color(0, 0, 0, 220)
                    );
                    text(
                        r,
                        ui.chatLines[i],
                        12.f,
                        y,
                        9,
                        sf::Color(245, 245, 240)
                    );
                    y += 12.f;
                }

                const std::string inputLine =
                    "> " + ui.chatInput + "_";

                text(
                    r,
                    inputLine,
                    13.f,
                    342.f,
                    9,
                    sf::Color(0, 0, 0, 230)
                );
                text(
                    r,
                    inputLine,
                    12.f,
                    341.f,
                    9,
                    sf::Color(249, 218, 155)
                );
            }

            if (
                !ui.menu &&
                ui.client &&
                ui.status.find(
                    "disconnected"
                )
                != std::string::npos
                )
            {
                panel(
                    r,
                    80.f,
                    132.f,
                    480.f,
                    90.f
                );

                text(
                    r,
                    "HOST DISCONNECTED",
                    100.f,
                    148.f,
                    20,
                    { 249, 168, 135 }
                );

                text(
                    r,
                    "Room closed. ESC returns to the menu.",
                    100.f,
                    185.f,
                    12
                );
            }

            flush(r);
        }
    };

    class Audio
    {
        std::array<
            sf::SoundBuffer,
            3>
            buffers;

        std::array<
            std::unique_ptr<
            sf::Sound>,
            3>
            sounds;

        std::uint32_t fired = 0;
        std::uint32_t hit = 0;

        Phase last =
            Phase::Lobby;

    public:
        bool muted = false;

        Audio()
        {
            for (
                int k = 0;
                k < 3;
                ++k
                )
            {
                int n =
                    k == 2
                    ? 11025
                    : 4410;

                std::vector<
                    std::int16_t>
                    samples(n);

                std::uint32_t
                    seed = 827;

                for (
                    int i = 0;
                    i < n;
                    ++i
                    )
                {
                    float t =
                        i / 22050.f;

                    float envelope =
                        1.f -
                        static_cast<float>(
                            i
                            )
                        / n;

                    seed =
                        seed *
                        1664525 +
                        1013904223;

                    float noise =
                        static_cast<float>(
                            (
                                seed >> 16
                                )
                            &
                            65535
                            )
                        / 32768.f
                        - 1.f;

                    float tone =
                        std::sin(
                            2.f *
                            Pi *
                            (
                                k == 0
                                ? 100.f
                                : k == 1
                                ? 70.f
                                : 440.f
                                )
                            *
                            t
                        );

                    samples[i] =
                        static_cast<
                        std::int16_t>(
                            (
                                k < 2
                                ?
                                0.65f *
                                noise +
                                0.35f *
                                tone
                                :
                                tone
                                )
                            *
                            envelope
                            *
                            7000.f
                            );
                }

                if (
                    buffers[k]
                    .loadFromSamples(
                        samples.data(),
                        samples.size(),
                        1,
                        22050,
                        {
                            sf::SoundChannel::Mono
                        }
                    )
                    )
                {
                    sounds[k] =
                        std::make_unique<
                        sf::Sound>(
                            buffers[k]
                        );

                    sounds[k]
                        ->setVolume(
                            20.f
                        );
                }
            }
        }

        void update(
            const Game& g)
        {
            if (!muted)
            {
                if (
                    g.shotsFired >
                    fired &&
                    sounds[0]
                    )
                {
                    sounds[0]->play();
                }

                if (
                    g.impacts >
                    hit &&
                    sounds[1]
                    )
                {
                    sounds[1]->play();
                }

                if (
                    g.phase != last &&
                    (
                        g.phase ==
                        Phase::Reward ||
                        g.phase ==
                        Phase::Won
                        )
                    &&
                    sounds[2]
                    )
                {
                    sounds[2]->play();
                }
            }

            fired =
                g.shotsFired;

            hit =
                g.impacts;

            last =
                g.phase;
        }
    };
}