#include "mediaplayer.h"
#include <QDebug>
#include <QUrl>
#include <QMediaDevices>
#include <QFile>
#include <QAudioDevice>
#include <QTimer>
#include <QFileInfo>
#include <QAudioFormat>
#include <QEventLoop>

MediaPlayer::MediaPlayer(QObject *parent)
    : QObject(parent)
    , m_player(nullptr)
    , m_audioOutput(nullptr)
    , m_audioSink(nullptr)
    , m_mediaDevicesAvailable(false)
{
    qDebug() << "MediaPlayer constructor called";

    // Check available multimedia backends
    qDebug() << "Checking available multimedia backends...";

    // Try to create QMediaPlayer first
    m_player = new QMediaPlayer(this);
    m_audioOutput = new QAudioOutput(this);
    m_player->setAudioOutput(m_audioOutput);
    m_mediaDevicesAvailable = true;

    // Note: In Qt6, QMediaPlayer is always created, but may not have backends available
    // We'll check actual functionality during playback
    qDebug() << "QMediaPlayer created, will check functionality during playback";

    // Get available audio devices
    QList<QAudioDevice> audioDevices = QMediaDevices::audioOutputs();
    qDebug() << "Available audio devices:" << audioDevices.size();

    for (const QAudioDevice &device : audioDevices) {
        qDebug() << "Audio device:" << device.description() << "ID:" << device.id();
    }

    // Set default audio device if available
    if (!audioDevices.isEmpty()) {
        if (m_audioOutput) {
            m_audioOutput->setDevice(audioDevices.first());
        }
        qDebug() << "Using audio device:" << audioDevices.first().description();
    } else {
        qWarning() << "No audio devices found!";
    }

    // Set volume and check if it's applied
    if (m_audioOutput) {
        m_audioOutput->setVolume(0.5); // Default volume 50%
        qDebug() << "Audio output volume set to:" << m_audioOutput->volume();
    }

    // If QMediaPlayer is not available, try to use QAudioSink directly
    if (!m_mediaDevicesAvailable && !audioDevices.isEmpty()) {
        qDebug() << "Creating QAudioSink for direct audio playback";

        // Create a default audio format for the sink
        QAudioFormat format;
        format.setSampleRate(44100);
        format.setChannelCount(2);
        format.setSampleFormat(QAudioFormat::Int16);

        m_audioSink = new QAudioSink(audioDevices.first(), format, this);

        if (m_audioSink) {
            qDebug() << "QAudioSink created successfully";
            m_audioSink->setVolume(0.5);
            qDebug() << "QAudioSink volume set to:" << m_audioSink->volume();
        } else {
            qWarning() << "Failed to create QAudioSink";
        }
    }

    // Connect signals if using QMediaPlayer
    if (m_player) {
        connect(m_player, &QMediaPlayer::mediaStatusChanged,
                this, &MediaPlayer::handleMediaStatusChanged);
        connect(m_player, &QMediaPlayer::positionChanged,
                this, &MediaPlayer::handlePositionChanged);
        connect(m_player, &QMediaPlayer::durationChanged,
                this, &MediaPlayer::handleDurationChanged);
        connect(m_audioOutput, &QAudioOutput::volumeChanged,
                this, &MediaPlayer::handleVolumeChanged);
        connect(m_player, &QMediaPlayer::errorOccurred,
                this, &MediaPlayer::handleError);
        connect(m_player, &QMediaPlayer::playbackStateChanged,
                this, &MediaPlayer::stateChanged);
    }

    qDebug() << "MediaPlayer created successfully";
}

MediaPlayer::~MediaPlayer()
{
    qDebug() << "MediaPlayer destructor called";
    stop();

    if (m_audioSink) {
        delete m_audioSink;
        m_audioSink = nullptr;
    }

    qDebug() << "MediaPlayer destroyed";
}

