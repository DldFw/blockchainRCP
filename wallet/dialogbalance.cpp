#include "dialogbalance.h"
#include "ui_dialogbalance.h"
#include "mainwindow.h"
#include "rpc.h"
#include "multy_core/src/api/big_int_impl.h"
#include "utility.h"

namespace
{
using namespace wallet_utility;
//using namespace multy_core::internal;

} // namespace

DialogBalance::DialogBalance(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogBalance)
{
    ui->setupUi(this);
    model_ = nullptr;
   // updateBalance();
}

DialogBalance::~DialogBalance()
{
    delete ui;
}

void DialogBalance::resetModel()
{
    updateBalance();

    model_ = new QSqlTableModel(ui->tableView);
   // model_->setEditStrategy(QSqlTableModel::OnManualSubmit);
    model_->setTable("balance");

    // Set the localized header captions:
     model_->setHeaderData(model_->fieldIndex("address"),
                          Qt::Horizontal, tr("address"));
     model_->setHeaderData(model_->fieldIndex("coin"),
                          Qt::Horizontal, tr("coin"));
     model_->setHeaderData(model_->fieldIndex("amount"),
                          Qt::Horizontal, tr("amount"));

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
     ui->tableView->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);

}

void DialogBalance::on_buttonBox_accepted()
{
    if(model_)
    {
        model_->clear();
        delete model_;
        model_ = nullptr;
    }
}

void DialogBalance::updateBalance()
{
    updateBtcBalance();
    updateEthBalance();
    updateAitdBalance();
}

void DialogBalance::updateBtcBalance()
{
    QString coin = "BTC";
    std::vector<QString> vect_address;
    MainWindow::wallet_.getAddress(vect_address, coin);

    for (size_t i = 0; i < vect_address.size(); i++)
    {
        std::vector<Rpc::Utxo> vect_utxo;
        double total = 0;
        Rpc::instance().getUtxo(vect_address[i].toStdString(), vect_utxo, total);

        QString amount = std::to_string(total).c_str();
        MainWindow::wallet_.updateBalance(vect_address[i], amount, "BTC");
    }
}

void DialogBalance::updateAitdBalance()
{
    QString coin = "AITD";
    std::vector<QString> vect_address;
    MainWindow::wallet_.getAddress(vect_address, coin);

    for (size_t i = 0; i < vect_address.size(); i++)
    {
        std::vector<Rpc::Utxo> vect_utxo;
        double total = 0;
        Rpc::instance().accountInfo(vect_address[i].toStdString(), total);
        QString amount = std::to_string(total).c_str();
        MainWindow::wallet_.updateBalance(vect_address[i], amount, "AITD");
    }
}

void DialogBalance::updateEthBalance()
{
    std::vector<QString> vect_address;
    MainWindow::wallet_.getAddress(vect_address, "ETH");
    std::vector<WalletDB::EthToken> vect_token;
    MainWindow::wallet_.getEthTokenInfo(vect_token);

    for (size_t i = 0; i < vect_address.size(); i++)
    {
        Rpc::EthAccount account;
        account.address = vect_address[i].toStdString();
        account.token = false;
        account.decimal = 18;
        std::string amount = "0";
        Rpc::instance().getBalance(account, amount);
        amount = amount.substr(2);
        BigInt amount_big_int(amount.c_str(), 16);
        MainWindow::wallet_.updateBalance(vect_address[i], QString(conversion(amount_big_int, account.decimal).c_str()), "ETH");

        for (size_t j = 0; j < vect_token.size(); j++)
        {
            amount = "0";
            account.contract = vect_token[j].contract_address.toStdString();
            account.decimal = vect_token[j].decimal;
            account.token = true;
            Rpc::instance().getBalance(account, amount);
            amount = amount.substr(2);
            BigInt amount_big_int(amount.c_str(), 16);
            MainWindow::wallet_.updateBalance(vect_address[i], QString(conversion(amount_big_int, account.decimal).c_str()), vect_token[j].name);
        }
    }
}



void DialogBalance::on_tableView_doubleClicked(const QModelIndex &index)
{
    QString  coin = model_->record(index.row()).value(1).toString();
    QString  address = model_->record(index.row()).value(0).toString();
    emit sendAddress(address, coin);
    std::cout << coin.toStdString() << " : " << address.toStdString() << std::endl;


}
