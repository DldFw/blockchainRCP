#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "coinview.h"
#include <QFileDialog>
#include "json.hpp"
#include "walletdb.h"
#include <QMessageBox>
using json = nlohmann::json;
#include <sstream>
#include <fstream>
#include "rpc.h"
#include "dialogtokenbranch.h"

WalletDB MainWindow::wallet_;
static std::map<size_t, WalletDB::EthToken> s_map_index_ethtoken;
static std::map<size_t, WalletDB::Branch> s_map_index_branch;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    open_wallet_ = false;
    ui->lineEdit_btc_amount->setValidator(new QDoubleValidator (0,100, 100));
    ui->lineEdit_eth_amount->setValidator(new QDoubleValidator (0,1000000000, 100));
    ui->lineEdit_btc_fee->setValidator(new QDoubleValidator (0,100, 100));
    ui->lineEdit_eth_from->setValidator(new QDoubleValidator (0,1000000000, 100));
    ui->lineEdit_aitd_fee->setValidator(new QDoubleValidator (0,10, 100));
    ui->lineEdit_aitd_amount->setValidator(new QDoubleValidator (0,100000000000, 100));
    ui->lineEdit_aitd_tag->setText("123456");
    ui->lineEdit_aitd_fee->setText("0.001");
    QObject::connect(&view_balance_, SIGNAL(sendAddress(const QString&, const QString&)), this, SLOT(updateAddress(const QString&, const QString&)));
}

MainWindow::~MainWindow()
{
    wallet_.close();
    delete ui;
}

void MainWindow::updateAddress(const QString &address, const QString &coin)
{
   if (coin == "BTC")
   {
       ui->lineEdit_btc_from->setText(address);
   }
   else if(coin == "ETH")
   {
       ui->lineEdit_eth_from->setText(address);
   }
   else if(coin == "AITD")
   {
       ui->lineEdit_aitd_from->setText(address);
   }
}

void MainWindow::on_btn_eth_submit_clicked()
{
    if (!open_wallet_)
    {
        QMessageBox::warning(this,"wallet","please open config file");
        return;
    }

    if (lock_)
    {
        pass_dlg_.setConfirm(false);
        pass_dlg_.exec();
        QString password = pass_dlg_.getPassword();
        if (password_ != password)
        {
            QMessageBox::warning(this,"wallet","error password");
            return;
        }
    }

    QString from = ui->lineEdit_eth_from->text();
    QString to = ui->lineEdit_eth_to->text();
    QString amount = ui->lineEdit_eth_amount->text();
    QString fee = ui->lineEdit_eth_fee->text();
    QString privkey ;
    bool ret = false;
    double d_amount = std::atof(ui->lineEdit_eth_amount->text().toStdString().c_str());
    double d_fee = std::atof(ui->lineEdit_eth_fee->text().toStdString().c_str());
    QMessageBox::StandardButton question = QMessageBox::question(nullptr,"ETH OR TOKEN tranfers ", std::to_string(d_amount).c_str());
    if (question == QMessageBox::No || question == QMessageBox::Close)
    {
        return;
    }

    if(map_address_privkey_.find(from) != map_address_privkey_.end())
    {
        privkey = map_address_privkey_[from];
    }
    else
    {
        ret = wallet_.getPrivkey(from, privkey);
        if(!ret)
        {
            return;
        }
        map_address_privkey_[from] = privkey;
    }

    HDwallet::Transfer transfer;
    size_t index = ui->comboBox_token->currentIndex();
    transfer.coin_type = HDwallet::ETH;
    transfer.amount = amount.toStdString();
    transfer.fee = fee.toStdString();
    transfer.to = to.toStdString();
    transfer.from = from.toStdString();

    if (index > 0)
    {
        WalletDB::EthToken token = s_map_index_ethtoken[index];
        transfer.is_token = true;
        transfer.contract = token.contract_address.toStdString();
        transfer.decimal = token.decimal;
    }

    std::string txid;
    ret = hd_wallet_.transferCoin(transfer,txid);

    if(ret)
    {
         QMessageBox::warning(this,"txid",txid.c_str());
         qInfo(txid.c_str());
         WalletDB::Tx tx;
         tx.amount = transfer.amount.c_str();
         tx.height = 0;
         tx.to = transfer.to.c_str();
         tx.txid = txid.c_str();
         tx.from = transfer.from.c_str();
         tx.coin = "ETH";
         if (transfer.is_token)
         {
             tx.coin = ui->comboBox_token->currentText();
         }
         wallet_.addTx(tx);
    }
    else
    {
        QMessageBox::warning(this, "error", "transaction fail");
    }

    ui->lineEdit_eth_from->clear();
    ui->lineEdit_eth_to->clear();
    ui->lineEdit_eth_amount->clear();
    ui->lineEdit_eth_fee->clear();
    refreshFee();
}

