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

/*!
  \mainpage

  FIXME, should insert doc on at least these issues:

    - what is volume rendering
    - why volume rendering
    - list features of SimVoleon

      (Here's one were we're better than TGS VolumeViz:) The volume
      rendering of SimVoleon works well on volume data sets of any
      dimensions. With other volume rendering systems, is often
      necessary to accommodate the rendering system by pre-processing
      the dataset to be of power-of-two dimensions, either to avoid
      the rendering to take up an extraordinary amount of resources
      related to texture-mapping, or from down-right failing. This
      restriction is not present in SimVoleon, which works well with
      different dimensions along the principal axes, and with any
      non-power-of-two dimension.

    - list limitations of SimVoleon

  The simplest possible usage example, which sets up a complete
  environment for voxel visualization with the SimVoleon library:

  \code
  #include <Inventor/Qt/SoQt.h>
  #include <Inventor/Qt/viewers/SoQtExaminerViewer.h>
  #include <Inventor/nodes/SoSeparator.h>
  #include <VolumeViz/nodes/SoTransferFunction.h>
  #include <VolumeViz/nodes/SoVolumeData.h>
  #include <VolumeViz/nodes/SoVolumeRender.h>
  #include <VolumeViz/nodes/SoVolumeRendering.h>
  
  static uint8_t *
  generate8bitVoxelSet(SbVec3s & dim)
  {
    const size_t blocksize = dim[0] * dim[1] * dim[2];
    uint8_t * voxels = new uint8_t[blocksize];
    (void)memset(voxels, 0, blocksize);
  
    float t = 0;
  
    while (t < 50) {
      SbVec3f v(sin((t + 1.4234) * 1.9) * sin(t) * 0.45 + 0.5,
                cos((t * 2.5) - 10) * 0.45 + 0.5,
                cos((t - 0.23123) * 3) * sin(t + 0.5) * cos(t) * 0.45 + 0.5);
  
      assert(v[0] < 1.0f && v[1] < 1.0f && v[2] < 1.0f);
      const int nx = int(dim[0] * v[0]);
      const int ny = int(dim[1] * v[1]);
      const int nz = int(dim[2] * v[2]);
  
      const int memposition = nz*dim[0]*dim[1] + ny*dim[0] + nx;
      voxels[memposition] = (uint8_t)(255.0 * cos(t));
  
      t += 0.001;
    }
  
    return voxels;
  }
  
  int
  main(int argc, char ** argv)
  {
    QWidget * window = SoQt::init(argv[0]);
    SoVolumeRendering::init();
  
    SoSeparator * root = new SoSeparator;
    root->ref();
  
    SbVec3s dim = SbVec3s(64, 64, 64);
    uint8_t * voxeldata = generate8bitVoxelSet(dim);
  
    // Add SoVolumeData to scene graph
    SoVolumeData * volumedata = new SoVolumeData();
    volumedata->setVolumeData(dim, voxeldata, SoVolumeData::UNSIGNED_BYTE);
    root->addChild(volumedata);
  
    // Add TransferFunction (color map) to scene graph
    SoTransferFunction * transfunc = new SoTransferFunction();
    root->addChild(transfunc);
  
    // Add VolumeRender to scene graph
    SoVolumeRender * volrend = new SoVolumeRender();
    root->addChild(volrend);
  
    SoQtExaminerViewer * viewer = new SoQtExaminerViewer(window);
    viewer->setBackgroundColor(SbColor(0.1f, 0.3f, 0.5f));
    viewer->setSceneGraph(root);
  
    viewer->show();
    SoQt::show(window);
    SoQt::mainLoop();
    delete viewer;
  
    root->unref();
    delete[] voxeldata;
  
    return 0;
  }
  \endcode
*/

/*!
  \class SoVolumeRendering VolumeViz/nodes/SoVolumeRendering.h
  \brief Abstract base class for all nodes related to volume rendering.
  \ingroup volviz

  The sole purpose of this class is really just to initialize the
  volume rendering framework, and to provide a convenient method for
  the application programmer to make queries about the capabilities of
  the underlying visualization library.

  This class should never have been a node class, as it has no
  distinguishing features in that regard. We duplicate this design
  flaw for the sake of being compatible with code written for the TGS
  VolumeViz extension library.
*/

#include <VolumeViz/nodes/SoVolumeRendering.h>
#include <VolumeViz/nodes/SoVolumeData.h>
#include <VolumeViz/elements/SoVolumeDataElement.h>
#include <VolumeViz/elements/SoTransferFunctionElement.h>
#include <VolumeViz/nodes/SoVolumeRender.h>
#include <VolumeViz/nodes/SoTransferFunction.h>
#include <VolumeViz/nodes/SoOrthoSlice.h>
#include <VolumeViz/details/SoVolumeRenderDetail.h>
#include <VolumeViz/details/SoOrthoSliceDetail.h>
#include <VolumeViz/render/common/CvrTextureObject.h>

#include <Inventor/actions/SoGLRenderAction.h>


// *************************************************************************

SO_NODE_SOURCE(SoVolumeRendering);

// *************************************************************************

class SoVolumeRenderingP {
public:
  static SbBool wasinitialized;
};

SbBool SoVolumeRenderingP::wasinitialized = FALSE;

#define PRIVATE(p) (p->pimpl)
#define PUBLIC(p) (p->master)

// *************************************************************************

SoVolumeRendering::SoVolumeRendering(void)
{
  SO_NODE_CONSTRUCTOR(SoVolumeRendering);

  PRIVATE(this) = NULL; // pimpl-class not yet needed
}

SoVolumeRendering::~SoVolumeRendering()
{
  delete PRIVATE(this);
}

/*!
  Does all necessary class initializations of the volume rendering
  system.

  Application programmers must call this method explicitly at the
  start of the application, before any volume rendering nodes are
  made. It must be invoked \e after SoXt::init() / SoQt::init() /
  SoWin::init(), though.
 */
void
SoVolumeRendering::init(void)
{
  if (SoVolumeRenderingP::wasinitialized) return;
  SoVolumeRenderingP::wasinitialized = TRUE;

  SoVolumeDataElement::initClass();
  SoTransferFunctionElement::initClass();

  SoVolumeRendering::initClass();
  SoVolumeData::initClass();
  SoTransferFunction::initClass();

  SoVolumeRender::initClass();
  SoOrthoSlice::initClass();

  SoVolumeRenderDetail::initClass();
  SoOrthoSliceDetail::initClass();

  // Internal classes follows:

  CvrTextureObject::initClass();
}

void
SoVolumeRendering::initClass(void)
{
  SO_NODE_INIT_CLASS(SoVolumeRendering, SoNode, "SoNode");
}

SoVolumeRendering::HW_SupportStatus
SoVolumeRendering::isSupported(HW_Feature feature)
{
  switch (feature) {

  case SoVolumeRendering::HW_VOLUMEPRO:
    return SoVolumeRendering::NO;

  case SoVolumeRendering::HW_3DTEXMAP:
    // FIXME: update this when support is in place. 20021106 mortene.
    return SoVolumeRendering::NO;

  case SoVolumeRendering::HW_TEXCOLORMAP:
    // FIXME: update this when support is in place. 20021106 mortene.
    return SoVolumeRendering::NO;

  case SoVolumeRendering::HW_TEXCOMPRESSION:
    // FIXME: update this when support is in place. 20021106 mortene.
    return SoVolumeRendering::NO;

  default:
    assert(FALSE && "unknown feature");
    break;
  }

  return SoVolumeRendering::NO;
}
