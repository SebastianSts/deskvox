#include <GL/glew.h>
#include "vvvoldesc.h"
#include "vvsparserenderer.h"
#include "vvshaderfactory.h"
#include "vvglslprogram.h"
#include "gl/util.h"
#include "vvshaderprogram.h"
#include "private/vvgltools.h"
#include "vvtextureutil.h"
#include <fstream>
#include "vvspaceskip.h"

using namespace std;
namespace gl = virvo::gl;
//size of the window

int pass = 1;                                         ///< 1 = first pass, 2 = second pass
float g_stepSize;                                     ///< Size of steps used by raymarcher
GLuint nodeSize = sizeof(GLfloat) + sizeof(GLuint);   ///< Float for depth, uint for pointer to next node
int first =0;
//bool sortEveryCube = true;
bool sortEveryCube = false;
float scaleX = 1.0;
float scaleY;
float scaleZ;


enum{
    COUNTER_BUFFER,
    LINKED_LIST_BUFFER
    };

void vvSparseRenderer::initHeadPtrTex(GLuint bfTexWidth, GLuint bfTexHeight)
{
  // The buffer for the head pointers
  g_HeadPtrTex.reset(gl::createTexture());
  glBindTexture(GL_TEXTURE_2D, g_HeadPtrTex.get());
  glTexStorage2D(GL_TEXTURE_2D, 1, GL_R32UI, bfTexWidth, bfTexHeight);
  glBindImageTexture(0, g_HeadPtrTex.get(), 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32UI);
}