void MediaPlayer::play(const QString &filePath)
{
    if (filePath.isEmpty()) {
        qWarning() << "Cannot play empty file path";
        emit errorOccurred("Empty file path");
        return;
    }

    m_currentFile = filePath;
    QUrl fileUrl = QUrl::fromLocalFile(filePath);

    qDebug() << "Attempting to play file:" << filePath;
    qDebug() << "File URL:" << fileUrl.toString();
    qDebug() << "File exists:" << QFile::exists(filePath);

    if (!fileUrl.isValid()) {
        qWarning() << "Invalid file URL:" << filePath;
        emit errorOccurred("Invalid file path");
        return;
    }

    if (!QFile::exists(filePath)) {
        qWarning() << "File does not exist:" << filePath;
        emit errorOccurred("File does not exist");
        return;
    }

    // Check if we have QMediaPlayer available
    if (m_mediaDevicesAvailable && m_player) {
        qDebug() << "Using QMediaPlayer for playback";

        // Check if player is already playing
        if (m_player->playbackState() == QMediaPlayer::PlayingState) {
            m_player->stop();
            qDebug() << "Stopped current playback before starting new one";
        }

        // Set media source and start playing
        m_player->setSource(fileUrl);
        qDebug() << "Media source set to:" << fileUrl.toString();

        // Check if audio output is ready
        qDebug() << "Audio output volume:" << m_audioOutput->volume();
        qDebug() << "Audio output device:" << m_audioOutput->device().description();

        // Check if media is valid before playing
        if (m_player->mediaStatus() == QMediaPlayer::InvalidMedia) {
            qWarning() << "Invalid media detected before playback";
            emit errorOccurred("Invalid media file format");
            return;
        }

        m_player->play();
        qDebug() << "Playback started with QMediaPlayer";

        // Add debug timer to check state after playback starts
        QTimer::singleShot(2000, this, [this]() {
            qDebug() << "DEBUG: Player state after 2 seconds:" << m_player->playbackState();
            qDebug() << "DEBUG: Player position:" << m_player->position();
            qDebug() << "DEBUG: Player duration:" << m_player->duration();
            qDebug() << "DEBUG: Player media status:" << m_player->mediaStatus();
            qDebug() << "DEBUG: Player error:" << m_player->error() << m_player->errorString();

            if (m_player->playbackState() == QMediaPlayer::StoppedState && m_player->duration() == 0) {
                qWarning() << "Playback failed - checking possible reasons...";

                // Check file format by extension
                if (!m_currentFile.endsWith(".mp3", Qt::CaseInsensitive) &&
                    !m_currentFile.endsWith(".wav", Qt::CaseInsensitive) &&
                    !m_currentFile.endsWith(".ogg", Qt::CaseInsensitive)) {
                    qWarning() << "File extension not supported:" << QFileInfo(m_currentFile).suffix();
                    emit errorOccurred("Unsupported file format. Only MP3, WAV, and OGG are supported.");
                }

                // Check if file is readable
                QFile testFile(m_currentFile);
                if (!testFile.open(QIODevice::ReadOnly)) {
                    qWarning() << "File cannot be opened for reading";
                    emit errorOccurred("File cannot be opened for reading.");
                } else {
                    testFile.close();
                }
            }
        });
    } else if (m_audioSink) {
        qDebug() << "Using QAudioSink for direct audio playback";

        // Try to play using QAudioSink and QMediaDevices
        // This is a more direct approach that doesn't require multimedia backends

        // For now, we'll just emit that we're trying to play
        // In a real implementation, we would decode the audio file and feed it to QAudioSink
        qDebug() << "QAudioSink playback not fully implemented yet";
        emit errorOccurred("Direct audio playback not implemented. QMediaPlayer backend not available.");
    } else {
        qWarning() << "No audio playback method available";
        emit errorOccurred("No audio playback method available. Please install multimedia codecs.");
    }

    emit currentFileChanged(filePath);
}

void MediaPlayer::pause()
{
    if (m_player && m_player->playbackState() == QMediaPlayer::PlayingState) {
        m_player->pause();
        qDebug() << "Playback paused";
    } else if (m_audioSink) {
        // For QAudioSink, we would need to implement pause functionality
        qDebug() << "Pause not implemented for QAudioSink";
    } else {
        qDebug() << "Cannot pause - no active playback method";
    }
}

void MediaPlayer::stop()
{
    if (m_player) {
        qDebug() << "Stopping playback, current state:" << m_player->playbackState();
        m_player->stop();
    } else if (m_audioSink) {
        // For QAudioSink, we would need to implement stop functionality
        qDebug() << "Stopping QAudioSink playback";
    }

    m_currentFile.clear();
    qDebug() << "Playback stopped";
}

void MediaPlayer::setVolume(int volume)
{
    qDebug() << "Setting volume to:" << volume;
    // Clamp volume between 0 and 100
    volume = qBound(0, volume, 100);

    if (m_audioOutput) {
        m_audioOutput->setVolume(volume / 100.0);
        qDebug() << "Actual audio output volume:" << m_audioOutput->volume();
    } else if (m_audioSink) {
        m_audioSink->setVolume(volume / 100.0);
        qDebug() << "Actual QAudioSink volume:" << m_audioSink->volume();
    }
}

int MediaPlayer::volume() const
{
    if (m_audioOutput) {
        return static_cast<int>(m_audioOutput->volume() * 100);
    } else if (m_audioSink) {
        return static_cast<int>(m_audioSink->volume() * 100);
    }
    return 0;
}

QMediaPlayer::PlaybackState MediaPlayer::state() const
{
    if (m_player) {
        return m_player->playbackState();
    }
    return QMediaPlayer::StoppedState;
}

bool MediaPlayer::isPlaying() const
{
    if (m_player) {
        return m_player->playbackState() == QMediaPlayer::PlayingState;
    }
    return false;
}

qint64 MediaPlayer::position() const
{
    if (m_player) {
        return m_player->position();
    }
    return 0;
}

