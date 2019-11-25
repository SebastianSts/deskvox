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
    void initCounter();							///< Init atomic counter
    void initVBO(std::vector<virvo::aabb> boxes, bool newBuffer);	///< Init VBO for the first render pass
    void initVBOFace();							///< Init VBO for the second render pass
    void initVol3DTex();						///< Init 3D volume 
    void initHeadPtrTex(GLuint bfTexWidth, GLuint bfTexHeight);		///< Init 2D texture for headpointers
    void resizeBuffer(uint newWidth, uint newHeight);			///< Resize linked list when size of viewport changes		
    void clearBuffers();						///< Reset nodeIdx 
    void clearCounter();						///< Reset counter
    void setUniformsPassOne(vvShaderProgram* shader);			///< Uniforms for pass 1 (generate linked list)
    void setUniformsPassTwo(vvShaderProgram* shader);			///< Uniforms for pass 2 (sort & render fragments of linked list) 
    void render();							///< Render (pass 1, pass2)     


    boost::scoped_ptr<vvShaderFactory> _shaderFactory; 			///< Factory for shader-creation
    boost::scoped_ptr<vvShaderProgram> _shader;   			///< Shader 
    vvShaderProgram* initShader(); 					///< Init shaders, you guessed it   
    GLint g_winWidth, g_winHeight;		 			///< Size of the window							
    GLfloat texSizeX, texSizeY,texSizeZ;				///< Size of the texture
    virvo::gl::Texture g_HeadPtrTex;					///< Handle for headpointer texture
    virvo::gl::Texture g_volTexObj;					///< Handle for volume
    virvo::gl::Texture g_tffTexObj;					///< Handle for transfer function    
    GLuint g_vao;							///< Handle for vertex array object (used in the first render pass)
    GLuint g_vaoFace;							///< Handle for vertex array object (used in the second render pass)
    GLuint clearBuf;							///< Handle for clear buffer
    GLuint buffers[2];							
    GLuint gbo[2];							
    vvShaderProgram* shaderPassOne;
    vvShaderProgram* shaderPassTwo;
    uint8_t *data;
    GLuint maxNodes;  							///< sets the maximum number of elements for the per-pixel linked lists   
    virvo::mat4 view_matrix;										
    virvo::mat4 proj_matrix;						
    virvo::mat4 scaleMatrix;						
    virvo::mat4 mvp;    						///< ModelViewProjection matrix (used in vertex shader of the first render pass)
    virvo::mat4 invMVP;							///< InversModelViewProjection matrix (used in fragment shader in the second render pass)
    virvo::recti viewport;    
    virvo::aabb bbox;							///< Axis aligned bounding boxes
    GLuint baseIndices[36];						///< Indices for the AABBs	
    bool newBuffer;
    GLuint numBoxes;    
    float scaleX ;
    float scaleY;
    float scaleZ;



};

#endif // VVSPARSERENDERER_H
