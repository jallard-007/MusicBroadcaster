#ifndef DEBUG_H
#define DEBUG_H

// set to false to remove debug statements
#define DEBUG true

#if DEBUG
#define DEBUG_P(a) a
#else
#define DEBUG_P(a)
#endif

#endif