qint64 MediaPlayer::duration() const
{
    if (m_player) {
        return m_player->duration();
    }
    return 0;
}

void MediaPlayer::setPosition(qint64 position)
{
    if (m_player) {
        qDebug() << "Seeking to position:" << position << "ms";

        // Ensure position is within valid range
        qint64 duration = m_player->duration();
        if (duration > 0) {
            position = qBound(0, position, duration);
            qDebug() << "Clamped position:" << position << "ms (duration:" << duration << "ms)";
        }

        // Check if media is seekable and ready
        if (m_player->mediaStatus() == QMediaPlayer::LoadedMedia ||
            m_player->mediaStatus() == QMediaPlayer::BufferedMedia) {

            // Ensure player is in a state that allows seeking
            QMediaPlayer::PlaybackState currentState = m_player->playbackState();
            bool wasPlaying = (currentState == QMediaPlayer::PlayingState);

            // Pause if currently playing to ensure seek operation completes
            if (wasPlaying) {
                m_player->pause();
                qDebug() << "Paused playback before seeking";
            }

            // Perform the seek operation
            m_player->setPosition(position);

            // Wait briefly for the seek to complete
            QEventLoop loop;
            QTimer::singleShot(100, &loop, &QEventLoop::quit);
            loop.exec();

            // Verify the position was set correctly
            qint64 actualPosition = m_player->position();
            qDebug() << "Actual position after seeking:" << actualPosition << "ms";

            // Restart playback if it was playing before
            if (wasPlaying) {
                m_player->play();
                qDebug() << "Resumed playback after seeking";
            }

            if (qAbs(position - actualPosition) > 100) { // Allow 100ms tolerance
                qWarning() << "Position mismatch - requested:" << position << "actual:" << actualPosition;

                // Try an alternative approach for problematic files
                if (actualPosition == 0 && position > 0) {
                    qDebug() << "Attempting alternative seek method...";
                    m_player->stop();
                    m_player->setPosition(position);
                    if (wasPlaying) {
                        m_player->play();
                    }
                }
            }
        } else {
            qWarning() << "Cannot seek - media not ready. Current status:" << m_player->mediaStatus();
            emit errorOccurred("Media not ready for seeking");
        }
    } else {
        qDebug() << "Seeking not supported for current playback method";
    }
}

QString MediaPlayer::currentFile() const
{
    return m_currentFile;
}

void MediaPlayer::handleMediaStatusChanged(QMediaPlayer::MediaStatus status)
{
    qDebug() << "Media status changed:" << status;

    switch (status) {
        case QMediaPlayer::NoMedia:
            qDebug() << "No media";
            break;
        case QMediaPlayer::LoadingMedia:
            qDebug() << "Loading media...";
            break;
        case QMediaPlayer::LoadedMedia:
            qDebug() << "Media loaded successfully";
            break;
        case QMediaPlayer::StalledMedia:
            qDebug() << "Media stalled (buffering)";
            break;
        case QMediaPlayer::BufferingMedia:
            qDebug() << "Media buffering...";
            break;
        case QMediaPlayer::BufferedMedia:
            qDebug() << "Media buffered and ready to play";
            break;
        case QMediaPlayer::EndOfMedia:
            qDebug() << "End of media reached";
            break;
        case QMediaPlayer::InvalidMedia:
            qWarning() << "Invalid media file";
            emit errorOccurred("Invalid media file");
            break;
    }
}

void MediaPlayer::handlePositionChanged(qint64 position)
{
    emit positionChanged(position);
}

void MediaPlayer::handleDurationChanged(qint64 duration)
{
    qDebug() << "Media duration changed:" << duration << "ms";
    emit durationChanged(duration);
}

void MediaPlayer::handleVolumeChanged(float volume)
{
    qDebug() << "Volume changed to:" << volume;
    emit volumeChanged(static_cast<int>(volume * 100));
}

void MediaPlayer::handleError(QMediaPlayer::Error error, const QString &errorString)
{
    qWarning() << "Media player error:" << error << errorString;

    // More detailed error handling
    switch (error) {
        case QMediaPlayer::NoError:
            qDebug() << "No error";
            break;
        case QMediaPlayer::ResourceError:
            qWarning() << "Resource error - cannot access media resource";
            emit errorOccurred("Cannot access media resource: " + errorString);
            break;
        case QMediaPlayer::FormatError:
            qWarning() << "Format error - unsupported media format";
            emit errorOccurred("Unsupported media format: " + errorString);
            break;
        case QMediaPlayer::NetworkError:
            qWarning() << "Network error - cannot access network resource";
            emit errorOccurred("Network error: " + errorString);
            break;
        case QMediaPlayer::AccessDeniedError:
            qWarning() << "Access denied - no permission to access resource";
            emit errorOccurred("Access denied: " + errorString);
            break;
        default:
            qWarning() << "Unknown error occurred";
            emit errorOccurred("Unknown error: " + errorString);
            break;
    }
}
