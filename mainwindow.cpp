#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "mediaplayer.h"
#include "settingsdialog.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QStandardPaths>
#include <QDir>
#include <QFileInfo>
#include <QDateTime>
#include <QMimeDatabase>
#include <QMimeType>
#include <QImageReader>
#include <QBuffer>
#include <QPixmap>
#include <QDebug>

#include "taglib/fileref.h"
#include "taglib/tag.h"
#include "taglib/audioproperties.h"
#include "taglib/mpeg/mpegfile.h"
#include "taglib/mpeg/id3v2/id3v2tag.h"
#include "taglib/mpeg/id3v2/frames/attachedpictureframe.h"
#include "taglib/mpeg/id3v2/frames/textidentificationframe.h"
#include "taglib/toolkit/tpropertymap.h"

#include <algorithm>
#include <memory>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , fileSystemModel(new QFileSystemModel(this))
    , mediaPlayer(new MediaPlayer(this))
    , currentFilePath("")
    , undoPerformed(false)
    , settings(new QSettings("Mp3TagQt", "Settings", this))
{
    ui->setupUi(this);
    setupUI();
    setupFileSystemModel();
    setupConnections();

    // Apply saved theme
    QString theme = settings->value("theme", "light").toString();
    applyTheme(theme);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupUI()
{
    // Set up UI elements
    fileTreeView = ui->fileTreeView;
    fileCountLabel = ui->fileCountLabel;
    totalSizeLabel = ui->totalSizeLabel;
    coverLabel = ui->coverLabel;
    titleEdit = ui->titleEdit;
    artistEdit = ui->artistEdit;
    albumEdit = ui->albumEdit;
    yearEdit = ui->yearEdit;
    genreEdit = ui->genreEdit;
    commentEdit = ui->commentEdit;
    trackEdit = ui->trackEdit;
    discEdit = ui->discEdit;
    composerEdit = ui->composerEdit;
    albumArtistEdit = ui->albumArtistEdit;
    fileNameValue = ui->fileNameValue;
    filePathValue = ui->filePathValue;
    fileSizeValue = ui->fileSizeValue;
    fileTypeValue = ui->fileTypeValue;
    fileDurationValue = ui->fileDurationValue;
    fileBitrateValue = ui->fileBitrateValue;
    fileSampleRateValue = ui->fileSampleRateValue;
    fileChannelsValue = ui->fileChannelsValue;

    // Set up actions from UI
    actionOpen = ui->actionOpen;
    actionSave = ui->actionSave;
    actionRemove = ui->actionRemove;
    actionExit = ui->actionExit;
    actionAbout = ui->actionAbout;
    actionSettings = new QAction(tr("Settings"), this);
    actionUndo = ui->actionUndo;
    actionRedo = ui->actionRedo;

    // Try to load icons if available
    if (QFile::exists(":/icons/open.png")) {
        actionOpen->setIcon(QIcon(":/icons/open.png"));
        actionSave->setIcon(QIcon(":/icons/save.png"));
        actionRemove->setIcon(QIcon(":/icons/remove.png"));
        actionUndo->setIcon(QIcon(":/icons/undo.png"));
        actionRedo->setIcon(QIcon(":/icons/redo.png"));
        actionSettings->setIcon(QIcon(":/icons/settings.png"));
    }

    // Add actions to toolbar
    ui->mainToolBar->addAction(actionOpen);
    ui->mainToolBar->addAction(actionSave);
    ui->mainToolBar->addAction(actionRemove);
    ui->mainToolBar->addAction(actionSettings);
    ui->mainToolBar->addSeparator();
    ui->mainToolBar->addAction(actionUndo);
    ui->mainToolBar->addAction(actionRedo);

    // Set initial state
    actionSave->setEnabled(false);
    actionRemove->setEnabled(false);
    actionUndo->setEnabled(false);
    actionRedo->setEnabled(false);

    // Set up cover label
    coverLabel->setStyleSheet("QLabel { background-color: #f0f0f0; border: 1px solid #ccc; }");
    coverLabel->setAlignment(Qt::AlignCenter);

    // Set up file info labels
    fileNameValue->setText("-");
    filePathValue->setText("-");
    fileSizeValue->setText("-");
    fileTypeValue->setText("-");
    fileDurationValue->setText("-");
    fileBitrateValue->setText("-");
    fileSampleRateValue->setText("-");
    fileChannelsValue->setText("-");

    // Set up player UI elements
    playButton = ui->playButton;
    pauseButton = ui->pauseButton;
    stopButton = ui->stopButton;
    volumeSlider = ui->volumeSlider;
    playbackSlider = ui->playbackSlider;
    currentFileValue = ui->currentFileValue;
    playbackStatusValue = ui->playbackStatusValue;
    playbackPositionValue = ui->playbackPositionValue;
    playbackDurationValue = ui->playbackDurationValue;
    playerCoverLabel = ui->playerCoverLabel;
    changeCoverButton = ui->changeCoverButton;

    // Set up player cover label
    playerCoverLabel->setStyleSheet("QLabel { background-color: #f0f0f0; border: 1px solid #ccc; }");
    playerCoverLabel->setAlignment(Qt::AlignCenter);

    // Set up player UI
    playButton->setEnabled(false);
    pauseButton->setEnabled(false);
    stopButton->setEnabled(false);
    currentFileValue->setText("-");
    playbackStatusValue->setText("Stopped");
    playbackPositionValue->setText("00:00");
    playbackDurationValue->setText("00:00");

    // Set up splitter sizes
    ui->mainSplitter->setStretchFactor(0, 1);
    ui->mainSplitter->setStretchFactor(1, 2);
}

void MainWindow::setupFileSystemModel()
{
    // Set up file system model
    fileSystemModel->setRootPath(QDir::homePath());
    fileSystemModel->setFilter(QDir::AllDirs | QDir::Files | QDir::NoDotAndDotDot);
    fileSystemModel->setNameFilters(QStringList() << "*.mp3" << "*.flac" << "*.ogg" << "*.wma" << "*.m4a");
    fileSystemModel->setNameFilterDisables(false);

    // Set model to tree view
    fileTreeView->setModel(fileSystemModel);
    fileTreeView->setRootIndex(fileSystemModel->index(QDir::homePath()));
    fileTreeView->setColumnHidden(1, true); // Hide size column
    fileTreeView->setColumnHidden(2, true); // Hide type column
    fileTreeView->setColumnHidden(3, true); // Hide date column

    // Set header
    fileSystemModel->setHeaderData(0, Qt::Horizontal, tr("Files"));

    // Set initial file count
    fileCountLabel->setText(tr("Files: 0"));
    totalSizeLabel->setText(tr("Size: 0 KB"));
}

void MainWindow::setupConnections()
{
    // Connect menu actions
    connect(actionExit, &QAction::triggered, this, &MainWindow::on_actionExit_triggered);
    connect(actionAbout, &QAction::triggered, this, &MainWindow::on_actionAbout_triggered);

    // Connect our custom actions
    connect(actionOpen, &QAction::triggered, this, &MainWindow::on_actionOpen_triggered);
    connect(actionSave, &QAction::triggered, this, &MainWindow::on_actionSave_triggered);
    connect(actionRemove, &QAction::triggered, this, &MainWindow::on_actionRemove_triggered);
    connect(actionUndo, &QAction::triggered, this, &MainWindow::on_actionUndo_triggered);
    connect(actionRedo, &QAction::triggered, this, &MainWindow::on_actionRedo_triggered);
    connect(actionSettings, &QAction::triggered, this, &MainWindow::on_actionSettings_triggered);

    // Connect file tree view
    connect(fileTreeView, &QTreeView::doubleClicked, this, &MainWindow::on_fileTreeView_doubleClicked);

    // Connect tag edit fields
    connect(titleEdit, &QLineEdit::textChanged, this, &MainWindow::on_titleEdit_textChanged);
    connect(artistEdit, &QLineEdit::textChanged, this, &MainWindow::on_artistEdit_textChanged);
    connect(albumEdit, &QLineEdit::textChanged, this, &MainWindow::on_albumEdit_textChanged);
    connect(yearEdit, &QLineEdit::textChanged, this, &MainWindow::on_yearEdit_textChanged);
    connect(genreEdit, &QLineEdit::textChanged, this, &MainWindow::on_genreEdit_textChanged);
    connect(commentEdit, &QLineEdit::textChanged, this, &MainWindow::on_commentEdit_textChanged);
    connect(trackEdit, &QLineEdit::textChanged, this, &MainWindow::on_trackEdit_textChanged);
    connect(discEdit, &QLineEdit::textChanged, this, &MainWindow::on_discEdit_textChanged);
    connect(composerEdit, &QLineEdit::textChanged, this, &MainWindow::on_composerEdit_textChanged);
    connect(albumArtistEdit, &QLineEdit::textChanged, this, &MainWindow::on_albumArtistEdit_textChanged);

    // Connect player controls
    connect(playButton, &QPushButton::clicked, this, &MainWindow::on_playButton_clicked);
    connect(pauseButton, &QPushButton::clicked, this, &MainWindow::on_pauseButton_clicked);
    connect(stopButton, &QPushButton::clicked, this, &MainWindow::on_stopButton_clicked);
    connect(volumeSlider, &QSlider::valueChanged, this, &MainWindow::on_volumeSlider_valueChanged);
    connect(playbackSlider, &QSlider::sliderMoved, this, &MainWindow::handle_playbackSlider_moved);
    connect(changeCoverButton, &QPushButton::clicked, this, &MainWindow::on_changeCoverButton_clicked);

    // Connect media player signals
    connect(mediaPlayer, &MediaPlayer::stateChanged, this, &MainWindow::handle_mediaPlayer_stateChanged);
    connect(mediaPlayer, &MediaPlayer::positionChanged, this, &MainWindow::handle_mediaPlayer_positionChanged);
    connect(mediaPlayer, &MediaPlayer::durationChanged, this, &MainWindow::handle_mediaPlayer_durationChanged);
    connect(mediaPlayer, &MediaPlayer::volumeChanged, this, &MainWindow::handle_mediaPlayer_volumeChanged);
    connect(mediaPlayer, &MediaPlayer::errorOccurred, this, &MainWindow::handle_mediaPlayer_errorOccurred);
}

void MainWindow::on_actionOpen_triggered()
{
    QStringList filePaths = QFileDialog::getOpenFileNames(
        this,
        tr("Open Audio Files"),
        QStandardPaths::standardLocations(QStandardPaths::MusicLocation).value(0, QDir::homePath()),
        tr("Audio Files (*.mp3 *.flac *.ogg *.wma *.m4a);;All Files (*)"));

    if (!filePaths.isEmpty()) {
        // For simplicity, just load the first file
        loadMp3File(filePaths.first());
    }
}

void MainWindow::on_actionSave_triggered()
{
    if (currentFilePath.isEmpty()) {
        return;
    }

    if (writeMp3Tags()) {
        updateStatusBar(tr("Tags saved successfully"));
        enableSaveActions(false);
    } else {
        updateStatusBar(tr("Failed to save tags"));
    }
}

void MainWindow::on_actionRemove_triggered()
{
    if (currentFilePath.isEmpty()) {
        return;
    }

    // Clear all tags
    clearTags();
    updateUIWithTags();
    updateStatusBar(tr("Tags cleared"));
    enableSaveActions(true);
}

void MainWindow::on_actionExit_triggered()
{
    close();
}

void MainWindow::on_actionAbout_triggered()
{
    showAboutDialog();
}

void MainWindow::on_actionSettings_triggered()
{
    SettingsDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        QString theme = dialog.getSelectedTheme();
        applyTheme(theme);
    }
}

