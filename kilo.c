#include <unistd.h>
#include <termios.h>
#include <stdlib.h>

struct termios orig_termios;

void disableRawMode()
{
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
  //                      ^ discard any unread input
}

void enableRawMode()
{
  tcgetattr(STDIN_FILENO, &orig_termios);
  atexit(disableRawMode); // return to original atters at exist

  struct termios raw = orig_termios;
  raw.c_lflag &= ~(ECHO);
  //          ^  ^ bitwise-NOT
  //          | bitwise-AND
  // ~(ECHO) gives us '00000000000000000000000000001000'
  // We then bitwise-AND this value with the flags field, which forces the fourth bit in the flags field to become 0, and causes every other bit to retain its current value

  tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

int main() {
  enableRawMode();
  char c;
  while (read(STDIN_FILENO, &c, 1) == 1 && c != 'q')
    ;

  return 0;
}
