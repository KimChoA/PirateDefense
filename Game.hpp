#pragma once
#include <SFML/System/Vector2.hpp>
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <string>
#include <vector>
#include "ItemSystem.hpp"

namespace dw
{
    constexpr float StageSeconds = 90, ShipMax = 1000, Pi = 3.14159265f;
    constexpr float WaveSeconds = 30.f;
    constexpr int MaxStage = 3;
    constexpr int MaxWave = 3;
    constexpr int MaxPlayers = 5;
    constexpr int CannonCount = 3;

    // 640 x 360 기준 새 맵 배치
    // 왼쪽: 좁은 바다 + 낚시 공간
    // 중앙: 플레이어 갑판
    // 오른쪽: 적이 들어오는 전투 구역
    constexpr float DeckLeft = 92.f;
    constexpr float DeckRight = 332.f;
    constexpr float DeckTop = 58.f;
    constexpr float DeckBottom = 322.f;

    constexpr float ShipHitLeft = 88.f;
    constexpr float ShipHitRight = 339.f;

    using V = sf::Vector2f;

    inline const V AmmoPoint{ 165.f, 180.f };
    inline const V FishingPoint{ 62.f, 180.f };
    inline constexpr float FishingInteractRadius = 60.f;
    inline float length(V v)
    {
        return std::sqrt(v.x * v.x + v.y * v.y);
    }
    inline float dist(V a, V b)
    {
        return length(a - b);
    }
    inline V unit(V v)
    {
        float l = length(v);
        return l > 0 ? v / l : V{ 1, 0 };
    }
    enum class Phase : int
    {
        Lobby,
        Play,
        Reward,
        Won,
        Lost,
        Shop
    };

    enum Shell : int
    {
        Normal = 0,
        Spread,
        Pierce,
        Blast,
        Flame,
        Heavy,
        ShellCount
    };

    inline const char* shellName(int n)
    {
        static const char* names[] =
        {
            "IRON",
            "SPREAD",
            "PIERCE",
            "BLAST",
            "FLAME",
            "HEAVY"
        };

        return n >= 0 && n < ShellCount
            ? names[n]
            : "NONE";
    }
    struct Input
    {
        float x = 0, y = 0;
        V aim{ 0, 180 };

        bool hold = false;
        bool tap = false;
        bool drop = false;
        bool fire = false;


        bool melee = false;
        bool brace = false;

        // 유동호 병합: 물고기 섭취(회복) 입력
        bool useFish = false;

        // 상점 입력
        int buy = -1;              // 구매 제안 상품 번호
        int sell = 0;              // 판매할 물고기 수
        bool ready = false;        // 상점 준비 완료

        int shopVisit = 0;         // 현재 몇 번째 상점인지

        int purchaseVote = -1;     // -1 미투표 / 0 반대 / 1 찬성
        int purchaseVoteId = 0;    // 현재 구매 투표 번호

        int select = -1;
        int vote = -1;

        void edgesOff()
        {
            tap = drop = fire = melee = useFish = false;

            select = vote = buy = -1;

            sell = 0;
            ready = false;
            purchaseVote = -1;
        }
    };

    inline void mergeInput(
        Input& pending,
        const Input& fresh)
    {
        Input merged = fresh;

        merged.tap |= pending.tap;
        merged.drop |= pending.drop;
        merged.fire |= pending.fire;
        merged.melee |= pending.melee;
        merged.useFish |= pending.useFish;
        merged.ready |= pending.ready;

        if (merged.select < 0)
            merged.select = pending.select;

        if (merged.vote < 0)
            merged.vote = pending.vote;

        if (merged.buy < 0)
            merged.buy = pending.buy;

        if (merged.sell == 0)
            merged.sell = pending.sell;

        if (merged.purchaseVote < 0)
            merged.purchaseVote =
            pending.purchaseVote;

        if (merged.shopVisit == 0)
            merged.shopVisit =
            pending.shopVisit;

        if (merged.purchaseVoteId == 0)
            merged.purchaseVoteId =
            pending.purchaseVoteId;

        pending = merged;
    }

    struct Player
    {
        bool active = false;
        V p{ 320, 218 };
        int held = -1, selected = 0, mount = -1, vote = -1;

        // 유동호 병합: 낚시 상태는 플레이어에 귀속
        bool fishing = false;
        float fishProgress = 0.f; // 0.0 ~ 1.0
        bool fishCancelArmed = false; // 시작 E가 끝난 뒤 다음 E부터 취소 허용

        // 낚시 획득 연출
        float fishCatchFx = 0.f;
        int fishCatchAmount = 0;
    };

    struct Cannon
    {
        V p;
        int side = 0, ammo = -1, owner = -1, loader = -1;
        float progress = 0, angle = 0;
    };
    struct Hazard
    {
        V p;
        bool fire = false;
        float progress = 0, age = 0;
    };
    struct Drop
    {
        V p;
        int type = 0;
    };
    struct Enemy
    {
        int id = 0, type = 0, side = 0;
        V p;

        float hp = 0, maxHp = 0, cooldown = 0, burn = 0, freeze = 0, age = 0;

        // 잡몹 방어력
        float defense = 0.f;
    };
    struct Shot
    {
        V p, v;
        int type = 0;

        bool hostile = false;

        float damage = 0;
        float life = 4;

        std::vector<int> hit;

        bool black = false;
    };

    struct Effect
    {
        V p;
        float life = 0, maxLife = 0;
        int kind = 0;
    };

    enum class BossType
    {
        None = 0,
        GhostShip,   // 스테이지 1
        Leviathan,   // 스테이지 2
        Kraken       // 스테이지 3
    };

    struct Boss
    {
        bool active = false;
        V p{ 565, 180 };
        float hp = 0, maxHp = 0, cooldown = 5, warning = 0, burn = 0, freeze = 0;
        int pattern = 0, side = 1;
        BossType type = BossType::None;
        float specialCooldown = 0.f;
    };

    // 유동호 병합: 보스 사각형 장판 공격
    struct AreaAttack
    {
        float x;
        float y;
        float width;
        float height;
        float damage;

        bool contains(const V& pos, float margin = 6.f) const
        {
            return
                pos.x >= x - margin &&
                pos.x <= x + width + margin &&
                pos.y >= y - margin &&
                pos.y <= y + height + margin;
        }
    };

    inline const AreaAttack TentacleSlamArea{
        240.f, 238.f, 160.f, 24.f, 40.f
    };

    inline const AreaAttack FireSurgeArea{
        240.f, 118.f, 160.f, 24.f, 30.f
    };

    struct CrewCombat
    {
        float hp = 100.f;             // 개인 체력
        float maxHp = 100.f;          // 개인 최대 체력
        float invulnerable = 0.f;     // 피격 후 무적시간
        float revive = 0.f;           // 부활 대기시간

        float attackCooldown = 0.f;   // C 근접공격 쿨타임
        float swing = 0.f;            // 근접공격 애니메이션 시간

        bool bracing = false;         // C 홀드 파도 방어
    };



    // 유령선 보스 - 유령 선원

    struct GhostSailor
    {
        V p;                          // 현재 위치
        V strike;                     // 공격 목표 위치

        float hp = 60.f;

        float boarding = 1.2f;        // 승선 시간
        float cooldown = 0.6f;        // 공격 쿨타임
        float windup = 0.f;           // 공격 준비시간
    };



    // 크라켄 보스 - 촉수

    struct DeckTentacle
    {
        V p;

        float hp = 90.f;

        float warning = 1.6f;
        float slamTimer = 5.f;
        float flash = 0.f;

        float slamAnimation = 0.f;

        // 촉수가 어느 방향으로 쓰러질지
        float slamDirection() const
        {
            return p.x > (DeckLeft + DeckRight) * 0.5f
                ? -1.f
                : 1.f;
        }

        // 촉수 공격 지점
        V slamPoint() const
        {
            return p + V{ slamDirection() * 48.f, 0.f };
        }

        // 촉수 회전각
        float slamAngle() const
        {
            if (slamAnimation > 0.f)
            {
                float recovery =
                    std::clamp(
                        (0.45f - slamAnimation - 0.08f) / 0.37f,
                        0.f,
                        1.f
                    );

                recovery =
                    recovery * recovery * (3.f - 2.f * recovery);

                return (Pi * 0.5f) * (1.f - recovery);
            }

            if (slamTimer > 0.75f)
                return 0.f;

            if (slamTimer > 0.16f)
            {
                float t = (0.75f - slamTimer) / 0.59f;
                return -0.6f * t * t * (3.f - 2.f * t);
            }

            float t =
                std::clamp(
                    (0.16f - slamTimer) / 0.16f,
                    0.f,
                    1.f
                );

            return -0.6f + (Pi * 0.5f + 0.6f) * t * t;
        }

