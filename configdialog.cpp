#include "configdialog.h"
#include "ui_configdialog.h"
#include "mainwindow.h"
#include <QHeaderView>
#include "dialogpassword.h"
#include <QFileDialog>
#include <QMessageBox>
#include "hdwallet.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include "json.hpp"
#include "rpc.h"
using json = nlohmann::json;

ConfigDialog::ConfigDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConfigDialog)
{
    ui->setupUi(this);
    model_ = nullptr;
}

ConfigDialog::~ConfigDialog()
{
    delete ui;
}

void ConfigDialog::setUrl(const QString &nodeurl)
{
    ui->lineEdit_url->setText(nodeurl);
}

bool ConfigDialog::getHDInfo(const QString &file, ConfigDialog::HDInfo &hdinfo)
{
    bool ret = false;
    if(file.isEmpty())
        return ret;
    std::ifstream jfile(file.toStdString().c_str());
    if(!jfile)
    {
        jfile.close();
    }
    json json_conf;
    jfile >> json_conf;
    if(!json_conf.is_object())
    {
        jfile.close();
    }
    jfile.close();

    if (json_conf.find("mnemonic") == json_conf.end())
    {
        return ret;
    }
    else
    {
        hdinfo.maskey_mnemonic = json_conf["maskey_mnemonic"].get<std::string>();
        hdinfo.child_code = 1;//json_conf["child_code"].get<int>();
        hdinfo.index = 1;//json_conf["index"].get<int>();
        ret = true;
    }

    return ret;
}

void ConfigDialog::on_btn_show_clicked()
{
    model_ = new QSqlTableModel(ui->tableView);
   // model_->setEditStrategy(QSqlTableModel::OnManualSubmit);
    model_->setTable("account");

    // Set the localized header captions:
     model_->setHeaderData(model_->fieldIndex("address"),
                          Qt::Horizontal, tr("address"));
     model_->setHeaderData(model_->fieldIndex("privkey"),
                          Qt::Horizontal, tr("privkey"));

     if (!model_->select())
     {
         //showError(model_->lastError());
         qWarning("reset model fail");
         return;
     }

     ui->tableView->setModel(model_);
     ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
     ui->tableView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
     ui->tableView->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);

}

void ConfigDialog::on_buttonBox_accepted()
{
    if (model_)
    {
        model_->clear();
        delete  model_;
        model_ = nullptr;
    }
    delete model_;
}

void ConfigDialog::on_btn_import_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(
                this,
                tr("open a wallet file."),
                "./",
                tr("config(*.json);;All files(*.*)"));

    //qWarning(fileName.toStdString().c_str());
    HDInfo hdinfo;
    if (!getHDInfo(fileName, hdinfo))
    {
        QMessageBox::warning(this,"hdimport","open file failed");
        return ;
    }

    DialogPassword pass_dlg;
    pass_dlg.setConfirm(false);
    pass_dlg.exec();
    QString password = pass_dlg.getPassword();
    HDwallet hdwallet;
    hdwallet.importMasterKey(hdinfo.maskey_mnemonic, password.toStdString().c_str());
}

void ConfigDialog::on_btn_generate_clicked()
{
    DialogPassword pass_dlg;
    pass_dlg.exec();
    QDateTime time = QDateTime::currentDateTime();
    int timeT = time.toTime_t();
    if (!pass_dlg.confirmSuccess())
    {
         QMessageBox::warning(this,"password","confirm is not same from password");
    }
    else
    {
        QString password = pass_dlg.getPassword();
        HDwallet hdwallet;
        hdwallet.createMasterKey(password.toStdString());
        HDwallet::Account account;
        hdwallet.getAccount(account, HDwallet::BTC, 1, timeT);

        WalletDB::Account  db_account;
        db_account.address = account.address.c_str();
        db_account.priv_key = account.privkey.c_str();
        db_account.coin = "BTC";
        MainWindow::wallet_.addAccount(db_account);
        bool ret = Rpc::instance().addadress(account.address);
        if (!ret)
        {
            qWarning("rpc error addaddress");
        }

        WalletDB::Balance balance;
        balance.address = account.address.c_str();
        balance.amount = "0";
        balance.coin = "BTC";
        MainWindow::wallet_.addBalance(balance);

        hdwallet.getAccount(account, HDwallet::ETH, 2, timeT);
        db_account.address = account.address.c_str();
        db_account.priv_key = account.privkey.c_str();
        db_account.coin = "ETH";
        MainWindow::wallet_.addAccount(db_account);

        balance.address = account.address.c_str();
        balance.amount = "0";
        balance.coin = "ETH";
        MainWindow::wallet_.addBalance(balance);
    }

}

void ConfigDialog::on_btn_aitd_clicked()
{
    DialogPassword pass_dlg;
    pass_dlg.exec();

    if (!pass_dlg.confirmSuccess())
    {
         QMessageBox::warning(this,"password","confirm is not same from password");
    }

    std::string address, privkey;
    Rpc::instance().walletPropose(address, privkey);
    WalletDB::Account account;
    account.address = address.c_str();
    account.priv_key = privkey.c_str();
    account.coin = "AITD";
    MainWindow::wallet_.addAccount(account);

    WalletDB::Balance balance;
    balance.address = account.address;
    balance.amount = "0";
    balance.coin = "AITD";
    MainWindow::wallet_.addBalance(balance);

}
