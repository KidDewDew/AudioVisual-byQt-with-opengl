#include "spectrum3d.h"
#include <QJSValueIterator>
#include <qmath.h>

Spectrum3D::Spectrum3D(){}

QQuickFramebufferObject::Renderer* Spectrum3D::createRenderer() const {
    return new Spectrum3D_Renderer;
}

void Spectrum3D_Renderer::drawBar(GLfloat ax,GLfloat az,GLfloat h) {
    mMat.setToIdentity();
    mMat.translate(ax,20-56*h,az);
    mMat.scale(1.0f,h,1.0f);

    QMatrix4x4 mvMat = vMat * mMat;
    invTrMat = mvMat.inverted().transposed(); //mv矩阵的逆转置
    int posLoc = shaderProgram.attributeLocation("position");
    int normLoc = shaderProgram.attributeLocation("vertNormal");
    //int posLoc = 0, normLoc = 1;
    shaderProgram.setUniformValue("mvMat",mvMat);
    shaderProgram.setUniformValue("pMat",pMat);
    shaderProgram.setUniformValue("nMat",invTrMat); //逆转置矩阵
    shaderProgram.setUniformValue("mix_val",std::sqrt(h));
    shaderProgram.setUniformValue("rx",-ax);
    shaderProgram.setUniformValue("ry",-az);

    vBuffer.bind();
    shaderProgram.enableAttributeArray(posLoc); //顶点
    shaderProgram.setAttributeBuffer(posLoc,GL_FLOAT,0,3,0);
    normBuffer.bind();
    shaderProgram.enableAttributeArray(normLoc);  //法线
    shaderProgram.setAttributeBuffer(normLoc,GL_FLOAT,0,3,0);
    idBuffer.bind();   //索引
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_BYTE, 0);
}

void Spectrum3D_Renderer::setLight() {
    auto lightPosV = vMat * QVector4D(lightPos[0],lightPos[1],lightPos[2],1.0);
    shaderProgram.setUniformValue("globalAmbient",globalAmbient);
    shaderProgram.setUniformValue("light.ambient",lightAmbient);
    shaderProgram.setUniformValue("light.diffuse",globalAmbient);
    shaderProgram.setUniformValue("light.specular",lightSpecular);
    shaderProgram.setUniformValue("light.position",lightPosV.toVector3D());
    shaderProgram.setUniformValue("material.ambient",mAmb);
    shaderProgram.setUniformValue("material.diffuse",mDiff);
    shaderProgram.setUniformValue("material.specular",mSpec);
    shaderProgram.setUniformValue("material.shininess",mShin);
}

void Spectrum3D_Renderer::resetLight() {

}

//渲染函数 用opengl绘制
void Spectrum3D_Renderer::render() {

    glClearColor(offOn,offOn,offOn,1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST); //深度测试
    glEnable(GL_CULL_FACE);  //剔除背面
    glDepthMask(true);
    glDepthFunc(GL_LEQUAL);
    glFrontFace(GL_CW);      //顺时针为正面

    vMat.setToIdentity();
    qreal sinv = qSin(hoverAngle/180.0*3.1415927);
    vMat.translate(0.0f,0.0f,-hoverAngle*1.5+20.0f);
    vMat.rotate(-hoverAngle,1.0f,0.0f,0.0f);
    vMat.translate(0.0f,48.0f,-191.04f+sinv*180);
    if(! spining)
        vMat.rotate(-20.0f,0.0f,1.0f,0.0f);
    else {
        vMat.rotate(-angle,0.0f,1.0f,0.0f);
        angle += 0.7;
    }
    vMat.translate(50,0,50);

    shaderProgram.bind();
    shaderProgram.setUniformValue("enableLight",enable_light);
    shaderProgram.setUniformValue("colorMode",colorMode);
    if(enable_light) {
        setLight(); //启用灯光
    }


    if(data.length() > 0) {
        int sqn = std::sqrt(data.length());
        for(int x = 0; x < sqn; ++x) {
            for(int z = 0; z < sqn; ++z) {
                int rx = sqn-x, ry = sqn-z, s = rx+ry;
                int i;
                if(s <= sqn+1)
                    i = (s-1)*(s-2)/2+rx-1;
                else {
                    rx = z+1; ry = x+1;
                    s = rx+ry;
                    i = sqn*sqn-1-((s-1)*(s-2)/2+rx-1);
                }
                if(i >= data.length()) continue;
                drawBar(-x*4.8*20/sqn,-z*4.8*20/sqn,data[i]*18*h_scale);
            }
        }
    }

    vBuffer.release();
    idBuffer.release();
    shaderProgram.release();
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    resetLight();
}


Spectrum3D_Renderer::Spectrum3D_Renderer()
    :vBuffer(QOpenGLBuffer::VertexBuffer),
    idBuffer(QOpenGLBuffer::IndexBuffer),
    normBuffer(QOpenGLBuffer::VertexBuffer)
{
    init();
}

void Spectrum3D_Renderer::init() {  //初始化
    shaderProgram.addShaderFromSourceFile(QOpenGLShader::Vertex,":/shader/vShader.glsl");
    shaderProgram.addShaderFromSourceFile(QOpenGLShader::Fragment,":/shader/fShader.glsl");
    shaderProgram.link();

    pMat.setToIdentity();
    pMat.perspective(45.0f,1.0,0.5f,500.0f);

    const GLfloat length = 2.0f;
    const GLfloat vertices[] =
        {
            length, 0, length,
            length, 0, -length,
            -length, 0, -length,
            -length, 0, length,
            length, 28*length, length,
            length, 28*length, -length,
            -length, 28*length, -length,
            -length, 28*length, length
        };
    const GLfloat normVerts[] =
    {
        1,-0.5,1,
        1,-0.5,-1,
        -1,-0.5,-1,
        -1,-0.5,1,
        1,0.5,1,
        1,0.5,-1
        -1,0.5,-1
        -1,0.5,1
    };
    const GLubyte indices[] =
        {
            0, 1, 2, 0, 2, 3,// 以下
            7, 6, 4, 6, 5, 4,// 上面
            7, 4, 3, 4, 0, 3,// 左面
            5, 6, 1, 6, 2, 1,// 右面
            4, 5, 0, 5, 1, 0,// 前面
            3, 2, 6, 3, 6, 7,// 背面
        };
    vBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw); //每帧都不同
    vBuffer.create();
    vBuffer.bind();
    vBuffer.allocate(vertices,sizeof vertices);
    normBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
    normBuffer.create();
    normBuffer.bind();
    normBuffer.allocate(normVerts, sizeof normVerts);
    idBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw); //每帧都不同
    idBuffer.create();
    idBuffer.bind();
    idBuffer.allocate(indices,sizeof indices);
}