        // 촉수 그릴 때 각 마디 위치
        V segmentPoint(int j, float age) const
        {
            float f = j / 8.f;
            float angle = slamAngle();

            bool moving =
                slamAnimation > 0.f ||
                slamTimer <= 0.75f;

            return p + V{
                slamDirection() *
                    std::sin(angle) *
                    48.f *
                    f,

                -std::cos(angle) *
                    48.f *
                    f
            }
                + V{
                    0.f,
                    moving
                        ? 0.f
                        : std::sin(age * 2.f + j * 0.4f)
                            * 3.f * f
            };
        }
    };


    // 레비아탄 / 크라켄 - 큰 파도
    struct GreatWave
    {
        bool active = false;          // 파도 존재 여부
        bool impacting = false;       // 실제 피격 판정 시작 여부

        float warning = 0.f;          // 경고 시간
        float front = DeckRight + 30.f;

        // 이미 맞은 플레이어 기록
        std::uint32_t hitPlayers = 0;
    };
    class Game
    {
    public:
        Phase phase = Phase::Lobby;
        int stage = 1, wave = 1, kills = 0, nextId = 1, reward = 0;
        // 현재 웨이브 안에서 몇 번째 적 무리가 생성됐는지
        int spawnGroup = 0;

        float waveTimer = WaveSeconds;
        float waveTransitionTimer = 0.f;
        bool bossWave = false;

        float hp = ShipMax, maxHp = ShipMax, time = WaveSeconds, age = 0, spawn = 2, repairBonus = 0, leakScale = 1;


        std::uint32_t rng = 91827, shotsFired = 0, impacts = 0;
        std::array<Player, MaxPlayers> players{};

        AmmoManager ammo;
        GoldManager gold;
        UpgradeManager upgrades;

        int fishCount = 0;

        // 상점 상태
        int shopVisit = 0;

        float shopRemaining = 0.f;

        // 각 플레이어 준비 여부
        std::array<bool, MaxPlayers> shopReady{};

        // 구매 결과 메시지용
        // 0 없음 / 1 구매성공 / 2 골드부족
        // 3 최대강화 / 4 판매완료 / 5 투표부결
        std::array<int, MaxPlayers> shopResult{};

        std::array<float, MaxPlayers> shopResultTime{};


        // 구매 투표
        bool purchaseVoteActive = false;

        int purchaseVoteItem = -1;
        int purchaseVoteProposer = -1;
        int purchaseVoteId = 0;

        float purchaseVoteRemaining = 0.f;

        // -1 미투표 / 0 반대 / 1 찬성
        std::array<int, MaxPlayers> purchaseBallots{
            -1, -1, -1, -1, -1
        };

        std::array<CrewCombat, MaxPlayers> crew{};

        // 오른쪽 한쪽에 대포 3문 배치
        std::array<Cannon, CannonCount> cannons{
             {{{307.f, 110.f}, 1},
              {{307.f, 180.f}, 1},
              {{307.f, 250.f}, 1}}
        };
        std::vector<Enemy> enemies;
        std::vector<Shot> shots;
        std::vector<Hazard> hazards;
        std::vector<Drop> drops;
        std::vector<Effect> effects;

        Boss boss;

        std::vector<GhostSailor> sailors;
        std::vector<DeckTentacle> tentacles;

        GreatWave greatWave;
        float deckSpawn = 3.f;

        Game()
        {
            players[0].active = true;
            crew[0].hp = 100.f;
            crew[0].maxHp = 100.f;
            players[0].fishing = false;
            players[0].fishProgress = 0.f;
            players[0].fishCancelArmed = false;
            players[0].fishCatchFx = 0.f;
            players[0].fishCatchAmount = 0;
        }

        // 스테이지에 따른 보스 종류 반환
        BossType bossTypeForStage() const
        {
            switch (stage)
            {
            case 1:
                return BossType::GhostShip;

            case 2:
                return BossType::Leviathan;

            case 3:
                return BossType::Kraken;

            default:
                return BossType::None;
            }
        }

        void openShop()
        {
            phase = Phase::Shop;

            ++shopVisit;

            // 상점 이용 시간 30초
            shopRemaining = 30.f;

            // 플레이어 준비 상태 초기화
            shopReady.fill(false);

            // 결과 메시지 초기화
            shopResult.fill(0);
            shopResultTime.fill(0.f);

            // 구매 투표 초기화
            purchaseVoteActive = false;

            purchaseVoteItem = -1;
            purchaseVoteProposer = -1;

            purchaseVoteRemaining = 0.f;

            purchaseBallots.fill(-1);
        }

        int purchaseVoteWeight(int choice) const
        {
            int total = 0;

            for (int id = 0; id < MaxPlayers; ++id)
            {
                if (
                    players[id].active &&
                    purchaseBallots[id] == choice
                    )
                {
                    // 방장 P1 = 1.5표
                    // 참가자 = 1표
                    //
                    // 소수점 대신
                    // 방장 3 / 참가자 2 단위로 계산
                    total += (id == 0) ? 3 : 2;
                }
            }

            return total;
        }

        void finishPurchaseVote()
        {
            if (!purchaseVoteActive)
                return;

            purchaseVoteActive = false;
            purchaseVoteRemaining = 0.f;

            // 기본값: 부결
            int result = 5;

            // 찬성이 반대보다 많아야 통과
            // 동률은 부결
            if (
                purchaseVoteWeight(1) >
                purchaseVoteWeight(0)
                )
            {
                ShopManager shop(
                    gold,
                    ammo,
                    upgrades
                );

                const auto item =
                    ShopItems[purchaseVoteItem];

                // 배 HP 강화 전 보너스 저장
                int oldHpBonus =
                    upgrades.getShipHpBonus();

                if (shop.isMaxLevel(item))
                {
                    result = 3;
                }
                else if (shop.buyItem(item))
                {
                    result = 1;

                    // 배 HP 강화 구매 시
                    // 실제 Game maxHp에도 적용
                    int newHpBonus =
                        upgrades.getShipHpBonus();

                    int extraHp =
                        newHpBonus - oldHpBonus;

                    if (extraHp > 0)
                    {
                        maxHp +=
                            static_cast<float>(extraHp);

                        hp = std::min(
                            maxHp,
                            hp + static_cast<float>(extraHp)
                        );
                    }
                }
                else
                {
                    // 골드 부족 등
                    result = 2;
                }
            }

            // 모든 플레이어에게 결과 표시
            for (int id = 0; id < MaxPlayers; ++id)
            {
                if (!players[id].active)
                    continue;

                shopResult[id] = result;
                shopResultTime[id] = 3.f;
            }

            purchaseVoteItem = -1;
            purchaseVoteProposer = -1;
        }


