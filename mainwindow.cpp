#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QPixmap>
#include <QBuffer>
#include <QDebug>
#include <QTemporaryFile>

#include <fileref.h>
#include <tag.h>
#include <id3v2tag.h>
#include <attachedpictureframe.h>
#include <mpegfile.h>
#include <id3v2frame.h>
#include <id3v2header.h>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    qDebug() << "MainWindow constructor called";
    ui->setupUi(this);

    // Initialize UI elements
    loadButton = ui->loadButton;
    fileLabel = ui->fileLabel;
    coverLabel = ui->coverLabel;
    titleEdit = ui->titleEdit;
    artistEdit = ui->artistEdit;
    albumEdit = ui->albumEdit;
    yearEdit = ui->yearEdit;
    genreEdit = ui->genreEdit;
    commentEdit = ui->commentEdit;
    saveButton = ui->saveButton;
    resetButton = ui->resetButton;

    // Configure cover label for proper album cover display
    qDebug() << "Configuring cover label";
    coverLabel->setMinimumSize(200, 200); // Set minimum size for cover
    coverLabel->setMaximumSize(200, 200); // Set maximum size for cover
    coverLabel->setAlignment(Qt::AlignCenter);
    coverLabel->setStyleSheet("background-color: #f0f0f0; border: 1px solid #ccc;");
    coverLabel->setScaledContents(false); // We handle scaling manually
    qDebug() << "Cover label configured with size:" << coverLabel->size();

    // Setup connections
    setupUIConnections();

    // Initialize UI state
    clearTags();
    qDebug() << "MainWindow initialized";
}

MainWindow::~MainWindow()
{
    qDebug() << "MainWindow destructor called";
    delete ui;
    qDebug() << "MainWindow destroyed";
}

void MainWindow::setupUIConnections()
{
    qDebug() << "Setting up UI connections";
    connect(loadButton, &QPushButton::clicked, this, &MainWindow::on_loadButton_clicked);
    connect(saveButton, &QPushButton::clicked, this, &MainWindow::on_saveButton_clicked);
    connect(resetButton, &QPushButton::clicked, this, &MainWindow::on_resetButton_clicked);
    qDebug() << "UI connections set up";
}

void MainWindow::on_loadButton_clicked()
{
    QString filePath = QFileDialog::getOpenFileName(this, "Open MP3 File", "",
                                                    "MP3 Files (*.mp3);;All Files (*)");

    if (!filePath.isEmpty()) {
        qDebug() << "Loading file:" << filePath;
        if (loadMp3File(filePath)) {
            fileLabel->setText(QFileInfo(filePath).fileName());
            saveButton->setEnabled(true);
            resetButton->setEnabled(true);
            qDebug() << "File loaded successfully";
        } else {
            QMessageBox::warning(this, "Error", "Failed to load MP3 file");
            qDebug() << "Failed to load file";
        }
    } else {
        qDebug() << "No file selected";
    }
}

void MainWindow::on_saveButton_clicked()
{
    qDebug() << "Saving tags...";
    if (writeMp3Tags()) {
        QMessageBox::information(this, "Success", "Tags saved successfully");
        // Update original values
        originalTitle = titleEdit->text();
        originalArtist = artistEdit->text();
        originalAlbum = albumEdit->text();
        originalYear = yearEdit->text();
        originalGenre = genreEdit->text();
        originalComment = commentEdit->text();
        qDebug() << "Tags saved successfully";
    } else {
        QMessageBox::warning(this, "Error", "Failed to save tags");
        qDebug() << "Failed to save tags";
    }
}

void MainWindow::on_resetButton_clicked()
{
    qDebug() << "Resetting tags to original values";
    updateUIWithTags();
    qDebug() << "Tags reset to original values";
}

