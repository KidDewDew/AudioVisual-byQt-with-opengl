#ifndef SPECTRUM3D_H
#define SPECTRUM3D_H

#include <QQuickItem>
#include <QQuickFramebufferObject>
#include <QQuickWindow>
#include <QOpenGLFunctions>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>
#include <QOpenGLFramebufferObject>
#include <QQmlListProperty>
#include <QJSValueIterator>

#define MAX_WIDTH 12

class Spectrum3D_Renderer;

class Spectrum3D: public QQuickFramebufferObject
{
    Q_OBJECT
    Q_PROPERTY(QJSValue data WRITE setData NOTIFY dataChanged FINAL)
    Q_PROPERTY(qreal h_scale READ getH_scale WRITE setH_scale FINAL)
    Q_PROPERTY(bool enable_light WRITE setEnableLight FINAL)
    Q_PROPERTY(int colorMode WRITE setColorMode FINAL)
    Q_PROPERTY(qreal hoverAngle WRITE setHoverAngle FINAL)
    friend class Spectrum3D_Renderer;
public:
    Spectrum3D();
    Renderer* createRenderer() const override;
    void setData(QJSValue d) {
        data = d;
    }
    qreal getH_scale() const { return h_scale; }
    void setH_scale(qreal v) { h_scale = v; }
    Q_INVOKABLE void setOffOn(qreal val) {
        offOn = val;
    }
    Q_INVOKABLE void setSpining(bool val) {
        spining = val;
    }
    void setEnableLight(bool e) {
        enable_light = e;
    }
    void setColorMode(int m) {
        colorMode = m;
    }
    void setHoverAngle(qreal a) {
        hoverAngle = a;
    }
signals:
    void dataChanged();
private:
    mutable Spectrum3D_Renderer* renderer=0;
    QJSValue data;
    qreal h_scale = 1.0f;
    qreal offOn = 0.0;
    qreal hoverAngle = 20.0f;  //俯瞰角
    bool spining = false;
    bool enable_light = true;  //启用光照
    int colorMode = 0;         //配色模式
};


class Spectrum3D_Renderer: public QQuickFramebufferObject::Renderer {
public:
    Spectrum3D_Renderer();
    void render() override;
    QOpenGLFramebufferObject *createFramebufferObject(const QSize &size) override {
        QOpenGLFramebufferObjectFormat format;
        format.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);
        format.setSamples(4);   //开启深度缓冲区！！！
        return new QOpenGLFramebufferObject(size, format);
    }
    void synchronize(QQuickFramebufferObject* fbo) override { //主线程-同步
        Spectrum3D *s = dynamic_cast<Spectrum3D*>(fbo);
        if(!s) return;
        h_scale = s->h_scale;
        offOn = s->offOn;
        hoverAngle = s->hoverAngle;
        colorMode = s->colorMode;
        enable_light = s->enable_light;
        spining = s->spining;
        if(data.length() < 400) data.resize(400);
        if(s->data.hasProperty("length")) {
            int N = s->data.property("length").toUInt();   //声道数
            int n = s->data.property(0).property("length").toInt();  //样点数
            int sqn = std::sqrt(n);
            //qDebug() << sqn;
            if(data.length() > sqn*sqn) {
                data.resize(sqn*sqn);
            }
            auto getVal = [&](int i) { //计算第i帧各个声道最大值
                double r = 0;
                if(i >= n) return 0.0;
                for(int j = 0; j < N; ++j) {
                    r = qMax(r,s->data.property(j).property(i).toNumber());
                }
                return r;
            };
            for(int i = 0; i < std::min(400,sqn*sqn); ++i) {
                data[i] = getVal(i);
            }
        }
    }
private:
    void init();
    void setLight();
    void resetLight();
    void drawBar(GLfloat ax,GLfloat ay,GLfloat h); //绘制一个柱形
private:
    bool enable_light = true;  //启用光照
    int colorMode = 0;         //配色模式
    QMatrix4x4             mMat;
    QMatrix4x4             vMat;
    QMatrix4x4             pMat;
    QMatrix4x4             invTrMat;
    QOpenGLBuffer          vBuffer, idBuffer, normBuffer;
    //QOpenGLBuffer          colorBuffer;
    QOpenGLShaderProgram   shaderProgram;  //着色器
    QVector<qreal> data;
    qreal h_scale = 1.0f, offOn = 0.0f, angle = 0.0f, hoverAngle = 20.0f;
    bool spining = false;

    float lightPos[3]{40.0f,27.0f,0.0f}; //光源位置
    //白光
    QVector4D globalAmbient = {0.0f,0.2f,0.2f,1.0f};
    QVector4D lightAmbient = {1.0f, 1.0f, 1.0f, 1.0f};
    QVector4D lightDiffuse = {1.0f,1.0f,1.0f,1.0f};
    QVector4D lightSpecular = {1.0f,1.0f,1.0f,1.0f};
    //黄金材质
    QVector4D mAmb = {0.2f, 0.2f, 0.2f, 1.0f};
    QVector4D mDiff = {0.8f,0.8f,0.8f,1.0f};
    QVector4D mSpec = {0.8f,0.8f,0.8f,1.0f};
    float mShin = 78.0f;
};



#endif // SPECTRUM3D_H
