#ifndef UTIL_H_
#define UTIL_H_

#define return_defer(value) do { result = (value); goto defer; } while (0)
typedef int Errno;

#define SCREEN_WIDTH  800
#define SCREEN_HEIGHT 600

#endif  // UTIL_H_
