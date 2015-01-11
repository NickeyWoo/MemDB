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

void rbtreeReleaseSubtree(rbNode *n);
void rbtreeRotateLeft(rbtree *tree, rbNode *node);
void rbtreeRotateRight(rbtree *tree, rbNode *node);
void rbtreeTransplantLeft(rbtree *tree, rbNode *node);
void rbtreeTransplantRight(rbtree *tree, rbNode *node);

void rbtreeInsertFixup(rbtree *tree, rbNode *node);
void rbtreeDeleteFixup(rbtree *tree, rbNode *parent, rbNode *node);
rbNode *rbtreeDeleteNode(rbtree *tree, rbNode *node);
rbNode *rbtreeDeleteOneChildNode(rbtree *tree, rbNode *node);

rbtree *rbtreeCreate()
{
    return rbtreeCreateWithCompare(compareStringObjects);
}

rbtree *rbtreeCreateWithCompare(int (*compare)(robj *v1, robj *v2))
{
    rbtree *t = zmalloc(sizeof(*t));
    t->root = NULL;
    t->compare = compare;
    return t;
}

void rbtreeReleaseSubtree(rbNode *n)
{
    if ( !n ) return;

    if ( n->left ) {
        rbtreeReleaseSubtree(n->left);
        n->left = NULL;
    }
    if ( n->right ) {
        rbtreeReleaseSubtree(n->right);
        n->right = NULL;
    }

    rbtreeReleaseNode(n);
}

void rbtreeRelease(rbtree *tree)
{
    rbtreeReleaseSubtree(tree->root);
    zfree(tree);
}

rbNode *rbtreeCreateNode(robj *obj)
{
    rbNode *n = zmalloc(sizeof(*n));
    n->color = RBNODE_RED;
    n->parent = n->left = n->right = NULL;
    n->obj = obj;
    n->left_count = 0;
    incrRefCount(obj);
    return n;
}

void rbtreeReleaseNode(rbNode *n)
{
    decrRefCount(n->obj);
    zfree(n);
}

void rbtreeInsertFixup(rbtree *tree, rbNode *node)
{
    rbNode *parent = NULL, *grandpa = NULL, *uncle = NULL;
    while ( node && node->color == RBNODE_RED &&
            (parent = node->parent) && parent->color == RBNODE_RED) {
        grandpa = parent->parent;
        if ( grandpa->left == parent) {
            uncle = grandpa->right;

            if ( uncle && uncle->color == RBNODE_RED ) {
                parent->color = RBNODE_BLACK;
                uncle->color = RBNODE_BLACK;
                grandpa->color = RBNODE_RED;
                node = grandpa;
                continue;
            }

            if ( node == parent->right ) {
                node->color = RBNODE_BLACK;
                rbtreeRotateLeft(tree, parent);
            } else {
                parent->color = RBNODE_BLACK;
            }

            grandpa->color = RBNODE_RED;
            rbtreeRotateRight(tree, grandpa);
        } else {
            uncle = grandpa->left;

            if ( uncle && uncle->color == RBNODE_RED ) {
                parent->color = RBNODE_BLACK;
                uncle->color = RBNODE_BLACK;
                grandpa->color = RBNODE_RED;
                node = grandpa;
                continue;
            }

            if ( node == parent->left ) {
                node->color = RBNODE_BLACK;
                rbtreeRotateRight(tree, parent);
            } else {
                parent->color = RBNODE_BLACK;
            }

            grandpa->color = RBNODE_RED;
            rbtreeRotateLeft(tree, grandpa);
        }
    }
    tree->root->color = RBNODE_BLACK;
}

void rbtreeRotateLeft(rbtree *tree, rbNode *node)
{
    rbNode *right = node->right;
    rbNode *parent = node->parent;

    if ( !right ) return;
    
    right->parent = parent;

    if ( tree->root == node ) {
        tree->root = right;
    } else {
        if ( parent->left == node ) 
            parent->left = right;
        else 
            parent->right = right;
    }

    node->parent = right;
    node->right = right->left;

    if ( right->left )
        right->left->parent = node;

    right->left = node;
}

void rbtreeRotateRight(rbtree *tree, rbNode *node)
{
    rbNode *left = node->left;
    rbNode *parent = node->parent;

    if ( !left ) return;

    left->parent = parent;

    if ( tree->root == node ) {
        tree->root = left;
    } else {
        if ( parent->left == node )
            parent->left = left;
        else
            parent->right = left;
    }

    node->parent = left;
    node->left = left->right;

    if ( left->right )
        left->right->parent = node;

    left->right = node;
}

rbNode *rbtreeInsert(rbtree *tree, robj *obj)
{
    int c;
    rbNode *parent = NULL, **node = &tree->root;
    while ( *node ) {
        c = tree->compare((*node)->obj, obj);
        if ( c == 0 ) {
            return (*node);
        } else if ( c < 0 ) {
            parent = *node;
            node = &((*node)->right);
        } else {
            parent = *node;
            node = &((*node)->left);
        }
    }

    *node = rbtreeCreateNode(obj);
    (*node)->parent = parent;

    rbtreeInsertFixup(tree, *node);
    return *node;
}

