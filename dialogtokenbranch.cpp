#include "dialogtokenbranch.h"
#include "ui_dialogtokenbranch.h"

DialogTokenBranch::DialogTokenBranch(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogTokenBranch)
{
    ui->setupUi(this);
}

DialogTokenBranch::~DialogTokenBranch()
{
    delete ui;
}

void DialogTokenBranch::activeBranch()
{
    ui->lab_contract->setText("nodeurl");
    ui->lab_dec->setText("auth");
}

bool DialogTokenBranch::getBranch(WalletDB::Branch &branch)
{
    bool ret = true;
    branch.auth = ui->lineEdit_dec->text();
    branch.name = ui->lineEdit_name->text();
    branch.url = ui->lineEdit_contract->text();
    return ret;
}

bool DialogTokenBranch::getToken(WalletDB::EthToken &token)
{
    bool ret = true;
    token.contract_address = ui->lineEdit_contract->text();
    token.decimal = ui->lineEdit_dec->text().toInt();
    token.name = ui->lineEdit_name->text();
    return ret;
}

void DialogTokenBranch::on_buttonBox_accepted()
{

}
