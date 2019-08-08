#include <GL/glew.h>
#include "vvvoldesc.h"
#include "vvsparserenderer.h"
#include "vvshaderfactory.h"
#include "vvglslprogram.h"
#include "gl/util.h"
#include "vvshaderprogram.h"
#include "vvtextureutil.h"
#include <fstream>

using namespace std;
namespace gl = virvo::gl;
//size of the window

int pass = 1;                                        ///< 1 = first pass, 2 = second pass
float g_stepSize = 0.001f;                           ///< Size of steps used by raymarcher
GLuint nodeSize = sizeof(GLfloat) + sizeof(GLuint);  ///< Float for depth, uint for pointer to next node


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
       glTexParameteri(GL_TEXTURE_3D_EXT, GL_TEXTURE_WRAP_S, GL_REPEAT);
       glTexParameteri(GL_TEXTURE_3D_EXT, GL_TEXTURE_WRAP_T, GL_REPEAT);
       glTexParameteri(GL_TEXTURE_3D_EXT, GL_TEXTURE_WRAP_R_EXT, GL_CLAMP_TO_EDGE);
       glPixelStorei(GL_UNPACK_ALIGNMENT,1);
       glTexImage3D(GL_TEXTURE_3D_EXT, 0, GL_INTENSITY, texSizeX, texSizeY, texSizeZ, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE,data);
}


vvSparseRenderer::vvSparseRenderer(vvVolDesc* vd, vvRenderState renderState)
    : vvRenderer(vd, renderState){

        rendererType = RAYRENDSPARSE;
        glewInit();
        glGetIntegerv(GL_VIEWPORT, viewport.data());       
        g_winWidth = viewport[2];
        g_winHeight = viewport[3];
        g_tffTexObj.reset(gl::createTexture());
        maxNodes = g_winWidth * g_winHeight * 20;
        bbox = vd->getBoundingBox();
        initVBO();
        initVBOFace();

        _shaderFactory.reset(new vvShaderFactory());
        shaderPassOne = _shaderFactory->createProgram("linkedList1", "", "linkedList1");
        shaderPassTwo = _shaderFactory->createProgram("linkedList2", "", "linkedList2");

        texSizeX = vd->vox[0];
        texSizeY = vd->vox[1];
        texSizeZ = vd->vox[2];
        data = vd->getRaw(0);
        initVol3DTex();
        initHeadPtrTex(g_winWidth,g_winHeight);
        initLinkedList();
        updateTransferFunction();
        initClearBuffers();
    }
vvSparseRenderer::~vvSparseRenderer(){
    }

