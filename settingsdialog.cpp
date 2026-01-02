#include "settingsdialog.h"

SettingsDialog::SettingsDialog(QWidget *parent)
    : QDialog(parent)
    , settings(new QSettings("Mp3TagQt", "Settings", this))
{
    setWindowTitle(tr("Settings"));
    setModal(true);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // Theme group
    QGroupBox *themeGroup = new QGroupBox(tr("Appearance"), this);
    QVBoxLayout *themeLayout = new QVBoxLayout(themeGroup);

    QHBoxLayout *themeRow = new QHBoxLayout();
    QLabel *themeLabel = new QLabel(tr("Theme:"), this);
    themeComboBox = new QComboBox(this);
    themeComboBox->addItem(tr("Light"), "light");
    themeComboBox->addItem(tr("Dark"), "dark");

    // Load current theme
    QString currentTheme = settings->value("theme", "light").toString();
    if (currentTheme == "dark") {
        themeComboBox->setCurrentIndex(1);
    } else {
        themeComboBox->setCurrentIndex(0);
    }

    themeRow->addWidget(themeLabel);
    themeRow->addWidget(themeComboBox);
    themeRow->addStretch();

    themeLayout->addLayout(themeRow);
    mainLayout->addWidget(themeGroup);

    // Buttons
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    okButton = new QPushButton(tr("OK"), this);
    cancelButton = new QPushButton(tr("Cancel"), this);

    buttonLayout->addStretch();
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);

    mainLayout->addLayout(buttonLayout);

    connect(okButton, &QPushButton::clicked, this, &SettingsDialog::onAccept);
    connect(cancelButton, &QPushButton::clicked, this, &SettingsDialog::onReject);
}

SettingsDialog::~SettingsDialog()
{
    delete settings;
}

QString SettingsDialog::getSelectedTheme() const
{
    return themeComboBox->currentData().toString();
}

void SettingsDialog::onAccept()
{
    settings->setValue("theme", getSelectedTheme());
    accept();
}

void SettingsDialog::onReject()
{
    reject();
}