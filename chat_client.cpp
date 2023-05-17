#include "utils.h"
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
bool check_server_access_allowance(char *str1) {
  char msg[3][256] = {"Client_name is invalid.", "Client_id is not defined.",
                      "Client_id exist."};
  for (int i = 0; i < 3; i++) {
    if (!strcmp(str1, msg[i]))
      return false;
  }
  return true;
}
int main() {
  int client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = inet_addr("127.0.0.1");
  addr.sin_port = htons(9000);

  if (connect(client, (struct sockaddr *)&addr, sizeof(addr))) {
    perror("connect() failed");
    return 1;
  }

  fd_set fdread;
  char buf[256];
  char ans[256];
  bool isAccessible = false;
  char *client_id = (char *)malloc(256 * sizeof(char));
  char *client_name = (char *)malloc(256 * sizeof(char));
  int ret;
  strcpy(ans, "");
  while (1) {
    // Xóa tập fdread (Xóa tất cả các socket đã được gắn vào)
    FD_ZERO(&fdread);

    // Gắn mô tả STDIN (bàn phím) vào tập fdread
    printf("Input your client_id: \n");
    FD_SET(STDIN_FILENO, &fdread);
    // Gắn socket vào tập fdread
    FD_SET(client, &fdread);

    // Chờ đến khi sự kiện xảy ra với các mô tả đã được gắn vào fdread
    int ret = select(client + 1, &fdread, NULL, NULL, NULL);

    if (FD_ISSET(STDIN_FILENO, &fdread)) {
      fgets(client_id, sizeof(client_id), stdin);
      client_id[strlen(client_id) - 1] = 0;
    }

    printf("Input your client_name: \n");
    if (FD_ISSET(STDIN_FILENO, &fdread)) {
      fgets(client_name, sizeof(client_name), stdin);
      client_name[strlen(client_name) - 1] = 0;
      snprintf(buf, sizeof(buf), "%s: %s", client_id, client_name);
      printf("Send: %s\n", buf);
      send(client, buf, strlen(buf), 0);
      ret = recv(client, ans, sizeof(ans), 0);
      if (ret <= 0) {
        printf("Connection is closed.\n");
      }
      if (ret < sizeof(ans)) {
        ans[ret] = 0;
      }
      printf("Received: %s\n", ans);
      if (check_server_access_allowance(ans))
        break;
    }

    // if (FD_ISSET(client, &fdread)) {
    // }
    printf("Ans: %s\n", ans);
    fflush(stdout);
  }
  while (1) {
    // Xóa tập fdread (Xóa tất cả các socket đã được gắn vào)
    FD_ZERO(&fdread);

    // Gắn mô tả STDIN (bàn phím) vào tập fdread
    printf("Send message:\n");
    FD_SET(STDIN_FILENO, &fdread);
    // Gắn socket vào tập fdread
    FD_SET(client, &fdread);

    // Chờ đến khi sự kiện xảy ra với các mô tả đã được gắn vào fdread
    int ret = select(client + 1, &fdread, NULL, NULL, NULL);
    // Kiểm tra sự kiện có dữ liệu từ bàn phím
    // send message
    if (FD_ISSET(STDIN_FILENO, &fdread)) {
      fgets(buf, sizeof(buf), stdin);
      buf[strlen(buf) - 1] = 0;
      send(client, buf, strlen(buf), 0);
    }
    // Kiểm tra sự kiện có dữ liệu từ socket
    // receive message
    if (FD_ISSET(client, &fdread)) {
      ret = recv(client, buf, sizeof(buf), 0);
      buf[ret] = 0;
      printf("Received: %s\n", buf);
    }
  }

  close(client);
  free(client_id);
  free(client_name);
  return 0;
}