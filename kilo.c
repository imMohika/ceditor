#include <unistd.h>
#include <termios.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <errno.h>

#define WITH_CTRL(k) ((k)&0x1f)

void f(const char *s)
{
  perror(s);
  exit(1);
}

struct termios orig_termios;

void disableRawMode()
{
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1)
    //                          ^ discard any unread input ^
    //                                                     ^ error handling at it's finest
    f("tcsetattr");
}

void enableRawMode()
{
  if (tcgetattr(STDIN_FILENO, &orig_termios) == -1)
    f("tcgetattr");
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

  // disabling some misc flags...
  raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);

  raw.c_cc[VMIN] = 0;
  raw.c_cc[VTIME] = 1; // in tenths of a second

  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1)
    f("tcsetattr");
}

void clearScreen()
{
  write(STDOUT_FILENO, "\x1b[2J", 4);
}

int main() {
  enableRawMode();
  while (1)
  {
    clearScreen();
    char c = '\0';
    if (read(STDIN_FILENO, &c, 1) == -1 && errno != EAGAIN)
      f("read");

    if (c == WITH_CTRL('q'))
      break;

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