void MainWindow::on_btn_btc_submit_clicked()
{
    if (!open_wallet_)
    {
        QMessageBox::warning(this,"wallet","please open config file");
        return;
    }
    if (lock_)
    {
        pass_dlg_.setConfirm(false);
        pass_dlg_.exec();
        QString password = pass_dlg_.getPassword();
        if (password_ != password)
        {
            QMessageBox::warning(this,"wallet","error password");
            return;
        }
    }

    HDwallet::Transfer transfer;
    QString from = ui->lineEdit_btc_from->text();
    transfer.coin_type = HDwallet::BTC;
    transfer.from = from.toStdString();
    transfer.to = ui->lineEdit_btc_to->text().toStdString();
    double amount = std::atof(ui->lineEdit_btc_amount->text().toStdString().c_str());
    double fee = std::atof(ui->lineEdit_btc_fee->text().toStdString().c_str());
    QMessageBox::StandardButton question = QMessageBox::question(nullptr,"BTC tranfers ", std::to_string(amount).c_str());
    if (question == QMessageBox::No || question == QMessageBox::Close)
    {
        return;
    }

    transfer.amount = ui->lineEdit_btc_amount->text().toStdString();
    transfer.fee = ui->lineEdit_btc_fee->text().toStdString();
    QString privkey ;
    bool ret = false;

    if(map_address_privkey_.find(from) != map_address_privkey_.end())
    {
        privkey = map_address_privkey_[from];
    }
    else
    {
        ret = wallet_.getPrivkey(from, privkey);
        if(!ret)
        {
            return;
        }
        map_address_privkey_[from] = privkey;
    }
    size_t index = ui->comboBox_branch->currentIndex();
    if (index > 0)
    {
        WalletDB::Branch branch = s_map_index_branch[index];
    }

    std::string txid;
    ret = hd_wallet_.transferCoin(transfer,txid);

    if(ret)
    {
         QMessageBox::warning(this,"txid",txid.c_str());
         qInfo(txid.c_str());
         WalletDB::Tx tx;
         tx.amount = transfer.amount.c_str();
         tx.height = 0;
         tx.to = transfer.to.c_str();
         tx.txid = txid.c_str();
         tx.from = transfer.from.c_str();
         tx.coin = "BTC";
         if (index > 0)
         {
             tx.coin = ui->comboBox_branch->currentText();
         }
         wallet_.addTx(tx);
    }
    else
    {
        QMessageBox::warning(this, "error", "transaction fail");
    }

    ui->lineEdit_btc_from->clear();
    ui->lineEdit_btc_to->clear();
    ui->lineEdit_btc_amount->clear();
    ui->lineEdit_btc_fee->clear();
    refreshFee();
}

void MainWindow::on_actiontx_triggered()
{
    if (!open_wallet_)
    {
        QMessageBox::warning(this,"Title","please open config file");
        return;
    }
    wallet_.open();
    view_table_.show();
    //wallet_.close();
    refreshFee();
}

void MainWindow::on_actiongenerate_triggered()
{
    /*if (!open_wallet_)
    {
        QMessageBox::warning(this,"Title","please open config file");
        return;
    }*/
   // wallet_.open();
    //std::string url = Rpc::instance().getProxy();
    //conf_dlg_.setUrl(QString(url.c_str()));
    conf_dlg_.show();
   // refreshFee();
}

