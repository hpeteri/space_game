#ifndef BREAKPOINT_H
#define BREAKPOINT_H



#if defined(_MSC_VER)

#define BREAK_POINT DebugBreak();

#elif defined(__GNUC__)

#define BREAK_POINT asm("int3");

#endif




#endif