void MainWindow::on_actionUndo_triggered()
{
    // Simple undo - restore original values
    if (!undoPerformed && !currentFilePath.isEmpty()) {
        // Save current values for redo
        undoneTitle = titleEdit->text();
        undoneArtist = artistEdit->text();
        undoneAlbum = albumEdit->text();
        undoneYear = yearEdit->text();
        undoneGenre = genreEdit->text();
        undoneComment = commentEdit->text();
        undoneTrack = trackEdit->text();
        undoneDisc = discEdit->text();
        undoneComposer = composerEdit->text();
        undoneAlbumArtist = albumArtistEdit->text();

        // Restore to original
        titleEdit->setText(originalTitle);
        artistEdit->setText(originalArtist);
        albumEdit->setText(originalAlbum);
        yearEdit->setText(originalYear);
        genreEdit->setText(originalGenre);
        commentEdit->setText(originalComment);
        trackEdit->setText(originalTrack);
        discEdit->setText(originalDisc);
        composerEdit->setText(originalComposer);
        albumArtistEdit->setText(originalAlbumArtist);

        undoPerformed = true;
        updateStatusBar(tr("Changes undone"));
        actionUndo->setEnabled(false);
        actionRedo->setEnabled(true);
    }
}

void MainWindow::on_actionRedo_triggered()
{
    if (undoPerformed && !currentFilePath.isEmpty()) {
        // Restore to undone values
        titleEdit->setText(undoneTitle);
        artistEdit->setText(undoneArtist);
        albumEdit->setText(undoneAlbum);
        yearEdit->setText(undoneYear);
        genreEdit->setText(undoneGenre);
        commentEdit->setText(undoneComment);
        trackEdit->setText(undoneTrack);
        discEdit->setText(undoneDisc);
        composerEdit->setText(undoneComposer);
        albumArtistEdit->setText(undoneAlbumArtist);

        undoPerformed = false;
        updateStatusBar(tr("Changes redone"));
        actionRedo->setEnabled(false);
        enableSaveActions(true); // Changes are present after redo
    }
}