void MainWindow::on_actionopen_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(
                this,
                tr("open a file."),
                "./",
                tr("config(*.json);;All files(*.*)"));

    if(fileName.isEmpty())
        return;
    std::ifstream jfile(fileName.toStdString().c_str());
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
    if (lock_)
    {
        pass_dlg_.setConfirm(false);
        pass_dlg_.exec();
        QString password = pass_dlg_.getPassword();
        if (password_ != password)
        {
            QMessageBox::warning(this,"wallet","error");
            return;
        }
    }
    else
    {
        pass_dlg_.setConfirm(true);
        pass_dlg_.exec();
        password_ = pass_dlg_.getPassword();
        lock_ = true;
    }

    std::string db_path = json_conf["path"].get<std::string>();
    std::string db_name = json_conf["db"].get<std::string>();
    std::string proxy_url = json_conf["proxy"]["url"].get<std::string>();
    std::string auth = json_conf["proxy"]["auth"].get<std::string>();
    Rpc::instance().initProxy(proxy_url, auth);

    wallet_.init(db_path,db_name);
    config_file_ = fileName;
    open_wallet_ = true;
    std::vector<WalletDB::EthToken> vect_token;
    wallet_.getEthTokenInfo(vect_token);
    ui->comboBox_token->insertItem(0, "");
    for (size_t i = 0; i < vect_token.size(); i++)
    {
        s_map_index_ethtoken[i+1] = vect_token[i];
        ui->comboBox_token->insertItem(i+1, vect_token[i].name);
    }

    std::vector<WalletDB::Branch> vect_branch;
    wallet_.getBtcBranch(vect_branch);
    ui->comboBox_branch->insertItem(0, "BTC");
    for (size_t i = 0; i < vect_branch.size(); i++)
    {
         s_map_index_branch[i+1] = vect_branch[i];
         ui->comboBox_branch->insertItem(i+1, vect_branch[i].name);
    }
    refreshFee();
}

void MainWindow::on_actionbalance_triggered()
{
    if (!open_wallet_)
    {
        QMessageBox::warning(this,"Title","please open config file");
        return;
    }
    wallet_.open();
    view_balance_.resetModel();
    view_balance_.show();
}

void MainWindow::refreshFee()
{
    setFeeEth();
    setFeeBtc();
}

void MainWindow::setFeeEth()
{
    size_t index = ui->comboBox_token->currentIndex();
    uint64_t gas_limit ;
    std::string gas_price ;
    Rpc::instance().getGasPrice(gas_price);

    if(index > 0)
    {
        gas_limit = 90000;
    }
    else
    {
        gas_limit = 21000;
    }
    uint64_t fee = gas_limit * QString(gas_price.c_str()).toLong(nullptr,16);
    double fee_eth = (double)fee / 1000000000.0 / 1000000000.0;
    ui->lineEdit_eth_fee->setText(std::to_string(fee_eth).c_str());

}

void MainWindow::setFeeBtc()
{
    QString fee = "0.00001";
    int conf_target = 10;
    double fee_data = 0;
    Rpc::instance().estimatesmartfee(conf_target, fee_data);
    fee = std::to_string(fee_data).c_str();
    ui->lineEdit_btc_fee->setText(fee);
}

void MainWindow::on_btn_eth_open_clicked()
{
    QString address;
//    setAddressPrivkey(address);
    ui->lineEdit_eth_from->setText(address);
}

void MainWindow::setAddressPrivkey(/*QString& address*/)
{
    QString fileName = QFileDialog::getOpenFileName(
                this,
                tr("open a wallet file."),
                "D:/",
                tr("config(*.json);;All files(*.*)"));

    if (fileName.isEmpty())
    {
        return;
    }
    pass_dlg_.setConfirm(false);
    pass_dlg_.exec();

    //qInfo(fileName.toStdString().c_str());
    QString password = pass_dlg_.getPassword();
 /*   Account account;
    if (!ETHAPI::getInstance().decryptKeystore(fileName.toStdString(), password.toStdString(), account))
    {
        return;
    }
    qWarning(account.privateKey.c_str());
    qWarning(account.address.c_str());
    address = account.address.c_str();
    map_address_privkey_[address] = QString(account.privateKey.c_str());*/
}



