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
  Least Recently Used (LRU) system

    To support large sets of volume data, a simple memory management
    system is implemented. The scene graph's VolumeData-node contains
    a logical clock that's incremented for each run through it's
    GLRender. All in-memory subpages are tagged with a timestamp at
    the time they're loaded. The VolumeData-node has a max limit for
    the amount of HW/SW memory it should occupy, and whenever it
    exceeds this limit it throws out the subpage with the oldest
    timestamp. A simple LRU-cache, in other words. Of course, all
    subpages' timestamps are "touched" (updated) whenever they're
    rendered.
*/

// FIXME: suggestion found on comp.graphics.api.opengl on how to check
// how much texture VRAM is available:
//
// [...]
// Once you create texture(s), you can give it(them) a priority using
// glPrioritizeTexture
// (http://www.3dlabs.com/support/developer/GLmanpages/glprioritizetextures.htm)
// 
// this should tell the ICD to put textures with the highest priority
// (and according to available texture memory on your hardware) into
// the actual video ram on you sweet geforce (or whatever card you
// happen to use:-).
// 
// and then, you can see if that texture(s) is really resident (stored
// in vram) by calling glAreTexturesResident
// (http://www.3dlabs.com/support/developer/GLmanpages/glaretexturesresident.htm)
// 
// So if you want to test how many textures you can cram into your
// vram, you can just keep on creating new textures and setting their
// priority to high (1) and for every new one you create see if it is
// really stored in vram.  Keep in mind that you need to use a given
// texture at least once before it can be "prioritized", put into
// vram.
// 
// This is really really cool, because say you want to use 10 textures
// interchangeably. If you put them in vram then calls to
// glBindTexture will be ultra fricking fast, because the texture will
// no longer have to go through the AGP bus.
// [...]
//
// 20021130 mortene.
//
// Update 20021201 mortene: I think the technique above will be
// dangerous on UMA-machines (like the SGI O2) were tex mem is the
// same as other system mem. One would at least have to set a upper
// limit before running the test.


// For reference, here's some information from Thomas Roell of Xi
// Graphics on glPrioritizeTextures() from c.g.a.opengl:
//
// [...]
//
//   Texture priorities would be a nice thing, but only few OpenGL
//   implementations actually use them. There are a lot of reasons
//   that they ignore priorities. One is that the default priority is
//   set to 1.0, which is the highest priority. That means unless all
//   textures for all you applications running at the same time
//   explicitely use texture priorities, the one that uses them
//   (i.e. lower priorities) will be at a disadvantage. The other
//   problem is that typically textures are not the only objects that
//   live in HW accessable memory. There are display lists, color
//   tables, vertex array objects and so on. However there is no way
//   to prioritize them. Hence if you are using textures and display
//   lists at the same time, useng priorities might cause a lot of
//   texture cache trashing.
//
// [...]
//
// 20021201 mortene.