void MainWindow::on_fileTreeView_doubleClicked(const QModelIndex &index)
{
    if (!fileSystemModel->isDir(index)) {
        QString filePath = fileSystemModel->filePath(index);
        QFileInfo fileInfo(filePath);

        // Check if it's an audio file
        QStringList audioExtensions = {"mp3", "flac", "ogg", "wma", "m4a"};
        if (audioExtensions.contains(fileInfo.suffix().toLower())) {
            loadMp3File(filePath);
        }
    }
}

void MainWindow::on_titleEdit_textChanged(const QString &text)
{
    enableSaveActions(text != originalTitle);
}

void MainWindow::on_artistEdit_textChanged(const QString &text)
{
    enableSaveActions(text != originalArtist);
}

void MainWindow::on_albumEdit_textChanged(const QString &text)
{
    enableSaveActions(text != originalAlbum);
}

void MainWindow::on_yearEdit_textChanged(const QString &text)
{
    enableSaveActions(text != originalYear);
}

void MainWindow::on_genreEdit_textChanged(const QString &text)
{
    enableSaveActions(text != originalGenre);
}

void MainWindow::on_commentEdit_textChanged(const QString &text)
{
    enableSaveActions(text != originalComment);
}

void MainWindow::on_trackEdit_textChanged(const QString &text)
{
    enableSaveActions(text != originalTrack);
}

