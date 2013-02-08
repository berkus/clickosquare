#ifndef __DEBUG_H__
#define __DEBUG_H__

/*
 * Simple debug messages. Add:
 *
 *    #define DEBUG_LEVEL <n>
 *    #include "debug.h"
 *
 * to the file you are working on, or let DEBUG_LEVEL undefined to
 * disable all messages. A higher level denotes a greater importance
 * of the message.
 */

#include <glib.h>

# ifdef DEBUG_LEVEL
#    define DEBUG_MSG(level, fmt...) \
        ( (DEBUG_LEVEL) && ((level) >= DEBUG_LEVEL) ) ? \
        g_print(fmt) : (level)
#    define DEBUG_ENTER(level, fmt...) \
        ( (DEBUG_LEVEL) && ((level) >= DEBUG_LEVEL) ) ? \
        g_print(__FUNCTION__ " " fmt) : (level)
#    define DEBUG_LEAVE(level, fmt...) \
        ( (DEBUG_LEVEL) && ((level) >= DEBUG_LEVEL) ) ? \
        g_print(__FUNCTION__ " " fmt) : (level)
# else
#    define DEBUG_MSG(level, fmt...)
#    define DEBUG_ENTER(level, fmt...)
#    define DEBUG_LEAVE(level, fmt...)
# endif /* DEBUG_LEVEL */

#endif /* __DEBUG_H__ */
