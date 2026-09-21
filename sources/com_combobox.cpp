#include "com_combobox.h"
#include <QDebug>
#include <QSettings>

COM_combobox::COM_combobox(QWidget* parent) : QComboBox(parent) {
    connect(this, QOverload<int>::of(&QComboBox::activated),
            this, &COM_combobox::on_COM_ComboBox_activated);
}

void COM_combobox::setStand(hwe::Conserial* stand)
{
    stand_ = stand;
    if (stand_) {
        updateItems();
    }
}

void COM_combobox::showPopup() {
    updateItems();
    QComboBox::showPopup();
}

void COM_combobox::updateItems()
{
    QString previousPort = currentText();

    std::vector<std::string> comPorts = stand_->GetFTDIComPorts();

    QStringList comList;

    for (const auto& port : comPorts) {
        comList << QString::fromStdString(port);
    }

    blockSignals(true);

    clear();
    addItems(comList);

    int index = findText(previousPort);

    if (index >= 0) {
        setCurrentIndex(index);
    }
    else if (count() > 0) {
        setCurrentIndex(0);
    }

    blockSignals(false);

    // Автоматически подключаемся к выбранному порту
    if (currentIndex() >= 0) {
        QString port = currentText();

        qDebug() << "Автоматическое подключение к COM-порту:" << port;

        QSettings settings("./config.ini", QSettings::IniFormat);
        settings.beginGroup("Settings");
        settings.setValue("port", port);
        settings.endGroup();
        settings.sync();

        stand_->SetComPortName(port.toStdString());
    }
}

void COM_combobox::on_COM_ComboBox_activated(int index)
{
    if (index < 0 || index >= count())
        return;

    QString port = itemText(index);

    qDebug() << "Выбран COM-порт:" << port;

    QSettings settings("./config.ini", QSettings::IniFormat);

    settings.beginGroup("Settings");
    settings.setValue("port", port);
    settings.endGroup();
    settings.sync();

    stand_->SetComPortName(port.toStdString());
    emit comPortChanged(port);

}

QString COM_combobox::getComPortName() const
{
    return currentText();
}


