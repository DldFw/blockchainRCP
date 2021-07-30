#ifndef DIALOGTOKENBRANCH_H
#define DIALOGTOKENBRANCH_H

#include <QDialog>
#include "walletdb.h"

namespace Ui {
class DialogTokenBranch;
}

class DialogTokenBranch : public QDialog
{
    Q_OBJECT

public:
    explicit DialogTokenBranch(QWidget *parent = nullptr);
    ~DialogTokenBranch();

    void activeBranch();

    bool getBranch(WalletDB::Branch& branch);

    bool getToken(WalletDB::EthToken& token);


private slots:
    void on_buttonBox_accepted();

private:
    Ui::DialogTokenBranch *ui;
    bool is_token = true;
};

#endif // DIALOGTOKENBRANCH_H