void rbtreeTransplantLeft(rbtree *tree, rbNode *node)
{
    rbNode *parent = node->parent;
    rbNode *left = node->left;

    if ( left ) left->parent = parent;

    if ( tree->root == node ) {
        tree->root = left;
    } else {
        if ( parent->left == node )
            parent->left = left;
        else
            parent->right = left;
    }
}

void rbtreeTransplantRight(rbtree *tree, rbNode *node)
{
    rbNode *parent = node->parent;
    rbNode *right = node->right;

    if ( right ) right->parent = parent;

    if ( tree->root == node ) {
        tree->root = right;
    } else {
        if ( parent->left == node )
            parent->left = right;
        else
            parent->right = right;
    }
}

void rbtreeDeleteFixup(rbtree *tree, rbNode *parent, rbNode *node)
{
    rbNode *brother, *brotherLeft, *brotherRight;
    while ( (!node || node->color == RBNODE_BLACK) && 
            node != tree->root ) {

        if ( parent->left == node ) {
            brother = parent->right;

            if ( brother->color == RBNODE_RED ) {
                brother->color = RBNODE_BLACK;
                parent->color = RBNODE_RED;
                rbtreeRotateLeft(tree, parent);

                brother = parent->right;
            }

            brotherLeft = brother->left;
            brotherRight = brother->right;

            if ( (!brotherLeft || brotherLeft->color == RBNODE_BLACK) &&
                 (!brotherRight || brotherRight->color == RBNODE_BLACK) ) {

                brother->color = RBNODE_RED;
                node = parent;
                parent = parent->parent;

            } else {

                if ( !brotherRight || brotherRight->color == RBNODE_BLACK ) {
                    brotherLeft->color = RBNODE_BLACK;
                    brother->color = RBNODE_RED;
                    rbtreeRotateRight(tree, brother);

                    brotherRight = brother;
                    brother = brotherLeft;
                }
                brother->color = parent->color;
                parent->color = RBNODE_BLACK;
                brotherRight->color = RBNODE_BLACK;
                rbtreeRotateLeft(tree, parent);

                node = tree->root;
                break;
            }

        } else {
            brother = parent->left;

            if ( brother->color == RBNODE_RED ) {
                brother->color = RBNODE_BLACK;
                parent->color = RBNODE_RED;
                rbtreeRotateRight(tree, parent);

                brother = parent->left;
            }

            brotherLeft = brother->left;
            brotherRight = brother->right;

            if ( (!brotherLeft || brotherLeft->color == RBNODE_BLACK) &&
                 (!brotherRight || brotherRight->color == RBNODE_BLACK) ) {

                brother->color = RBNODE_RED;
                node = parent;
                parent = parent->parent;

            } else {

                if ( !brotherLeft || brotherLeft->color == RBNODE_BLACK ) {
                    brotherRight->color = RBNODE_BLACK;
                    brother->color = RBNODE_RED;
                    rbtreeRotateLeft(tree, brother);

                    brotherLeft = brother;
                    brother = brotherRight;
                }
                brother->color = parent->color;
                parent->color = RBNODE_BLACK;
                brotherLeft->color = RBNODE_BLACK;
                rbtreeRotateRight(tree, parent);

                node = tree->root;
                break;
            }

        }

    }

    if ( node ) node->color = RBNODE_BLACK;
}

rbNode *rbtreeDeleteOneChildNode(rbtree *tree, rbNode *node)
{
    rbNode *fix = NULL;
    rbNode *parent = node->parent;

    if ( node->right ) {
        fix = node->right;
        rbtreeTransplantRight(tree, node);
    } else {
        fix = node->left;
        rbtreeTransplantLeft(tree, node);
    }

    if ( node->color == RBNODE_BLACK )
        rbtreeDeleteFixup(tree, parent, fix);
    return node;
}

rbNode *rbtreeDeleteNode(rbtree *tree, rbNode *node)
{
    robj *val;
    rbNode *mini;
    if ( !node->left || !node->right ) {
        return rbtreeDeleteOneChildNode(tree, node);
    } else {
        mini = rbtreeMinimun(node->right);

        val = node->obj;
        node->obj = mini->obj;
        mini->obj = val;
        
        return rbtreeDeleteOneChildNode(tree, mini);
    }
}

rbNode *rbtreeDelete(rbtree *tree, robj *obj)
{
    int c;
    rbNode *node = tree->root;
    while ( node ) {
        c = tree->compare(node->obj, obj);
        if ( c == 0 ) {
            return rbtreeDeleteNode(tree, node);
        } else if ( c < 0 ) {
            node = node->right;
        } else {
            node = node->left;
        }
    }
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
        return rbtreeNext(parent);
    else
        return parent;
}

rbNode *rbtreeNext(rbNode *n)
{
    rbNode *min, *parent;
    if ( n->right ) {
        min = rbtreeMinimun(n->right);
        return min;
    } else {
        while ( (parent = n->parent) && n == parent->right ) {
            n = parent;
        }
        return parent;
    }
}

rbNode *rbtreePrev(rbNode *n)
{
    rbNode *max, *parent;
    if ( n->left ) {
        max = rbtreeMaximun(n->left);
        return max;
    } else {
        while ( (parent = n->parent) && n == parent->left ) {
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

rbNode *rbtreeMaximun(rbNode *n)
{
    while ( n && n->right )
        n = n->right;
    return n;
}