        // 상점 업데이트
        void updateShop(
            float dt,
            const std::array<Input, MaxPlayers>& inputs)
        {
            // 구매 투표 중에는 상점 30초 타이머 정지
            if (!purchaseVoteActive)
            {
                shopRemaining =
                    std::max(
                        0.f,
                        shopRemaining - dt
                    );
            }

            // 구매 투표는 별도 10초
            if (purchaseVoteActive)
            {
                purchaseVoteRemaining =
                    std::max(
                        0.f,
                        purchaseVoteRemaining - dt
                    );
            }

            bool proposedThisStep = false;

            // 플레이어별 상점 입력
            if (shopRemaining > 0.f)
            {
                for (int id = 0; id < MaxPlayers; ++id)
                {
                    shopResultTime[id] =
                        std::max(
                            0.f,
                            shopResultTime[id] - dt
                        );

                    if (!players[id].active)
                        continue;

                    const auto& in =
                        inputs[id];

                    // 이전 상점 입력 무시
                    if (in.shopVisit != shopVisit)
                        continue;


                    // 구매 찬반 투표
                    if (
                        purchaseVoteActive &&
                        in.purchaseVoteId == purchaseVoteId &&
                        (
                            in.purchaseVote == 0 ||
                            in.purchaseVote == 1
                            ) &&
                        purchaseBallots[id] < 0
                        )
                    {
                        purchaseBallots[id] =
                            in.purchaseVote;
                    }


                    // 준비 완료한 플레이어는 새 구매 제안 불가
                    if (shopReady[id])
                        continue;

                    // 물고기 판매
                    if (
                        in.sell > 0 &&
                        fishCount > 0 &&
                        !purchaseVoteActive
                        )
                    {
                        int amount =
                            std::min(
                                in.sell,
                                fishCount
                            );

                        fishCount -= amount;

                        gold.addGold(
                            amount * FishPrice
                        );

                        shopResult[id] = 4;
                        shopResultTime[id] = 2.f;

                        continue;
                    }

                    // 구매 제안
                    if (
                        in.buy >= 0 &&
                        in.buy < ShopItemCount &&
                        !purchaseVoteActive &&
                        !proposedThisStep &&
                        in.purchaseVoteId == purchaseVoteId
                        )
                    {
                        ShopManager shop(
                            gold,
                            ammo,
                            upgrades
                        );

                        const auto item =
                            ShopItems[in.buy];

                        // 최대 강화
                        if (shop.isMaxLevel(item))
                        {
                            shopResult[id] = 3;
                            shopResultTime[id] = 2.f;
                        }

                        // 골드 부족
                        else if (
                            !gold.canAfford(
                                shop.getPrice(item)
                            )
                            )
                        {
                            shopResult[id] = 2;
                            shopResultTime[id] = 2.f;
                        }

                        else
                        {
                            // 구매 투표 시작
                            purchaseVoteActive = true;
                            proposedThisStep = true;

                            purchaseVoteItem =
                                in.buy;

                            purchaseVoteProposer =
                                id;

                            ++purchaseVoteId;

                            purchaseVoteRemaining =
                                10.f;

                            purchaseBallots.fill(-1);

                            // 제안자는 자동 찬성
                            purchaseBallots[id] = 1;
                        }
                    }


                    // 준비 완료
                    if (
                        in.ready &&
                        !purchaseVoteActive
                        )
                    {
                        shopReady[id] = true;
                    }
                }
            }


            // 구매 투표 종료
            if (purchaseVoteActive)
            {
                bool allVoted = true;

                for (int id = 0; id < MaxPlayers; ++id)
                {
                    if (
                        players[id].active &&
                        purchaseBallots[id] < 0
                        )
                    {
                        allVoted = false;
                    }
                }

                if (
                    allVoted ||
                    purchaseVoteRemaining <= 0.f
                    )
                {
                    finishPurchaseVote();
                }
            }


            // 전원 준비 확인
            bool allReady = true;

            for (int id = 0; id < MaxPlayers; ++id)
            {
                if (
                    players[id].active &&
                    !shopReady[id]
                    )
                {
                    allReady = false;
                }
            }


            // 상점 종료
            if (
                shopRemaining <= 0.f ||
                (
                    allReady &&
                    !purchaseVoteActive
                    )
                )
            {
                closeShopAndStartNextStage();
            }
        }

        void closeShopAndStartNextStage()
        {
            // 다음 스테이지
            ++stage;

            wave = 1;

            phase = Phase::Play;

            // 일반 웨이브 30초
            time = WaveSeconds;

            spawn = 3.f;
            spawnGroup = 0;

            // 이전 전투 상태 정리
            enemies.clear();
            shots.clear();
            hazards.clear();
            drops.clear();
            effects.clear();

            sailors.clear();
            tentacles.clear();
            greatWave = {};

            boss = Boss{};

            // 대포 초기화
            for (auto& c : cannons)
            {
                c.ammo = -1;
                c.owner = -1;
                c.loader = -1;
                c.progress = 0.f;
            }

            // 플레이어 위치/상태 초기화
            for (int i = 0; i < MaxPlayers; ++i)
            {
                auto& p = players[i];

                p.mount = -1;
                p.held = -1;
                p.vote = -1;

                crew[i] = CrewCombat{};
                p.fishing = false;
                p.fishProgress = 0.f;

                if (p.active)
                {
                    p.p = {
                        178.f + i * 15.f,
                        218.f
                    };
                }
            }

            // 상점 상태 종료
            shopRemaining = 0.f;
            shopReady.fill(false);

            purchaseVoteActive = false;
            purchaseVoteItem = -1;
            purchaseVoteProposer = -1;
            purchaseVoteRemaining = 0.f;
            purchaseBallots.fill(-1);
        }

        float enemyDamage(
            const Enemy& enemy,
            float rawDamage) const
        {
            // 방어력만큼 직접 피해 감소
            // 아무리 방어력이 높아도 최소 1 피해
            return std::max(
                1.f,
                rawDamage - enemy.defense
            );
        }

        void damageEnemy(
            Enemy& enemy,
            float rawDamage)
        {
            enemy.hp -= enemyDamage(enemy, rawDamage);
        }
        int ammoCount(int type) const
        {
            return ammo.getAmmoCount(
                itemAmmo(type)
            );
        }
        int count() const
        {
            return static_cast<int>(
                std::count_if(players.begin(), players.end(), [](const Player& p) { return p.active; }));
        }
        int difficulty() const
        {
            int population = count();

            if (population <= 1)
                return 0;

            if (population <= 3)
                return 1;

            return 2;
        }
        float random()
        {
            rng ^= rng << 13;
            rng ^= rng >> 17;
            rng ^= rng << 5;
            return (rng % 100000) / 100000.f;
        }
        int nearCannon(const Player& p) const
        {
            int n = -1;
            float best = 25;
            for (int i = 0; i < static_cast<int>(cannons.size()); ++i)
                if (float d = dist(p.p, cannons[i].p); d < best)
                {
                    best = d;
                    n = i;
                }
            return n;
        }
        int nearHazard(const Player& p) const
        {
            int n = -1;
            float best = 20;
            for (int i = 0; i < static_cast<int>(hazards.size()); ++i)
                if (float d = dist(p.p, hazards[i].p); d < best)
                {
                    best = d;
                    n = i;
                }
            return n;
        }
        void addPlayer(int id)
        {
            players[id] = Player{};
            players[id].active = true;
            players[id].p = { 178.f + id * 15.f, 218.f };

            crew[id] = CrewCombat{};
            crew[id].hp = 100.f;
            crew[id].maxHp = 100.f;

            players[id].fishing = false;
            players[id].fishProgress = 0.f;
            players[id].fishCancelArmed = false;
            players[id].fishCatchFx = 0.f;
            players[id].fishCatchAmount = 0;
        }
        void leave(int id)
        {
            auto& p = players[id];
            if (p.held >= 0 && drops.size() < 32)
                drops.push_back({ p.p, p.held });
            for (auto& c : cannons)
            {
                if (c.owner == id)
                    c.owner = -1;
                if (c.loader == id)
                {
                    c.loader = -1;
                    c.progress = 0;
                }
            }
            p = Player{};
        }
        void start()
        {
            auto old = players;
            *this = Game{};
            for (int i = 0; i < 5; ++i)
            {
                players[i].active = old[i].active;
                players[i].p = { 178.f + i * 15.f, 218.f };
                players[i].fishing = false;
                players[i].fishProgress = 0.f;
                players[i].fishCancelArmed = false;
                players[i].fishCatchFx = 0.f;
                players[i].fishCatchAmount = 0;

                crew[i] = CrewCombat{};
                crew[i].hp = 100.f;
                crew[i].maxHp = 100.f;
            }
            phase = Phase::Play;
        }
        void effect(V p, int kind, float life = .45f)
        {
            if (effects.size() < 100)
                effects.push_back({ p, life, life, kind });
        }
        void shipHit(V p, float amount, bool fire)
        {
            hp = std::max(0.f, hp - amount);
            ++impacts;
            effect(p, 1);
            // 새 갑판 안쪽에 수리 지점을 만들고 탄약고와 겹치지 않게 함.
            V site{
                std::clamp(p.x, 112.f, 310.f),
                std::clamp(p.y, 70.f, 308.f)
            };

            if (dist(site, AmmoPoint) < 34.f)
                site.y = site.y < AmmoPoint.y ? 140.f : 220.f;
            if (hazards.size() < 10 && std::none_of(hazards.begin(), hazards.end(),
                [&](const Hazard& h) { return dist(h.p, site) < 22; }))
                hazards.push_back({ site, fire });
        }
        void spawnEnemyGroup()
        {
            ++spawnGroup;

            int diff = difficulty();

            int spawnCount =
                diff == 0 ? 1 :
                diff == 1 ? 2 :
                4;

            int cap =
                diff == 0 ? 3 :
                diff == 1 ? 5 :
                7;

            int present =
                static_cast<int>(
                    std::count_if(
                        enemies.begin(),
                        enemies.end(),
                        [](const Enemy& e)
                        {
                            return
                                e.side == 1 &&
                                e.hp > 0;
                        }
                    )
                    );

            for (
                int j = 0;
                j < spawnCount &&
                present < cap;
                ++j
                )
            {
                int type =
                    stage == 1
                    ? (
                        spawnGroup % 4 == 0
                        ? 1
                        : 0
                        )
                    : static_cast<int>(
                        random() * 3
                        );

                float health =
                    type == 2
                    ? 110.f
                    : type == 1
                    ? 40.f
                    : 55.f;

                float defense =
                    type == 2
                    ? 10.f
                    : type == 1
                    ? 0.f
                    : 4.f;

                enemies.push_back({
                    nextId++,
                    type,
                    1,
                    {
                        660.f,
                        70.f +
                        random() * 230.f
                    },
                    health,
                    health,
                    4.f +
                    random() * 3.f
                    });

                enemies.back().defense =
                    defense;

                ++present;
            }

            float baseSpawn =
                13.f -
                stage * 1.8f -
                (wave - 1) * 0.8f;

            if (diff == 0)
                spawn = baseSpawn * 1.35f;
            else if (diff == 1)
                spawn = baseSpawn;
            else
                spawn = baseSpawn * 0.78f;
        }

