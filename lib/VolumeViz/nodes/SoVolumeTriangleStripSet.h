#ifndef SOVOLUMETRIANGLESTRIPSET_H
#define SOVOLUMETRIANGLESTRIPSET_H

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

#include <Inventor/nodes/SoTriangleStripSet.h>
#include <Inventor/fields/SoSFBool.h>
#include <Inventor/fields/SoSFFloat.h>

#include <VolumeViz/C/basic.h>
#include <VolumeViz/render/3D/Cvr3DTexCube.h>


class SIMVOLEON_DLL_API SoVolumeTriangleStripSet : public SoTriangleStripSet {
  typedef SoTriangleStripSet inherited;
  SO_NODE_HEADER(SoVolumeTriangleStripSet);
  
public:
  static void initClass();  
  SoVolumeTriangleStripSet();

  SoSFBool clipGeometry;  
  SoSFFloat offset;
   
protected:
  ~SoVolumeTriangleStripSet();

  virtual void GLRender(SoGLRenderAction *action);

private:  
  friend class SoVolumeTriangleStripSetP;
  class SoVolumeTriangleStripSetP * pimpl;

  friend class CvrNonIndexedSetRenderBaseP;
  friend class CvrTriangleStripSetRenderP;

};

#endif /* SOVOLUMETRIANGLESTRIPSET_H */
