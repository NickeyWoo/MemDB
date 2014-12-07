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

#include "redis.h"
#include <dlfcn.h>

int redisLoadModule(const char* szModule)
{
    redisModule* (*entry)(void);

    redisModuleInfo *info = zmalloc(sizeof(redisModuleInfo));
    if (!info) return REDIS_ERR;

    info->handle = dlopen(szModule, RTLD_LAZY);
    if (!info->handle) {
        zfree(info);
        return REDIS_ERR;
    }

    dlerror();

    entry = (redisModule* (*)(void))dlsym(info->handle, "redis_get_module");
    
    if (dlerror() != NULL) goto loaderr;

    info->module = (*entry)();
    if (!info->module || !info->module->name || !info->module->init || !info->module->commands)
        goto loaderr;

    if (REDIS_OK != info->module->init()) goto loaderr;

    int i = 0;
    while(info->module->commands[i].name) {
        sds name = sdsnew(info->module->commands[i].name);

        if (DICT_OK != dictAdd(server.commands, name, &info->module->commands[i])) {
            redisLog(REDIS_WARNING, "command name `%s` already exists", name);
            sdsfree(name);
        }
        ++i;
    }

    info->path = zstrdup(szModule);
    listAddNodeTail(server.modules, info);
    return REDIS_OK;

loaderr:
    dlclose(info->handle);
    zfree(info);
    return REDIS_ERR;
}



