#ifndef NETWORK_PROTOCOL_H
#define NETWORK_PROTOCOL_H

#include <stdint.h>

#define PORT 8080
#define MAX_PLAYERS 2

// パケット種別の定義
typedef enum {
    PACKET_TYPE_CLIENT_INPUT = 1,
    PACKET_TYPE_GAME_STATE   = 2
} PacketType;

#pragma pack(push, 1)

// クライアント -> サーバー：プレイヤーの入力データ (PlayerInput)[cite: 5, 11]
typedef struct {
    uint8_t  packet_type;   // PACKET_TYPE_CLIENT_INPUT
    uint8_t  player_id;     // 1P or 2P (1 or 2)[cite: 2, 5]
    float    move_x;        // 移動X
    float    move_y;        // 移動Y
    uint8_t  ninjutsu_id;   // 0:なし, 1:クナイ等[cite: 4]
    uint8_t  action_trigger;// アクションボタン
} PlayerInputPacket;

// プレイヤー単体の同期データ[cite: 5]
typedef struct {
    uint8_t  player_id;
    float    pos_x;
    float    pos_y;
    int32_t  hp;            // HP[cite: 5]
} PlayerSyncData;

// サーバー -> クライアント：全体のゲーム状態データ (GameState)
typedef struct {
    uint8_t        packet_type;           // PACKET_TYPE_GAME_STATE
    float          remaining_time;        // 残り時間[cite: 4]
    PlayerSyncData players[MAX_PLAYERS];  // 1P / 2P 情報[cite: 2, 5]
    uint8_t        gimmick_active;        // バリスタ起動フラグ等[cite: 4, 5]
} GameStatePacket;

#pragma pack(pop)

#endif // NETWORK_PROTOCOL_H