        void startBoss()
        {
            // 낮/저녁에 남아 있던 적은 밤 보스전에도 자연스럽게 이어짐
            // 적을 강제로 지우지 않고 기존 탄환만 정리한다.
            shots.clear();

            boss = Boss{};

            // 보스전 특수 오브젝트 초기화
            sailors.clear();
            tentacles.clear();
            greatWave = {};

            deckSpawn = 3.f;

            // 플레이어 전투 상태 초기화
            for (auto& c : crew)
            {
                c = CrewCombat{};
            }

            boss.active = true;
            boss.side = 1;
            boss.p = { 565.f, 180.f };

            // 현재 스테이지에 맞는 보스 종류
            boss.type = bossTypeForStage();

            // 스테이지별 보스 체력
            switch (boss.type)
            {
            case BossType::GhostShip:
                // Stage 1 보스
                boss.hp = 300.f;
                boss.maxHp = 300.f;
                break;

            case BossType::Leviathan:
                // Stage 2 보스
                boss.hp = 500.f;
                boss.maxHp = 500.f;
                break;

            case BossType::Kraken:
                // Stage 3 보스
                boss.hp = 800.f;
                boss.maxHp = 800.f;
                break;

            default:
                boss.hp = 300.f;
                boss.maxHp = 300.f;
                break;
            }

            // 일반 공격 쿨타임
            boss.cooldown = 4.f;

            // 보스 고유 패턴 쿨타임
            boss.specialCooldown = 6.f;

            effect(boss.p, 3, 1.5f);
        }

        void chooseReward(int option)
        {
            if (phase != Phase::Reward || option < 0 || option > 2)
                return;

            // 보상 적용
            if (option == 0)
            {
                maxHp += 150.f;
                hp = std::min(maxHp, hp + 250.f);
            }

            if (option == 1)
            {
                repairBonus += 25.f;
                hp = std::min(maxHp, hp + 160.f);
            }

            if (option == 2)
            {
                leakScale *= 0.65f;
                hp = std::min(maxHp, hp + 160.f);
            }

            reward = option;

            // 유동호 병합: 다음 라운드 진입 전 전원 완전 부활
            reviveAllPlayers(1.0f);

            // 이전 전투 상태 정리
            enemies.clear();
            shots.clear();
            hazards.clear();
            effects.clear();
            drops.clear();

            for (auto& c : cannons)
            {
                c.ammo = -1;
                c.owner = -1;
                c.loader = -1;
                c.progress = 0.f;
            }

            for (auto& p : players)
            {
                p.mount = -1;
                p.held = -1;
                p.vote = -1;

                p.p = { 320.f, 218.f };
            }

            // 다음 Wave / Stage 결정
            if (wave < MaxWave)
            {
                // 같은 스테이지의 다음 웨이브
                ++wave;
            }
            else
            {
                // Wave 3 보스까지 끝났으면
                // 다음 스테이지로 이동
                ++stage;

                wave = 1;
            }

            // 다음 전투 준비
            phase = Phase::Play;

            time = WaveSeconds;

            spawn = 3.f;
            spawnGroup = 0;

            boss = Boss{};


            // Wave 3은 일반 웨이브가 아니라 보스전
            if (wave == MaxWave)
            {
                startBoss();
            }
        }
        void releaseCannon(int id)
        {
            for (auto& c : cannons)
            {
                if (c.owner == id)
                    c.owner = -1;

                if (c.loader == id)
                {
                    c.loader = -1;
                    c.progress = 0.f;
                }
            }

            players[id].mount = -1;
        }

        bool hurtCrew(
            int id,
            float damage,
            V push = { 0.f, 0.f })
        {
            auto& c = crew[id];
            auto& p = players[id];

            // 비활성 플레이어
            if (!p.active)
                return false;

            // 이미 쓰러진 플레이어는 추가 피격 없음
            if (c.hp <= 0.f)
                return false;

            // 피격 무적시간 중
            if (c.invulnerable > 0.f)
                return false;


            // 개인 HP 감소
            c.hp = std::max(
                0.f,
                c.hp - damage
            );

            // 연속 피격 방지
            c.invulnerable = 0.8f;


            // 대포 사용 중이었다면 해제
            releaseCannon(id);


            // 밀려나는 효과
            p.p += push;

            // 배 바깥으로 밀려나지 않도록 제한
            p.p.x = std::clamp(
                p.p.x,
                DeckLeft + 10.f,
                DeckRight - 10.f
            );

            p.p.y = std::clamp(
                p.p.y,
                DeckTop + 10.f,
                DeckBottom - 10.f
            );


            effect(
                p.p,
                1,
                0.35f
            );



            // HP 0 → 사망

            if (c.hp <= 0.f)
            {
                c.hp = 0.f;

                // 유동호 병합: 자동 부활하지 않고 다음 보상/스테이지까지 DOWN 유지
                c.revive = 0.f;
                c.bracing = false;


                // 들고 있던 포탄이 있으면 바닥에 떨어뜨림
                if (
                    p.held >= 0 &&
                    drops.size() < 32
                    )
                {
                    drops.push_back({
                        p.p,
                        p.held
                        });
                }

                p.held = -1;
            }

            return true;
        }

        void fireCannon(Cannon& cannon)
        {
            const int type = cannon.ammo;

            // 포탄 종류별 기본 속도 / 데미지
            const auto stats = shellStats(type);

            // 일반탄만 공격력 강화 적용
            float damage = stats.damage;

            if (type == Normal)
            {
                damage *= upgrades.getNormalDamageMultiplier();
            }

            // 확산탄만 5발 발사
            const int shotCount =
                type == Spread ? 5 : 1;

            for (int i = 0; i < shotCount; ++i)
            {
                // 확산탄:
                // -20도 / -10도 / 0도 / +10도 / +20도
                float angle =
                    cannon.angle +
                    (
                        type == Spread
                        ? (i - 2) * Pi / 18.f
                        : 0.f
                        );

                V velocity{
                    std::cos(angle) * cannon.side * stats.speed,
                    std::sin(angle) * stats.speed
                };

                shots.push_back({
                    cannon.p + unit(velocity) * 16.f,
                    velocity,
                    type,
                    false,
                    damage,
                    4.f,
                    {}
                    });
            }

            // 장전된 포탄 소모
            cannon.ammo = -1;

            ++shotsFired;

            effect(
                cannon.p,
                0,
                0.15f
            );
        }

