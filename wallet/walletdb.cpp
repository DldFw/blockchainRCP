#include "walletdb.h"
#include <iostream>

WalletDB::WalletDB()
{

}

bool WalletDB::init(const std::string &dbpath, const std::string &dbname)
{
    bool ret = false;
    db_ = QSqlDatabase::addDatabase("QSQLITE");
    QString path_dbname =QString(dbpath.c_str()) + QString("/")+ QString(dbname.c_str());
    db_.setDatabaseName(path_dbname);
    if(!open())
    {
        qWarning("init wallet db fail");
        return ret;
    }

   QStringList tables = db_.tables();

    if (tables.contains("account", Qt::CaseInsensitive)
        && tables.contains("balance", Qt::CaseInsensitive)
        && tables.contains("tx", Qt::CaseInsensitive)
        && tables.contains("eth_token", Qt::CaseInsensitive)
        && tables.contains("btc_branch", Qt::CaseInsensitive))
    {
        ret = true;
        return ret;
    }

    QSqlQuery q;
    if (!q.exec(QLatin1String("create table eth_token(contract_address varchar primary key, token_name varchar, token_dec int)")) ||
        !q.exec(QLatin1String("create table btc_branch(coin varchar primary key, url varchar, auth varchar)")) ||
        !q.exec(QLatin1String("create table account(privkey varchar primary key, address varchar, coin varchar)")) ||
        !q.exec(QLatin1String("create table balance(address varchar, coin varchar, amount varchar)")) ||
        !q.exec(QLatin1String("create table tx(vin varchar, vout varchar, txid varchar, amount varchar, height integer, coin varchar)"))
        )
    {
        ret = false;
    }
    return ret;
}

bool WalletDB::open()
{
    if(db_.isOpen())
    {
        return true;
    }
    return db_.open();
}

bool WalletDB::close()
{
    db_.close();
    QSqlDatabase::removeDatabase( QSqlDatabase::defaultConnection);
    if (db_.isOpen())
    {
        qDebug("db is open, close fail");
    }
    if (db_.isValid())
    {
        qDebug("db is valid, close fail");
        std::cout << "db is valid" << std::endl;
    }
    return true;
}

bool WalletDB::addAccount(const WalletDB::Account &account)
{
    bool ret = false;
    if(!open())
    {
       qWarning("account open db fail" );
       return ret;
    }

    QSqlQuery query;
    if (!query.prepare(QLatin1String("insert into account(privkey, address, coin) values(?, ?, ?)")))
    {
        return ret;
    }
    query.addBindValue(account.priv_key);
    query.addBindValue(account.address);
    query.addBindValue(account.coin);
    ret = query.exec();
    return ret;
}


bool WalletDB::addBalance(const WalletDB::Balance &balance)
{
    bool ret = false;
    if(!open())
    {
       qWarning("add balance open db fail" );
       return ret;
    }
    QSqlQuery query;
    if (!query.prepare(QLatin1String("insert into balance(address, coin, amount) values(?, ?, ?)")))
        return ret;

    query.addBindValue(balance.address);
    query.addBindValue(balance.coin);
    query.addBindValue(balance.amount);
    query.exec();
    return true;
}

bool WalletDB::addTx(const WalletDB::Tx &tx)
{
    bool ret = false;
    if(!open())
    {
       qWarning("add tx open db fail" );
       return ret;
    }
    QSqlQuery query;
    if (!query.prepare(QLatin1String("insert into tx(vin, vout, txid, amount, height, coin) values(?, ?, ?, ?, ?, ?)")))
        return ret;

    QString str_height = std::to_string(tx.height).c_str();

    query.addBindValue(tx.from);
    query.addBindValue(tx.to);
    query.addBindValue(tx.txid);
    query.addBindValue(tx.amount);
    query.addBindValue(str_height);
    query.addBindValue(tx.coin);
    query.exec();
    return true;
}

bool WalletDB::getPrivkey(const QString &address, QString &priv_key)
{
    bool ret = false;
    if(!open())
    {
       qWarning("get privkey open db fail" );
       return ret;
    }
    QSqlQuery query;
    QString sql = "select privkey from account where address = '" + address + "';";
    query.exec(sql);

    printf("getPrivkey, sql: %s\n", sql.toStdString().c_str());
    while(query.next())
    {
        priv_key = query.value(0).toString();
        qInfo(priv_key.toStdString().c_str());
        ret = true;
    }
    return ret;
}