void MainWindow::on_discEdit_textChanged(const QString &text)
{
    enableSaveActions(text != originalDisc);
}

void MainWindow::on_composerEdit_textChanged(const QString &text)
{
    enableSaveActions(text != originalComposer);
}

void MainWindow::on_albumArtistEdit_textChanged(const QString &text)
{
    enableSaveActions(text != originalAlbumArtist);
}

bool MainWindow::loadMp3File(const QString &filePath)
{
    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists()) {
        QMessageBox::warning(this, tr("Error"), tr("File does not exist"));
        return false;
    }

    // Clear previous data
    clearTags();

    try {
        TagLib::FileRef fileRef(filePath.toUtf8().constData());

        if (fileRef.isNull()) {
            QMessageBox::warning(this, tr("Error"), tr("Could not open file for reading"));
            return false;
        }

        // Read tags
        readMp3Tags(fileRef);

        // Update UI
        currentFilePath = filePath;
        updateUIWithTags();
        updateFileInfo(filePath);
        updatePlayerUI(); // Update player UI when file is loaded

        updateStatusBar(tr("Loaded: %1").arg(fileInfo.fileName()));
        enableSaveActions(false);
        undoPerformed = false;

        return true;
    } catch (const std::exception &e) {
        QMessageBox::warning(this, tr("Error"), tr("Failed to load file: %1").arg(e.what()));
        return false;
    }
}

