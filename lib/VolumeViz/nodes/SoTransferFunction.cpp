/**************************************************************************\
 *
 *  Copyright (C) 1998-2000 by Systems in Motion.  All rights reserved.
 *
 *  Systems in Motion AS, Prof. Brochs gate 6, N-7030 Trondheim, NORWAY
 *  http://www.sim.no/ sales@sim.no Voice: +47 22114160 Fax: +47 67172912
 *
\**************************************************************************/


#include <VolumeViz/nodes/SoTransferFunction.h>

#if HAVE_CONFIG_H
#include <config.h>
#endif // HAVE_CONFIG_H

// *************************************************************************

SO_NODE_SOURCE(SoTransferFunction);

// *************************************************************************

class SoTransferFunctionP{
public:
  SoTransferFunctionP(SoTransferFunction * master) {
    this->master = master;
  }

private:
  SoTransferFunction * master;
};


#define PRIVATE(p) (p->pimpl)
#define PUBLIC(p) (p->master)

// *************************************************************************

/*!
  Constructor.
*/
SoTransferFunction::SoTransferFunction(void)
{
  PRIVATE(this) = new SoTransferFunctionP(this);

}//Constructor




/*!
  Destructor.
*/
SoTransferFunction::~SoTransferFunction()
{
  delete PRIVATE(this);
}


void 
SoTransferFunction::reMap(int min, int max)
{}