bool WalletDB::isCoinInDb(const QString& address, const QString &coin)
{
    if(!open())
    {
       qWarning("open db fail" );
       return false;
    }
    QSqlQuery query;
    QString sql = "select * from balance where address = '" + address + "' and coin = '" + coin +"'; ";
    query.exec(sql);
    qInfo(sql.toStdString().c_str());
    while(query.next())
    {
        return true;
    }
    qDebug("select balance end" );
    return false;
}

bool WalletDB::updateBalance(const QString &address, const QString &balance, const QString& coin)
{
    if(!open())
    {
       qWarning("open db fail" );
       return false;
    }

    auto isInDb = isCoinInDb(address, coin);
    if (isInDb)
    {
        QSqlQuery query;
        QString sql = "update balance set amount = '" + balance +"' where address = '" + address + "' and coin = '" + coin +"'; ";
        query.exec(sql);
        qDebug(sql.toStdString().c_str());
        qDebug("update balance end" );
    } else {
        WalletDB::Balance st_balance;
        st_balance.address = address;
        st_balance.amount = balance;
        st_balance.coin = coin;
        addBalance(st_balance);
    }
    return true;
}

bool WalletDB::getAddress(std::vector<QString> &vect_address, const QString &coin)
{
    bool ret = false;
    if(!open())
    {
       qWarning("get address open db fail");
       return ret;
    }
    QSqlQuery query;
    QString sql = "select address from account where coin = '" + coin + "';";
    query.exec(sql);

    while(query.next())
    {
        QString address = query.value(0).toString();
        vect_address.push_back(address);
        qInfo(address.toStdString().c_str());
        ret = true;
    }
    return ret;
}

bool WalletDB::getEthTokenInfo(std::vector<WalletDB::EthToken> &vect_token)
{
    bool ret = false;
    if(!open())
    {
       qWarning("eth token open db fail");
       return ret;
    }
    QSqlQuery query;
    QString sql = "select token_name, token_dec, contract_address from eth_token;";
    query.exec(sql);

    while(query.next())
    {
        EthToken token;
        token.name = query.value(0).toString();
        token.decimal = query.value(1).toInt();
        token.contract_address = query.value(2).toString();
        vect_token.push_back(token);
        ret = true;
    }
  //  close();
    return ret;
}

bool WalletDB::getBtcBranch(std::vector<WalletDB::Branch> &vect_branch)
{
    bool ret = false;
    if(!open())
    {
       qWarning("btc branch open db fail");
       return ret;
    }
    QSqlQuery query;
    //if (!q.exec(QLatin1String("create table btc_branch(coin varchar primary key, url varchar, auth varchar)")))
    QString sql = "select coin, url, auth from btc_branch;";
    query.exec(sql);

    while(query.next())
    {
        Branch branch;
        branch.name = query.value(0).toString();
        branch.url = query.value(1).toString();
        branch.auth = query.value(2).toString();
        vect_branch.push_back(branch);
        ret = true;
    }
    //close();
    return ret;
}

bool WalletDB::addBranch(const WalletDB::Branch &branch)
{
    bool ret = false;
    if(!open())
    {
       qWarning("add branch open db fail" );
       return ret;
    }
    QSqlQuery query;
    if (!query.prepare(QLatin1String("insert into btc_branch(coin, url, auth) values(?, ?, ?)")))
        return ret;
    query.addBindValue(branch.name);
    query.addBindValue(branch.url);
    query.addBindValue(branch.auth);
    query.exec();
   // close();
    ret = true;
    return ret;

}

bool WalletDB::addEthToken(const WalletDB::EthToken &token)
{
    bool ret = false;
    if(!open())
    {
       qWarning("add token open db fail" );
       return ret;
    }
    QSqlQuery query;
    if (!query.prepare(QLatin1String("insert into eth_token(contract_address, token_name, token_dec) values(?, ?, ?)")))
        return ret;

    query.addBindValue(token.contract_address);
    query.addBindValue(token.name);
    query.addBindValue(token.decimal);
    query.exec();
   // close();
    ret = true;
    return ret;

}
