#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include "network_protocol.h"

int main() {
    int server_sock;
    struct sockaddr_in address;

    if ((server_sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // ポートの即時再利用を設定
    int opt = 1;
    setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_sock, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    printf("[ServerNetworkModule] Tick制御対応サーバー起動：ポート %d (Tick間隔: %d ms)\n", 
           PORT, TICK_INTERVAL_US / 1000);

    PlayerInputPacket latest_input = {0};
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    int client_connected = 0;

    while (1) {
        // selectのタイムアウト設定
        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = TICK_INTERVAL_US;

        fd_set read_fds;
        FD_ZERO(&read_fds);
        FD_SET(server_sock, &read_fds);

        int activity = select(server_sock + 1, &read_fds, NULL, NULL, &tv);

        if (activity < 0) {
            perror("Select error");
            break;
        }

        // クライアントからデータが届いた場合
        if (FD_ISSET(server_sock, &read_fds)) {
            PlayerInputPacket input_packet;
            ssize_t bytes = recvfrom(server_sock, &input_packet, sizeof(input_packet), 0,
                                     (struct sockaddr *)&client_addr, &client_addr_len);

            if (bytes == sizeof(input_packet) && input_packet.packet_type == PACKET_TYPE_CLIENT_INPUT) {
                latest_input = input_packet;
                client_connected = 1;
            }
        }

        // 一定のTickごとにゲーム状態の更新・配信を実行
        if (client_connected) {
            GameStatePacket state_packet;
            state_packet.packet_type = PACKET_TYPE_GAME_STATE;
            state_packet.remaining_time = 135.0f;[cite: 4]
            state_packet.players[0].player_id = latest_input.player_id;
            state_packet.players[0].pos_x = latest_input.move_x * 5.0f;
            state_packet.players[0].pos_y = latest_input.move_y * 5.0f;
            state_packet.players[0].hp = 100;
            state_packet.gimmick_active = 0;[cite: 4]

            sendto(server_sock, &state_packet, sizeof(state_packet), 0,
                   (struct sockaddr *)&client_addr, client_addr_len);
        }
    }

    close(server_sock);
    return 0;
}