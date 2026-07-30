#ifndef COM_COMBOBOX_H
#define COM_COMBOBOX_H

#include <QComboBox>
#include <conserial.h>
#include <QStringList>

class COM_combobox : public QComboBox {
    Q_OBJECT
public:
    explicit COM_combobox(QWidget* parent = nullptr);
    void showPopup() override;
    void on_COM_ComboBox_activated(int index);
    void setStand(hwe::Conserial* stand);

protected:


private slots:
    void updateItems();

private:
    QStringList getDataFromDatabase();
    hwe::Conserial* stand_ = nullptr;
};

#endif // COM_COMBOBOX_H
