// Virvo - Virtual Reality Volume Rendering
// Copyright (C) 1999-2003 University of Stuttgart, 2004-2005 Brown University
// Contact: Jurgen P. Schulze, jschulze@ucsd.edu
//
// This file is part of Virvo.
//
// Virvo is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library (see license.txt); if not, write to the
// Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA

#ifndef _VV_IMAGECLIENT_H_
#define _VV_IMAGECLIENT_H_

#include "vvbsptree.h"
#include "vvexport.h"
#include "vvopengl.h"
#include "vvremoteclient.h"
#include "vvvoldesc.h"
#include "vvpthread.h"
#include "vvshaderfactory.h"
#include "vvshaderprogram.h"
#include "vvgltools.h"
#include "vvrayrend.h"

#include <vector>

class vvRenderer;
class vvSlaveVisitor;

class VIRVOEXPORT vvImageClient : public vvRemoteClient
{
public:
  vvImageClient(vvVolDesc *vd, vvRenderState renderState,
              const char* slaveNames, int slavePorts,
              const char* slaveFileNames);
  ~vvImageClient();

  ErrorType render();                                     ///< render image with depth-values
  void exit();                                            ///< check out from servers

private:
  GLuint _rgbaTex;                                        ///< Texture names for RGBA image

  vvMatrix _currentMv;                                    ///< Current modelview matrix
  vvMatrix _currentPr;                                    ///< Current projection matrix
  vvRemoteClient::ErrorType requestFrame();               ///< remember envoironment and send image-request to server
  vvImage *_image;
};

#endif
// vim: sw=2:expandtab:softtabstop=2:ts=2:cino=\:0g0t0