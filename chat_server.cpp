#include "utils.h"
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
bool check_id_exist(char **client_ids, int *clients, int num_clients,
                    char *id) {
  for (int i = 0; i <= num_clients; i++) {
    if (!strcmp(client_ids[clients[i]], id))
      return true;
  }
  return false;
}
int main() {
  // fd_set: mảng các bit mô tả trạng thái sự kiện của các socket
  fd_set fdread;

  int *clients = (int *)malloc(64 * sizeof(int));
  char **client_ids = (char **)malloc(256 * sizeof(char *));
  for (int i = 0; i < 256; i++) {
    client_ids[i] = (char *)malloc(256 * sizeof(char));
  }
  for (int i = 0; i < 256; i++) {
    strcpy(client_ids[i], "");
  }
  char *token = (char *)malloc(256 * sizeof(char));
  int num_clients = 0;
  int ret;
  // char *buf = (char *)malloc(256 * sizeof(char));
  char buf[256];
  char answer[256];
  char temp[256];
  struct tm tm;
  int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (listener == -1) {
    perror("socket() failed");
    return 1;
  }

  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  addr.sin_port = htons(9000);

  if (bind(listener, (struct sockaddr *)&addr, sizeof(addr))) {
    perror("bind() failed");
    return 1;
  }

  if (listen(listener, 5)) {
    perror("listen() failed");
    return 1;
  }

  while (1) {
    // Xóa tất cả socket trong tập fdread
    FD_ZERO(&fdread);

    // Thêm socket listener vào tập fdread
    FD_SET(listener, &fdread);
    int maxdp = listener + 1;

    // Thêm các socket client vào tập fdread
    for (int i = 0; i < num_clients; i++) {
      FD_SET(clients[i], &fdread);
      if (maxdp < clients[i] + 1)
        maxdp = clients[i] + 1;
    }

    // Chờ đến khi sự kiện xảy ra
    int ret = select(maxdp, &fdread, NULL, NULL, NULL);

    if (ret < 0) {
      perror("select() failed");
      return 1;
    }

    // Kiểm tra sự kiện có yêu cầu kết nối (kiểm tra sự kiện listener xảy đến
    // với fdread)
    if (FD_ISSET(listener, &fdread)) {
      int client = accept(listener, NULL, NULL);
      printf("Ket noi moi: %d\n", client);
      clients[num_clients++] = client;
    }

    // Kiểm tra sự kiện có dữ liệu truyền đến socket client
    for (int i = 0; i < num_clients; i++)
      // kiểm tra sự kiện clients[i] xảy đến với fdread
      if (FD_ISSET(clients[i], &fdread)) {
        ret = recv(clients[i], buf, sizeof(buf), 0);
        time_t t = time(NULL);
        tm = *localtime(&t);
        if (ret <= 0) {
          // TODO: Client đã ngắt kết nối, xóa client ra khỏi mảng
          continue;
        }
        buf[ret] = 0;
        printf("Du lieu nhan tu %d: %s\n", clients[i], buf);
        int cmp = strcmp(client_ids[clients[i]], "");
        // printf("cmp: %d\n", cmp);
        if (cmp == 0) {
          printf("Check client name.");
          strcpy(temp, buf);
          token = strtok(temp, ": ");
          printf("token:%s buf:%s ss:%d\n", token, buf, strcmp(buf, token));
          // printf("token:%s buf:%s", token, buf);
          if (!strcmp(buf, token)) {
            // printf("Client_name is invalid.");
            strcpy(buf, "Client_name is invalid.");
          } else {
            if (!strcmp(token, "")) {
              strcpy(buf, "Client_id is not defined.");
            } else {
              if (check_id_exist(client_ids, clients, num_clients, token))
                strcpy(buf, "Client_id exist.");
              else {
                printf("save client id\n");
                strcpy(client_ids[clients[i]], token);
                snprintf(buf, sizeof(buf), "client_id: %s",
                         client_ids[clients[i]]);
              }
            }
          }
          printf("send: %s\n", buf);
          send(clients[i], buf, strlen(buf), 0);
        } else {
          snprintf(answer, sizeof(answer), "%d/%d/%d %d:%d:%d %s: %s",
                   tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour,
                   tm.tm_min, tm.tm_sec, client_ids[clients[i]], buf);
          printf("Send message: %s", answer);
          for (int i = 0; i < num_clients; i++) {
            send(clients[i], answer, sizeof(answer), 0);
          }
          if (strcmp(client_ids[clients[num_clients]], ""))
            send(clients[num_clients], answer, sizeof(answer), 0);
        }
        printf("\n");
      }
  }

  close(listener);
  free(clients);
  for (int i = 0; i < 256; i++) {
    free(client_ids[i]);
  }
  free(client_ids);
  free(token);
  return 0;
}