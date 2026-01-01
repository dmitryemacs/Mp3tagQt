#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>

// Forward declaration for TagLib
namespace TagLib {
    class FileRef;
}

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
    void on_loadButton_clicked();
    void on_saveButton_clicked();
    void on_resetButton_clicked();

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

    // UI elements
    QPushButton *loadButton;
    QLabel *fileLabel;
    QLabel *coverLabel;
    QLineEdit *titleEdit;
    QLineEdit *artistEdit;
    QLineEdit *albumEdit;
    QLineEdit *yearEdit;
    QLineEdit *genreEdit;
    QLineEdit *commentEdit;
    QPushButton *saveButton;
    QPushButton *resetButton;

    // MP3 tag functions
    bool loadMp3File(const QString &filePath);
    void readMp3Tags(TagLib::FileRef &fileRef);
    bool writeMp3Tags();
    void updateUIWithTags();
    void clearTags();
    void setupUIConnections();
};
#endif // MAINWINDOW_H