void vvSparseRenderer::initVol3DTex()
{       
       g_volTexObj.reset(gl::createTexture());
       glBindTexture(GL_TEXTURE_3D, g_volTexObj.get());
       glPixelStorei(GL_UNPACK_ALIGNMENT,1);
       glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
       glTexParameteri(GL_TEXTURE_3D_EXT, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
       glTexParameteri(GL_TEXTURE_3D_EXT, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
       glTexParameteri(GL_TEXTURE_3D_EXT, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
       glTexParameteri(GL_TEXTURE_3D_EXT, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
       glTexParameteri(GL_TEXTURE_3D_EXT, GL_TEXTURE_WRAP_R_EXT, GL_CLAMP_TO_EDGE);
       glPixelStorei(GL_UNPACK_ALIGNMENT,1);
       glTexImage3D(GL_TEXTURE_3D_EXT, 0, GL_INTENSITY, texSizeX, texSizeY, texSizeZ, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE,data);

}


void vvSparseRenderer::initCounter()
{
    g_CounterImage.reset(gl::createTexture());
}

vvSparseRenderer::vvSparseRenderer(vvVolDesc* vd, vvRenderState renderState)
    : vvRenderer(vd, renderState)
    //      , tree(virvo::SkipTree::Grid)
    //      , tree(virvo::SkipTree::LBVH)
          , tree(virvo::SkipTree::SVTKdTree)
    //        , tree(virvo::SkipTree::SVTKdTreeCU)
{

        rendererType = RAYRENDSPARSE;
        glewInit();
        glGetIntegerv(GL_VIEWPORT, viewport.data());       
        g_winWidth = viewport[2];
        g_winHeight = viewport[3];
        g_tffTexObj.reset(gl::createTexture());
        maxNodes = g_winWidth * g_winHeight*200 ;
        bbox = vd->getBoundingBox();
       // std::cout<<bbox.size()<<std::endl;
        _shaderFactory.reset(new vvShaderFactory());
        shaderPassOne = _shaderFactory->createProgram("linkedList1", "", "linkedList1");
        shaderPassTwo = _shaderFactory->createProgram("linkedList2", "", "linkedList2");
        texSizeX = vd->vox[0];
        texSizeY = vd->vox[1];
        texSizeZ = vd->vox[2];
        data = vd->getRaw(0);
        initVol3DTex();
        initHeadPtrTex(g_winWidth,g_winHeight);
        initCounter();
        initLinkedList();
        initClearBuffers();
        updateVolumeData();
        updateTransferFunction();      
        initVBOFace();
        std::cout<<"good"<<std::endl;
    }
vvSparseRenderer::~vvSparseRenderer(){
    }

void vvSparseRenderer::initVBO(std::vector<virvo::aabb> boxes, bool newBuffer)
{

        std::vector<GLfloat> vertices(boxes.size()*24);
        std::vector<GLuint> indices(boxes.size()*36);
        int one = 0;
        for (uint i = 0; i< boxes.size(); i++)
        {

            GLfloat tmp[24]  = {
                  boxes[i].min.x, boxes[i].min.y, boxes[i].min.z,
                  boxes[i].min.x, boxes[i].min.y, boxes[i].max.z,
                  boxes[i].min.x, boxes[i].max.y, boxes[i].min.z,
                  boxes[i].min.x, boxes[i].max.y, boxes[i].max.z,
                  boxes[i].max.x, boxes[i].min.y, boxes[i].min.z,
                  boxes[i].max.x, boxes[i].min.y, boxes[i].max.z,
                  boxes[i].max.x, boxes[i].max.y, boxes[i].min.z,
                  boxes[i].max.x, boxes[i].max.y, boxes[i].max.z,
                  };

             if(one==0)
               {
                    virvo::vec3 middle[6] = {
                        virvo::vec3((boxes[i].min.x+boxes[i].max.x)*0.5, (boxes[i].min.y+boxes[i].max.y)*0.5, (boxes[i].min.z+boxes[i].min.z)*0.5), //front
                        virvo::vec3((boxes[i].min.x+boxes[i].max.x)*0.5, (boxes[i].min.y+boxes[i].max.y)*0.5, (boxes[i].max.z+boxes[i].max.z)*0.5), //back
                        virvo::vec3((boxes[i].min.x+boxes[i].min.x)*0.5, (boxes[i].min.y+boxes[i].max.y)*0.5, (boxes[i].min.z+boxes[i].max.z)*0.5), //left
                        virvo::vec3((boxes[i].max.x+boxes[i].max.x)*0.5, (boxes[i].min.y+boxes[i].max.y)*0.5, (boxes[i].min.z+boxes[i].max.z)*0.5), //right
                        virvo::vec3((boxes[i].min.x+boxes[i].max.x)*0.5, (boxes[i].max.y+boxes[i].max.y)*0.5, (boxes[i].min.z+boxes[i].max.z)*0.5), //top
                        virvo::vec3((boxes[i].min.x+boxes[i].max.x)*0.5, (boxes[i].min.y+boxes[i].min.y)*0.5, (boxes[i].min.z+boxes[i].max.z)*0.5), //bottom
                        };
                    virvo::vec3* testMiddle = middle;

                   virvo::vec3* first = reinterpret_cast<virvo::vec3*>(middle);
                   virvo::vec3* last = reinterpret_cast<virvo::vec3*>(middle + 6);
                   virvo::vec3 eye(getEyePosition().x, getEyePosition().y, getEyePosition().z);

                   std::sort(first, last, [eye](virvo::vec3 const& a, virvo::vec3 const& b)
                        {
                        auto len1 = length(a-eye);
                        auto len2 = length(b-eye);
                        return len1 >len2;
                        });

                  int j=0;
                  while( j<6)
                    {
                        if(middle[j]==testMiddle[0])//virvo::vec3((boxes[i].min.x+boxes[i].max.x)*0.5, (boxes[i].min.y+boxes[i].max.y)*0.5, (boxes[i].min.z+boxes[i].min.z)*0.5))
                          {
                          baseIndices[j*6]=0+i*8;
                          baseIndices[j*6+1]=2+i*8;
                          baseIndices[j*6+2]=6+i*8;
                          baseIndices[j*6+3]=6+i*8;
                          baseIndices[j*6+4]=4+i*8;
                          baseIndices[j*6+5]=0+i*8;
                          }
                        if(middle[j]==testMiddle[1])//virvo::vec3((boxes[i].min.x+boxes[i].max.x)*0.5, (boxes[i].min.y+boxes[i].max.y)*0.5, (boxes[i].max.z+boxes[i].max.z)*0.5))
                          {
                          baseIndices[j*6]=1+i*8;
                          baseIndices[j*6+1]=5+i*8;
                          baseIndices[j*6+2]=7+i*8;
                          baseIndices[j*6+3]=7+i*8;
                          baseIndices[j*6+4]=3+i*8;
                          baseIndices[j*6+5]=1+i*8;

                         }
                       if(middle[j]==testMiddle[2])//virvo::vec3((boxes[i].min.x+boxes[i].min.x)*0.5, (boxes[i].min.y+boxes[i].max.y)*0.5, (boxes[i].min.z+boxes[i].max.z)*0.5))
                         {
                         baseIndices[j*6]=0+i*8;
                         baseIndices[j*6+1]=1+i*8;
                         baseIndices[j*6+2]=3+i*8;
                         baseIndices[j*6+3]=3+i*8;
                         baseIndices[j*6+4]=2+i*8;
                         baseIndices[j*6+5]=0+i*8;

                         }
                       if(middle[j]==testMiddle[3])//virvo::vec3((boxes[i].max.x+boxes[i].max.x)*0.5, (boxes[i].min.y+boxes[i].max.y)*0.5, (boxes[i].min.z+boxes[i].max.z)*0.5))
                         {
                        baseIndices[j*6]=7+i*8;
                        baseIndices[j*6+1]=5+i*8;
                        baseIndices[j*6+2]=4+i*8;
                        baseIndices[j*6+3]=4+i*8;
                        baseIndices[j*6+4]=6+i*8;
                        baseIndices[j*6+5]=7+i*8;

                         }
                       if(middle[j]==testMiddle[4])//virvo::vec3((boxes[i].min.x+boxes[i].max.x)*0.5, (boxes[i].max.y+boxes[i].max.y)*0.5, (boxes[i].min.z+boxes[i].max.z)*0.5))
                        {
                        baseIndices[j*6]=2+i*8;
                        baseIndices[j*6+1]=3+i*8;
                        baseIndices[j*6+2]=7+i*8;
                        baseIndices[j*6+3]=7+i*8;
                        baseIndices[j*6+4]=6+i*8;
                        baseIndices[j*6+5]=2+i*8;
                        }
                      if(middle[j]==testMiddle[5])//virvo::vec3((boxes[i].min.x+boxes[i].max.x)*0.5, (boxes[i].min.y+boxes[i].min.y)*0.5, (boxes[i].min.z+boxes[i].max.z)*0.5))
                        {
                        baseIndices[j*6]=1+i*8;
                        baseIndices[j*6+1]=0+i*8;
                        baseIndices[j*6+2]=4+i*8;
                        baseIndices[j*6+3]=4+i*8;
                        baseIndices[j*6+4]=5+i*8;
                        baseIndices[j*6+5]=1+i*8;
                        }
                        j++;
                     }
                  if(sortEveryCube == false)
                     one++;
                 }

           GLuint index[36];
           for(int j = 0; j<36; j++)
                   {
                   if(sortEveryCube == true)
                   index[j] = baseIndices[j];
                   else
                   index[j] = baseIndices[j]+i*8;
                   }

            memcpy(&indices[i*36], index, sizeof(GLuint)*36);
            memcpy(&vertices[i*24], tmp, sizeof(GLfloat)*24);
          }

    if(newBuffer == true)
    glGenVertexArrays(1, &g_vao);
    glBindVertexArray(g_vao);
    if(newBuffer == true)
    glGenBuffers(2, gbo);
    glBindBuffer(GL_ARRAY_BUFFER, gbo[0]);
    glBufferData(GL_ARRAY_BUFFER, boxes.size()*24*sizeof(GLfloat), vertices.data(), GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLfloat *)NULL);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gbo[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, boxes.size()*36*sizeof(GLuint), indices.data(), GL_DYNAMIC_DRAW);
    glBindVertexArray(0);
}

void vvSparseRenderer::initVBOFace(){
    GLfloat verticesFace[12] = {-1, -1, 0, // bottom left corner
                                -1,  1, 0, // top left corner
                                 1,  1, 0, // top right corner
                                 1, -1, 0};

    GLuint indicesFace[4] = {0,1,2,3}; // first triangle (bottom left - top left - top right)
    GLuint vaoFace;
    GLuint gboFace[2];
    glGenVertexArrays(1, &vaoFace);
    glBindVertexArray(vaoFace);
    glGenBuffers(2, gboFace);
    glBindBuffer(GL_ARRAY_BUFFER, gboFace[0]);
    glBufferData(GL_ARRAY_BUFFER, 12*sizeof(GLfloat), verticesFace, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gboFace[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 4*sizeof(GLuint), indicesFace, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLfloat *)NULL);
    glBindVertexArray(0);
    g_vaoFace = vaoFace;
}

void vvSparseRenderer::resizeBuffer(uint newWidth, uint newHeight)
    {
    g_winWidth = newWidth;
    g_winHeight = newHeight;
    maxNodes = g_winHeight * g_winWidth * 50;
    initHeadPtrTex(g_winWidth,g_winHeight);
    initLinkedList();
    initClearBuffers();
}

void vvSparseRenderer::initLinkedList()
    {
    glGenBuffers(2, buffers);
    // Atomic counter
    glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, buffers[COUNTER_BUFFER]);
    glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(GLuint), NULL, GL_DYNAMIC_DRAW);
    // Buffer of linked lists
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, buffers[LINKED_LIST_BUFFER]);
    glBufferData(GL_SHADER_STORAGE_BUFFER, maxNodes * nodeSize, NULL, GL_DYNAMIC_DRAW);

    }