void MainWindow::readMp3Tags(TagLib::FileRef &fileRef)
{
    TagLib::Tag *tag = fileRef.tag();

    // Store original values
    originalTitle = QString::fromWCharArray(tag->title().toCWString());
    originalArtist = QString::fromWCharArray(tag->artist().toCWString());
    originalAlbum = QString::fromWCharArray(tag->album().toCWString());
    originalYear = QString::number(tag->year());
    originalGenre = QString::fromWCharArray(tag->genre().toCWString());
    originalComment = QString::fromWCharArray(tag->comment().toCWString());

    // Try to get additional tags from ID3v2 if available
    TagLib::MPEG::File *mpegFile = dynamic_cast<TagLib::MPEG::File*>(fileRef.file());
    if (mpegFile && mpegFile->ID3v2Tag()) {
        TagLib::ID3v2::Tag *id3v2Tag = mpegFile->ID3v2Tag();

        // Get track number
        TagLib::ID3v2::FrameList frameList = id3v2Tag->frameList("TRCK");
        if (!frameList.isEmpty()) {
            TagLib::ID3v2::TextIdentificationFrame *frame =
                static_cast<TagLib::ID3v2::TextIdentificationFrame*>(frameList.front());
            originalTrack = QString::fromUtf8(frame->toString().to8Bit(true));
        }

        // Get disc number
        frameList = id3v2Tag->frameList("TPOS");
        if (!frameList.isEmpty()) {
            TagLib::ID3v2::TextIdentificationFrame *frame =
                static_cast<TagLib::ID3v2::TextIdentificationFrame*>(frameList.front());
            originalDisc = QString::fromUtf8(frame->toString().to8Bit(true));
        }

        // Get composer
        frameList = id3v2Tag->frameList("TCOM");
        if (!frameList.isEmpty()) {
            TagLib::ID3v2::TextIdentificationFrame *frame =
                static_cast<TagLib::ID3v2::TextIdentificationFrame*>(frameList.front());
            originalComposer = QString::fromUtf8(frame->toString().to8Bit(true));
        }

        // Get album artist
        frameList = id3v2Tag->frameList("TPE2");
        if (!frameList.isEmpty()) {
            TagLib::ID3v2::TextIdentificationFrame *frame =
                static_cast<TagLib::ID3v2::TextIdentificationFrame*>(frameList.front());
            originalAlbumArtist = QString::fromUtf8(frame->toString().to8Bit(true));
        }

            // Get cover art
            frameList = id3v2Tag->frameList("APIC");
            if (!frameList.isEmpty()) {
                TagLib::ID3v2::AttachedPictureFrame *pictureFrame =
                    static_cast<TagLib::ID3v2::AttachedPictureFrame*>(frameList.front());

                QImage image;
                if (image.loadFromData(reinterpret_cast<const uchar*>(pictureFrame->picture().data()),
                                     pictureFrame->picture().size())) {
                    QPixmap pixmap = QPixmap::fromImage(image);
                    coverLabel->setPixmap(pixmap.scaled(250, 250, Qt::KeepAspectRatio, Qt::SmoothTransformation));
                    playerCoverLabel->setPixmap(pixmap.scaled(250, 250, Qt::KeepAspectRatio, Qt::SmoothTransformation));
                }
            }
    }
}

