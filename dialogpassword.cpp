#include "dialogpassword.h"
#include "ui_dialogpassword.h"
#include <QMessageBox>

DialogPassword::DialogPassword(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogPassword)
{
    ui->setupUi(this);
    ui->lineEdit_password->setEchoMode(QLineEdit::Password);
}

DialogPassword::~DialogPassword()
{
    delete ui;
}

void DialogPassword::setConfirm(bool need_confirm)
{
    need_confirm_ = need_confirm;
    if(need_confirm)
    {
        ui->lineEdit_password->setHidden(false);
        ui->label_confirm->setHidden(false);
    }
    else
    {
        ui->lineEdit_confirm->setHidden(true);
        ui->label_confirm->setHidden(true);
    }
}

void DialogPassword::on_buttonBox_accepted()
{
   if(need_confirm_)
   {
       QString password = ui->lineEdit_password->text();
       QString confirm = ui->lineEdit_confirm->text();

       if (confirm == password)
       {
           password_ = ui->lineEdit_password->text();
           ui->lineEdit_password->clear();
           ui->lineEdit_confirm->clear();
           confirm_success_ = true;
       }
       else
       {
           QMessageBox::warning(this,"Password","password is different from confirm");
           confirm_success_ = false;
       }
   }
   else
   {
        password_ = ui->lineEdit_password->text();
        ui->lineEdit_password->clear();
   }
}
