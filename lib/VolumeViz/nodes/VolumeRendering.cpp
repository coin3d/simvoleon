/*!
  \mainpage

  FIXME, should insert doc on at least these issues:

    - what is volume rendering
    - why volume rendering
    - list features of SimVoleon
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
  volume rendering framework.
*/
// FIXME: simplest programming example here on how to use the
// vol-rendering. 20021106 mortene.

#include <VolumeViz/nodes/SoVolumeRendering.h>
#include <VolumeViz/nodes/SoVolumeData.h>
#include <VolumeViz/elements/SoVolumeDataElement.h>
#include <VolumeViz/elements/SoTransferFunctionElement.h>
#include <VolumeViz/nodes/SoVolumeRender.h>
#include <VolumeViz/nodes/SoTransferFunction.h>
#include <VolumeViz/nodes/SoROI.h>
#include <VolumeViz/details/SoVolumeRenderDetail.h>
#include <VolumeViz/details/SoOrthoSliceDetail.h>
#include <VolumeViz/details/SoObliqueSliceDetail.h>
#include <VolumeViz/render/2D/CvrTextureObject.h>

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
  SoROI::initClass();
  SoTransferFunction::initClass();

  SoVolumeRender::initClass();

  SoVolumeRenderDetail::initClass();
  SoOrthoSliceDetail::initClass();
  SoObliqueSliceDetail::initClass();

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
