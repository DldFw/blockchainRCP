#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "coinview.h"
#include "configdialog.h"
#include "walletdb.h"
#include "dialogbalance.h"
#include "dialogpassword.h"
#include "hdwallet.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void setHDWallet(const HDwallet& hdwallet)
    {
        hd_wallet_ = hdwallet;
    }


private slots:

    void updateAddress(const QString& address, const QString& coin);

    void on_btn_eth_submit_clicked();

    void on_actiongenerate_triggered();

    void on_actionopen_triggered();

    void on_actionbalance_triggered();

    void on_btn_eth_open_clicked();

    void on_actiontx_triggered();

    void on_btn_btc_insert_clicked();

    void on_btn_eth_insert_clicked();

    void on_btn_btc_submit_clicked();

    void on_comboBox_token_currentIndexChanged(int index);

    void on_btn_aitd_open_clicked();

    void on_btn_aitd_submit_clicked();


protected:
    void setAddressPrivkey();

private:
    Ui::MainWindow *ui;
    coinview view_table_;
    ConfigDialog conf_dlg_;
    QString config_file_;
    QString nodeurl_;
    bool open_wallet_;
    DialogPassword pass_dlg_;
    DialogBalance view_balance_;
    std::map<QString,QString> map_address_privkey_;

protected:
    void refreshFee();

    void setFeeEth();

    void setFeeBtc();

public:
    static WalletDB wallet_;
    HDwallet hd_wallet_;
    bool lock_ = false;
    QString password_ ;
};

#endif // MAINWINDOW_H