void vvSparseRenderer::initClearBuffers()
    {
    vector<GLuint> headPtrClearBuf(g_winWidth*g_winHeight, 0xffffffff);
    glGenBuffers(1, &clearBuf);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, clearBuf);
    glBufferData(GL_PIXEL_UNPACK_BUFFER, headPtrClearBuf.size() * sizeof(GLuint),
                 &headPtrClearBuf[0], GL_STATIC_COPY);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
    }

void vvSparseRenderer::clearBuffers()
    {
    GLuint start = 0;
    glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, buffers[COUNTER_BUFFER] );
    glBufferSubData(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint), &start);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, clearBuf);
    glBindTexture(GL_TEXTURE_2D, g_HeadPtrTex.get());
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, g_winWidth, g_winHeight, GL_RED_INTEGER,
                    GL_UNSIGNED_INT, NULL);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
    }

void vvSparseRenderer::clearCounter()
    {
    vector<GLuint> nulls(g_winWidth*g_winHeight*4, 0x0);
    glBindTexture(GL_TEXTURE_2D, g_CounterImage.get());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glPixelStorei(GL_UNPACK_ALIGNMENT,1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, g_winWidth, g_winHeight, 0, GL_RGBA,
                    GL_UNSIGNED_INT, nulls.data());
    }


