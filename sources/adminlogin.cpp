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
    //проверка версии протокола
    api::versionFirmwareResponse response1;
    response1 = stand_->GetCurrentFirmwareVersion();
    if((response1.major_ == 1 && response1.minor_ == 0 && response1.micro_ == 0)||(response1.major_ == 1 && response1.minor_ == 2 && response1.micro_ == 0) || (response1.major_ == 0 && response1.minor_ == 0 && response1.micro_ == 0)){ //для протокола 1.0 и 1.2
        if(password == "admin"){
            emit firstWindow();
            close();
        }
        else{                 QMessageBox::critical(this,"Ошибка!",
                                  "Неверный пароль!",
                                  QMessageBox::Ok);
            ui ->Password->clear();
        }
    }
    else{
        response = stand_->OpenConfigMode(password.toStdString()); //для протокола 1.5
        if(response.errorCode_==0){
            if(response.adcResponse_== 1){
                emit firstWindow();
                close();
            }
            else{                 QMessageBox::critical(this,"Ошибка!",
                                      "Неверный пароль!",
                                      QMessageBox::Ok);
                ui ->Password->clear();
            }
        }
    }
}

