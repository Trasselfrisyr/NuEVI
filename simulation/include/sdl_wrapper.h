#if defined(__APPLE__)

#include <SDL2/SDL.h>
#elif defined(__LINUX__)

#include <SDL.h>


#else
#error "Platform not supported, fix include paths and stuff.."
#endif
