#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <arpa/inet.h>
#include "network_protocol.h"

//時間を取得する関数
long long get_current_time_us() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (long long)ts.tv_sec * 1000000LL + (ts.tv_nsec / 1000LL);
}

int main() {
    int client_sock;
    struct sockaddr_in serv_addr;
    char *server_ip = "127.0.0.1";

    if ((client_sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation error");
        exit(EXIT_FAILURE);
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, server_ip, &serv_addr.sin_addr);

    if (connect(client_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection Failed");
        exit(EXIT_FAILURE);
    }

    printf("[ClientNetworkModule] Tick制御対応クライアント起動 (Tick間隔: %d ms)\n", 
           TICK_INTERVAL_US / 1000);

    float move_count = 0.0f;
    long long next_tick_time = get_current_time_us() + TICK_INTERVAL_US;

    while (1) {
        long long current_time = get_current_time_us();

        if (current_time >= next_tick_time) {
            PlayerInputPacket input_packet;
            input_packet.packet_type = PACKET_TYPE_CLIENT_INPUT;
            input_packet.player_id = 1;
            input_packet.move_x = move_count;
            input_packet.move_y = 1.0f;
            input_packet.ninjutsu_id = 1;
            input_packet.action_trigger = 1;

            send(client_sock, &input_packet, sizeof(input_packet), 0);
            move_count += 0.1f; 

            GameStatePacket state_packet;
            ssize_t bytes = recv(client_sock, &state_packet, sizeof(state_packet), MSG_DONTWAIT);
            
            if (bytes == sizeof(GameStatePacket) && state_packet.packet_type == PACKET_TYPE_GAME_STATE) {
                printf("[Tick受信] 残り時間: %.1f秒 | 1P HP: %d | 1P位置X: %.2f\n",
                       state_packet.remaining_time,
                       state_packet.players[0].hp,
                       state_packet.players[0].pos_x);
            }

            next_tick_time += TICK_INTERVAL_US;
        } else {
            long long sleep_time = next_tick_time - current_time;
            if (sleep_time > 1000) {
                usleep(sleep_time / 2);
            }
        }
    }

    close(client_sock);
    return 0;
}
