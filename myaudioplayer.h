#ifndef MYAUDIOPLAYER_H
#define MYAUDIOPLAYER_H

#include <QObject>
#include <QAudioProbe>
#include <QMediaPlayer>
#include <QAudioBuffer>
#include <QFile>

class MyAudioPlayer : public QObject
{
    Q_OBJECT
    Q_PROPERTY(qint64 position READ position WRITE setPosition)
    Q_PROPERTY(qint64 duration READ duration)
public slots:
    void errorOccured(QMediaPlayer::Error err) {
        QFile f("error.txt");
        f.open(QIODevice::WriteOnly);
        f.write(player->errorString().toUtf8());
        f.close();
    }
public:
    MyAudioPlayer(QObject* parent = nullptr):QObject(parent){
        void(QMediaPlayer::*func)(QMediaPlayer::Error) = &QMediaPlayer::error;
        connect(player,func,this,&MyAudioPlayer::errorOccured);
    }

    Q_INVOKABLE void play() { player->play(); }
    Q_INVOKABLE void stop() { player->stop(); }
    Q_INVOKABLE void pause() { player->pause(); }
    Q_INVOKABLE void setSource(const QUrl& url) {
        if(m_source == url) return;
        emit disProbeSysInput();
        m_source = url;
        player->setMedia(url);
        player->play();
    }
    Q_INVOKABLE bool playing() {
        return player->state() == QMediaPlayer::PlayingState;
    }
    Q_INVOKABLE void setVolume(int v) {
        player->setVolume(v);
    }
    Q_INVOKABLE const QUrl& source() {
        return m_source;
    }
    Q_INVOKABLE qint64 position() {
        return player->position();
    }
    Q_INVOKABLE void setPosition(int v) {
        player->setPosition(v);
    }
    Q_INVOKABLE qint64 duration() {
        return player->duration();
    }
    Q_INVOKABLE void setSourceSysOInput() { //从系统输入获取源(麦克风)
        emit ProbeSysInput();
    }
    QMediaPlayer* mediaPlayer() { return player; }
signals:
    void sourceChanged();
    void positionChanged();
    void ProbeSysInput();    //侦测系统声音输入
    void disProbeSysInput();
    void ProbeSysOutput();   //侦测系统声音输出
    void ProbeMediaPlayer();  //从MediaPlayer获取源
private:
    QMediaPlayer *player = new QMediaPlayer(this);
    QUrl m_source;
};

#endif // MYAUDIOPLAYER_H
