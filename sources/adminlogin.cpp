#include <adminlogin.h>
#include <ui_adminlogin.h>
#include <QMessageBox>
#include <QTimer>

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
        QMessageBox::critical(this, "Ошибка!",
                            "Неверный пароль!\nКод ошибки: " +
                            QString::number(response.errorCode_),
                            QMessageBox::Ok);
        ui ->Password->clear();
    }
    else {
        QMessageBox::critical(this, "Ошибка!",
                            "Код ошибки: " + QString::number(response.errorCode_),
                            QMessageBox::Ok);
        ui ->Password->clear();
    }
    //emit firstWindow();
}


void Adminlogin::on_showPasswordButton_clicked()
{
    ui->Password->setEchoMode(QLineEdit::Normal);
    ui->showPasswordButton->setText("🙉");
    QTimer::singleShot(1000, this, [this]()
    {
        ui->Password->setEchoMode(QLineEdit::Password);
        ui->showPasswordButton->setText("🙈");
    });
}

