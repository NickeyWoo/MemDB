/*
 * Copyright (c) 2014, Nickey Woo <thenickey at gmail dot com>
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

#include "redis.h"
#include "sortedtable.h"
#include <sys/types.h>

int sortedtable_module_init()
{
    redisLog(REDIS_WARNING, "\033[31mlibsortedtable.so initialize ...\033[0m");
    return 0;
}

START_FUNCTIONS(sortedtable_module_functions)
    FUNCTION("hcreate", hcreateCommand, 2, "w", REDIS_CMD_WRITE),
    FUNCTION("hadd", haddCommand, 2, "wm", REDIS_CMD_WRITE|REDIS_CMD_DENYOOM),
    FUNCTION("hrem", hremCommand, 2, "w", REDIS_CMD_WRITE),
END_FUNCTIONS()

redisModule sortedtable_module = {
    "sortedtable",
    sortedtable_module_init,
    sortedtable_module_functions
};

REDIS_GET_MODULE(sortedtable_module)


