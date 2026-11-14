
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <SDL2/SDL.h>

#define MAX_PLAYERS 2

typedef struct {
    SDL_Joystick *joystick;
    SDL_JoystickID instance_id;
    bool connected;

    Sint16 axis[8];
    Uint8 buttons[32];
    Uint8 hats[4];

    char name[128];
} JoyCon;

JoyCon players[MAX_PLAYERS];

// コントローラーを接続する
void connect_joycon(int device_index) {

    // 空いているプレイヤー枠を探す
    int player = -1;

    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (!players[i].connected) {
            player = i;
            break;
        }
    }

    if (player == -1) {
        printf("接続可能なプレイヤー枠がありません。\n");
        return;
    }

    SDL_Joystick *joy =
        SDL_JoystickOpen(device_index);

    if (joy == NULL) {
        printf("Joy-Conの接続に失敗: %s\n",
               SDL_GetError());
        return;
    }

    players[player].joystick = joy;
    players[player].instance_id =
        SDL_JoystickInstanceID(joy);

    players[player].connected = true;

    snprintf(players[player].name,
             sizeof(players[player].name),
             "%s", SDL_JoystickName(joy));

    printf("\nプレイヤー%d 接続\n", player + 1);
    printf("名前: %s\n", players[player].name);
    printf("ボタン数: %d\n",
           SDL_JoystickNumButtons(joy));
    printf("軸数: %d\n",
           SDL_JoystickNumAxes(joy));
    printf("ハット数: %d\n",
           SDL_JoystickNumHats(joy));
}

// コントローラーを切断する
void disconnect_joycon(SDL_JoystickID id) {

    for (int i = 0; i < MAX_PLAYERS; i++) {

        if (players[i].connected &&
            players[i].instance_id == id) {

            printf("\nプレイヤー%d 切断\n", i + 1);

            SDL_JoystickClose(players[i].joystick);

            players[i].joystick = NULL;
            players[i].connected = false;

            return;
        }
    }
}

// 入力情報を取得する
void update_joycon(int player) {

    JoyCon *jc = &players[player];

    if (!jc->connected) {
        return;
    }

    SDL_JoystickUpdate();

    int axes = SDL_JoystickNumAxes(jc->joystick);
    int buttons = SDL_JoystickNumButtons(jc->joystick);
    int hats = SDL_JoystickNumHats(jc->joystick);

    for (int i = 0; i < 8; i++) {
        jc->axis[i] =
            (i < axes)
            ? SDL_JoystickGetAxis(jc->joystick, i)
            : 0;
    }

    for (int i = 0; i < 32; i++) {
        jc->buttons[i] =
            (i < buttons)
            ? SDL_JoystickGetButton(jc->joystick, i)
            : 0;
    }

    for (int i = 0; i < 4; i++) {
        jc->hats[i] =
            (i < hats)
            ? SDL_JoystickGetHat(jc->joystick, i)
            : SDL_HAT_CENTERED;
    }
}

// ゲーム側で使用する入力処理
void game_input() {

    for (int i = 0; i < MAX_PLAYERS; i++) {

        if (!players[i].connected) {
            continue;
        }

        JoyCon *jc = &players[i];

        printf("\nPlayer %d\n", i + 1);

        // ボタン入力
        for (int b = 0; b < 32; b++) {
            if (jc->buttons[b]) {
                printf("Button %d : ON\n", b);
            }
        }

        // スティック入力
        for (int a = 0; a < 8; a++) {
            if (jc->axis[a] != 0) {
                printf("Axis %d : %d\n",
                       a, jc->axis[a]);
            }
        }

        // ハット入力
        for (int h = 0; h < 4; h++) {
            if (jc->hats[h] != SDL_HAT_CENTERED) {
                printf("Hat %d : %d\n",
                       h, jc->hats[h]);
            }
        }
    }
}

int main(int argc, char *argv[]) {

    // SDL初期化
    if (SDL_Init(SDL_INIT_JOYSTICK) < 0) {
        printf("SDL初期化エラー: %s\n",
               SDL_GetError());
        return 1;
    }

    SDL_JoystickEventState(SDL_ENABLE);

    printf("Joy-Con (R) 接続プログラム\n");
    printf("--------------------------\n");

    // 現在接続されているコントローラーを検出
    int count = SDL_NumJoysticks();

    printf("検出されたコントローラー: %d\n", count);

    for (int i = 0; i < count; i++) {
        connect_joycon(i);
    }

    SDL_Event event;
    bool running = true;

    while (running) {

        // イベント処理
        while (SDL_PollEvent(&event)) {

            switch (event.type) {

                case SDL_JOYDEVICEADDED:
                    connect_joycon(event.jdevice.which);
                    break;

                case SDL_JOYDEVICEREMOVED:
                    disconnect_joycon(
                        event.jdevice.which);
                    break;

                case SDL_QUIT:
                    running = false;
                    break;

                case SDL_KEYDOWN:
                    if (event.key.keysym.sym == SDLK_ESCAPE) {
                        running = false;
                    }
                    break;
            }
        }

        // Joy-Con入力更新
        for (int i = 0; i < MAX_PLAYERS; i++) {
            update_joycon(i);
        }

        // ゲーム側の入力処理
        game_input();

        SDL_Delay(100);
    }

    // 終了処理
    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (players[i].connected) {
            SDL_JoystickClose(players[i].joystick);
        }
    }

    SDL_Quit();

    return 0;
}
