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

#include <iostream>

#include <string.h>
#include "vvopengl.h"
#include "vvdebugmsg.h"

#ifdef VV_DEBUG_MEMORY
#include <crtdbg.h>
#define new new(_NORMAL_BLOCK,__FILE__, __LINE__)
#endif

#include "vvgltools.h"

using namespace std;

//============================================================================
// Method Definitions
//============================================================================

//----------------------------------------------------------------------------
/** Check OpenGL for errors.
    @param  error string if there was an error, otherwise return NULL
*/
void vvGLTools::printGLError(const char* msg)
{
  const GLenum err = glGetError();
  if(err != GL_NO_ERROR)
  {
    const char* str = (const char*)gluErrorString(err);
    cerr << "GL error: " << msg << ", " << str << endl;
  }
}

//----------------------------------------------------------------------------
/** Checks OpenGL for a specific OpenGL version.
    @param major OpenGL major version to check
    @param minor OpenGL minor version to check
    @param release OpenGL release version to check
    @return true if version is supported
*/
bool vvGLTools::isGLVersionSupported(int major, int minor, int release)
{
  (void)release;
  // Get version string from OpenGL:
  const GLubyte* verstring = glGetString(GL_VERSION);
  if (verstring=='\0') return false;

  int ver[3] = { 0, 0, 0 };
  int idx = 0;
  for (const GLubyte *p = verstring;
      *p && *p != ' ' && idx < 3;
      ++p)
  {
    if (*p == '.')
    {
      ++idx;
    }
    else if (*p >= '0' && *p <= '9')
    {
      ver[idx] *= 10;
      ver[idx] += *p-'0';
    }
    else
      return false;
  }

  vvDebugMsg::msg(3, "GL version ", ver[0], ver[1], ver[2]);

  if(ver[0] < major)
    return false;
  if(ver[0] > major)
    return true;

  if(ver[1] < minor)
    return false;
  if(ver[1] >= minor)
    return true;

  return false;
}

//----------------------------------------------------------------------------
/** Checks OpenGL for a specific extension.
    @param extension OpenGL extension to check for (e.g. "GL_EXT_bgra")
    @return true if extension is supported
*/
bool vvGLTools::isGLextensionSupported(const char* extension)
{
  // Check requested extension name for existence and for spaces:
  const GLubyte* where = (GLubyte*)strchr(extension, ' ');
  if (where || *extension=='\0') return false;

  // Get extensions string from OpenGL:
  const GLubyte* extensions = glGetString(GL_EXTENSIONS);
  if (extensions=='\0') return false;

  // Parse OpenGL extensions string:
  const GLubyte* start = extensions;
  for (;;)
  {
    where = (GLubyte*)strstr((const char*)start, extension);
    if (!where) return false;
    const GLubyte* terminator = where + strlen(extension);
    if (where==start || *(where - 1)==' ')
      if (*terminator==' ' || *terminator=='\0')
        return true;
    start = terminator;
  }
}

//----------------------------------------------------------------------------
/** Display the OpenGL extensions which are supported by the system at
  run time.
  @param style display style
*/
void vvGLTools::displayOpenGLextensions(const DisplayStyle style)
{
  char* extCopy;                                  // local copy of extensions string for modifications

  const char* extensions = (const char*)glGetString(GL_EXTENSIONS);

  switch (style)
  {
    default:
    case CONSECUTIVE:
      cerr << extensions << endl;
      break;
    case ONE_BY_ONE:
      extCopy = new char[strlen(extensions) + 1];
      strcpy(extCopy, extensions);
      for (int i=0; i<(int)strlen(extCopy); ++i)
        if (extCopy[i] == ' ') extCopy[i] = '\n';
      cerr << extCopy << endl;
      delete[] extCopy;
      break;
  }
}

