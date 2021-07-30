#ifndef WALLETDB_H
#define WALLETDB_H
#include <QtSql>

class WalletDB
{
public:
    WalletDB();
    ~WalletDB()
    {
    }
    struct Account
    {
        QString priv_key;
        QString address;
        QString coin;
    };

    struct Balance
    {
      QString address;
      QString coin;
      QString amount;
    };

    struct Tx
    {
        QString from;
        QString to;
        QString txid;
        QString amount;
        uint64_t height;
        QString coin;
    };

    struct EthToken
    {
        QString name;
        QString contract_address;
        int decimal;
    };

    struct Branch
    {
        QString name;
        QString url;
        QString auth;
    };


    bool init(const std::string& dbpath, const std::string& dbname);

    bool open();

    bool close();

    bool addAccount(const Account&account);

    bool addBalance(const Balance& balance);

    bool addTx( const Tx& tx);

    bool getPrivkey(const QString& address, QString& priv_key);

    bool updateBalance(const QString& address, const QString& balance, const QString &coin);

    bool isCoinInDb(const QString& address, const QString &coin);

    bool getAddress(std::vector<QString>& vect_address, const QString& coin);

    bool getEthTokenInfo(std::vector<EthToken>& vect_token);

    bool getBtcBranch(std::vector<Branch>& vect_branch);

    bool addBranch(const Branch& branch);

    bool addEthToken(const EthToken& token);

private:
    QSqlDatabase db_;
};

#endif // WALLETDB_H
