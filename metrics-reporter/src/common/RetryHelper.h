#pragma once

#define RETRY_TIMEOUT   (100) /* ms */

/* Macro to execute expression until condition with timeout.
 * Example:
 *   RETRY(<max-times>) {
 *     ...
 *   } DONE(<condition>);
 */

#define RETRY(_n) \
do { unsigned int _retry(_n), _tmo(RETRY_TIMEOUT); do

#define RETRY_W_TMO(_n, _ms) \
do { unsigned int _retry(_n), _tmo(_ms); do

#define DONE(_cond) \
    while (delay(_tmo), !(_cond) && ((--_retry) != 0)); \
} while (false)
