#include <VolumeViz/nodes/SoVolumeRendering.h>
#include <VolumeViz/nodes/SoVolumeData.h>
#include <VolumeViz/nodes/SoVolumeRender.h>
#include <VolumeViz/nodes/SoTransferFunction.h>
#include <Inventor/Qt/SoQtRenderArea.h>
#include <qapplication.h>
#include <Inventor/Qt/SoQt.h>
#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/nodes/SoPerspectiveCamera.h>
#include <Inventor/nodes/SoDirectionalLight.h>
#include <qvbox.h>
#include <Inventor/nodes/SoCube.h>
#include <Inventor/manips/SoTrackballManip.h>
#include <Inventor/nodes/SoDrawStyle.h>
#include <math.h>


void setDot(int *pData, 
            int xDim, int yDim, int zDim, 
            float X, float Y, float Z, 
            unsigned char R,
            unsigned char G,
            unsigned char B,
            unsigned char A)
{
  int nX = int(xDim*X);
  int nY = int(yDim*Y);
  int nZ = int(zDim*Z);

  int pixel = (A<<24) + (B<<16) + (G<<8) + R;
  pData[nZ*xDim*yDim + nY*xDim + nX] = pixel;

  pixel = ((A>>2)<<24) + (B<<16) + (G<<8) + R ;
  pData[(nZ+1)*xDim*yDim + nY*xDim + nX] = pixel;
  pData[(nZ-1)*xDim*yDim + nY*xDim + nX] = pixel;
  pData[nZ*xDim*yDim + (nY+1)*xDim + nX] = pixel;
  pData[nZ*xDim*yDim + (nY-1)*xDim + nX] = pixel;
  pData[nZ*xDim*yDim + nY*xDim + (nX+1)] = pixel;
  pData[nZ*xDim*yDim + nY*xDim + (nX-1)] = pixel;
}

void * generateFancy3DTexture(int xDim, int yDim, int zDim)
{
  int * texture = new int[xDim*yDim*zDim];
  memset(texture, 0, xDim*yDim*zDim*4);


  float t = 0;
  
  while (t < 50) {
    float x = sin((t + 1.4234)*1.9);
    float y = cos((t*2.5) - 10);
    float z = cos((t - 0.23123)*3)*sin(t + 0.5);

    setDot( texture,
            xDim, yDim, zDim,
            x*sin(t)*0.45 + 0.5, 
            y*0.45 + 0.5, 
            z*cos(t)*0.45 + 0.5, 
            255.0*cos(t), 255.0*sin(t), 255.0, 255.0);

    t += 0.001;
  }//while*/

  return texture;
}



main(int argc, char **argv)
{

  // Initialize Qt and SoQt.
  QApplication app(argc, argv);
  SoQt::init((QWidget *)NULL);
  SoVolumeRendering::initClass();

  // Setting up renderarea, camera, lights
  QVBox * box = new QVBox;
  SoQtRenderArea * renderArea = new SoQtRenderArea(box);

  SoSeparator *root = new SoSeparator;
  root->ref();

  SoPerspectiveCamera *myCamera = new SoPerspectiveCamera;
  myCamera->position = SbVec3f(0.0f, 0.0f, 5.0f);
  root->addChild(myCamera);
  root->addChild(new SoDirectionalLight);

  SoDrawStyle *drawStyle = new SoDrawStyle;
  drawStyle->style = SoDrawStyle::LINES;
  root->addChild(new SoTrackballManip);
  root->addChild(drawStyle);
  root->addChild(new SoCube);

  // Read the data (application supplied function)
  SbVec3s dim = SbVec3s(64, 64, 64);

  int *pData = (int *)generateFancy3DTexture(dim[0], dim[1], dim[2]);

  // Add VolumeData to scene graph
  SoVolumeData *pVolData = new SoVolumeData();
  pVolData->setVolumeData( dim, (unsigned char *)pData );
  pVolData->setVolumeSize(SbBox3f(-1, -1, -1, 1, 1, 1));

  root->addChild( pVolData );

  // Add TransferFunction (color map) to scene graph
  SoTransferFunction *pTransFunc = new SoTransferFunction();
  root->addChild( pTransFunc );

  // Add VolumeRender to scene graph
  SoVolumeRender *pVolRend = new SoVolumeRender();
  root->addChild( pVolRend );
  pVolRend->numSlices = 64;

  box->resize(500, 500);

  myCamera->viewAll(root, renderArea->getViewportRegion());
  renderArea->setSceneGraph(root);
  
  app.setMainWidget( box );
  box->show();
  return app.exec();
}