bool MainWindow::writeMp3Tags()
{
    if (currentFilePath.isEmpty()) {
        return false;
    }

    try {
        TagLib::FileRef fileRef(currentFilePath.toUtf8().constData());

        if (fileRef.isNull()) {
            QMessageBox::warning(this, tr("Error"), tr("Could not open file for writing"));
            return false;
        }

        TagLib::Tag *tag = fileRef.tag();

        // Set standard tags
        tag->setTitle(titleEdit->text().toStdWString());
        tag->setArtist(artistEdit->text().toStdWString());
        tag->setAlbum(albumEdit->text().toStdWString());
        tag->setYear(yearEdit->text().toInt());
        tag->setGenre(genreEdit->text().toStdWString());
        tag->setComment(commentEdit->text().toStdWString());

        // Try to set additional tags in ID3v2 if available
        TagLib::MPEG::File *mpegFile = dynamic_cast<TagLib::MPEG::File*>(fileRef.file());
        if (mpegFile && mpegFile->ID3v2Tag()) {
            TagLib::ID3v2::Tag *id3v2Tag = mpegFile->ID3v2Tag();

            // Set track number
            if (!trackEdit->text().isEmpty()) {
                TagLib::ID3v2::TextIdentificationFrame *frame =
                    new TagLib::ID3v2::TextIdentificationFrame(TagLib::ByteVector("TRCK"), TagLib::String::UTF8);
                frame->setText(trackEdit->text().toUtf8().constData());
                id3v2Tag->addFrame(frame);
            }

            // Set disc number
            if (!discEdit->text().isEmpty()) {
                TagLib::ID3v2::TextIdentificationFrame *frame =
                    new TagLib::ID3v2::TextIdentificationFrame(TagLib::ByteVector("TPOS"), TagLib::String::UTF8);
                frame->setText(discEdit->text().toUtf8().constData());
                id3v2Tag->addFrame(frame);
            }

            // Set composer
            if (!composerEdit->text().isEmpty()) {
                TagLib::ID3v2::TextIdentificationFrame *frame =
                    new TagLib::ID3v2::TextIdentificationFrame(TagLib::ByteVector("TCOM"), TagLib::String::UTF8);
                frame->setText(composerEdit->text().toUtf8().constData());
                id3v2Tag->addFrame(frame);
            }

            // Set album artist
            if (!albumArtistEdit->text().isEmpty()) {
                TagLib::ID3v2::TextIdentificationFrame *frame =
                    new TagLib::ID3v2::TextIdentificationFrame(TagLib::ByteVector("TPE2"), TagLib::String::UTF8);
                frame->setText(albumArtistEdit->text().toUtf8().constData());
                id3v2Tag->addFrame(frame);
            }

        // Save cover image if available
        QPixmap currentPixmap = coverLabel->pixmap();
        if (currentPixmap.isNull()) {
            // Remove existing cover art if any
            TagLib::ID3v2::FrameList frameList = id3v2Tag->frameList("APIC");
            if (!frameList.isEmpty()) {
                id3v2Tag->removeFrame(frameList.front());
            }
        } else {
            // Save the current cover image
            QImage image = currentPixmap.toImage();

            if (!image.isNull()) {
                // Convert QImage to byte array
                QByteArray byteArray;
                QBuffer buffer(&byteArray);
                buffer.open(QIODevice::WriteOnly);
                image.save(&buffer, "JPEG"); // Save as JPEG

                // Create and add the picture frame
                TagLib::ID3v2::AttachedPictureFrame *pictureFrame =
                    new TagLib::ID3v2::AttachedPictureFrame();
                pictureFrame->setPicture(TagLib::ByteVector(byteArray.constData(), byteArray.size()));
                pictureFrame->setType(TagLib::ID3v2::AttachedPictureFrame::FrontCover);
                pictureFrame->setMimeType("image/jpeg");

                // Remove existing cover art first
                TagLib::ID3v2::FrameList frameList = id3v2Tag->frameList("APIC");
                if (!frameList.isEmpty()) {
                    id3v2Tag->removeFrame(frameList.front());
                }

                id3v2Tag->addFrame(pictureFrame);
            }
        }

    }

    // Save changes
    if (!fileRef.save()) {
        QMessageBox::warning(this, tr("Error"), tr("Failed to save tags"));
        return false;
    }

    // Update original values
    originalTitle = titleEdit->text();
    originalArtist = artistEdit->text();
    originalAlbum = albumEdit->text();
    originalYear = yearEdit->text();
    originalGenre = genreEdit->text();
    originalComment = commentEdit->text();
    originalTrack = trackEdit->text();
    originalDisc = discEdit->text();
    originalComposer = composerEdit->text();
    originalAlbumArtist = albumArtistEdit->text();
    undoPerformed = false;

    return true;
    } catch (const std::exception &e) {
        QMessageBox::warning(this, tr("Error"), tr("Failed to save tags: %1").arg(e.what()));
        return false;
    }
}

void MainWindow::updateUIWithTags()
{
    titleEdit->setText(originalTitle);
    artistEdit->setText(originalArtist);
    albumEdit->setText(originalAlbum);
    yearEdit->setText(originalYear);
    genreEdit->setText(originalGenre);
    commentEdit->setText(originalComment);
    trackEdit->setText(originalTrack);
    discEdit->setText(originalDisc);
    composerEdit->setText(originalComposer);
    albumArtistEdit->setText(originalAlbumArtist);
}

void MainWindow::clearTags()
{
    originalTitle.clear();
    originalArtist.clear();
    originalAlbum.clear();
    originalYear.clear();
    originalGenre.clear();
    originalComment.clear();
    originalTrack.clear();
    originalDisc.clear();
    originalComposer.clear();
    originalAlbumArtist.clear();

    titleEdit->clear();
    artistEdit->clear();
    albumEdit->clear();
    yearEdit->clear();
    genreEdit->clear();
    commentEdit->clear();
    trackEdit->clear();
    discEdit->clear();
    composerEdit->clear();
    albumArtistEdit->clear();

    coverLabel->setText("No Cover");
    coverLabel->setPixmap(QPixmap());
    playerCoverLabel->setText("No Cover");
    playerCoverLabel->setPixmap(QPixmap());

    // Reset player UI when tags are cleared
    mediaPlayer->stop();
    updatePlayerUI();
}