void vvSparseRenderer::setUniformsPassOne(vvShaderProgram* shader){
     shader->setParameterMatrix4f("MVP",mvp);
     shader->setParameter1i("MaxNodes",maxNodes);
    }


void vvSparseRenderer::setUniformsPassTwo(vvShaderProgram* shader){
     shader->setParameterTex3D("VolumeTex",g_volTexObj.get());
     shader->setParameterTex2D("Counter",g_CounterImage.get());
     shader->setParameterTex1D("TransferFunc",g_tffTexObj.get());
     shader->setParameter1f("StepSize",g_stepSize);
     shader->setParameter1f("ScaleX",scaleX);
     shader->setParameter1f("ScaleY",scaleY);
     shader->setParameter1f("ScaleZ",scaleZ);
     shader->setParameter1f("ScreenSizeX",g_winWidth);
     shader->setParameter1f("ScreenSizeY",g_winHeight);
     shader->setParameterMatrix4f("texMat",texMat);

    }
void vvSparseRenderer::updateTransferFunction()
    {
     std::vector<virvo::vec4> tf(256 * 1 * 1);
     vd->computeTFTexture(0, 256, 1, 1, reinterpret_cast<float*>(tf.data()));
     static int parameter = 0;
     glBindTexture(GL_TEXTURE_1D, g_tffTexObj.get());
     if (!parameter) {
         glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
         glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);
         glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
         glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
         parameter++;
         }
     glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA32F, 256, 0, GL_RGBA, GL_FLOAT, tf.data());
     tree.updateTransfunc(reinterpret_cast<const uint8_t*>(tf.data()), 256, 1, 1, virvo::PF_RGBA32F);
     virvo::vec3 eye(getEyePosition().x, getEyePosition().y, getEyePosition().z);
     bool frontToBack = false;
     std::vector<virvo::aabb> bricks = tree.getSortedBricks(eye, frontToBack);
     numBoxes = bricks.size();


     newBuffer = true;
     glDeleteBuffers(2, gbo);
     glDeleteVertexArrays(1, &g_vao);
     initVBO(bricks, newBuffer);
}
void vvSparseRenderer::updateVolumeData(){
    tree.updateVolume(*vd);
}

void vvSparseRenderer::render()
{
    //first pass builds linked list
    if(pass == 1)
    {
    glBindVertexArray(g_vao);
    glDrawElements(GL_TRIANGLES, numBoxes*36, GL_UNSIGNED_INT, (GLuint *)NULL);
    glBindVertexArray(0);
    }

    //second pass
    if(pass == 2)
    {
    glBindVertexArray(g_vaoFace);
    glDrawElements(GL_QUADS, 4, GL_UNSIGNED_INT, (GLuint *)NULL);
    glBindVertexArray(0);
    }

}