//----------------------------------------------------------------------------
/** Check for some specific OpenGL extensions.
  Displays the status of volume rendering related extensions, each on a separate line.
*/
void vvGLTools::checkOpenGLextensions()
{
  const char* status[2] = {"supported", "not found"};

  cerr << "GL_EXT_texture3D...............";
  cerr << ((vvGLTools::isGLextensionSupported("GL_EXT_texture3D")) ? status[0] : status[1]) << endl;

  cerr << "GL_EXT_texture_edge_clamp......";
  cerr << ((vvGLTools::isGLextensionSupported("GL_EXT_texture_edge_clamp")) ? status[0] : status[1]) << endl;

  cerr << "GL_SGI_texture_color_table.....";
  cerr << ((vvGLTools::isGLextensionSupported("GL_SGI_texture_color_table")) ? status[0] : status[1]) << endl;

  cerr << "GL_EXT_paletted_texture........";
  cerr << ((vvGLTools::isGLextensionSupported("GL_EXT_paletted_texture")) ? status[0] : status[1]) << endl;

  cerr << "GL_EXT_blend_equation..........";
  cerr << ((vvGLTools::isGLextensionSupported("GL_EXT_blend_equation")) ? status[0] : status[1]) << endl;

  cerr << "GL_EXT_shared_texture_palette..";
  cerr << ((vvGLTools::isGLextensionSupported("GL_EXT_shared_texture_palette")) ? status[0] : status[1]) << endl;

  cerr << "GL_EXT_blend_minmax............";
  cerr << ((vvGLTools::isGLextensionSupported("GL_EXT_blend_minmax")) ? status[0] : status[1]) << endl;

  cerr << "GL_ARB_multitexture............";
  cerr << ((vvGLTools::isGLextensionSupported("GL_ARB_multitexture")) ? status[0] : status[1]) << endl;

  cerr << "GL_NV_texture_shader...........";
  cerr << ((vvGLTools::isGLextensionSupported("GL_NV_texture_shader")) ? status[0] : status[1]) << endl;

  cerr << "GL_NV_texture_shader2..........";
  cerr << ((vvGLTools::isGLextensionSupported("GL_NV_texture_shader2")) ? status[0] : status[1]) << endl;

  cerr << "GL_NV_texture_shader3..........";
  cerr << ((vvGLTools::isGLextensionSupported("GL_NV_texture_shader3")) ? status[0] : status[1]) << endl;

  cerr << "GL_ARB_texture_env_combine.....";
  cerr << ((vvGLTools::isGLextensionSupported("GL_ARB_texture_env_combine")) ? status[0] : status[1]) << endl;

  cerr << "GL_NV_register_combiners.......";
  cerr << ((vvGLTools::isGLextensionSupported("GL_NV_register_combiners")) ? status[0] : status[1]) << endl;

  cerr << "GL_NV_register_combiners2......";
  cerr << ((vvGLTools::isGLextensionSupported("GL_NV_register_combiners2")) ? status[0] : status[1]) << endl;

  cerr << "GL_ARB_fragment_program........";
  cerr << ((vvGLTools::isGLextensionSupported("GL_ARB_fragment_program")) ? status[0] : status[1]) << endl;

  cerr << "GL_ATI_fragment_shader.........";
  cerr << ((vvGLTools::isGLextensionSupported("GL_ATI_fragment_shader")) ? status[0] : status[1]) << endl;

  cerr << "GL_ARB_imaging.................";
  cerr << ((vvGLTools::isGLextensionSupported("GL_ARB_imaging")) ? status[0] : status[1]) << endl;
}

void vvGLTools::draw(vvVector3* vec)
{
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_LIGHTING);
  glColor3f(1.0f, 1.0f, 1.0f);

  glRasterPos3f(vec->e[0], vec->e[1], vec->e[2]);
  GLubyte v[2][2][3];

  for (int i=0; i<2; i++)
  {
      for (int j=0; j<2; j++)
      {
          for (int k=0; k<3; k++)
          {
              v[i][j][k] = 0;
          }
      }
  }
  glDrawPixels(2, 2, GL_RGB, GL_UNSIGNED_BYTE, v);
  glEnable(GL_LIGHTING);
  glEnable(GL_DEPTH_TEST);
}

//----------------------------------------------------------------------------
/** Draw view aligned quad. If no vertex coordinates are specified,
    these default to: (-1.0f, -1.0f) (1.0f, 1.0f). No multi texture coordinates
    supported.
*/
void vvGLTools::drawViewAlignedQuad(const float x1, const float y1,
                                    const float x2, const float y2)
{
  glBegin(GL_QUADS);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    glNormal3f(0.0f, 0.0f, 1.0f);

    glTexCoord2f(0.0f, 0.0f);
    glVertex2f(x1, y1);

    glTexCoord2f(1.0f, 0.0f);
    glVertex2f(x2, y1);

    glTexCoord2f(1.0f, 1.0f);
    glVertex2f(x2, y2);

    glTexCoord2f(0.0f, 1.0f);
    glVertex2f(x1, y2);
  glEnd();
}

//----------------------------------------------------------------------------
/** Get OpenGL viewport info: 0 ==> x, 1 ==> y, 2 ==> width, 3 ==> height.
*/
vvGLTools::Viewport vvGLTools::getViewport()
{
  Viewport result;
  glGetIntegerv(GL_VIEWPORT, result.values);
  return result;
}

//============================================================================
// End of File
//============================================================================
