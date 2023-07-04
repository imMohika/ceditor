#include <unistd.h>
#include <termios.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>

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

  // turn off canonical mode
  raw.c_lflag &= ~(ICANON);

  // disable ctrl-z and ctrl-c (keys for SIGTSTP signal)
  raw.c_lflag &= ~(ISIG);

  // disable ctrl-s and ctrl-q (software flow control keys)
  raw.c_lflag &= ~(IXON);

  // disable ctrl-v and ctrl-o (on macos)
  raw.c_lflag &= ~(IEXTEN);

  // disable ctrl-m
  raw.c_lflag &= ~(IEXTEN);

  // disable output processing (eg. converting \n to \r\n)
  raw.c_lflag &= ~(OPOST);

  tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

int main() {
  enableRawMode();
  char c;
  while (read(STDIN_FILENO, &c, 1) == 1 && c != 'q')
  {
    if (iscntrl(c))
    //  ^ checks if character is a 'control character'
    {
      // print the ASCII of the control character
      printf("%d\n", c);
    }
    else
    {
      // print both the ASCII and the character of the printable character
      printf("%d ('%c')\n", c, c);
    }
  }

  return 0;
}