        void meleeAttack(int id)
        {
            auto& c = crew[id];

            // 죽어 있거나 쿨타임 중이면 공격 불가
            if (c.hp <= 0.f || c.attackCooldown > 0.f)
                return;

            // 대포 사용 중이었다면 해제
            releaseCannon(id);

            // 공격 쿨타임 0.4초
            c.attackCooldown = 0.4f;

            // 나중에 Render에서 칼 휘두르는 연출용
            c.swing = 0.18f;

            V p = players[id].p;


            // 유령 선원 공격
            for (auto& e : sailors)
            {
                // 아직 배에 올라오는 중이면 공격 불가
                if (e.boarding > 0.f)
                    continue;

                if (e.hp <= 0.f)
                    continue;

                // 플레이어와 42픽셀 이내
                if (dist(p, e.p) <= 42.f)
                {
                    e.hp = std::max(
                        0.f,
                        e.hp - 30.f
                    );

                    effect(e.p, 2, 0.2f);
                }
            }


            // 크라켄 촉수 공격
            for (auto& t : tentacles)
            {
                // 등장 경고 중에는 공격 불가
                if (t.warning > 0.f)
                    continue;

                if (t.hp <= 0.f)
                    continue;

                // 플레이어와 45픽셀 이내
                if (dist(p, t.p) <= 45.f)
                {
                    t.hp = std::max(
                        0.f,
                        t.hp - 30.f
                    );

                    effect(t.p, 2, 0.2f);

                    // 촉수 파괴 시 보스에게 추가 피해
                    if (t.hp == 0.f)
                    {
                        boss.hp = std::max(
                            0.f,
                            boss.hp - 20.f
                        );
                    }
                }
            }
        }
        void updateCrew(
            float dt,
            const std::array<Input, MaxPlayers>& inputs)
        {
            for (int i = 0; i < MaxPlayers; ++i)
            {
                auto& c = crew[i];

                if (!players[i].active)
                    continue;


                // 각종 타이머 감소
                c.invulnerable =
                    std::max(0.f, c.invulnerable - dt);

                c.attackCooldown =
                    std::max(0.f, c.attackCooldown - dt);

                c.swing =
                    std::max(0.f, c.swing - dt);


                // 기본적으로 가드 해제
                c.bracing = false;

                // 유동호 병합: 쓰러진 플레이어는 다음 부활 처리 전까지 전투 조작 불가
                if (c.hp <= 0.f)
                {
                    c.bracing = false;
                    continue;
                }


                // 보스전이 아니면 근접 전투 기능 사용 안 함
                if (!boss.active || boss.hp <= 0.f)
                    continue;


                // 레비아탄 / 크라켄에서 C 홀드 = 파도 가드
                c.bracing =
                    (
                        boss.type == BossType::Leviathan ||
                        boss.type == BossType::Kraken
                        )
                    &&
                    inputs[i].brace;


                // C 누른 순간 = 근접공격
                // 크라켄은 C 홀드 상태여도
                // 촉수 공격이 가능하도록 처리
                if (
                    inputs[i].melee &&
                    (
                        !c.bracing ||
                        boss.type == BossType::Kraken
                        )
                    )
                {
                    meleeAttack(i);
                }


                // 가드 중이면 대포 사용 해제
                if (c.bracing)
                {
                    releaseCannon(i);
                }
            }
        }

        void updateSailors(float dt)
        {
            // 유령선 보스가 아닐 때는 실행 안 함
            if (!boss.active ||
                boss.type != BossType::GhostShip ||
                boss.hp <= 0.f)
            {
                return;
            }
            // 유령 선원 생성

            deckSpawn -= dt;

            if (deckSpawn <= 0.f)
            {
                // 보스 HP가 절반 이하이면 조금 더 강해짐
                bool enraged =
                    boss.maxHp > 0.f &&
                    boss.hp <= boss.maxHp * 0.5f;

                // 기본 2명 / 분노 상태 3명
                int diff = difficulty();

                int number =
                    diff == 0
                    ? 1
                    : diff == 1
                    ? 2
                    : enraged
                    ? 3
                    : 2;

                int maxSailors =
                    diff == 0
                    ? 2
                    : diff == 1
                    ? 3
                    : 4;

                for (
                    int i = 0;
                    i < number && sailors.size() < static_cast<std::size_t>(maxSailors);
                    ++i
                    )
                {
                    GhostSailor e;

                    // 배 오른쪽에서 승선
                    e.p = {
                        DeckRight - 18.f,
                        95.f + random() * 185.f
                    };

                    sailors.push_back(e);
                }

                // 다음 승선 시간
                if (diff == 0)
                {
                    deckSpawn =
                        enraged ? 11.f : 15.f;
                }
                else if (diff == 1)
                {
                    deckSpawn =
                        enraged ? 8.f : 12.f;
                }
                else
                {
                    deckSpawn =
                        enraged ? 6.f : 9.f;
                }
            }
            // 각 유령 선원 행동

            for (auto& e : sailors)
            {
                if (e.hp <= 0.f)
                    continue;
                // 승선 중
                if (e.boarding > 0.f)
                {
                    e.boarding =
                        std::max(
                            0.f,
                            e.boarding - dt
                        );

                    continue;
                }

                // 가장 가까운 플레이어 탐색
                int target = -1;
                float nearest = 10000.f;

                for (int i = 0; i < MaxPlayers; ++i)
                {
                    if (!players[i].active)
                        continue;

                    if (crew[i].hp <= 0.f)
                        continue;

                    float d =
                        dist(
                            e.p,
                            players[i].p
                        );

                    if (d < nearest)
                    {
                        nearest = d;
                        target = i;
                    }
                }

                // 공격 준비 중
                if (e.windup > 0.f)
                {
                    e.windup =
                        std::max(
                            0.f,
                            e.windup - dt
                        );

                    // 준비 시간이 끝나면 공격
                    if (e.windup == 0.f)
                    {
                        for (int i = 0; i < MaxPlayers; ++i)
                        {
                            if (!players[i].active)
                                continue;

                            if (crew[i].hp <= 0.f)
                                continue;

                            // 공격 시작할 때 찍어둔 위치 기준
                            if (
                                dist(
                                    players[i].p,
                                    e.strike
                                ) < 23.f
                                )
                            {
                                hurtCrew(
                                    i,
                                    15.f,
                                    unit(
                                        players[i].p - e.p
                                    ) * 12.f
                                );
                            }
                        }

                        effect(
                            e.strike,
                            1,
                            0.2f
                        );

                        e.cooldown = 1.2f;
                    }

                    continue;
                }


                if (target < 0)
                    continue;


                // 공격 쿨타임 감소
                e.cooldown =
                    std::max(
                        0.f,
                        e.cooldown - dt
                    );

                // 플레이어 추적

                if (nearest > 21.f)
                {
                    e.p +=
                        unit(
                            players[target].p - e.p
                        )
                        *
                        std::min(
                            nearest - 21.f,
                            dt * 38.f
                        );
                }

                // 근접 공격 시작

                else if (e.cooldown == 0.f)
                {
                    // 공격 시작 순간 플레이어 위치를 저장
                    // → 플레이어가 피하면 공격 회피 가능
                    e.strike =
                        players[target].p;

                    // 0.65초 공격 예고
                    e.windup = 0.65f;
                }
            }

            // 죽은 유령 선원 제거
            sailors.erase(
                std::remove_if(
                    sailors.begin(),
                    sailors.end(),

                    [](const GhostSailor& e)
                    {
                        return e.hp <= 0.f;
                    }
                ),

                sailors.end()
            );
        }

        void updateGreatWave(float dt)
        {
            // 레비아탄 보스가 아니면 실행 안 함
            if (!boss.active ||
                boss.type != BossType::Leviathan ||
                boss.hp <= 0.f)
            {
                return;
            }


            // 아직 파도가 없는 상태
            if (!greatWave.active)
            {
                boss.specialCooldown -= dt;

                if (boss.specialCooldown <= 0.f)
                {
                    greatWave = {};

                    greatWave.active = true;
                    greatWave.warning = 1.5f;
                    greatWave.impacting = false;
                    greatWave.front = DeckRight + 35.f;
                    greatWave.hitPlayers = 0;

                    // 보스 체력 절반 이하에서는 더 자주 사용
                    bool enraged =
                        boss.maxHp > 0.f &&
                        boss.hp <= boss.maxHp * 0.5f;

                    int diff = difficulty();

                    if (diff == 0)
                    {
                        boss.specialCooldown =
                            enraged ? 7.f : 9.f;
                    }
                    else if (diff == 1)
                    {
                        boss.specialCooldown =
                            enraged ? 4.5f : 6.f;
                    }
                    else
                    {
                        boss.specialCooldown =
                            enraged ? 3.5f : 5.f;
                    }
                }

                return;
            }


            // 파도 경고 시간
            if (greatWave.warning > 0.f)
            {
                float before = greatWave.warning;

                greatWave.warning =
                    std::max(
                        0.f,
                        greatWave.warning - dt
                    );

                // 경고가 끝나는 순간
                if (before > 0.f &&
                    greatWave.warning == 0.f)
                {
                    greatWave.impacting = true;

                    // 파도는 가드해도 배에는 피해
                    hp = std::max(
                        0.f,
                        hp - 40.f
                    );

                    effect(
                        { DeckRight - 20.f,
                          (DeckTop + DeckBottom) * 0.5f },
                        3,
                        0.45f
                    );
                }

                return;
            }


            // 실제 큰 파도 이동
            if (greatWave.impacting)
            {
                greatWave.front -=
                    240.f * dt;


                // 플레이어 충돌 검사
                for (int i = 0; i < MaxPlayers; ++i)
                {
                    if (!players[i].active)
                        continue;

                    if (crew[i].hp <= 0.f)
                        continue;


                    // 이미 이 파도에 맞은 플레이어
                    std::uint32_t mask =
                        (1u << i);

                    if (greatWave.hitPlayers & mask)
                        continue;


                    // 파도 앞부분이 플레이어 위치에 도달
                    if (
                        std::abs(
                            players[i].p.x -
                            greatWave.front
                        ) <= 14.f
                        )
                    {
                        greatWave.hitPlayers |= mask;


                        // C 홀드 중이면 파도 방어
                        if (crew[i].bracing)
                        {
                            effect(
                                players[i].p,
                                2,
                                0.25f
                            );

                            continue;
                        }


                        // 가드하지 않았으면 개인 HP 피해
                        hurtCrew(
                            i,
                            30.f,
                            { -22.f, 0.f }
                        );
                    }
                }


                // 화면 왼쪽을 완전히 지나가면 종료
                if (greatWave.front < DeckLeft - 40.f)
                {
                    greatWave = {};
                }
            }
        }

