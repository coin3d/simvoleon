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

struct cvr_rc_resource {
  void * resource;
  cvr_rc_deletion_cb * delcb;
  void * delcbclosure;
};

/* ********************************************************************** */

/* Global dictionaries which stores the mappings from a context id to
   a new hash, with resource pointers. That hash maps from resource
   pointers to the cvr_rc_resource struct.

   There's one hash for live resources, one for dying ones (i.e. ones
   ready to be killed off the next time the context in question is
   made current).
*/

static cc_hash * ctxlivehash = NULL;
static cc_hash * ctxdeadhash = NULL;

/* ********************************************************************** */

static void
insert_resource_in_hash(cc_hash * ctxhash, uint32_t ctxid,
                        void * resource, struct cvr_rc_resource * res)
{
  SbBool ok, newentry;
  cc_hash * rshash;
  void * tmp;
  struct cvr_rc_resource * resblock;

  /* find (or make) dict of resources in the specified context */
  ok = cc_hash_get(ctxhash, (unsigned long)ctxid, &tmp);
  if (!ok) {
    rshash = cc_hash_construct(1024, 0.75f);
    newentry = cc_hash_put(ctxhash, ctxid, rshash);
    assert(newentry);
  }
  else {
    rshash = (cc_hash *)tmp;
  }

  /* FIXME: dangerous cast? 20040715 mortene. */
  newentry = cc_hash_put(rshash, (unsigned long)resource, res);
  assert(newentry);
}

/*! Use this when having allocated a new resource to bind to a
    specific context.

    Registers a function that will be called whenever the resource
    should be killed off on the client side, e.g. by doing
    glDeleteTextures() if it's a GL texture.

    Note that this function may be invoked without the
    cvr_rc_tag_resource_dead() function having been used on the
    resource, for instance when the context is about to be deleted, or
    when the GL resource has "aged" sufficiently to be thrown out to
    make room. The client code is expected to handle those cases, by
    actually destructing the resource.
*/
void
cvr_rc_register_resource(uint32_t ctxid, void * resource,
                         cvr_rc_deletion_cb * delcb, void * cbclosure)
{
  struct cvr_rc_resource * resblock;

  if (ctxlivehash == NULL) {
    ctxlivehash = cc_hash_construct(8, 0.75f);
    /* FIXME: leak, cleanup. 20040714 mortene. */
  }
 
  /* map resource ptr to block of resource data: */
  resblock = (struct cvr_rc_resource *)malloc(sizeof(struct cvr_rc_resource));
  resblock->resource = resource;
  resblock->delcb = delcb;
  resblock->delcbclosure = cbclosure;

  insert_resource_in_hash(ctxlivehash, ctxid, resource, resblock);
}

/* ********************************************************************** */

static struct cvr_rc_resource *
take_resource_out_of_hash(cc_hash * ctxhash, uint32_t ctxid, void * resource)
{
  SbBool ok;
  cc_hash * rshash;
  void * tmp;
  struct cvr_rc_resource * resblock;

  assert(ctxhash);
  ok = cc_hash_get(ctxhash, (unsigned long)ctxid, &tmp);
  assert(ok);
  rshash = (cc_hash *)tmp;
  ok = cc_hash_get(rshash, (unsigned long)resource, &tmp);
  assert(ok);
  resblock = (struct cvr_rc_resource *)tmp;
  ok = cc_hash_remove(rshash, (unsigned long)resource);
  assert(ok);

  return resblock;
}

/*! Just take a resource out from the resource handler, not calling
    the deletion callback.
*/
void
cvr_rc_unregister_resource(uint32_t ctxid, void * resource)
{
  struct cvr_rc_resource * resblock =
    take_resource_out_of_hash(ctxlivehash, ctxid, resource);
  free(resblock);
}

/* ********************************************************************** */

/*! Tag a resource as dead, i.e. to be deleted next time the context
    is made current.
*/
void
cvr_rc_tag_resource_dead(uint32_t ctxid, void * resource)
{
  struct cvr_rc_resource * resblock =
    take_resource_out_of_hash(ctxlivehash, ctxid, resource);

  if (ctxdeadhash == NULL) {
    ctxdeadhash = cc_hash_construct(8, 0.75f);
    /* FIXME: leak, cleanup. 20040714 mortene. */
  }
 
  insert_resource_in_hash(ctxdeadhash, ctxid, resource, resblock);
}

/* ********************************************************************** */
