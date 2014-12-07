/*
 * Copyright (c) 2009-2014, Nickey Woo <thenickey at gmail dot com>
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

#ifndef __MODULE_H
#define __MODULE_H

typedef int redisModuleInit(void);

typedef struct redisModule {
    const char* name;
    redisModuleInit* init;
    struct redisCommand* commands;
} redisModule;

#ifdef __cplusplus
    #define REDIS_GET_MODULE(module)                                \
        extern "C" redisModule* redis_get_module(void)              \
        {                                                           \
            return &module;                                         \
        }
#else
    #define REDIS_GET_MODULE(module)                                \
        redisModule* redis_get_module(void)                         \
        {                                                           \
            return &module;                                         \
        }
#endif

#define START_FUNCTIONS(module)                                     \
    struct redisCommand module[] = {

#define FUNCTION(name, func, arity, sflags, flags)                  \
        {name, func, arity, sflags, flags, NULL, 0, 0, 0, 0, 0}

#define END_FUNCTIONS()                                             \
        {NULL, NULL, 0, NULL, 0, NULL, 0, 0, 0, 0, 0}               \
    };

int redisLoadModule(const char* szModule);

#endif
