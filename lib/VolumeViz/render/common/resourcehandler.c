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

/* These are the lists connected to each resource owner. */
struct cvr_rc_owner {
  cc_list * live_resources;
  cc_list * dead_resources;
};

/* ********************************************************************** */

/* Global dictionary which stores the mappings from a context id to a
   new hash, with resource owners, having one or more resources in
   this context. */
static cc_hash * ctxhash = NULL;

/* ********************************************************************** */

/* List of callback functions to be used upon resource deletion, and
   corresponding list of closure data. */

static cc_list * deletioncbs = NULL;
static cc_list * deletioncb_closures = NULL;

/* ********************************************************************** */

/*! Use this when having allocated a new resource to bind to a
    specific context and owner.

    Note that an owner can only store one resource pr context id.
*/
void
cvr_rc_bind_resource(uint32_t ctxid, void * owner, void * resource)
{
  SbBool ok, newentry;
  cc_hash * ownershash;
  void * tmp;

  if (ctxhash == NULL) {
    ctxhash = cc_hash_construct(256, 0.75f);
    /* FIXME: leak, cleanup. 20040714 mortene. */
  }

  /* find (or make) dict of owners in the specified context */
  ok = cc_hash_get(ctxhash, ctxid, &tmp);
  if (!ok) {
    ownershash = cc_hash_construct(128, 0.75f);
    newentry = cc_hash_put(ctxhash, ctxid, ownershash);
    assert(newentry);
  }
  else {
    ownershash = (cc_hash *)tmp;
  }

  /* map resource ptr to owner */
  /* FIXME: dangerous cast? 20040715 mortene. */
  newentry = cc_hash_put(ownershash, (unsigned long)owner, resource);
  assert(newentry);
}

/*! Finds the resource connected to a specific owner in a specific
    context.

    Returns \c TRUE if a resource has been bound, \c FALSE otherwise.
*/
SbBool
cvr_rc_find_resource(uint32_t ctxid, void * owner, void ** resource)
{
  SbBool ok;
  cc_hash * ownershash;
  void * tmp;

  if (ctxhash == NULL) { return NULL; }

  /* find dict of owners in the specified context */
  ok = cc_hash_get(ctxhash, ctxid, &tmp);
  if (!ok) { return FALSE; }
  ownershash = (cc_hash *)tmp;

  /* find resource ptr from owner */
  /* FIXME: dangerous cast? 20040715 mortene. */
  ok = cc_hash_get(ownershash, (unsigned long)owner, &tmp);
  if (!ok) { return FALSE; }

  *resource = tmp;
  return TRUE;
}

// *************************************************************************

/*! Tag resources in all contexts for an owner as dead, i.e. to be
    deleted next time the context is made current.
*/
void
cvr_rc_tag_resources_dead(void * owner)
{
  /* FIXME: implement */
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
cvr_rc_add_deletion_func(void * owner, cvr_rc_deletion_cb * func, void * closure)
{
#if 0 /* xxx obsolete */
  if (deletioncbs == NULL) {
    /* FIXME: leak, clean up. 20040714 mortene. */
    deletioncbs = cc_list_construct();
    deletioncb_closures = cc_list_construct();
  }

  cc_list_append(deletioncbs, func);
  cc_list_append(deletioncb_closures, closure);
#else
  /* FIXME: implement */
#endif
}

/* ********************************************************************** */