bool MainWindow::loadMp3File(const QString &filePath)
{
    currentFilePath = filePath;

    try {
        // Check if file exists and is readable
        QFile file(currentFilePath);
        if (!file.exists()) {
            qDebug() << "File does not exist:" << currentFilePath;
            QMessageBox::warning(this, "Error", "The selected file does not exist.");
            return false;
        }

        if (!file.open(QIODevice::ReadOnly)) {
            qDebug() << "File cannot be opened for reading:" << currentFilePath;
            QMessageBox::warning(this, "Error", "The selected file cannot be opened. It may be locked or you don't have permission.");
            return false;
        }
        file.close();

        // Get absolute file path to avoid relative path issues
        QFileInfo fileInfo(currentFilePath);
        QString absolutePath = fileInfo.absoluteFilePath();
        qDebug() << "Using absolute path:" << absolutePath;

        // Convert file path to UTF-8 and handle spaces/special characters
        QByteArray filePathBytes = absolutePath.toUtf8();
        qDebug() << "Attempting to open file:" << filePathBytes;

        // Try different path formats for TagLib
        // First try with the absolute path directly
        qDebug() << "Trying to open file with absolute path";
        TagLib::FileRef fileRef(filePathBytes.constData());

        if (fileRef.isNull() || !fileRef.file()) {
            qDebug() << "FileRef is null with absolute path, trying with quotes";

            // Try with quotes around the path (for paths with spaces)
            QString quotedPath = "\"" + absolutePath + "\"";
            QByteArray quotedPathBytes = quotedPath.toUtf8();
            fileRef = TagLib::FileRef(quotedPathBytes.constData());

            if (fileRef.isNull() || !fileRef.file()) {
                qDebug() << "FileRef is null with quoted path, trying native path";

                // Try using native file path
                fileRef = TagLib::FileRef(absolutePath.toLocal8Bit().constData());

                if (fileRef.isNull() || !fileRef.file()) {
                    qDebug() << "FileRef is null with all path formats - file cannot be opened by TagLib";

                    // Check if file is actually an MP3 file
                    if (!currentFilePath.endsWith(".mp3", Qt::CaseInsensitive)) {
                        QMessageBox::warning(this, "Error", "The selected file is not an MP3 file.");
                    } else {
                        QMessageBox::warning(this, "Error", "Failed to load MP3 file. The file may be corrupted, not a valid MP3 file, or inaccessible.");
                    }
                    return false;
                }
            }
        }

        qDebug() << "FileRef successfully created";

        // Read tags using the already opened FileRef
        readMp3Tags(fileRef);
        updateUIWithTags();

        // Check if all tags are empty
        if (originalTitle.isEmpty() && originalArtist.isEmpty() && originalAlbum.isEmpty() &&
            originalYear.isEmpty() && originalGenre.isEmpty() && originalComment.isEmpty()) {
            QMessageBox::information(this, "No Tags", "The MP3 file was loaded successfully, but it contains no tag information.\n\nYou can add tag information and save it.");
        } else {
            QMessageBox::information(this, "Success", "The MP3 file was loaded successfully.\n\nTag information is displayed below.");
        }

        return true;
    } catch (const std::exception &e) {
        qDebug() << "Error loading MP3 file:" << e.what();
        QMessageBox::warning(this, "Error", "Failed to load MP3 file. Please check the file and try again.");
        return false;
    }
}

