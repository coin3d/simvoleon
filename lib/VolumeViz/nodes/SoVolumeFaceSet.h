#ifndef SIMVOLEON_SOVOLUMEFACESET_H
#define SIMVOLEON_SOVOLUMEFACESET_H

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
 *  Systems in Motion, Postboks 1283, Pirsenteret, 7462 Trondheim, NORWAY.
 *  <URL:http://www.sim.no/>.
 *
\**************************************************************************/

#include <Inventor/nodes/SoFaceSet.h>
#include <Inventor/fields/SoSFBool.h>
#include <Inventor/fields/SoSFFloat.h>

#include <VolumeViz/C/basic.h>

// *************************************************************************

class SIMVOLEON_DLL_API SoVolumeFaceSet : public SoFaceSet {
  typedef SoFaceSet inherited;
  SO_NODE_HEADER(SoVolumeFaceSet);
  
public:
  static void initClass(void);  
  SoVolumeFaceSet(void);

  SoSFBool clipGeometry;  
  SoSFFloat offset;
   
protected:
  ~SoVolumeFaceSet();

  virtual void GLRender(SoGLRenderAction * action);

private:  
  friend class SoVolumeFaceSetP;
  class SoVolumeFaceSetP * pimpl;

  friend class CvrFaceSetRenderP;
};

// *************************************************************************

#endif // !SIMVOLEON_SOVOLUMEFACESET_H
