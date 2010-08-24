/**************************************************************************\
 *
 *  This file is part of the SIM Voleon visualization library.
 *  Copyright (C) by Kongsberg Oil & Gas Technologies.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  ("GPL") version 2 as published by the Free Software Foundation.
 *  See the file LICENSE.GPL at the root directory of this source
 *  distribution for additional information about the GNU GPL.
 *
 *  For using SIM Voleon with software that can not be combined with
 *  the GNU GPL, and for taking advantage of the additional benefits
 *  of our support services, please contact Kongsberg Oil & Gas
 *  Technologies about acquiring a SIM Voleon Professional Edition
 *  License.
 *
 *  See http://www.coin3d.org/ for more information.
 *
 *  Kongsberg Oil & Gas Technologies, Bygdoy Alle 5, 0257 Oslo, NORWAY.
 *  http://www.sim.no/  sales@sim.no  coin-support@coin3d.org
 *
\**************************************************************************/

#include "SIMVoleon.h"
#include <VolumeViz/nodes/SoVolumeRendering.h>
#include <VolumeViz/render/raycast/SoRaycastRendering.h>

#include <cassert>

/*!
  Does all necessary class initializations of the volume rendering
  system.

  Note: This function will also call SoVolumeRendering::init(void).

  Note: SoRaycastRendering::init() will be called if "libCLVol" was
  detected during configuration of SIMVoleon.

  \since SIMVoleon 3.0
 */
void
SIMVoleon::init()
{
  SoVolumeRendering::init(); // Initialize the regular SIMVoleon nodes.
  
  // FIXME: Wrap this in a HAVE_LIBCLVOL ifdef (20100824 handegar)
#ifdef HAVE_LIBCLVOL
  SoRaycastRendering::init(); // Initialize the ray-cast engine (if available)
#endif
}


/*!
  Invoke this method as the last call of the application code, to
  trigger a clean-up of all static resources used by the SIMVoleon library.

  This is usually not necessary for stand-alone executable
  applications, as the operating system will take care of cleaning up
  after the process as it exits.

  It may be necessary to invoke this method to avoid leaks for
  "special" execution environments, though, like if the SIMVoleon
  library is used as e.g. a browser plug-in, or some other type of
  component which can be started, shut down and restarted multiple
  times.

  // FIXME: Not implemented yet (20100824 handegar)

  \since SIMVoleon 3.0
 */
void
SIMVoleon::finish()
{
  assert(0 && "Not implemented yet");
}