void MainWindow::readMp3Tags(TagLib::FileRef &fileRef)
{
    try {
        qDebug() << "Reading tags from already opened FileRef";

        if (fileRef.isNull() || !fileRef.tag()) {
            qDebug() << "FileRef is null or tag is null in readMp3Tags";
            clearTags();
            return;
        }

        TagLib::Tag *tag = fileRef.tag();
        qDebug() << "Tag object created successfully";

        // Convert TagLib strings to QString using proper UTF-8 conversion
        // TagLib::String has to8Bit() method that returns std::string
        originalTitle = QString::fromUtf8(tag->title().to8Bit(true));
        originalArtist = QString::fromUtf8(tag->artist().to8Bit(true));
        originalAlbum = QString::fromUtf8(tag->album().to8Bit(true));
        originalYear = QString::number(tag->year());
        originalGenre = QString::fromUtf8(tag->genre().to8Bit(true));
        originalComment = QString::fromUtf8(tag->comment().to8Bit(true));

        // Debug output to see what tags are read
        qDebug() << "Read tags - Title:" << originalTitle
                 << "Artist:" << originalArtist
                 << "Album:" << originalAlbum
                 << "Year:" << originalYear
                 << "Genre:" << originalGenre
                 << "Comment:" << originalComment;

        // Try to read album cover using the already opened FileRef
        try {
            qDebug() << "Attempting to read album cover using existing FileRef";

            // Check if the FileRef has ID3v2 tag with album cover
            TagLib::MPEG::File *mpegFile = dynamic_cast<TagLib::MPEG::File*>(fileRef.file());
            if (mpegFile && mpegFile->ID3v2Tag()) {
                qDebug() << "FileRef has ID3v2 tag, looking for album cover";
                TagLib::ID3v2::Tag *id3v2Tag = mpegFile->ID3v2Tag();
                TagLib::ID3v2::FrameList frameList = id3v2Tag->frameList("APIC");

                if (!frameList.isEmpty()) {
                    qDebug() << "Found" << frameList.size() << "APIC frames";

                    // Try each frame until we find a valid image
                    for (TagLib::ID3v2::FrameList::Iterator it = frameList.begin(); it != frameList.end(); ++it) {
                        TagLib::ID3v2::AttachedPictureFrame *pictureFrame =
                            static_cast<TagLib::ID3v2::AttachedPictureFrame*>(*it);

                        if (pictureFrame) {
                            qDebug() << "Processing picture frame, size:" << pictureFrame->picture().size();

                            if (pictureFrame->picture().size() > 0) {
                                QByteArray imageData(pictureFrame->picture().data(), pictureFrame->picture().size());
                                qDebug() << "Image data size:" << imageData.size() << "bytes";

                                // Try to determine image format from MIME type
                                QString mimeType = QString::fromLatin1(pictureFrame->mimeType().toCString());
                                qDebug() << "MIME type:" << mimeType;

                                QPixmap pixmap;
                                if (pixmap.loadFromData(imageData)) {
                                    qDebug() << "Successfully loaded image from data";
                                    coverLabel->setPixmap(pixmap.scaled(200, 200, Qt::KeepAspectRatio, Qt::SmoothTransformation));
                                    coverLabel->setAlignment(Qt::AlignCenter);
                                    qDebug() << "Album cover loaded and displayed successfully";
                                    break; // Stop after first successful image
                                } else {
                                    qDebug() << "Failed to load image from data, trying alternative approach";

                                    // Try saving to temporary file and loading from there
                                    QTemporaryFile tempFile;
                                    if (tempFile.open()) {
                                        tempFile.write(imageData);
                                        tempFile.close();

                                        if (pixmap.load(tempFile.fileName())) {
                                            qDebug() << "Successfully loaded image from temporary file";
                                            coverLabel->setPixmap(pixmap.scaled(200, 200, Qt::KeepAspectRatio, Qt::SmoothTransformation));
                                            coverLabel->setAlignment(Qt::AlignCenter);
                                            qDebug() << "Album cover loaded from temp file and displayed";
                                            break;
                                        } else {
                                            qDebug() << "Failed to load image from temporary file";
                                        }
                                    }
                                }
                            } else {
                                qDebug() << "Picture frame has zero size";
                            }
                        }
                    }
                } else {
                    qDebug() << "No APIC frames found in ID3v2 tag";
                }
            } else {
                qDebug() << "FileRef has no ID3v2 tag, cannot read album cover";
            }
        } catch (const std::exception &e) {
            qDebug() << "Error reading album cover:" << e.what();
            // Don't clear tags if we can't read the cover
        }

    } catch (const std::exception &e) {
        qDebug() << "Error reading MP3 tags:" << e.what();
        clearTags();
    }
}

