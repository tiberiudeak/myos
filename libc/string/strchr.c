#include <string.h>

char *strchr(const char *str, int c) {
  char *temp = (char *)str;

  while (*temp != '\0') {
    if (*temp == c) {
      return temp;
    }

    temp++;
  }

  if (c == '\0') {
    return temp;
  }

  return NULL;
}
