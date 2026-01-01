#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include <QGroupBox>
#include <QSettings>

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = nullptr);
    ~SettingsDialog();

    QString getSelectedTheme() const;

private slots:
    void onAccept();
    void onReject();

private:
    QComboBox *themeComboBox;
    QPushButton *okButton;
    QPushButton *cancelButton;

    QSettings *settings;
};

#endif // SETTINGSDIALOG_H