        void updateTentacles(float dt)
        {
            // Stage 3 크라켄 보스가 아니면 실행 안 함
            if (!boss.active ||
                boss.type != BossType::Kraken ||
                boss.hp <= 0.f)
            {
                return;
            }


            // 촉수 생성

            deckSpawn -= dt;

            if (deckSpawn <= 0.f)
            {
                // 보스 체력이 절반 이하이면 촉수 증가
                bool enraged =
                    boss.maxHp > 0.f &&
                    boss.hp <= boss.maxHp * 0.5f;

                int diff = difficulty();

                int number = 1;
                int maxTentacles = 2;

                if (diff == 1)
                {
                    number =
                        enraged ? 2 : 1;

                    maxTentacles = 3;
                }
                else if (diff == 2)
                {
                    number =
                        enraged ? 3 : 2;

                    maxTentacles = 4;
                }

                for (
                    int i = 0;
                    i < number &&
                    tentacles.size() <
                    static_cast<std::size_t>(maxTentacles);
                    ++i
                    )
                {
                    DeckTentacle t;

                    // 갑판 위 랜덤 위치에 등장
                    t.p = {
                        DeckLeft + 30.f +
                            random() *
                            (DeckRight - DeckLeft - 60.f),

                        DeckTop + 35.f +
                            random() *
                            (DeckBottom - DeckTop - 70.f)
                    };

                    t.hp = 90.f;

                    // 1.6초 동안 등장 경고
                    t.warning = 1.6f;

                    // 등장 후 첫 공격까지
                    t.slamTimer = 3.f;

                    t.flash = 0.f;
                    t.slamAnimation = 0.f;

                    tentacles.push_back(t);
                }

                // 다음 촉수 생성 시간
                if (diff == 0)
                {
                    deckSpawn =
                        enraged ? 9.f : 12.f;
                }
                else if (diff == 1)
                {
                    deckSpawn =
                        enraged ? 6.f : 9.f;
                }
                else
                {
                    deckSpawn =
                        enraged ? 4.5f : 6.5f;
                }
            }


            // 촉수 행동

            for (auto& t : tentacles)
            {
                if (t.hp <= 0.f)
                    continue;


                // 점멸 타이머
                t.flash =
                    std::max(
                        0.f,
                        t.flash - dt
                    );


                // 등장 경고
                if (t.warning > 0.f)
                {
                    t.warning =
                        std::max(
                            0.f,
                            t.warning - dt
                        );

                    continue;
                }


                // 강타 애니메이션 중
                if (t.slamAnimation > 0.f)
                {
                    t.slamAnimation =
                        std::max(
                            0.f,
                            t.slamAnimation - dt
                        );

                    continue;
                }


                // 다음 공격까지 대기
                t.slamTimer -= dt;


                // 공격 직전 점멸
                if (t.slamTimer <= 0.75f)
                {
                    t.flash = 0.12f;
                }


                // 촉수 강타
                if (t.slamTimer <= 0.f)
                {
                    t.slamAnimation = 0.45f;

                    V hitPoint =
                        t.slamPoint();


                    // 플레이어 피해
                    for (int i = 0; i < MaxPlayers; ++i)
                    {
                        if (!players[i].active)
                            continue;

                        if (crew[i].hp <= 0.f)
                            continue;


                        if (
                            dist(
                                players[i].p,
                                hitPoint
                            ) <= 32.f
                            )
                        {
                            hurtCrew(
                                i,
                                25.f,
                                unit(
                                    players[i].p - t.p
                                ) * 18.f
                            );
                        }
                    }


                    // 배에도 약간의 피해
                    hp =
                        std::max(
                            0.f,
                            hp - 15.f
                        );


                    effect(
                        hitPoint,
                        1,
                        0.35f
                    );


                    // 보스 체력 절반 이하에서는 더 빨리 공격
                    bool enraged =
                        boss.maxHp > 0.f &&
                        boss.hp <= boss.maxHp * 0.5f;

                    t.slamTimer =
                        enraged ? 3.2f : 4.5f;
                }
            }


            // 죽은 촉수 제거

            tentacles.erase(
                std::remove_if(
                    tentacles.begin(),
                    tentacles.end(),

                    [](const DeckTentacle& t)
                    {
                        return t.hp <= 0.f;
                    }
                ),

                tentacles.end()
            );
        }

