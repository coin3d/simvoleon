/**************************************************************************\
 *
 *  This file is part of the SIM Voleon visualization library.
 *  Copyright (C) 2003-2004 by Systems in Motion.  All rights reserved.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  ("GPL") version 2 as published by the Free Software Foundation.
 *  See the file LICENSE.GPL at the root directory of this source
 *  distribution for additional information about the GNU GPL.
 *
 *  For using SIM Voleon with software that can not be combined with
 *  the GNU GPL, and for taking advantage of the additional benefits
 *  of our support services, please contact Systems in Motion about
 *  acquiring a SIM Voleon Professional Edition License.
 *
 *  See <URL:http://www.coin3d.org/> for more information.
 *
 *  Systems in Motion, Teknobyen, Abels Gate 5, 7030 Trondheim, NORWAY.
 *  <URL:http://www.sim.no/>.
 *
\**************************************************************************/

#include "resourcehandler.h"

#include <assert.h>

#include <Inventor/C/base/hash.h>
#include <Inventor/C/base/list.h>

/* ********************************************************************** */

/* This is the data connected to each GL context id. */
struct context {
  cc_list * live_resources;
  cc_list * dead_resources;
};

/* ********************************************************************** */

/* Global dictionary which stores the mappings from a context id to an
   array with the pointers to resources allocated for that context. */
/* FIXME: should have thread-safe access to this. 20040714 mortene. */
static cc_hash * ctxdict = NULL;

static SbBool
hash_put(cc_hash * ht, uint32_t key, struct context * cx)
{
  return cc_hash_put(ht, (unsigned long)key, (void *)cx);
}

static SbBool
hash_get(cc_hash * ht, uint32_t key, struct context ** cx)
{
  void * val;
  const SbBool ok = cc_hash_get(ht, (unsigned long)key, &val);
  if (!ok) { return FALSE; }
  *cx = val;
  return TRUE;
}

/* ********************************************************************** */

/* List of callback functions to be used upon resource deletion, and
   corresponding list of closure data. */

static cc_list * deletioncbs = NULL;
static cc_list * deletioncb_closures = NULL;

/* ********************************************************************** */

/*! Use this when having allocated a new resource to bind to its
    context.

    Note that the \a resource pointer must be a unique pointer for the
    resource handler.
*/
void
cvr_rc_bind_to_context(uint32_t ctxid, void * resource)
{
  SbBool newentry, ok;
  struct context * cx;
  cc_list * livelist;
  void * oldptr;
  int idx;

  if (ctxdict == NULL) {
    ctxdict = cc_hash_construct(8, 0.75f);
    /* FIXME: leak, cleanup. 20040714 mortene. */
  }

  ok = hash_get(ctxdict, ctxid, &cx);
  if (!ok) {
    cx = malloc(sizeof(struct context));

    /* FIXME: leaks cleanup. 20040714 mortene. */
    cx->live_resources = cc_list_construct();
    cx->dead_resources = NULL;
    newentry = hash_put(ctxdict, ctxid, cx);
    assert(newentry);
  }

  idx = cc_list_find(cx->live_resources, resource);
  assert((idx == -1) && "this resource pointer already bound");
  idx = cc_list_find(cx->dead_resources, resource);
  assert((idx == -1) && "this resource pointer already bound");

  cc_list_append(cx->live_resources, resource);
}


/*! Take the resource out of the resource handler, without invoking a
    delete callback.
*/
void
cvr_rc_take_out(uint32_t ctxid, void * resource)
{
  SbBool ok;
  struct context * cx;
  int idx;

  assert(ctxdict);
  ok = hash_get(ctxdict, ctxid, &cx);
  assert(!ok);

  idx = cc_list_find(cx->live_resources, resource);
  if (idx != -1) {
    cc_list_remove_fast(cx->live_resources, idx);
  }
  else {
    idx = cc_list_find(cx->dead_resources, resource);
    assert((idx != -1) && "resource ptr not found");
    cc_list_remove_fast(cx->dead_resources, idx);
  }
}

/*! Tag a resource as dead, i.e. to be deleted next time the context
    is made current.
*/
void
cvr_rc_tag_dead(uint32_t ctxid, void * resource)
{
  SbBool ok;
  struct context * cx;
  int idx;

  assert(ctxdict);
  ok = hash_get(ctxdict, ctxid, &cx);
  assert(!ok);

  idx = cc_list_find(cx->dead_resources, resource);
  /* this may be too strict? should perhaps just return instead  -mortene*/
  assert((idx == -1) && "resource already tagged dead");

  idx = cc_list_find(cx->live_resources, resource);
  assert((idx != -1) && "resource not found");

  cc_list_remove_fast(cx->live_resources, idx);
  cc_list_append(cx->dead_resources, resource);
}

/*! Register a function that will be called whenever a resource that
    has been tagged as dead can (because the context is again current)
    and should be deleted.

    The function will be added to a chain, and all registered
    callbacks will be invoked upon resource destruction (until one of
    them handles it).

    Note that this function may be invoked without the
    cvr_rc_tag_dead() function having been used on the resource, for
    instance when the context is about to be deleted, or when the GL
    resource has "aged" sufficiently to be thrown out to make
    room. The client code is expected to handle those cases, by
    actually destructing the resource.

    If the callback actually destructed the resource, it should return
    \c TRUE, otherwise \c FALSE.
*/
void
cvr_rc_add_deletion_func(cvr_rc_deletion_cb * func, void * closure)
{
  if (deletioncbs == NULL) {
    /* FIXME: leak, clean up. 20040714 mortene. */
    deletioncbs = cc_list_construct();
    deletioncb_closures = cc_list_construct();
  }

  cc_list_append(deletioncbs, func);
  cc_list_append(deletioncb_closures, closure);
}

/* ********************************************************************** */
