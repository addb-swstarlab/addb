/*
 * 2018.3.15
 * hssung@yonsei.ac.kr
 * Add File for relational model in Redis
 */

#include "server.h"

#define FPWRITE_NO_FLAGS 0
#define FPWRITE_NX (1<<0)     /* Set if key not exists. */
#define FPWRITE_XX (1<<1)     /* Set if key exists. */
#define FPWRITE_EX (1<<2)     /* Set if time in seconds is given */
#define FPWRITE_PX (1<<3)     /* Set if time in ms in given */

void fpWriteCommand(client *c){

	serverLog(LL_VERBOSE,"FPWRITE COMMAND START");
    int j;
    robj *expire = NULL;
    int unit = UNIT_SECONDS;
    int flags = FPWRITE_NO_FLAGS;

    for (j = 3; j < c->argc; j++) {
        char *a = c->argv[j]->ptr;
        robj *next = (j == c->argc-1) ? NULL : c->argv[j+1];

        if ((a[0] == 'n' || a[0] == 'N') &&
            (a[1] == 'x' || a[1] == 'X') && a[2] == '\0' &&
            !(flags & FPWRITE_XX))
        {
            flags |= FPWRITE_NX;
        } else if ((a[0] == 'x' || a[0] == 'X') &&
                   (a[1] == 'x' || a[1] == 'X') && a[2] == '\0' &&
                   !(flags & FPWRITE_NX))
        {
            flags |= FPWRITE_XX;
        } else if ((a[0] == 'e' || a[0] == 'E') &&
                   (a[1] == 'x' || a[1] == 'X') && a[2] == '\0' &&
                   !(flags & FPWRITE_PX) && next)
        {
            flags |= FPWRITE_EX;
            unit = UNIT_SECONDS;
            expire = next;
            j++;
        } else if ((a[0] == 'p' || a[0] == 'P') &&
                   (a[1] == 'x' || a[1] == 'X') && a[2] == '\0' &&
                   !(flags & FPWRITE_EX) && next)
        {
            flags |= FPWRITE_PX;
            unit = UNIT_MILLISECONDS;
            expire = next;
            j++;
        } else {
            addReply(c,shared.syntaxerr);
            return;
        }
    }

    c->argv[2] = tryObjectEncoding(c->argv[2]);
    setGenericCommand(c,flags,c->argv[1],c->argv[2],expire,unit,NULL,NULL);

}

void fpReadCommand(client *c) {
	serverLog(LL_VERBOSE,"FPREAD COMMAND START");
    getGenericCommand(c);
}