        void update(float dt, const std::array<Input, 5>& inputs)
        {
            if (phase == Phase::Reward)
            {
                int voted = 0;
                std::array<int, 3> votes{};
                for (int i = 0; i < 5; ++i)
                    if (players[i].active)
                    {
                        if (inputs[i].vote >= 0 && inputs[i].vote < 3)
                            players[i].vote = inputs[i].vote;
                        if (players[i].vote >= 0)
                        {
                            ++voted;
                            ++votes[players[i].vote];
                        }
                    }
                if (voted == count() && voted > 0)
                    chooseReward(static_cast<int>(std::max_element(votes.begin(), votes.end()) - votes.begin()));
                return;
            }

            // 상점 업데이트
            if (phase == Phase::Shop)
            {
                updateShop(dt, inputs);
                return;
            }

            if (phase != Phase::Play)
                return;

            age += dt;

            updateCrew(dt, inputs);

            // Stage 1 보스 - 유령 선원
            updateSailors(dt);

            // Stage 2 보스 - 레비아탄 큰 파도
            updateGreatWave(dt);

            // Stage 3 - 크라켄 촉수
            updateTentacles(dt);

            std::array<bool, CannonCount> loading{};
            std::vector<bool> repairing(hazards.size(), false);

            for (int id = 0; id < 5; ++id)
            {
                auto& p = players[id];
                if (!p.active)
                    continue;

                auto& c = crew[id];

                if (p.fishCatchFx > 0.f)
                {
                    p.fishCatchFx = std::max(0.f, p.fishCatchFx - dt * 1.45f);

                    if (p.fishCatchFx <= 0.f)
                        p.fishCatchAmount = 0;
                }

                // 유동호 병합: 쓰러진 플레이어는 이동/대포/수리/낚시 조작 불가
                if (c.hp <= 0.f)
                {
                    p.fishing = false;
                    p.fishProgress = 0.f;
                    p.fishCancelArmed = false;
                    continue;
                }

                const auto& in = inputs[id];

                // H키: 물고기 1마리 섭취 후 체력 회복
                if (in.useFish)
                    useFishToHeal(id, 35.f);
                if (in.select >= 0 && in.select < ShellCount)
                    p.selected = in.select;

                V movement{ in.x, in.y };
                if (length(movement) > 1)
                    movement = unit(movement);
                p.p += movement * 85.f * dt;
                p.p.x = std::clamp(p.p.x, DeckLeft + 10.f, DeckRight - 10.f);
                p.p.y = std::clamp(p.p.y, 58.f, 322.f);

                // 유동호 병합: 낚시 진행/취소
                bool isMoving =
                    (in.x != 0.f || in.y != 0.f);

                if (p.fishing)
                {
                    // 시작에 사용된 E 입력이 네트워크 지연으로 한 번 더 들어와도
                    // 바로 취소되지 않도록, E가 완전히 떼어진 프레임을 확인한 뒤
                    // 다음 E 입력부터 취소 입력으로 인정한다.
                    if (!p.fishCancelArmed)
                    {
                        if (!in.hold && !in.tap)
                            p.fishCancelArmed = true;
                    }

                    if (isMoving ||
                        (p.fishCancelArmed && in.tap))
                    {
                        p.fishing = false;
                        p.fishProgress = 0.f;
                        p.fishCancelArmed = false;
                    }
                    else
                    {
                        // 기본 2.5초, 낚시 강화 배율 적용
                        p.fishProgress +=
                            dt * 0.4f *
                            upgrades.getFishingSpeedMultiplier();

                        if (p.fishProgress >= 1.f)
                        {
                            addFish(1);
                            p.fishing = false;
                            p.fishProgress = 0.f;
                            p.fishCancelArmed = false;
                            p.fishCatchFx = 1.f;
                            p.fishCatchAmount = 1;
                            effect(FishingPoint, 2, 0.30f);
                            effect(p.p, 2, 0.4f);
                        }

                        // 낚시 중에는 다른 행동 차단
                        continue;
                    }
                }

                if (in.drop && p.held >= 0 && drops.size() < 32)
                {
                    drops.push_back({ p.p, p.held });
                    p.held = -1;
                }
                int h = nearHazard(p), n = nearCannon(p);
                if (h >= 0 && in.hold)
                {
                    repairing[h] = true;
                    continue;
                }
                if (n >= 0)
                {
                    auto& c = cannons[n];

                    c.angle = std::clamp(
                        std::atan2(
                            in.aim.y - c.p.y,
                            std::max(
                                8.f,
                                (in.aim.x - c.p.x) * c.side
                            )
                        ),
                        -1.02f,
                        1.02f
                    );

                    if (
                        c.ammo >= 0 &&
                        in.fire
                        )
                    {
                        fireCannon(c);
                        continue;
                    }

                    if (
                        p.held >= 0 &&
                        c.ammo < 0 &&
                        in.hold &&
                        (c.loader < 0 || c.loader == id)
                        )
                    {
                        c.loader = id;
                        loading[n] = true;

                        c.progress +=
                            dt / upgrades.getReloadMultiplier();

                        if (c.progress >= 1.f)
                        {
                            c.ammo = p.held;
                            p.held = -1;
                            c.progress = 0.f;
                            c.loader = -1;

                            effect(c.p, 2);
                        }
                    }
                }
                else if (in.tap && p.held < 0)
                {
                    auto it = std::find_if(drops.begin(), drops.end(),
                        [&](const Drop& d) { return dist(p.p, d.p) < 18; });
                    if (it != drops.end())
                    {
                        p.held = it->type;
                        drops.erase(it);
                    }
                    else if (dist(p.p, FishingPoint) < FishingInteractRadius)
                    {
                        p.fishing = true;
                        p.fishProgress = 0.f;
                        p.fishCancelArmed = false;
                    }
                    else if (
                        dist(p.p, AmmoPoint) < 32.f &&
                        ammo.useAmmo(
                            itemAmmo(p.selected)
                        )
                        )
                    {
                        p.held = p.selected;
                    }
                }
            }
            for (int i = 0; i < static_cast<int>(cannons.size()); ++i)
                if (!loading[i])
                {
                    cannons[i].progress = 0;
                    cannons[i].loader = -1;
                }
            // Each site gets at most dt progress, even when all five players repair it.
            for (int i = static_cast<int>(hazards.size()) - 1; i >= 0; --i)
            {
                auto& h = hazards[i];
                h.age += dt;
                hp -= dt * (h.fire ? 2.6f : 1.8f) * (1 + std::min(h.age / 30.f, 1.f)) * leakScale;
                if (repairing[i])
                    h.progress +=
                    dt / upgrades.getRepairMultiplier();
                else
                    h.progress = 0;
                if (h.progress >= (h.fire ? 1.5f : 2.f))
                {
                    hp = std::min(maxHp, hp + 55 + repairBonus);

                    // 수리 / 소화 완료 보상
                    gold.addGold(30);

                    effect(h.p, 2);
                    hazards.erase(hazards.begin() + i);
                }
            }
            if (!boss.active)
            {
                spawn -= dt;
                if (spawn <= 0)
                    spawnEnemyGroup();
            }
            for (auto& e : enemies)
            {
                e.age += dt;
                if (e.burn > 0)
                {
                    e.hp -= 12 * dt;
                    e.burn -= dt;
                }
                e.freeze = std::max(0.f, e.freeze - dt);
                if (e.hp <= 0)
                    continue;
                float target =
                    e.type == 1
                    ? 382.f
                    : 404.f;

                float speed =
                    e.type == 1
                    ? 32.f
                    : e.type == 2
                    ? 12.f
                    : 18.f;

                if (e.freeze > 0)
                    speed *= .35f;

                if (std::abs(e.p.x - target) > 2.f)
                {
                    // 오른쪽에서 왼쪽으로 접근
                    e.p.x -= speed * dt;
                }
                else
                {
                    e.cooldown -= dt * (e.freeze > 0 ? .4f : 1.f);

                    if (e.cooldown <= 0)
                    {
                        V destination{
                            DeckRight - 6.f,
                            std::clamp(
                                e.p.y + (random() - .5f) * 50.f,
                                DeckTop + 4.f,
                                DeckBottom - 7.f
                            )
                        };
                        shots.push_back({ e.p,
                                         unit(destination - e.p) * (e.type == 1 ? 95.f : 75.f),
                                         e.type == 2 ? Flame : Normal,
                                         true,
                                         e.type == 2 ? 24.f : 16.f,
                                         8,
                                         {} });

                        shots.back().black = random() < 0.30f;

                        e.cooldown = e.type == 1 ? 7.f : e.type == 2 ? 10.f : 9.f;
                    }
                }
            }
            if (boss.active)
            {
                if (boss.burn > 0)
                {
                    boss.hp -= 12 * dt;
                    boss.burn -= dt;
                }
                boss.freeze = std::max(0.f, boss.freeze - dt);
                boss.p.y = 180 + std::sin(age * .35f) * 45;
                if (boss.warning > 0)
                {
                    boss.warning -= dt;
                    if (boss.warning <= 0)
                    {
                        if (boss.pattern == 0)
                        {
                            int diff = difficulty();

                            int shotCount =
                                diff == 0 ? 3 :
                                diff == 1 ? 5 :
                                7;

                            for (int i = 0; i < shotCount; ++i)
                            {
                                float ratio =
                                    shotCount <= 1
                                    ? 0.5f
                                    : static_cast<float>(i) /
                                    static_cast<float>(shotCount - 1);

                                V target{
                                    DeckRight - 3.f,
                                    80.f + ratio * 208.f
                                };

                                shots.push_back({
                                    boss.p,
                                    unit(target - boss.p) * 80.f,
                                    Normal,
                                    true,
                                    18.f,
                                    8.f,
                                    {}
                                    });

                                shots.back().black =
                                    random() < 0.30f;
                            }
                        }
                        if (boss.pattern == 1)
                        {
                            for (int i = 0; i < 3; ++i)
                                shipHit({ 280.f + i * 40, 250.f }, 20, false);

                            for (int i = 0; i < MaxPlayers; ++i)
                            {
                                if (
                                    players[i].active &&
                                    TentacleSlamArea.contains(players[i].p)
                                    )
                                {
                                    damagePlayer(
                                        i,
                                        TentacleSlamArea.damage
                                    );
                                }
                            }
                        }
                        if (boss.pattern == 2)
                        {
                            for (int i = 0; i < 3; ++i)
                                shipHit({ 280.f + i * 40, 130.f }, 16, true);

                            for (int i = 0; i < MaxPlayers; ++i)
                            {
                                if (
                                    players[i].active &&
                                    FireSurgeArea.contains(players[i].p)
                                    )
                                {
                                    damagePlayer(
                                        i,
                                        FireSurgeArea.damage
                                    );
                                }
                            }

                            // 보스도 오른쪽을 유지
                            boss.side = 1;
                            boss.p.x = 565.f;
                        }
                        boss.pattern = (boss.pattern + 1) % 3;
                        int diff = difficulty();

                        boss.cooldown =
                            diff == 0 ? 9.f :
                            diff == 1 ? 7.f :
                            5.5f;
                    }
                }
                else
                {
                    boss.cooldown -= dt * (boss.freeze > 0 ? .6f : 1.f);
                    if (boss.cooldown <= 0)
                        boss.warning = 2.f;
                }
            }
            for (auto& s : shots)
            {
                s.life -= dt;
                s.p += s.v * dt;
                if (s.life <= 0)
                    continue;
                if (s.hostile)
                {
                    // 검은 탄막
                    // 배를 통과하고 플레이어를 직접 공격
                    if (s.black)
                    {
                        for (int i = 0; i < MaxPlayers; ++i)
                        {
                            if (!players[i].active)
                                continue;

                            if (crew[i].hp <= 0.f)
                                continue;

                            // 플레이어와 충돌
                            if (
                                dist(
                                    s.p,
                                    players[i].p
                                ) <= 12.f
                                )
                            {
                                V push{ 0.f, 0.f };

                                if (length(s.v) > 0.f)
                                {
                                    push =
                                        unit(s.v) * 10.f;
                                }

                                // 개인 HP 피해
                                hurtCrew(
                                    i,
                                    s.damage,
                                    push
                                );

                                // 플레이어와 닿으면 탄막 제거
                                s.life = 0.f;

                                break;
                            }
                        }
                    }

                    // 일반 적 탄환
                    // 기존처럼 배를 공격
                    else
                    {
                        if (
                            s.p.x >= ShipHitLeft &&
                            s.p.x <= ShipHitRight &&
                            s.p.y > 48.f &&
                            s.p.y < 329.f
                            )
                        {
                            shipHit(
                                s.p,
                                s.damage,
                                s.type == Flame ||
                                random() < .2f
                            );

                            s.life = 0.f;
                        }
                    }
                }
                else
                {
                    for (auto& e : enemies)
                        if (e.hp > 0 && dist(s.p, e.p) < (e.type == 2 ? 21 : 17) &&
                            std::find(s.hit.begin(), s.hit.end(), e.id) == s.hit.end())
                        {
                            s.hit.push_back(e.id);
                            damageEnemy(e, s.damage);
                            effect(e.p, 1);
                            if (s.type == Flame)
                                e.burn = 4;

                            if (s.type == Blast)
                            {
                                effect(s.p, 3);
                                for (auto& other : enemies)
                                    if (other.id != e.id && dist(other.p, e.p) < 65)
                                        damageEnemy(other, 45.f);
                            }
                            if (s.type != Pierce)
                            {
                                s.life = 0;
                                break;
                            }
                        }
                    if (s.life > 0 && boss.active && dist(s.p, boss.p) < 43 &&
                        std::find(s.hit.begin(), s.hit.end(), -1) == s.hit.end())
                    {
                        boss.hp -= s.damage;
                        s.hit.push_back(-1);
                        effect(s.p, 1);
                        if (s.type == Flame)
                            boss.burn = 4;

                        s.life = 0;
                    }
                }
                if (s.p.x < -40 || s.p.x > 680 || s.p.y < 35 || s.p.y > 340)
                    s.life = 0;
            }
            for (const auto& e : enemies)
                if (e.hp <= 0)
                {
                    ++kills;
                    // 잡몹 처치 보상
                    gold.addGold(30);
                    effect(e.p, 3);
                }
            enemies.erase(
                std::remove_if(enemies.begin(), enemies.end(), [](const Enemy& e) { return e.hp <= 0; }),
                enemies.end());
            shots.erase(std::remove_if(shots.begin(), shots.end(), [](const Shot& s) { return s.life <= 0; }),
                shots.end());
            for (auto& e : effects)
                e.life -= dt;
            effects.erase(
                std::remove_if(effects.begin(), effects.end(), [](const Effect& e) { return e.life <= 0; }),
                effects.end());
            hp = std::max(0.f, hp);
            if (hp <= 0)
            {
                phase = Phase::Lost;
                return;
            }

            // 유동호 병합: 활성 플레이어 전원이 쓰러지면 패배
            int activeCount = 0;
            int downedCount = 0;

            for (int i = 0; i < MaxPlayers; ++i)
            {
                if (!players[i].active)
                    continue;

                ++activeCount;

                if (crew[i].hp <= 0.f)
                    ++downedCount;
            }

            if (
                activeCount > 0 &&
                downedCount == activeCount
                )
            {
                phase = Phase::Lost;
                return;
            }

            if (boss.active && boss.hp <= 0)
            {
                boss.hp = 0.f;
                boss.active = false;

                // 보스 처치 보상
                gold.addGold(30);

                enemies.clear();
                shots.clear();

                // Stage 3 보스까지 잡으면 최종 승리
                if (stage == MaxStage)
                {
                    phase = Phase::Won;
                }
                else
                {
                    // Stage 1, 2 보스 처치 후 상점
                    openShop();
                }

                return;
            }
            if (!boss.active)
            {
                time = std::max(
                    0.f,
                    time - dt
                );

                if (time <= 0.f)
                {
                    // 웨이브 전환 시 살아 있는 적을 삭제하지 않는다.
                    // 낮 -> 저녁 -> 밤으로 넘어가도 기존 적이 계속 남아 싸운다.
                    if (wave < MaxWave)
                    {
                        ++wave;

                        time = WaveSeconds;
                        spawn = 3.f;
                        spawnGroup = 0;

                        if (wave == MaxWave)
                        {
                            startBoss();
                        }
                    }

                    return;
                }
            }
        }