void MainWindow::updateFileInfo(const QString &filePath)
{
    QFileInfo fileInfo(filePath);

    fileNameValue->setText(fileInfo.fileName());
    filePathValue->setText(fileInfo.absolutePath());
    fileSizeValue->setText(QString::number(fileInfo.size() / 1024) + " KB");

    // Get file type
    QMimeDatabase mimeDatabase;
    QMimeType mimeType = mimeDatabase.mimeTypeForFile(filePath);
    fileTypeValue->setText(mimeType.name());

    try {
        TagLib::FileRef fileRef(filePath.toUtf8().constData());
        if (!fileRef.isNull()) {
            TagLib::AudioProperties *properties = fileRef.audioProperties();
            if (properties) {
                // Duration
                int duration = properties->lengthInSeconds();
                int minutes = duration / 60;
                int seconds = duration % 60;
                fileDurationValue->setText(QString("%1:%2").arg(minutes, 2, 10, QLatin1Char('0')).arg(seconds, 2, 10, QLatin1Char('0')));

                // Bitrate
                fileBitrateValue->setText(QString::number(properties->bitrate()) + " kbps");

                // Sample rate
                fileSampleRateValue->setText(QString::number(properties->sampleRate()) + " Hz");

                // Channels
                fileChannelsValue->setText(QString::number(properties->channels()));
            }
        }
    } catch (const std::exception &e) {
        qWarning() << "Failed to get audio properties:" << e.what();
    }
}

void MainWindow::updateStatusBar(const QString &message)
{
    statusBar()->showMessage(message, 3000);
}

void MainWindow::enableSaveActions(bool enable)
{
    actionSave->setEnabled(enable);
    actionRemove->setEnabled(enable);
    actionUndo->setEnabled(enable);
    actionRedo->setEnabled(undoPerformed);
}

void MainWindow::showAboutDialog()
{
    QMessageBox::about(this, tr("About Mp3Tag Qt"),
        tr("<h2>Mp3Tag Qt</h2>"
           "<p>A Qt-based MP3 tag editor inspired by the famous Mp3Tag for Windows.</p>"
           "<p>Version: 1.0</p>"
           "<p>Copyright © 2026</p>"
           "<p>Uses TagLib for audio file tagging.</p>"));
}

// Player control slots
void MainWindow::on_playButton_clicked()
{
    if (currentFilePath.isEmpty()) {
        return;
    }

    // Check if file is an audio file
    QStringList audioExtensions = {"mp3", "flac", "ogg", "wma", "m4a"};
    QFileInfo fileInfo(currentFilePath);
    if (!audioExtensions.contains(fileInfo.suffix().toLower())) {
        QMessageBox::warning(this, tr("Error"), tr("Selected file is not a supported audio format"));
        return;
    }

    mediaPlayer->play(currentFilePath);
    updatePlayerUI();
}

void MainWindow::on_pauseButton_clicked()
{
    mediaPlayer->pause();
    updatePlayerUI();
}

void MainWindow::on_stopButton_clicked()
{
    mediaPlayer->stop();
    updatePlayerUI();
}

void MainWindow::on_volumeSlider_valueChanged(int value)
{
    mediaPlayer->setVolume(value);
}

void MainWindow::handle_playbackSlider_moved(int value)
{
    if (mediaPlayer->duration() > 0) {
        // Fix the position calculation to ensure proper seeking
        qint64 position = (value * mediaPlayer->duration()) / 100;
        qDebug() << "Seeking to position:" << position << "ms (slider value:" << value << ")";
        mediaPlayer->setPosition(position);

        // Ensure the player is in playing state after seeking
        if (mediaPlayer->state() == QMediaPlayer::PlayingState) {
            // No need to restart, just ensure position is updated
            qDebug() << "Position after seeking:" << mediaPlayer->position();
        } else if (mediaPlayer->state() == QMediaPlayer::PausedState) {
            // If paused, we still want to update the position
            qDebug() << "Updated position while paused:" << mediaPlayer->position();
        }
    } else {
        qDebug() << "Cannot seek - duration is 0 or invalid";
    }
}

// Media player signal handlers
void MainWindow::handle_mediaPlayer_stateChanged(QMediaPlayer::PlaybackState state)
{
    updatePlayerUI();

    switch (state) {
        case QMediaPlayer::PlayingState:
            playbackStatusValue->setText("Playing");
            break;
        case QMediaPlayer::PausedState:
            playbackStatusValue->setText("Paused");
            break;
        case QMediaPlayer::StoppedState:
            playbackStatusValue->setText("Stopped");
            playbackPositionValue->setText("00:00");
            playbackSlider->setValue(0);
            break;
    }
}

void MainWindow::handle_mediaPlayer_positionChanged(qint64 position)
{
    if (mediaPlayer->duration() > 0) {
        // Update position display
        int seconds = position / 1000;
        int minutes = seconds / 60;
        seconds = seconds % 60;
        playbackPositionValue->setText(QString("%1:%2").arg(minutes, 2, 10, QLatin1Char('0')).arg(seconds, 2, 10, QLatin1Char('0')));

        // Update slider position
        int sliderValue = (position * 100) / mediaPlayer->duration();
        playbackSlider->setValue(sliderValue);
    }
}

