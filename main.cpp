#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QAudioProbe>
#include <QMediaPlayer>
#include <QAudioBuffer>
#include <QAudioInput>
#include <QAudioOutput>
#include <QFont>
#include <QQmlContext>
#include <qmath.h>
#include <QJsonValue>
#include "spectrum3d.h"
#include "myaudioplayer.h"
#include "fftw3.h"
#include "AudioDeal.h"

#define FFTInterval 100
#define VolumeInterval 100

MyAudioPlayer *ap;

#pragma comment(lib,"F:/QT-projects/audio_player/libfftw3-3.lib")

QObject* qmlRoot;

void dealAudioFrame(const QAudioBuffer& buf) {
    static qint64 lastTime1 = 0,lastTime2=0;
    //计算响度
    if(abs(buf.startTime()-lastTime1) >= VolumeInterval) {
        lastTime1 = buf.startTime();
        int channelCount = buf.format().channelCount();
        auto vec_amp = getBufferLevels(buf); //计算各个声道响度
        QVariantList vl;
        for(auto v : vec_amp) vl << v;
        //qDebug() << "响度:" << vec_amp[0];
        QMetaObject::invokeMethod(qmlRoot,"updateVolumeBars",
                                    Q_ARG(QVariant,vl));
    }
    //傅里叶变换求频谱，解码格式audio/pcm
    if(abs(buf.startTime()-lastTime2) >= FFTInterval) {
        //qDebug() << buf.format().codec();
        if(buf.format().codec() != "audio/pcm") return;
        auto spectrums = calcSpectrum(buf);
        QVariantList vl;
        for(auto& s : spectrums) {
            auto vl2 = QVariantList();
            for(auto v : s) vl2 << v;
            vl.push_back(vl2);
        }
        QMetaObject::invokeMethod(qmlRoot,"updateSpectrumBars",
                                  Q_ARG(QVariant,vl));
        lastTime2 = buf.startTime();
    }
}

int main(int argc, char *argv[])
{
    ap = new MyAudioPlayer();
    QGuiApplication app(argc, argv);
    QQmlApplicationEngine engine;
    app.setFont(QFont("黑体"));
    auto probe = new QAudioProbe(&app); //Qt5的探针，Qt6却被删除了，不可理喻！
    probe->setSource(ap->mediaPlayer());

    QAudioFormat format;
    format.setChannelCount(2);
    format.setCodec("audio/pcm");
    format.setByteOrder(QAudioFormat::LittleEndian); //小端序
    format.setSampleRate(8000); //采样率
    format.setSampleSize(8);    //量化值
    format.setSampleType(QAudioFormat::UnSignedInt);

    //QAudioDeviceInfo info()

    QAudioInput audio_input(format,&app);

    QObject::connect(ap,&MyAudioPlayer::ProbeSysInput,[&]{  //麦克风输入模式
        QIODevice *f_audio = audio_input.start();
        QObject::connect(f_audio,&QIODevice::readyRead,[=]{
            static qint64 sTime = 32;
            QAudioBuffer buf(f_audio->readAll(),format,sTime);
            dealAudioFrame(buf);
            sTime += 100;
        });
    });

    QObject::connect(ap,&MyAudioPlayer::disProbeSysInput,[&]{
        audio_input.stop();
    });

    QObject::connect(probe,&QAudioProbe::audioBufferProbed,dealAudioFrame);
    engine.rootContext()->setContextProperty("player",ap);
    qmlRegisterType<Spectrum3D>("Spectrum3D.Charts",1,0,"Spectrum3D");
    engine.load("qrc:/Main.qml");
    qmlRoot = engine.rootObjects().first();
    return app.exec();
}
