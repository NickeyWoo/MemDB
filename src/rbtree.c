/* rbtree.c - red-black tree implementation
 *
 * Copyright (c) 2006-2014, nickey woo <thenickey at gmail dot com>
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

rbtree *rbtreeCreate()
{
    return rbtreeCreateWithCompare(rbtreeCompare);
}

rbtree *rbtreeCreateWithCompare(int (*compare)(robj *v1, robj *v2))
{
    rbtree *t = zmalloc(sizeof(*t));
    t->root = NULL;
    t->compare = compare;
    t->length = 0;
    return t;
}

void rbtreeRelease(rbtree *tree)
{

    zfree(tree);
}

int rbtreeCompare(robj *v1, robj *v2)
{
    return 0;
}

rbNode *rbtreeCreateNode(robj *obj)
{
    rbNode *n = zmalloc(sizeof(*n));
    n->color = RBNODE_RED;
    n->parent = n->left = n->right = NULL;
    n->obj = obj;
    incrRefCount(obj);
    return n;
}

void rbtreeReleaseNode(rbNode *n)
{
    decrRefCount(n->obj);
    zfree(n);
}

rbNode *rbtreeInsert(rbtree *tree, rbNode *n)
{
    return NULL;
}

rbNode *rbtreeDelete(rbtree *tree, rbNode *n)
{
    return NULL;
}

rbNode *rbtreeSearch(rbtree *tree, robj *obj)
{
    int c;
    rbNode *n = tree->root;
    while (n) {
        c = tree->compare(n->obj, obj);
        if ( c == 0 ) {
            return n;
        } else if ( c < 0 ) {
            n = n->right;
        } else {
            n = n->left;
        }
    }
    return NULL;
}

rbNode *rbtreeNearby(rbtree *tree, robj *obj)
{
    int c;
    rbNode *parent = NULL, **node = &tree->root;
    while ( *node ) {
        c = tree->compare((*node)->obj, obj);
        parent = *node;
        if ( c == 0 ) {
            return (*node);
        } else if ( c < 0 ) {
            node = &((*node)->right);
        } else {
            node = &((*node)->left);
        }
    }
    if ( !parent )
        return NULL;
    else if ( node == &parent->right )
        return parent->parent;
    else
        return parent;
}

rbNode *rbtreeNext(rbNode *n)
{
    rbNode *mini, *parent;
    if ( n->right ) {
        mini = rbtreeMinimun(n->right);
        return mini;
    } else {
        while ( (parent = n->parent) && n == parent->right ) {
            n = parent;
        }
        return parent;
    }
}

rbNode *rbtreeMinimun(rbNode *n)
{
    while ( n && n->left )
        n = n->left;
    return n;
}

rbNode *rbtreeMaximum(rbNode *n)
{
    while ( n && n->right )
        n = n->right;
    return n;
}


