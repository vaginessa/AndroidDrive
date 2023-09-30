#include "settingswindow.h"

QSet<SettingsWindow*> SettingsWindow::_instances;

SettingsWindow::SettingsWindow(const AndroidDevice *device):
    _device(device),
    _globalSettingsBox(QObject::tr("Global settings")),
    _autoConnect(QObject::tr("Automatially connect &drive")),
    _openInExplorer(QObject::tr("Open newly connected drives in &Explorer")),
    _hideDotFiles(QObject::tr("&Hide files beginning with a dot")),
    _okButton(QObject::tr("&OK")),
    _cancelButton(QObject::tr("&Cancel")),
    _applyButton(QObject::tr("&Apply"))
{
    SettingsWindow::_instances.insert(this);

    this->setWindowTitle(QObject::tr("AndroidDrive - Settings"));
    this->setWindowIcon(QIcon(":/icon.ico"));

    if(this->_device != nullptr){
        QObject::connect(&this->_driveLetter, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](){this->_applyButton.setEnabled(true);});
        this->_driveLetter.setWhatsThis(QObject::tr("Allows you to select a preferred drive letter for the selected Android device. If the preferred drive letter is unavailable when this device is connected, it will use the next available drive letter in alphabetical order. If you change the drive letter while a drive for this device is connected, you will have to disconnect it and re-connect it again for the changes to take effect."));
        this->_deviceSettingsLayout.addRow(QObject::tr("Drive &letter"), &this->_driveLetter);

        QObject::connect(&this->_autoConnect, &QCheckBox::clicked, this, [this](){this->_applyButton.setEnabled(true);});
        this->_autoConnect.setWhatsThis(QObject::tr("If this checkbox is checked, the selected device will be automatically connected as a drive whenever you plug it into your computer. Otherwise, you will have to connect it manually by going into Devices > Connect drive."));
        this->_deviceSettingsLayout.addRow(&this->_autoConnect);

        this->_deviceSettingsBox.setTitle(QObject::tr("Device settings for %1").arg(device->model()));
        this->_deviceSettingsBox.setWhatsThis(QObject::tr("These settings only affect the selected device."));
        this->_deviceSettingsBox.setLayout(&this->_deviceSettingsLayout);
        this->_layout.addWidget(&this->_deviceSettingsBox, 0, 0, 1, 3);
    }

    QObject::connect(&this->_openInExplorer, &QCheckBox::clicked, this, [this](){this->_applyButton.setEnabled(true);});
    this->_openInExplorer.setWhatsThis(QObject::tr("If this checkbox is checked, whenever AndroidDrive is finished connecting a drive, it will open that drive in Windows Explorer."));
    this->_globalSettingsLayout.addRow(&this->_openInExplorer);

    QObject::connect(&this->_hideDotFiles, &QCheckBox::clicked, this, [this](){this->_applyButton.setEnabled(true);});
    this->_hideDotFiles.setWhatsThis(QObject::tr("If this checkbox is checked, files that begin with a dot will be treated as hidden files, and will only be visible in Windows Explorer if Windows Explorer's \"Show hidden files\" option is activated."));
    this->_globalSettingsLayout.addRow(&this->_hideDotFiles);

    this->_globalSettingsBox.setWhatsThis(QObject::tr("These settings affect all drives connected with AndroidDrive."));
    this->_globalSettingsBox.setLayout(&this->_globalSettingsLayout);
    this->_layout.addWidget(&this->_globalSettingsBox, device != nullptr, 0, 1, 3);

    this->_layout.addWidget(&this->_okButton, 1 + (device != nullptr), 0);
    this->_layout.addWidget(&this->_cancelButton, 1 + (device != nullptr), 1);
    this->_layout.addWidget(&this->_applyButton, 1 + (device != nullptr), 2);

    this->setLayout(&this->_layout);

    Settings() >> this;

    QObject::connect(&this->_okButton, &QPushButton::clicked, this, [this](){
        this->_applyButton.click();
        this->close();
    });
    QObject::connect(&this->_cancelButton, &QPushButton::clicked, this, [this](){
        Settings() >> this;
        this->close();
    });
    QObject::connect(&this->_applyButton, &QPushButton::clicked, this, [this](){
        Settings settings;
        settings << this;
        for(SettingsWindow *settingsWindow: qAsConst(SettingsWindow::_instances)){
            settings >> settingsWindow;
        }
    });
}

SettingsWindow::~SettingsWindow(){
    SettingsWindow::_instances.remove(this);
}

Settings &operator<<(Settings &settings, const SettingsWindow *settingsWindow){
    if(settingsWindow->_device != nullptr){
        settings.setValue(settingsWindow->_device->model() + "_driveLetter", settingsWindow->_driveLetter.currentText().toLatin1().data()[0]);
        settings.setValue(settingsWindow->_device->model() + "_connectAutomatically", settingsWindow->_autoConnect.isChecked());
    }
    settings.setValue("openInExplorer", settingsWindow->_openInExplorer.isChecked());
    settings.setValue("hideDotFiles", settingsWindow->_hideDotFiles.isChecked());
    settingsWindow->_applyButton.setEnabled(false);
    return settings;
}

const Settings &operator>>(const Settings &settings, SettingsWindow *settingsWindow){
    if(settingsWindow->_device != nullptr){
        settingsWindow->_driveLetter.clear();
        for(char letter = 'A'; letter <= 'Z'; letter++){
            settingsWindow->_driveLetter.addItem(letter + QString(":"));
            if(letter == settings.driveLetter(settingsWindow->_device)){
                settingsWindow->_driveLetter.setCurrentIndex(settingsWindow->_driveLetter.count() - 1);
            }
        }
        settingsWindow->_autoConnect.setChecked(settings.autoConnect(settingsWindow->_device));
    }
    settingsWindow->_openInExplorer.setChecked(settings.openInExplorer());
    settingsWindow->_hideDotFiles.setChecked(settings.hideDotFiles());
    settingsWindow->_applyButton.setEnabled(false);
    return settings;
}

Settings::Settings(): QSettings("Gustav Lindberg", "AndroidDrive"){}

char Settings::driveLetter(const AndroidDevice *device) const{
    return this->value(device->model() + "_driveLetter", 'D').toChar().toLatin1();
}

bool Settings::autoConnect(const AndroidDevice *device) const{
    return this->value(device->model() + "_connectAutomatically", true).toBool();
}

bool Settings::openInExplorer() const{
    return this->value("openInExplorer", true).toBool();
}

bool Settings::hideDotFiles() const{
    return this->value("hideDotFiles", true).toBool();
}
