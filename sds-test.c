/* SDS (Simple Dynamic Strings), A C dynamic strings library.
 *
 * Copyright (c) 2006-2014, Salvatore Sanfilippo <antirez at gmail dot com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of Redis nor the names of its contributors may be used
 *     to endorse or promote products derived from this software without
 *     specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "sds.h"
#include "testhelp.h"

int main(void) {
    {
        struct sdshdr *sh;
        int status = SDS_OK;
        sds x = sdsnew("foo",&status), y;

        test_cond("Create a string and obtain the length",
            sdslen(x) == 3 && memcmp(x,"foo\0",4) == 0 && status == SDS_OK)

        sdsfree(x);
        x = sdsnewlen("foo",2,&status);
        test_cond("Create a string with specified length",
            sdslen(x) == 2 && memcmp(x,"fo\0",3) == 0 && status == SDS_OK)

        x = sdscat(x,"bar",&status);
        test_cond("Strings concatenation",
            sdslen(x) == 5 && memcmp(x,"fobar\0",6) == 0 && status == SDS_OK)

        x = sdscpy(x,"a",&status);
        test_cond("sdscpy() against an originally longer string",
            sdslen(x) == 1 && memcmp(x,"a\0",2) == 0 && status == SDS_OK)

        x = sdscpy(x,"xyzxxxxxxxxxxyyyyyyyyyykkkkkkkkkk",&status);
        test_cond("sdscpy() against an originally shorter string",
            sdslen(x) == 33 &&
            memcmp(x,"xyzxxxxxxxxxxyyyyyyyyyykkkkkkkkkk\0",33) == 0 && status == SDS_OK)

        sdsfree(x);
        x = sdscatprintf(sdsempty(),&status,"%d",123);
        test_cond("sdscatprintf() seems working in the base case",
            sdslen(x) == 3 && memcmp(x,"123\0",4) ==0 && status == SDS_OK)

        sdsfree(x);
        x = sdsnew("xxciaoyyy",&status);
        assert(status == SDS_OK);
        sdstrim(x,"xy");
        test_cond("sdstrim() correctly trims characters",
            sdslen(x) == 4 && memcmp(x,"ciao\0",5) == 0)

        y = sdsdup(x);
        sdsrange(y,1,1);
        test_cond("sdsrange(...,1,1)",
            sdslen(y) == 1 && memcmp(y,"i\0",2) == 0)

        sdsfree(y);
        y = sdsdup(x);
        sdsrange(y,1,-1);
        test_cond("sdsrange(...,1,-1)",
            sdslen(y) == 3 && memcmp(y,"iao\0",4) == 0)

        sdsfree(y);
        y = sdsdup(x);
        sdsrange(y,-2,-1);
        test_cond("sdsrange(...,-2,-1)",
            sdslen(y) == 2 && memcmp(y,"ao\0",3) == 0)

        sdsfree(y);
        y = sdsdup(x);
        sdsrange(y,2,1);
        test_cond("sdsrange(...,2,1)",
            sdslen(y) == 0 && memcmp(y,"\0",1) == 0)

        sdsfree(y);
        y = sdsdup(x);
        sdsrange(y,1,100);
        test_cond("sdsrange(...,1,100)",
            sdslen(y) == 3 && memcmp(y,"iao\0",4) == 0)

        sdsfree(y);
        y = sdsdup(x);
        sdsrange(y,100,100);
        test_cond("sdsrange(...,100,100)",
            sdslen(y) == 0 && memcmp(y,"\0",1) == 0)

        sdsfree(y);
        sdsfree(x);
        x = sdsnew("foo",&status);
        assert(status == SDS_OK);
        y = sdsnew("foa",&status);
        assert(status == SDS_OK);
        test_cond("sdscmp(foo,foa)", sdscmp(x,y) > 0)

        sdsfree(y);
        sdsfree(x);
        x = sdsnew("bar",&status);
        assert(status == SDS_OK);
        y = sdsnew("bar",&status);
        assert(status == SDS_OK);
        test_cond("sdscmp(bar,bar)", sdscmp(x,y) == 0)

        sdsfree(y);
        sdsfree(x);
        x = sdsnew("aar",&status);
        assert(status == SDS_OK);
        y = sdsnew("bar",&status);
        assert(status == SDS_OK);
        test_cond("sdscmp(bar,bar)", sdscmp(x,y) < 0)

        sdsfree(y);
        sdsfree(x);
        x = sdsnewlen("\a\n\0foo\r",7,&status);
        assert(status == SDS_OK);
        y = sdscatrepr(sdsempty(),x,sdslen(x),&status);
        test_cond("sdscatrepr(...data...)",
            memcmp(y,"\"\\a\\n\\x00foo\\r\"",15) == 0 && status == SDS_OK)

        {
            int oldfree;

            sdsfree(x);
            x = sdsnew("0",&status);
            assert(status == SDS_OK);
            sh = (void*) (x-(sizeof(struct sdshdr)));
            test_cond("sdsnew() free/len buffers", sh->len == 1 && sh->free == 0);
            x = sdsMakeRoomFor(x,1,&status);
            assert(status == SDS_OK);
            sh = (void*) (x-(sizeof(struct sdshdr)));
            test_cond("sdsMakeRoomFor()", sh->len == 1 && sh->free > 0);
            oldfree = sh->free;
            x[1] = '1';
            sdsIncrLen(x,1);
            test_cond("sdsIncrLen() -- content", x[0] == '0' && x[1] == '1');
            test_cond("sdsIncrLen() -- len", sh->len == 2);
            test_cond("sdsIncrLen() -- free", sh->free == oldfree-1);
        }

        /*
         * these tests may fail depending on kernel overcommit settings,
         * or if SDS_MAX_LEN is maximum value of size_t ...
         *
         * first assert below checks that second condition
         */

        assert(SDS_MAX_LEN < (size_t) -1);

        char *buf = malloc((size_t)SDS_MAX_LEN+1);
        assert(buf != NULL);

        status = SDS_OK;

        /* some tests cases below are not tight, due to prealloc... */

        sdsfree(x);
        x = sdsnewlen(buf,(size_t)SDS_MAX_LEN+1,&status);
        test_cond("sdsnewlen(...too big...)", x == NULL && status == SDS_ERR_LEN); 

        x = sdsnew("foo",&status);
        assert(status == SDS_OK);
        x = sdsgrowzero(x,(size_t)SDS_MAX_LEN+1,&status);
        test_cond("sdsgrowzero(...too big...)", x == NULL && status == SDS_ERR_LEN); 

        x = sdsnew("foo",&status);
        assert(status == SDS_OK);
        x = sdscatlen(x,buf,SDS_MAX_LEN-2,&status);
        test_cond("sdscatlen(...too big...)", x == NULL && status == SDS_ERR_LEN); 

        x = sdsnew("foo",&status);
        assert(status == SDS_OK);
        x = sdscpylen(x,buf,(size_t)SDS_MAX_LEN+1,&status);
        test_cond("sdscpylen(...too big...)", x == NULL && status == SDS_ERR_LEN); 

        x = sdsnew("foo",&status);
        assert(status == SDS_OK);
        x = sdsMakeRoomFor(x, SDS_MAX_LEN - 2, &status);
        test_cond("sdsMakeRoomFor(...too big...)", x == NULL && status == SDS_ERR_LEN); 

        free(buf);
    }
    test_report()
    return 0;
}