        // =========================================================
        // 유동호 병합: 플레이어 체력 / 물고기 인터페이스
        // =========================================================
        void damagePlayer(int id, float amount)
        {
            if (
                id < 0 ||
                id >= MaxPlayers ||
                !players[id].active ||
                amount <= 0.f
                )
            {
                return;
            }

            if (crew[id].hp <= 0.f)
                return;

            // 기존 개인 전투 시스템에 연결하여
            // 대포 해제, 피격 이펙트, 포탄 드롭 처리를 함께 사용
            hurtCrew(id, amount);
        }

        void healPlayer(int id, float amount)
        {
            if (
                id < 0 ||
                id >= MaxPlayers ||
                !players[id].active ||
                amount <= 0.f
                )
            {
                return;
            }

            // 쓰러진 플레이어는 일반 회복으로 부활하지 않음
            if (crew[id].hp <= 0.f)
                return;

            crew[id].hp =
                std::min(
                    crew[id].maxHp,
                    crew[id].hp + amount
                );

            effect(
                players[id].p,
                2,
                0.25f
            );
        }

        void reviveAllPlayers(float healRatio = 1.0f)
        {
            healRatio =
                std::clamp(
                    healRatio,
                    0.1f,
                    1.0f
                );

            for (int i = 0; i < MaxPlayers; ++i)
            {
                if (!players[i].active)
                    continue;

                crew[i].hp =
                    crew[i].maxHp * healRatio;

                crew[i].revive = 0.f;
                crew[i].invulnerable = 1.f;
                crew[i].bracing = false;

                players[i].fishing = false;
                players[i].fishProgress = 0.f;
                players[i].fishCancelArmed = false;
                players[i].fishCatchFx = 0.f;
                players[i].fishCatchAmount = 0;

                effect(
                    players[i].p,
                    2,
                    0.35f
                );
            }
        }

        void healLivingPlayers(float amount)
        {
            if (amount <= 0.f)
                return;

            for (int i = 0; i < MaxPlayers; ++i)
            {
                if (
                    players[i].active &&
                    crew[i].hp > 0.f
                    )
                {
                    healPlayer(i, amount);
                }
            }
        }

        void healLivingPlayersRatio(float ratio)
        {
            if (ratio <= 0.f)
                return;

            for (int i = 0; i < MaxPlayers; ++i)
            {
                if (
                    players[i].active &&
                    crew[i].hp > 0.f
                    )
                {
                    healPlayer(
                        i,
                        crew[i].maxHp * ratio
                    );
                }
            }
        }

        bool isPlayerAlive(int id) const
        {
            if (
                id < 0 ||
                id >= MaxPlayers ||
                !players[id].active
                )
            {
                return false;
            }

            return crew[id].hp > 0.f;
        }

        void addFish(int count = 1)
        {
            if (count > 0)
                fishCount += count;
        }

        bool consumeFish(int count = 1)
        {
            if (
                count <= 0 ||
                fishCount < count
                )
            {
                return false;
            }

            fishCount -= count;
            return true;
        }

        bool useFishToHeal(
            int playerId,
            float healAmount = 35.f)
        {
            if (
                playerId < 0 ||
                playerId >= MaxPlayers ||
                !players[playerId].active
                )
            {
                return false;
            }

            auto& c = crew[playerId];

            if (
                c.hp <= 0.f ||
                c.hp >= c.maxHp
                )
            {
                return false;
            }

            if (!consumeFish(1))
                return false;

            healPlayer(
                playerId,
                healAmount
            );

            return true;
        }

        int sellFishForGold(int amountToSell)
        {
            if (amountToSell <= 0)
                return 0;

            int actualSold =
                std::min(
                    fishCount,
                    amountToSell
                );

            fishCount -= actualSold;
            return actualSold;
        }

        std::string hint(int id) const
        {
            const auto& p = players[id];

            if (crew[id].hp <= 0.f)
                return "DOWNED - WAITING FOR NEXT STAGE";

            if (p.fishing)
                return "FISHING... (MOVE OR E TO CANCEL)";

            if (dist(p.p, FishingPoint) < FishingInteractRadius)
                return "E: start fishing / H: eat fish";

            if (nearHazard(p) >= 0)
                return "Hold E to repair / extinguish";

            int c = nearCannon(p);

            if (c >= 0)
            {
                if (cannons[c].ammo >= 0)
                    return "MOUSE aim / SPACE fire";

                if (p.held >= 0)
                    return "Hold E: load for 1 second";
            }

            if (dist(p.p, AmmoPoint) < 32.f)
                return "1-6 shell type / E collect shell";

            return "WASD move / E interact / Q drop / H eat fish";
        }
    };
}
