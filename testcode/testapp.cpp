#include <VolumeViz/nodes/SoVolumeRendering.h>
#include <VolumeViz/nodes/SoVolumeData.h>
#include <VolumeViz/nodes/SoVolumeRender.h>
#include <VolumeViz/nodes/SoTransferFunction.h>
#include <VolumeViz/nodes/SoROI.h>
#include <Inventor/Qt/viewers/SoQtExaminerViewer.h>
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
#include <Inventor/draggers/SoHandleBoxDragger.h>
#include <Inventor/sensors/SoFieldSensor.h>
#include <Inventor/nodes/SoPickStyle.h>


void scaleCallback(void *data, SoSensor *sensor);
void translateCallback(void *data, SoSensor *sensor);

SoFieldSensor * scaleSensor = NULL;
SoFieldSensor * translateSensor = NULL;
SoHandleBoxDragger * dragger = NULL;
SoVolumeData * volumeData = NULL;
SoROI * roi = NULL;

void setDotRGBA(int *pData,
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

void * generateFancy3DTextureRGBA(int xDim, int yDim, int zDim)
{
  int * texture = new int[xDim*yDim*zDim];
  memset(texture, 0, xDim*yDim*zDim*4);


  float t = 0;

  while (t < 100) {
    float x = sin((t + 1.4234)*1.9);
    float y = cos((t*2.5) - 10);
    float z = cos((t - 0.23123)*3)*sin(t + 0.5);

    setDotRGBA( texture,
            xDim, yDim, zDim,
            sin(t*12)*0.45 + 0.5,
            sin(t*15)*cos(t*5)*0.45 + 0.5,
            cos(t*4.5)*0.45 + 0.5,
            255.0*cos(t), 255.0*sin(t), 255.0, 255.0);

    t += 0.0005;
  }//while*/

  return texture;
}



void setDotPal16(unsigned short *pData,
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

  unsigned short pixel = (A<<12) + (B<<8) + (G<<4) + R;
  pData[nZ*xDim*yDim + nY*xDim + nX] = pixel;

  pixel = ((A>>1)<<12) + (B<<8) + (G<<4) + R ;
  pData[(nZ+1)*xDim*yDim + nY*xDim + nX] = pixel;
  pData[(nZ-1)*xDim*yDim + nY*xDim + nX] = pixel;
  pData[nZ*xDim*yDim + (nY+1)*xDim + nX] = pixel;
  pData[nZ*xDim*yDim + (nY-1)*xDim + nX] = pixel;
  pData[nZ*xDim*yDim + nY*xDim + (nX+1)] = pixel;
  pData[nZ*xDim*yDim + nY*xDim + (nX-1)] = pixel;
}


void * generateFancy3DTexturePal16(int xDim, int yDim, int zDim, float *& palette)
{
  // Constructing a palette with 4444 RGBA structure.
  palette = new float[4*65536];
  for (int i = 0; i < 65536; i++) {
    int R = (i & 15);
    int G = (i >>  4) & 15;
    int B = (i >>  8) & 15;
    int A = (i >> 12) & 15;

    palette[i*4 + 0] = float(R)/15.0;
    palette[i*4 + 1] = float(G)/15.0;
    palette[i*4 + 2] = float(B)/15.0;
    palette[i*4 + 3] = float(A)/15.0;
  }// for



  unsigned short * texture = new unsigned short[xDim*yDim*zDim];
  memset(texture, 0, sizeof(unsigned short)*xDim*yDim*zDim);
  float t = 0;

  while (t < 100) {
    float x = sin((t + 1.4234)*1.9);
    float y = cos((t*2.5) - 10);
    float z = cos((t - 0.23123)*3)*sin(t + 0.5);

    setDotPal16( texture,
            xDim, yDim, zDim,
            sin(t*12)*0.45 + 0.5,
            sin(t*15)*cos(t*5)*0.45 + 0.5,
            cos(t*4.5)*0.45 + 0.5,
            15.0*cos(t), 15.0*sin(t), 15.0, 15.0);


    t += 0.0005;
  }//while*/

  return texture;
}// generateFancy3DTextureShort



main(int argc, char **argv)
{


	// Initialize Inventor. This returns a main window to use.
	// If unsuccessful, exit.
	QWidget *myWindow = SoQt::init(argv[0]); // pass the app name
	if (myWindow == NULL) exit(1);

  // Initialize Qt and SoQt.
  SoVolumeRendering::initClass();

  SoSeparator *root = new SoSeparator;
  root->ref();

  SoPerspectiveCamera *myCamera = new SoPerspectiveCamera;
  myCamera->position = SbVec3f(0.0f, 0.0f, 5.0f);

  root->addChild(myCamera);
  root->addChild(new SoDirectionalLight);


  dragger = new SoHandleBoxDragger;
  root->addChild(dragger);
  scaleSensor = new SoFieldSensor(scaleCallback, NULL);
  scaleSensor->attach(&dragger->scaleFactor);
  translateSensor = new SoFieldSensor(translateCallback, NULL);
  translateSensor->attach(&dragger->translation);
  dragger->scaleFactor = SbVec3f(0.2, 0.2, 0.2);

  volumeData = new SoVolumeData();
  SoTransferFunction *pTransFunc = new SoTransferFunction();

  SbVec3s dim = SbVec3s(256, 256, 256);


  float * palette;
  unsigned short *pData =
    (unsigned short*)generateFancy3DTexturePal16(dim[0], dim[1], dim[2], palette);

  volumeData->setVolumeData( dim,
                            (unsigned char *)pData,
                            SoVolumeRendering::UNSIGNED_SHORT);

  pTransFunc->colorMap.setNum(65536*4);
  float * p = pTransFunc->colorMap.startEditing();
  memcpy(p, palette, 65536*sizeof(float)*4);
  pTransFunc->colorMap.finishEditing();



/*  unsigned int * pData =
    (unsigned int*)generateFancy3DTextureRGBA(dim[0], dim[1], dim[2]);
  volumeData->setVolumeData( dim,
                            (unsigned char *)pData,
                            SoVolumeRendering::RGBA);

  */

  volumeData->setVolumeSize(SbBox3f(-1, -1, -1, 1, 1, 1));
  volumeData->setPageSize(SbVec3s(32, 32, 32));
  volumeData->setTexMemorySize(1024*1024*7);
  root->addChild( volumeData );

  // Add TransferFunction (color map) to scene graph
  root->addChild( pTransFunc );


  // Add VolumeRender to scene graph
  //SoVolumeRender *pVolRend = new SoVolumeRender();
  //root->addChild( pVolRend );
  //pVolRend->numSlices = 64;

  roi = new SoROI;
  roi->box.setValue(0, 0, 0, 63, 63, 63);
  root->addChild(roi);

  SoDrawStyle *drawStyle = new SoDrawStyle;
  drawStyle->style = SoDrawStyle::LINES;
  root->addChild(drawStyle);
  SoPickStyle * pickStyle = new SoPickStyle;
  pickStyle->style = SoPickStyle::UNPICKABLE;
  root->addChild(pickStyle);
  root->addChild(new SoCube);

	myCamera->viewAll(root, myExaminerViewer->getViewportRegion());

	// Put our scene in myExaminerViewer, change the title
	myExaminerViewer->setSceneGraph(root);
	myExaminerViewer->setTitle("Hello Cone");
	myExaminerViewer->show();
	SoQt::show(myWindow); // Display main window
	SoQt::mainLoop(); // Main Inventor event loop
  return 0;
}



void scaleCallback(void *data, SoSensor *sensor)
{
  SbVec3f scale = dragger->scaleFactor.getValue();
  SbVec3f translate = dragger->translation.getValue();
  if (translate[0] + scale[0] > 1) scale[0] = 1 - translate[0];
  if (translate[1] + scale[1] > 1) scale[1] = 1 - translate[1];
  if (translate[2] + scale[2] > 1) scale[2] = 1 - translate[2];
  if (translate[0] - scale[0] < -1) scale[0] = translate[0] + 1;
  if (translate[1] - scale[1] < -1) scale[1] = translate[1] + 1;
  if (translate[2] - scale[2] < -1) scale[2] = translate[2] + 1;
  if (scale[0] < 0.1) scale[0] = 0.1;
  if (scale[1] < 0.1) scale[1] = 0.1;
  if (scale[2] < 0.1) scale[2] = 0.1;
  scaleSensor->detach();
  dragger->scaleFactor = scale;
  scaleSensor->attach(&dragger->scaleFactor);

  SbVec3s dimensions = volumeData->getDimensions();
  SbVec3s min, max;
  min[0] = ((translate[0] - scale[0])/2.0 + 0.5) * dimensions[0];
  min[1] = ((translate[1] - scale[1])/2.0 + 0.5) * dimensions[1];
  min[2] = ((translate[2] - scale[2])/2.0 + 0.5) * dimensions[2];
  max[0] = ((translate[0] + scale[0])/2.0 + 0.5) * dimensions[0];
  max[1] = ((translate[1] + scale[1])/2.0 + 0.5) * dimensions[1];
  max[2] = ((translate[2] + scale[2])/2.0 + 0.5) * dimensions[2];
  roi->box.setValue(min, max);
}//scaleCallback


void translateCallback(void *data, SoSensor *sensor)
{
  SbVec3f scale = dragger->scaleFactor.getValue();
  SbVec3f translate = dragger->translation.getValue();
  if (translate[0] + scale[0] > 1) translate[0] = 1 - scale[0];
  if (translate[1] + scale[1] > 1) translate[1] = 1 - scale[1];
  if (translate[2] + scale[2] > 1) translate[2] = 1 - scale[2];
  if (translate[0] - scale[0] < -1) translate[0] = -1 + scale[0];
  if (translate[1] - scale[1] < -1) translate[1] = -1 + scale[1];
  if (translate[2] - scale[2] < -1) translate[2] = -1 + scale[2];
  translateSensor->detach();
  dragger->translation = translate;
  translateSensor->attach(&dragger->translation);

  SbVec3s dimensions = volumeData->getDimensions();
  SbVec3s min, max;
  min[0] = ((translate[0] - scale[0])/2.0 + 0.5) * dimensions[0];
  min[1] = ((translate[1] - scale[1])/2.0 + 0.5) * dimensions[1];
  min[2] = ((translate[2] - scale[2])/2.0 + 0.5) * dimensions[2];
  max[0] = ((translate[0] + scale[0])/2.0 + 0.5) * dimensions[0];
  max[1] = ((translate[1] + scale[1])/2.0 + 0.5) * dimensions[1];
  max[2] = ((translate[2] + scale[2])/2.0 + 0.5) * dimensions[2];
  roi->box.setValue(min, max);
}//translateCallback
