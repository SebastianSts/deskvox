//****************************************************************************
// Project:         Virvo (Virtual Reality Volume Renderer)
// Copyright:       (c) 1999-2004 Jurgen P. Schulze. All rights reserved.
// Author's E-Mail: schulze@cs.brown.edu
// Affiliation:     Brown University, Department of Computer Science
//****************************************************************************

#ifdef _WIN32
#include <windows.h>
#endif

#include <math.h>
#include <string.h>
#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif

#include <iostream>

#ifdef VV_DEBUG_MEMORY
#include <crtdbg.h>
#define new new(_NORMAL_BLOCK,__FILE__, __LINE__)
#endif

#include "../src/vvdebugmsg.h"
#include "../src/vvtokenizer.h"
#include "vvobjview.h"

using std::cerr;
using std::cout;
using std::endl;

const float vvObjView::VIEWER_POS_X = 0.0f;
const float vvObjView::VIEWER_POS_Y = 0.0f;
const float vvObjView::VIEWER_POS_Z = -2.0f;

//----------------------------------------------------------------------------
  /// Constructor.
  vvObjView::vvObjView()
  {
  vvDebugMsg::msg(1, "vvObjView::vvObjView()");
  aspect = 1.0f;                                 // default aspect ratio is 1:1
  cameraString = "VIRVO_CAMERA";
  reset();
}


//----------------------------------------------------------------------------
/** Initialization routine.
  May be called by the application anytime to reset object orientation
  and projection matrix.
*/
void vvObjView::reset()
{
  vvDebugMsg::msg(1, "vvObjView::reset()");
  projType  = ORTHO;                             // default projection mode is orthographic
  eyeDist   = -0.03f;                            // default eye distance (empirical value)
  rotAngle  = 5.0f;                              // default rotational angle (empirical value)
  fov       = 2.0f;                              // default field of view
  zNear     = -100.0f;
  zFar      = 100.0f;
  resetMV();
}


//----------------------------------------------------------------------------
/** Reset modelview matrix only.
 */
void vvObjView::resetMV()
{
  vvDebugMsg::msg(1, "vvObjView::resetMV()");
  mv.identity();
  mv.translate(VIEWER_POS_X, VIEWER_POS_Y, VIEWER_POS_Z);
}


//----------------------------------------------------------------------------
/** Save modelview matrix to an ASCII file. If a file exists with the same
    name, it will be overwritten. Here is an example file contents:<BR>
  <PRE>
  VIRVO Camera
  0.944588 0.051051 0.324263 0.000000
  0.092229 0.906766 -0.411423 0.000000
  -0.315034 0.418532 0.851813 -2.000000
  0.000000 0.000000 0.000000 1.000000
  </PRE>
  @param filename name of file to save (convention for extension: .txt)
  @return true if file was written ok, false if file couldn't be written
*/
bool vvObjView::saveMV(const char* filename)
{
  FILE* fp;
  int i;

  vvDebugMsg::msg(1, "vvObjView::saveMV()");

  fp = fopen(filename, "wb");
  if (fp==NULL) return false;
  fputs(cameraString, fp);
  fputc('\n', fp);
  for (i=0; i<4; ++i)
  {
    fprintf(fp, "%f %f %f %f\n", mv.e[i][0], mv.e[i][1], mv.e[i][2], mv.e[i][3]);
  }
  fclose(fp);
  return true;
}


//----------------------------------------------------------------------------
/** Load modelview matrix from an ASCII file.
  @param filename name of file to load
  @return true if file was loaded ok, false if file couldn't be loaded
*/
bool vvObjView::loadMV(const char* filename)
{
  FILE* fp;
  int i,j;
  bool retval = false;
  vvMatrix camera;

  vvDebugMsg::msg(1, "vvObjView::loadMV()");

  fp = fopen(filename, "rb");
  if (fp==NULL) return false;

  // Initialize tokenizer:
  vvTokenizer::TokenType ttype;
  vvTokenizer* tokenizer = new vvTokenizer(fp);
  tokenizer->setCommentCharacter('#');
  tokenizer->setEOLisSignificant(false);
  tokenizer->setCaseConversion(vvTokenizer::VV_UPPER);
  tokenizer->setParseNumbers(true);

  // Parse file:
  ttype = tokenizer->nextToken();
  if (ttype!=vvTokenizer::VV_WORD) goto done;
  if (strcmp(tokenizer->sval, cameraString) != 0) goto done;
  for (i=0; i<4; ++i)
    for (j=0; j<4; ++j)
  {
    ttype = tokenizer->nextToken();
    if (ttype != vvTokenizer::VV_NUMBER) goto done;
    camera.e[i][j] = tokenizer->nval;
  }
  mv.copy(&camera);
  retval = true;

  done:
  delete tokenizer;
  fclose(fp);
  return retval;
}


//----------------------------------------------------------------------------
/** Set the projection matrix.
  @param pt        projection type: ORTHO, PERSPECTIVE, or FRUSTUM
  @param range     minimum horizontal and vertical viewing range, format depends
                   on projection type: for ORTHO and FRUSTUM range defines the
                   viewing range in world coordinates, for PERSPECTIVE it defines
                   the field of view in degrees.
  @param nearPlane distance from viewer to near clipping plane (>0)
  @param farPlane  distance from viewer to far clipping plane (>0)
*/
void vvObjView::setProjection(ProjectionType pt, float range, float nearPlane, float farPlane)
{
  const float MINIMUM = 0.0001f;                 // minimum value for perspective near/far planes

  vvDebugMsg::msg(2, "vvObjView::setProjection()");

  // Near and far planes need value checking for perspective modes:
  if (pt!=ORTHO)
  {
    if (nearPlane<=0.0f) nearPlane = MINIMUM;
    if (farPlane <=0.0f) farPlane  = MINIMUM;
  }

  fov   = range;
  zNear = nearPlane;
  zFar  = farPlane;
  projType  = pt;
  updateProjectionMatrix();
}


