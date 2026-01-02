#ifndef MEDIAPLAYER_H
#define MEDIAPLAYER_H

#include <QObject>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QAudioSink>

class MediaPlayer : public QObject
{
    Q_OBJECT

public:
    explicit MediaPlayer(QObject *parent = nullptr);
    ~MediaPlayer();

    // Basic playback controls
    void play(const QString &filePath);
    void pause();
    void stop();
    void setVolume(int volume);
    int volume() const;

    // Playback state
    QMediaPlayer::PlaybackState state() const;
    bool isPlaying() const;

    // Position and duration
    qint64 position() const;
    qint64 duration() const;
    void setPosition(qint64 position);

    // Current file
    QString currentFile() const;

signals:
    void stateChanged(QMediaPlayer::PlaybackState state);
    void positionChanged(qint64 position);
    void durationChanged(qint64 duration);
    void volumeChanged(int volume);
    void errorOccurred(const QString &errorString);
    void currentFileChanged(const QString &filePath);

private slots:
    void handleMediaStatusChanged(QMediaPlayer::MediaStatus status);
    void handlePositionChanged(qint64 position);
    void handleDurationChanged(qint64 duration);
    void handleVolumeChanged(float volume);
    void handleError(QMediaPlayer::Error error, const QString &errorString);

private:
    QMediaPlayer *m_player;
    QAudioOutput *m_audioOutput;
    QAudioSink *m_audioSink;
    QString m_currentFile;
    bool m_mediaDevicesAvailable;
};

#endif // MEDIAPLAYER_H
