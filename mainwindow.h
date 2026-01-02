#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QSlider>
#include <QTimer>
#include <QTreeView>
#include <QFileSystemModel>
#include <QAction>
#include <QMenu>
#include <QToolBar>
#include <QTabWidget>
#include <QGroupBox>
#include <QFrame>
#include <QSplitter>
#include <QMessageBox>
#include <QFileDialog>
#include <QStandardPaths>
#include <QMediaPlayer>

// Forward declaration for TagLib
namespace TagLib {
    class FileRef;
}

class MediaPlayer;

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_actionOpen_triggered();
    void on_actionSave_triggered();
    void on_actionRemove_triggered();
    void on_actionExit_triggered();
    void on_actionAbout_triggered();
    void on_actionUndo_triggered();
    void on_actionRedo_triggered();

    void on_fileTreeView_doubleClicked(const QModelIndex &index);
    void on_titleEdit_textChanged(const QString &text);
    void on_artistEdit_textChanged(const QString &text);
    void on_albumEdit_textChanged(const QString &text);
    void on_yearEdit_textChanged(const QString &text);
    void on_genreEdit_textChanged(const QString &text);
    void on_commentEdit_textChanged(const QString &text);
    void on_trackEdit_textChanged(const QString &text);
    void on_discEdit_textChanged(const QString &text);
    void on_composerEdit_textChanged(const QString &text);
    void on_albumArtistEdit_textChanged(const QString &text);

    // Player control slots
    void on_playButton_clicked();
    void on_pauseButton_clicked();
    void on_stopButton_clicked();
    void on_volumeSlider_valueChanged(int value);
    void handle_playbackSlider_moved(int value);
    void on_changeCoverButton_clicked();

    // Media player signal handlers
    void handle_mediaPlayer_stateChanged(QMediaPlayer::PlaybackState state);
    void handle_mediaPlayer_positionChanged(qint64 position);
    void handle_mediaPlayer_durationChanged(qint64 duration);
    void handle_mediaPlayer_volumeChanged(int volume);
    void handle_mediaPlayer_errorOccurred(const QString &errorString);

    // Helper methods
    void updatePlayerUI();

private:
    Ui::MainWindow *ui;

    // Current file and tags
    QString currentFilePath;
    QString originalTitle;
    QString originalArtist;
    QString originalAlbum;
    QString originalYear;
    QString originalGenre;
    QString originalComment;
    QString originalTrack;
    QString originalDisc;
    QString originalComposer;
    QString originalAlbumArtist;

    // Undone state for undo/redo
    QString undoneTitle;
    QString undoneArtist;
    QString undoneAlbum;
    QString undoneYear;
    QString undoneGenre;
    QString undoneComment;
    QString undoneTrack;
    QString undoneDisc;
    QString undoneComposer;
    QString undoneAlbumArtist;
    bool undoPerformed;

    // File system model
    QFileSystemModel *fileSystemModel;

    // UI elements
    QTreeView *fileTreeView;
    QLabel *fileCountLabel;
    QLabel *totalSizeLabel;
    QLabel *coverLabel;
    QLineEdit *titleEdit;
    QLineEdit *artistEdit;
    QLineEdit *albumEdit;
    QLineEdit *yearEdit;
    QLineEdit *genreEdit;
    QLineEdit *commentEdit;
    QLineEdit *trackEdit;
    QLineEdit *discEdit;
    QLineEdit *composerEdit;
    QLineEdit *albumArtistEdit;
    QLabel *fileNameValue;
    QLabel *filePathValue;
    QLabel *fileSizeValue;
    QLabel *fileTypeValue;
    QLabel *fileDurationValue;
    QLabel *fileBitrateValue;
    QLabel *fileSampleRateValue;
    QLabel *fileChannelsValue;
    QPushButton *changeCoverButton;

    // Player UI elements
    QPushButton *playButton;
    QPushButton *pauseButton;
    QPushButton *stopButton;
    QSlider *volumeSlider;
    QSlider *playbackSlider;
    QLabel *currentFileValue;
    QLabel *playbackStatusValue;
    QLabel *playbackPositionValue;
    QLabel *playbackDurationValue;
    QLabel *playerCoverLabel;

    // Media player
    MediaPlayer *mediaPlayer;

    // Actions
    QAction *actionOpen;
    QAction *actionSave;
    QAction *actionRemove;
    QAction *actionExit;
    QAction *actionAbout;
    QAction *actionUndo;
    QAction *actionRedo;

    // MP3 tag functions
    bool loadMp3File(const QString &filePath);
    void readMp3Tags(TagLib::FileRef &fileRef);
    bool writeMp3Tags();
    void updateUIWithTags();
    void clearTags();
    void setupUI();
    void setupFileSystemModel();
    void setupConnections();
    void updateFileInfo(const QString &filePath);
    void updateStatusBar(const QString &message);
    void enableSaveActions(bool enable);
    void showAboutDialog();
};

#endif // MAINWINDOW_H