//----------------------------------------------------------------------------
/** Set the OpenGL projection matrix according to current class values.
 */
void vvObjView::updateProjectionMatrix()
{
  GLint glsMatrixMode;                           // stores GL_MATRIX_MODE
  float xHalf, yHalf;                            // half x and y coordinate range
  float fovy;                                    // field of view in y direction

  vvDebugMsg::msg(2, "vvObjView::updateProjectionMatrix()");

  // Save matrix mode:
  glGetIntegerv(GL_MATRIX_MODE, &glsMatrixMode);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  // Precompute x and y range:
  xHalf = 0.5f * ((aspect < 1.0f) ? fov : (fov * aspect));
  yHalf = 0.5f * ((aspect > 1.0f) ? fov : (fov / aspect));
  fovy  = (aspect > 1.0f) ? fov : (180.0f / VV_PI * ((float)atan(tan(fov * VV_PI / 180.0f) / aspect)));

  // Set new projection matrix:
  switch (projType)
  {
  case ORTHO:
    glOrtho(-xHalf, xHalf, -yHalf, yHalf, zNear, zFar);
    break;
  case FRUSTUM:cerr << "FRUSTUM" << endl;
    glFrustum(-xHalf, xHalf, -yHalf, yHalf, zNear, zFar);
    break;
  case PERSPECTIVE:
    gluPerspective(fovy, aspect, zNear, zFar);
    break;
  default: break;
  }

  // Restore matrix mode:
  glMatrixMode(glsMatrixMode);
}


//----------------------------------------------------------------------------
/** Set the aspect ratio of the viewing window.
  @param ar aspect ratio (= viewing window width/height)
*/
void vvObjView::setAspectRatio(float ar)
{
  vvDebugMsg::msg(2, "vvObjView::setAspectRatio()");
  if (aspect>0.0f)
  {
    aspect = ar;
    updateProjectionMatrix();
  }
}


//----------------------------------------------------------------------------
/** Set the depth clipping planes positions
  @param newNP,newFP  positions of the new near and far clipping planes
*/
void vvObjView::setDepthRange(float newNP, float newFP)
{
  vvDebugMsg::msg(2, "vvObjView::setDepthRange()");
  zNear = newNP;
  zFar  = newFP;
  updateProjectionMatrix();
}


//----------------------------------------------------------------------------
/** Update the OpenGL modelview matrix for a particular eye position.
  The OpenGL draw buffer must be selected by the caller.
  @param eye  eye for which to draw the object
*/
void vvObjView::updateModelviewMatrix(EyeType eye)
{
  vvMatrix mvRight;                             // modelview matrix for right eye
  vvMatrix invRot;                              // inverse rotational matrix
  vvVector3 v(0.0, -1.0, 0.0);                   // rotational vector
  GLint glsMatrixMode;                           // stores GL_MATRIX_MODE
  float flat[16];

  vvDebugMsg::msg(2, "vvObjView::updateModelviewMatrix()");

  // Save matrix mode:
  glGetIntegerv(GL_MATRIX_MODE, &glsMatrixMode);
  glMatrixMode(GL_MODELVIEW);

  if (eye!=RIGHT_EYE)                             // use stored matrix for left eye
  {
    mv.makeGL(flat);
  }
  else                                           // convert for right eye
  {
    // Convert axis coordinates (v) from WCS to OCS:
    mvRight.copy(&mv);
    invRot.identity();
    invRot.copyRot(&mv);
    invRot.invertOrtho();
    v.multiply(&invRot);
    v.normalize();                              // normalize before rotation!

    mvRight.rotate(rotAngle * (float)VV_PI / 180.0f, v.e[0], v.e[1], v.e[2]);
    mvRight.translate(eyeDist, 0.0f, 0.0f);
    mvRight.makeGL(flat);
  }

  // Load matrix to OpenGL:
  glLoadMatrixf(flat);

  // Restore matrix mode:
  glMatrixMode(glsMatrixMode);
}


//-----------------------------------------------------------------------------
/** Rotates the model view matrix according to a fictitious trackball.
  @param width, height  window sizes in pixels
  @param fromX, fromY   mouse move starting position in pixels
  @param toX, toY       mouse move end position in pixels
*/
void vvObjView::trackballRotation(int width, int height, int fromX, int fromY, int toX, int toY)
{
  mv.trackballRotation(width, height, fromX, fromY, toX, toY);
}


//----------------------------------------------------------------------------
/** Set eye distance for stereo viewing.
  @param ed eye distance
*/
void vvObjView::setEyeDistance(float ed)
{
  eyeDist = ed;
}


//----------------------------------------------------------------------------
/** Set rotational angle for stereo viewing.
  @param angle new rotational angle
*/
void vvObjView::setRotationalAngle(float angle)
{
  rotAngle = angle;
}


//============================================================================
// End of File
//============================================================================
