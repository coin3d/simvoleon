#ifndef SIMVOLEON_RESOURCEHANDLER_H
#define SIMVOLEON_RESOURCEHANDLER_H

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

/*
  This implements a resource manager for OpenGL resources, binding
  them to a context id, tracking when they need to be deleted, when
  they _can_ be deleted, etc etc.

  For documentation on the individual interface components below, see
  the resourcehandler.c file.

  This was implemented in C, as I expect this will eventually make it
  to the public C interface surrounding the GL "glue" in Coin
  (i.e. the interface found in Inventor/C/glue/gl.h).
*/

/* ********************************************************************** */

#include <Inventor/C/basic.h>

/* ********************************************************************** */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#if 0 /* to get proper auto-indentation in emacs */
}
#endif /* emacs indentation */

typedef void cvr_rc_deletion_cb(uint32_t ctxid, void * resource, void * closure);

void cvr_rc_register_resource(uint32_t ctxid, void * resource,
                              cvr_rc_deletion_cb * delcb, void * cbclosure);

void cvr_rc_unregister_resource(uint32_t ctxid, void * resource);

void cvr_rc_tag_resource_dead(uint32_t ctxid, void * resource);


/* FIXME: should perhaps also implement an "aging" feature, something
   like this?:

   void cvr_rc_tick_to_be_used(uint32_t ctxid, const void * resource);
   void cvr_rc_context_made_current(uint32_t ctxid);

   With aging, this might also factor out some of the functionality of
   SoGLImage/SoGLBigImage in Coin, which would be good (those classes
   are a hodge-podge of functionality, and way too big).

   20040714 mortene.
*/

/* ********************************************************************** */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* ! SIMVOLEON_RESOURCEHANDLER_H */