bool MainWindow::writeMp3Tags()
{
    try {
        qDebug() << "Writing tags to file:" << currentFilePath;

        // Convert file path to UTF-8 and keep it alive during FileRef usage
        QByteArray filePathBytes = currentFilePath.toUtf8();
        TagLib::FileRef fileRef(filePathBytes.constData());

        if (fileRef.isNull() || !fileRef.tag()) {
            qDebug() << "FileRef is null or tag is null when saving";
            return false;
        }

        TagLib::Tag *tag = fileRef.tag();
        qDebug() << "Tag object created successfully for saving";

        // Debug output for what we're saving
        qDebug() << "Saving tags - Title:" << titleEdit->text()
                 << "Artist:" << artistEdit->text()
                 << "Album:" << albumEdit->text()
                 << "Year:" << yearEdit->text()
                 << "Genre:" << genreEdit->text()
                 << "Comment:" << commentEdit->text();

        // Set tag values using proper UTF-8 conversion
        // Convert QString to TagLib::String using UTF-8
        tag->setTitle(TagLib::String(titleEdit->text().toUtf8().constData(), TagLib::String::UTF8));
        tag->setArtist(TagLib::String(artistEdit->text().toUtf8().constData(), TagLib::String::UTF8));
        tag->setAlbum(TagLib::String(albumEdit->text().toUtf8().constData(), TagLib::String::UTF8));
        tag->setYear(yearEdit->text().toInt());
        tag->setGenre(TagLib::String(genreEdit->text().toUtf8().constData(), TagLib::String::UTF8));
        tag->setComment(TagLib::String(commentEdit->text().toUtf8().constData(), TagLib::String::UTF8));

        // Save changes
        bool success = fileRef.save();
        qDebug() << "Tags save result:" << success;
        return success;

    } catch (const std::exception &e) {
        qDebug() << "Error writing MP3 tags:" << e.what();
        QMessageBox::warning(this, "Error", "Failed to save tags. Please check the file and try again.");
        return false;
    }
}

void MainWindow::updateUIWithTags()
{
    // Debug output to see what values are being set
    qDebug() << "Updating UI with tags - Title:" << originalTitle
             << "Artist:" << originalArtist
             << "Album:" << originalAlbum
             << "Year:" << originalYear
             << "Genre:" << originalGenre
             << "Comment:" << originalComment;

    try {
        // Set text for each field, even if empty
        titleEdit->setText(originalTitle.isEmpty() ? "" : originalTitle);
        artistEdit->setText(originalArtist.isEmpty() ? "" : originalArtist);
        albumEdit->setText(originalAlbum.isEmpty() ? "" : originalAlbum);
        yearEdit->setText(originalYear.isEmpty() ? "" : originalYear);
        genreEdit->setText(originalGenre.isEmpty() ? "" : originalGenre);
        commentEdit->setText(originalComment.isEmpty() ? "" : originalComment);

        // Debug output after setting
        qDebug() << "UI updated - Title:" << titleEdit->text()
                 << "Artist:" << artistEdit->text()
                 << "Album:" << albumEdit->text()
                 << "Year:" << yearEdit->text()
                 << "Genre:" << genreEdit->text()
                 << "Comment:" << commentEdit->text();
    } catch (const std::exception &e) {
        qDebug() << "Error updating UI:" << e.what();
    }
}

void MainWindow::clearTags()
{
    qDebug() << "Clearing all tags and UI";
    currentFilePath.clear();
    originalTitle = "";
    originalArtist = "";
    originalAlbum = "";
    originalYear = "";
    originalGenre = "";
    originalComment = "";

    fileLabel->setText("No file loaded");
    coverLabel->setPixmap(QPixmap());
    coverLabel->setText("Album Cover");

    titleEdit->clear();
    artistEdit->clear();
    albumEdit->clear();
    yearEdit->clear();
    genreEdit->clear();
    commentEdit->clear();

    saveButton->setEnabled(false);
    resetButton->setEnabled(false);
    qDebug() << "Tags and UI cleared";
}
