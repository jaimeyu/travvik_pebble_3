#include "util.h"

// Thanks to https://stackoverflow.com/questions/3440726/what-is-the-proper-way-of-implementing-a-good-itoa-function
// So I don't have to roll my own.
int itoa(int value, char *sp, int radix)
{
  char tmp[16];// be careful with the length of the buffer
  char *tp = tmp;
  int i;
  unsigned v;
  for (int i = 0; i < 16; i++){
    sp[i] = '\0';
    tmp[i] = '\0';
    }

//  APP_LOG(APP_LOG_LEVEL_DEBUG, "%s:value:%d", __func__,value);
  int sign = (radix == 10 && value < 0);
  if (sign)
    v = -value;
  else
    v = (unsigned)value;

  while (v || tp == tmp)
  {
    i = v % radix;
    v /= radix; // v/=radix uses less CPU clocks than v=v/radix does
    if (i < 10)
      *tp++ = i+'0';
    else
      *tp++ = i + 'a' - 10;
  }

  int len = tp - tmp;

  if (sign)
  {
    *sp++ = '-';
    len++;
  }

  while (tp > tmp)
    *sp++ = *--tp;

  return len;
}