void vvSparseRenderer::initVBO()
{

        GLfloat vertices[24]=
                {bbox.min.x, bbox.min.y, bbox.min.z,
                 bbox.min.x, bbox.min.y, bbox.max.z,
                 bbox.min.x, bbox.max.y, bbox.min.z,
                 bbox.min.x, bbox.max.y, bbox.max.z,
                 bbox.max.x, bbox.min.y, bbox.min.z,
                 bbox.max.x, bbox.min.y, bbox.max.z,
                 bbox.max.x, bbox.max.y, bbox.min.z,
                 bbox.max.x, bbox.max.y, bbox.max.z,


    /*  GLfloat vertices[24]={ -128.0,-128.0,-112.5,
                             -128.0,-128.0, 112.5,
                             -128.0, 128.0,-112.5,
                             -128.0, 128.0, 112.5,
                              128.0,-128.0,-112.5,
                              128.0,-128.0, 112.5,
                              128.0, 128.0,-112.5,
                              128.0, 128.0, 112.5,*/
};
/* void initVBO()
{
    GLfloat vertices[24] = {

        0.0, 0.0, 0.0,//Front Face
        0.0, 0.0, 1.0,
        0.0, 1.0, 0.0,//Front Face
        0.0, 1.0, 1.0,
        1.0, 0.0, 0.0,//Front Face
        1.0, 0.0, 1.0,
        1.0, 1.0, 0.0,//Front Face
        1.0, 1.0, 1.0,

      /* 0.0, 0.5, 0.0,  //Front Face    //oben links vorne
        0.0, 0.5, 0.5,
        0.0, 1.0, 0.0,  //Front Face
        0.0, 1.0, 0.5,
        0.5, 0.5, 0.0,  //Front Face
        0.5, 0.5, 0.5,
        0.5, 1.0, 0.0,  //Front Face
        0.5, 1.0, 0.5,

        0.5, 0.5, 0.0,  //Front Face //oben rechts vorne
        0.5, 0.5, 0.5,
        0.5, 1.0, 0.0,//Front Face
        0.5, 1.0, 0.5,
        1.0, 0.5, 0.0,//Front Face
        1.0, 0.5, 0.5,
        1.0, 1.0, 0.0,//Front Face
        1.0, 1.0, 0.5,


        0.0, 0.0, 0.0, //unten links vorne
        0.0, 0.0, 0.5,
        0.0, 0.5, 0.0,
        0.0, 0.5, 0.5,
        0.5, 0.0, 0.0,
        0.5, 0.0, 0.5,
        0.5, 0.5, 0.0,
        0.5, 0.5, 0.5,

        0.5, 0.0, 0.0, // unten rechts vorne
        0.5, 0.0, 0.5,
        0.5, 0.5, 0.0,
        0.5, 0.5, 0.5,
        1.0, 0.0, 0.0,
        1.0, 0.0, 0.5,
        1.0, 0.5, 0.0,
        1.0, 0.5, 0.5,

        0.0, 0.5, 0.5, //oben links hinten
        0.0, 0.5, 1.0,
        0.0, 1.0, 0.5,
        0.0, 1.0, 1.0,
        0.5, 0.5, 0.5,
        0.5, 0.5, 1.0,
        0.5, 1.0, 0.5,
        0.5, 1.0, 1.0,

        0.5, 0.5, 0.5, //oben rechts hinten
        0.5, 0.5, 1.0,
        0.5, 1.0, 0.5,
        0.5, 1.0, 1.0,
        1.0, 0.5, 0.5,
        1.0, 0.5, 1.0,
        1.0, 1.0, 0.5,
        1.0, 1.0, 1.0,

        0.0, 0.0, 0.5, //unten links hinten
        0.0, 0.0, 1.0,
        0.0, 0.5, 0.5,
        0.0, 0.5, 1.0,
        0.5, 0.0, 0.5,
        0.5, 0.0, 1.0,
        0.5, 0.5, 0.5,
        0.5, 0.5, 1.0,

        0.5, 0.0, 0.5, //unten rechts hinten
        0.5, 0.0, 1.0,
        0.5, 0.5, 0.5,
        0.5, 0.5, 1.0,
        1.0, 0.0, 0.5,
        1.0, 0.0, 1.0,
        1.0, 0.5, 0.5,
        1.0, 0.5, 1.0,

        0.0, 0.5, 0.0, //oben links vorne
        0.0, 0.5, 0.25,
        0.0, 0.75, 0.0,
        0.0, 0.75, 0.25,
        0.25, 0.5, 0.0,
        0.25, 0.5, 0.25,
        0.25, 0.75, 0.0,
        0.25, 0.75, 0.25,

    };*/

    GLuint indices[36] = {
        1,5,7,
        7,3,1,
        0,2,6,
        6,4,0,
        0,1,3,
        3,2,0,
        7,5,4,
        4,6,7,
        2,3,7,
        7,6,2,
        1,0,4,
        4,5,1,

      /*  9,13,15,
        15,11,9,
        8,10,14,
        14,12,8,
        8,9,11,
        11,10,8,
        15,13,12,
        12,14,15,
        10,11,15,
        15,14,10,
        9,8,12,
        12,13,9,

        17,21,23,
        23,19,17,
        16,18,22,
        22,20,16,
        16,17,19,
        19,18,16,
        23,21,20,
        20,22,23,
        18,19,23,
        23,22,18,
        17,16,20,
        20,21,17,

        17+8,21+8,23+8,
        23+8,19+8,17+8,
        16+8,18+8,22+8,
        22+8,20+8,16+8,
        16+8,17+8,19+8,
        19+8,18+8,16+8,
        23+8,21+8,20+8,
        20+8,22+8,23+8,
        18+8,19+8,23+8,
        23+8,22+8,18+8,
        17+8,16+8,20+8,
        20+8,21+8,17+8,

        17+16,21+16,23+16,
        23+16,19+16,17+16,
        16+16,18+16,22+16,
        22+16,20+16,16+16,
        16+16,17+16,19+16,
        19+16,18+16,16+16,
        23+16,21+16,20+16,
        20+16,22+16,23+16,
        18+16,19+16,23+16,
        23+16,22+16,18+16,
        17+16,16+16,20+16,
        20+16,21+16,17+16,

        17+24,21+24,23+24,
        23+24,19+24,17+24,
        16+24,18+24,22+24,
        22+24,20+24,16+24,
        16+24,17+24,19+24,
        19+24,18+24,16+24,
        23+24,21+24,20+24,
        20+24,22+24,23+24,
        18+24,19+24,23+24,
        23+24,22+24,18+24,
        17+24,16+24,20+24,
        20+24,21+24,17+24,

        17+32,21+32,23+32,
        23+32,19+32,17+32,
        16+32,18+32,22+32,
        22+32,20+32,16+32,
        16+32,17+32,19+32,
        19+32,18+32,16+32,
        23+32,21+32,20+32,
        20+32,22+32,23+32,
        18+32,19+32,23+32,
        23+32,22+32,18+32,
        17+32,16+32,20+32,
        20+32,21+32,17+32,

        17+40,21+40,23+40,
        23+40,19+40,17+40,
        16+40,18+40,22+40,
        22+40,20+40,16+40,
        16+40,17+40,19+40,
        19+40,18+40,16+40,
        23+40,21+40,20+40,
        20+40,22+40,23+40,
        18+40,19+40,23+40,
        23+40,22+40,18+40,
        17+40,16+40,20+40,
        20+40,21+40,17+40,

        17+48,21+48,23+48,
        23+48,19+48,17+48,
        16+48,18+48,22+48,
        22+48,20+48,16+48,
        16+48,17+48,19+48,
        19+48,18+48,16+48,
        23+48,21+48,20+48,
        20+48,22+48,23+48,
        18+48,19+48,23+48,
        23+48,22+48,18+48,
        17+48,16+48,20+48,
        20+48,21+48,17+48,

        17+56,21+56,23+56,
        23+56,19+56,17+56,
        16+56,18+56,22+56,
        22+56,20+56,16+56,
        16+56,17+56,19+56,
        19+56,18+56,16+56,
        23+56,21+56,20+56,
        20+56,22+56,23+56,
        18+56,19+56,23+56,
        23+56,22+56,18+56,
        17+56,16+56,20+56,
        20+56,21+56,17+56,*/

    };

    GLfloat value[8] = {

        1.0,
               1.0,
               1.0,
               1.0,
               1.0,
               1.0,
               1.0,
               1.0,
   /*      0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,




        1.0,
        1.0,
        1.0,
        1.0,
        1.0,
        1.0,
        1.0,
        1.0,

        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,

        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,

        1.0,
        1.0,
        1.0,
        1.0,
        1.0,
        1.0,
        1.0,
        1.0,



        1.0,
        1.0,
        1.0,
        1.0,
        1.0,
        1.0,
        1.0,
        1.0,


        1.0,
        1.0,
        1.0,
        1.0,
        1.0,
        1.0,
        1.0,
        1.0,

        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,

        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,*/


    };

    GLuint vao;
    GLuint gbo[3];
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(3, gbo);
    glBindBuffer(GL_ARRAY_BUFFER, gbo[0]);
    glBufferData(GL_ARRAY_BUFFER, 24*sizeof(GLfloat), vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLfloat *)NULL);
    glBindBuffer(GL_ARRAY_BUFFER, gbo[1]);
    glBufferData(GL_ARRAY_BUFFER, 8*sizeof(GLfloat), value, GL_STATIC_DRAW);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 0, (GLfloat *)NULL);
    // used in glDrawElement()
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gbo[2]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 36*sizeof(GLuint), indices, GL_STATIC_DRAW);
    glBindVertexArray(0);
    g_vao = vao;

}

