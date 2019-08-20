#ifndef VVSPARSERENDERER_H
#define VVSPARSERENDERER_H

#include <boost/scoped_ptr.hpp>

// Virvo:
#include "vvexport.h"
#include "vvrenderer.h"
#include "vvopengl.h"
#include "gl/handle.h"
#include "vvspaceskip.h"

class vvShaderFactory;
class vvShaderProgram;
class vvVolDesc;


class vvSparseRenderer : public vvRenderer
{

public:
    VVAPI vvSparseRenderer(vvVolDesc* vd, vvRenderState renderState);
    VVAPI ~vvSparseRenderer();
    VVAPI virtual void renderVolumeGL() VV_OVERRIDE;
    VVAPI virtual void updateTransferFunction() VV_OVERRIDE;
    VVAPI virtual void updateVolumeData() VV_OVERRIDE;
    virvo::SkipTree tree;


enum{
    COUNTER_BUFFER,
    LINKED_LIST_BUFFER
    };





private:


    void initLinkedList();						///< Init linked list
    void initClearBuffers();						///< Init clear buffer to reset nodeIdx 	
    void initVBO(std::vector<virvo::aabb> boxes, bool newBuffer);
    void initVBOFace();
    void initVol3DTex();						///< Init volume on graphicscard
    void initHeadPtrTex(GLuint bfTexWidth, GLuint bfTexHeight);	
    void resizeBuffer(uint newWidth, uint newHeight);			///< Resize linked list when size of viewport changes		
    void clearBuffers();						///< Reset nodeIdx 
    void setUniformsPassOne(vvShaderProgram* shader);			///< Uniforms for pass 1 (generate linked list)
    void setUniformsPassTwo(vvShaderProgram* shader);			///< Uniforms for pass 2 (sort & render fragments of linked list) 
    void render();							///< Render (pass 1, pass2)     


    boost::scoped_ptr<vvShaderFactory> _shaderFactory; 			///< Factory for shader-creation
    boost::scoped_ptr<vvShaderProgram> _shader;   			///< Shader 
    vvShaderProgram* initShader(); 					///< Init shaders, you guessed it   
    GLint g_winWidth, g_winHeight;		 			///< Size of the window							
    GLfloat texSizeX, texSizeY,texSizeZ;				///< Size of the texture
    virvo::gl::Texture g_HeadPtrTex;						///< Handle for 
    virvo::gl::Texture g_volTexObj;					///< Handle for volume
    virvo::gl::Texture g_tffTexObj;					///< Handle for transfer function
    
    GLuint g_vao;
    GLuint g_vaoFace;
    GLuint clearBuf;
    GLuint buffers[2];
    GLuint gbo[2];
    vvShaderProgram* shaderPassOne;
    vvShaderProgram* shaderPassTwo;
    uint8_t *data;
    GLuint maxNodes;    
    virvo::mat4 view_matrix;
    virvo::mat4 proj_matrix;
    virvo::mat4 mvp;
    virvo::mat4 scaleMatrix;
    virvo::mat4 texMat;
    virvo::recti viewport;    
    virvo::aabb bbox;
    GLuint baseIndices[36];
    bool newBuffer;
    GLuint numBoxes;



};

#endif // VVSPARSERENDERER_H