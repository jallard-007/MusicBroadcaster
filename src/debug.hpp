#ifndef DEBUG_H
#define DEBUG_H

// set to true to add debug statements
#define DEBUG false

#if DEBUG
#define DEBUG_P(a) a
#else
#define DEBUG_P(a)
#endif

#endif