void vvSparseRenderer::renderVolumeGL(){


    virvo::vec3 eye(getEyePosition().x, getEyePosition().y, getEyePosition().z);
    bool frontToBack = false;
    std::vector<virvo::aabb> bricks = tree.getSortedBricks(eye, frontToBack);
    numBoxes = bricks.size();
    newBuffer = false;
    initVBO(bricks, newBuffer);
    glGetIntegerv(GL_VIEWPORT, viewport.data());
    //buffer needs to be resized if viewport changes
    if(g_winWidth != viewport[2] || g_winHeight != viewport[3])
           resizeBuffer(viewport[2],viewport[3]);
    //MVP & inverse MVP
    glGetFloatv(GL_MODELVIEW_MATRIX, view_matrix.data());
    glGetFloatv(GL_PROJECTION_MATRIX, proj_matrix.data());
    mvp = proj_matrix*view_matrix;
    texMat = inverse(mvp);
    texMat = translate(virvo::mat4::identity(), bbox.max)*texMat;
    texMat = scale(virvo::mat4::identity(), virvo::vec3f(1.f)/bbox.size())* texMat;

    int axis = 0;
        if (vd->getSize()[1] / vd->vox[1] < vd->getSize()[axis] / vd->vox[axis])
        {
            axis = 1;
        }
        if (vd->getSize()[2] / vd->vox[2] < vd->getSize()[axis] / vd->vox[axis])
        {
            axis = 2;
        }

         g_stepSize = (vd->getSize()[axis] / vd->vox[axis]) / _quality;

         virvo::vec3 pt1(0.f);
         virvo::vec3 pt2(g_stepSize,0,0);

         virvo::vec3 tpt1(
                 ( pt1.x + (bbox.size().x / 2) ) / bbox.size().x,
                 (-pt1.y + (bbox.size().y / 2) ) / bbox.size().y,
                 (-pt1.z + (bbox.size().z / 2) ) / bbox.size().z
                 );

         virvo::vec3 tpt2(
                 ( pt2.x + (bbox.size().x / 2) ) / bbox.size().x,
                 (-pt2.y + (bbox.size().y / 2) ) / bbox.size().y,
                 (-pt2.z + (bbox.size().z / 2) ) / bbox.size().z
                 );
         virvo::vec3 v = tpt2-tpt1;
         g_stepSize = virvo::length(v);
         scaleY = bbox.size().y / bbox.size().x;
         scaleZ = bbox.size().z / bbox.size().x;

    //clear HeadPtrTex
    clearBuffers();
    clearCounter();
    glClearColor(0.0f,0.0f,0.0f,1.0f);
    //first render pass
    pass= 1;
    shaderPassOne->enable();
    setUniformsPassOne(shaderPassOne);
    render();
    shaderPassOne->disable();
    //second renderpass
    pass= 2;
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );   
    shaderPassTwo->enable();
    setUniformsPassTwo(shaderPassTwo);
    render();
    shaderPassTwo->disable();

    {

        vector<GLuint> pixels(g_winWidth*g_winHeight*4);
        glBindTexture(GL_TEXTURE_2D, g_CounterImage.get());
        glGetnTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_INT,
                       g_winWidth*g_winHeight*4*4, pixels.data());
        for (auto ui : pixels)
            if (ui != 0) std::cout << ui << '\n';

    }

    if (_boundaries)
       {
           glEnable(GL_DEPTH_TEST);
           glDepthRange(0,0.95);
           glClearDepth(1.0f);
           glClear(GL_DEPTH_BUFFER_BIT);

           glLineWidth(3.0f);

           //glEnable(GL_LINE_SMOOTH);
           //glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

           //glEnable(GL_BLEND);
           //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

           bool isLightingEnabled = glIsEnabled(GL_LIGHTING);
           glDisable(GL_LIGHTING);

           virvo::vec4 clearColor = vvGLTools::queryClearColor();
           vvColor color(1.0f - clearColor[0], 1.0f - clearColor[1], 1.0f - clearColor[2]);
           tree.renderGL(color);

           //renderBoundingBox();

           if (isLightingEnabled)
              glEnable(GL_LIGHTING);
       }

    }