void MainWindow::on_btn_btc_insert_clicked()
{
    if (!open_wallet_)
    {
        QMessageBox::warning(this,"wallet","please open config file");
        return;
    }

    DialogTokenBranch dlg;
    dlg.activeBranch();
    WalletDB::Branch branch;
    dlg.exec();
    dlg.getBranch(branch);
    if(branch.auth.isEmpty() ||
       branch.name.isEmpty() ||
       branch.url.isEmpty()  )
    {
        qInfo("branch is empty");
        return ;
    }

    size_t index = s_map_index_branch.size() + 1;
    s_map_index_branch[index] = branch;
    ui->comboBox_token->insertItem(index, branch.name);
    wallet_.addBranch(branch);

}

void MainWindow::on_btn_eth_insert_clicked()
{
    if (!open_wallet_)
    {
        QMessageBox::warning(this,"wallet","please open config file");
        return;
    }

    DialogTokenBranch dlg;
    WalletDB::EthToken token;
    dlg.exec();
    dlg.getToken(token);
    if(token.contract_address.isEmpty() ||
       token.name.isEmpty() ||
       token.decimal <= 0   )
    {
        qWarning("token is empty");
        return ;
    }

    size_t index = s_map_index_ethtoken.size() + 1;
    s_map_index_ethtoken[index] = token;
    ui->comboBox_token->insertItem(index, token.name);
    wallet_.addEthToken(token);
}

void MainWindow::on_comboBox_token_currentIndexChanged(int index)
{
    setFeeEth();
}

void MainWindow::on_btn_aitd_open_clicked()
{

}

void MainWindow::on_btn_aitd_submit_clicked()
{
    if (!open_wallet_)
    {
        QMessageBox::warning(this,"wallet","please open config file");
        return;
    }
    if (lock_)
    {
        pass_dlg_.setConfirm(false);
        pass_dlg_.exec();
        QString password = pass_dlg_.getPassword();
        if (password_ != password)
        {
            QMessageBox::warning(this,"wallet","error password");
            return;
        }
    }

    HDwallet::Transfer transfer;
    QString from = ui->lineEdit_aitd_from->text();
    QString to = ui->lineEdit_aitd_to->text();
    uint64_t tag  = std::atol(ui->lineEdit_aitd_tag->text().toStdString().c_str());
    uint64_t amount =  1000000 * std::atof(ui->lineEdit_aitd_amount->text().toStdString().c_str());
    uint64_t fee = 1000000 * std::atof(ui->lineEdit_aitd_fee->text().toStdString().c_str());

    QString privkey ;
    bool ret = false;

    if(map_address_privkey_.find(from) != map_address_privkey_.end())
    {
        privkey = map_address_privkey_[from];
    }
    else
    {
        ret = wallet_.getPrivkey(from, privkey);
        if(!ret)
        {
            return;
        }
        map_address_privkey_[from] = privkey;
    }

    std::string txid;
    ret = Rpc::instance().submit(privkey.toStdString(), from.toStdString(), to.toStdString(),
                                 tag, amount, fee, txid);
    if(ret)
    {
         QMessageBox::warning(this,"txid",txid.c_str());
         qInfo(txid.c_str());
         WalletDB::Tx tx;
         tx.amount = ui->lineEdit_aitd_amount->text();
         tx.height = 0;
         tx.to = to;
         tx.txid = txid.c_str();
         tx.from = from;
         tx.coin = "AITD";
         wallet_.addTx(tx);
    }
    else
    {
        QMessageBox::warning(this, "error", "transaction fail");
    }

    ui->lineEdit_aitd_from->clear();
    ui->lineEdit_aitd_to->clear();
    ui->lineEdit_aitd_amount->clear();
    ui->lineEdit_aitd_fee->setText("0.0001");

}