void MainWindow::handle_mediaPlayer_durationChanged(qint64 duration)
{
    if (duration > 0) {
        // Update duration display
        int seconds = duration / 1000;
        int minutes = seconds / 60;
        seconds = seconds % 60;
        playbackDurationValue->setText(QString("%1:%2").arg(minutes, 2, 10, QLatin1Char('0')).arg(seconds, 2, 10, QLatin1Char('0')));
    }
}

void MainWindow::handle_mediaPlayer_volumeChanged(int volume)
{
    volumeSlider->setValue(volume);
}

void MainWindow::handle_mediaPlayer_errorOccurred(const QString &errorString)
{
    QMessageBox::warning(this, tr("Playback Error"), errorString);
    playbackStatusValue->setText("Error: " + errorString);
}

// Helper method to update player UI state
void MainWindow::updatePlayerUI()
{
    bool hasFile = !currentFilePath.isEmpty();
    bool isPlaying = mediaPlayer->isPlaying();

    playButton->setEnabled(hasFile && !isPlaying);
    pauseButton->setEnabled(hasFile && isPlaying);
    stopButton->setEnabled(hasFile && (isPlaying || mediaPlayer->state() == QMediaPlayer::PausedState));
    changeCoverButton->setEnabled(hasFile);

    if (hasFile) {
        QFileInfo fileInfo(currentFilePath);
        currentFileValue->setText(fileInfo.fileName());
    } else {
        currentFileValue->setText("-");
    }
}

// Cover change functionality
void MainWindow::on_changeCoverButton_clicked()
{
    if (currentFilePath.isEmpty()) {
        QMessageBox::warning(this, tr("Error"), tr("No file loaded"));
        return;
    }

    // Open file dialog to select an image
    QString imagePath = QFileDialog::getOpenFileName(
        this,
        tr("Select Cover Image"),
        QStandardPaths::standardLocations(QStandardPaths::PicturesLocation).value(0, QDir::homePath()),
        tr("Image Files (*.jpg *.jpeg *.png *.bmp *.gif);;All Files (*)"));

    if (imagePath.isEmpty()) {
        return; // User cancelled
    }

    // Load the selected image
    QImage image(imagePath);
    if (image.isNull()) {
        QMessageBox::warning(this, tr("Error"), tr("Failed to load image file"));
        return;
    }

    // Scale and display the image
    QPixmap pixmap = QPixmap::fromImage(image);
    pixmap = pixmap.scaled(200, 200, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    coverLabel->setPixmap(pixmap);
    playerCoverLabel->setPixmap(pixmap);

    // Mark as modified
    enableSaveActions(true);
    updateStatusBar(tr("Cover image changed"));
}

void MainWindow::applyTheme(const QString &theme)
{
    QPalette palette;

    if (theme == "dark") {
        // Dark theme
        palette.setColor(QPalette::Window, QColor(53, 53, 53));
        palette.setColor(QPalette::WindowText, Qt::white);
        palette.setColor(QPalette::Base, QColor(25, 25, 25));
        palette.setColor(QPalette::AlternateBase, QColor(53, 53, 53));
        palette.setColor(QPalette::ToolTipBase, Qt::white);
        palette.setColor(QPalette::ToolTipText, Qt::white);
        palette.setColor(QPalette::Text, Qt::white);
        palette.setColor(QPalette::Button, QColor(53, 53, 53));
        palette.setColor(QPalette::ButtonText, Qt::white);
        palette.setColor(QPalette::BrightText, Qt::red);
        palette.setColor(QPalette::Link, QColor(42, 130, 218));
        palette.setColor(QPalette::Highlight, QColor(42, 130, 218));
        palette.setColor(QPalette::HighlightedText, Qt::black);
    } else {
        // Light theme (default)
        palette = QApplication::palette(); // Reset to default
    }

    QApplication::setPalette(palette);
    setPaletteRecursive(this, palette);
    this->update();

    // Save theme setting
    settings->setValue("theme", theme);
}

void MainWindow::setPaletteRecursive(QWidget *widget, const QPalette &palette)
{
    widget->setPalette(palette);
    widget->update();

    for (QWidget *child : widget->findChildren<QWidget*>()) {
        setPaletteRecursive(child, palette);
    }
}
