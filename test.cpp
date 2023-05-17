#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int main() {
  char **arr = (char **)malloc(256 * sizeof(char *));
  for (int i = 0; i < 256; i++) {
    arr[i] = (char *)malloc(256 * sizeof(char));
  }
  strcpy(arr[1], "10");
  for (int i = 0; i < 10; i++) {
    printf("%s\n", arr[i]);
  }
  char str[256] = "hello: ";
  char *token = (char *)malloc(256 * sizeof(char));
  printf("str: %s\n", str);
  token = strtok(str, ": ");
  printf("%s\n%s\n%d", str, token, strcmp(str, token));
  return 0;
}