void vvSparseRenderer::initVBOFace(){
    GLfloat verticesFace[12]  {-1, -1, 0, // bottom left corner
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
    // used in glDrawElement()
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gboFace[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 4*sizeof(GLuint), indicesFace, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0); // for vertexloc
    glEnableVertexAttribArray(1); // for vertexcol
    // the vertex location is the same as the vertex color    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLfloat *)NULL);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (GLfloat *)NULL);  
    glBindVertexArray(0);
    g_vaoFace = vaoFace;
}

void vvSparseRenderer::resizeBuffer(uint newWidth, uint newHeight)
    {
    g_winWidth = newWidth;
    g_winHeight = newHeight;
    maxNodes = g_winHeight * g_winWidth * 20;
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


void vvSparseRenderer::setUniformsPassOne(vvShaderProgram* shader){
     shader->setParameterMatrix4f("MVP",mvp);
     shader->setParameter1i("MaxNodes",maxNodes);
    }


void vvSparseRenderer::setUniformsPassTwo(vvShaderProgram* shader){
     shader->setParameterTex3D("VolumeTex",g_volTexObj.get());
     shader->setParameterTex1D("TransferFunc",g_tffTexObj.get());
     shader->setParameter1f("StepSize",g_stepSize);
     shader->setParameter1f("ScreenSizeX",g_winWidth);
     shader->setParameter1f("ScreenSizeY",g_winHeight);
     shader->setParameterMatrix4f("invMVP",invmvp);

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
}
void vvSparseRenderer::render()
{
    //first pass builds linked list
    if(pass == 1)
    {
    glBindVertexArray(g_vao);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, (GLuint *)NULL);
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

    glGetIntegerv(GL_VIEWPORT, viewport.data());
    //buffer needs to be resized if viewport changes
    if(g_winWidth != viewport[2] || g_winHeight != viewport[3])
           resizeBuffer(viewport[2],viewport[3]);

    //MVP & inverse MVP
    glGetFloatv(GL_MODELVIEW_MATRIX, view_matrix.data());
    glGetFloatv(GL_PROJECTION_MATRIX, proj_matrix.data());
    mvp = proj_matrix*view_matrix;

    invmvp = inverse(mvp);
    invmvp = translate(virvo::mat4::identity(), bbox.max)*invmvp;
    invmvp = scale(virvo::mat4::identity(), virvo::vec3f(1.f)/bbox.size())* invmvp;

    //clear HeadPtrTex
    clearBuffers();
    glClearColor(1.0f,1.0f,1.0f,1.0f);

    //first render pass
    pass= 1;
    shaderPassOne->enable();
    setUniformsPassOne(shaderPassOne);
    render();
    shaderPassOne->disable();

    //second renderpass
    pass= 2;
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
   // glViewport(0, 0, g_winWidth, g_winHeight);
    shaderPassTwo->enable();
    setUniformsPassTwo(shaderPassTwo);
    render();
    shaderPassTwo->disable();

    }
