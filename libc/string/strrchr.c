#include <string.h>

char *strrchr(const char *str, int c) {
  char *temp = (char *)str;
  char *addr = NULL;

  while (*temp != '\0') {
    if (*temp == c) {
      addr = temp;
    }

    temp++;
  }

  if (c == '\0') {
    return temp;
  }

  return addr;
}