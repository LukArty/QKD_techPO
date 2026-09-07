#include <adminlogin.h>
#include <ui_adminlogin.h>
#include <QMessageBox>

Adminlogin::Adminlogin(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Adminlogin)
{
    ui->setupUi(this);
    connect(ui->Password, SIGNAL(returnPressed()), this, SLOT(on_Login_clicked()));
}

void Adminlogin::Adminlogin_init (hwe::Conserial *stand){
    stand_ = stand;
}

Adminlogin::~Adminlogin()
{
    delete ui;
}

void Adminlogin::on_Login_clicked()
{
    api::AdcResponse response;
    QString password = ui ->Password-> text();
    response = stand_->OpenConfigMode(password.toStdString());
    if(response.errorCode_==0 && response.adcResponse_== 1){
        emit firstWindow();
        ui->Password->clear();
        close();
    }
    else if (response.errorCode_==5 || response.adcResponse_== 0){
        QMessageBox::critical(this,"Ошибка!",
                              "Неверный пароль!",
                              QMessageBox::Ok);
        ui ->Password->clear();
    }
    else {
        QMessageBox::critical(this,"Ошибка!",
                              "Код ошибки: ",
                              QMessageBox::Ok);
        ui ->Password->clear();
    }
}

