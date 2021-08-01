#include "mainwindow.h"
#include <QApplication>
#include <iostream>
#include <QtSql>
#include "walletdb.h"
#include <memory>
#include <string.h>
#include "hdwallet.h"
#include <QDateTime>
#include <QMessageBox>
//

void outputMessage(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    static QMutex mutex;
    mutex.lock();
    QString text;
    QString prefix;
    switch(type)
    {
    case QtDebugMsg:
        text = QString("Debug:");
        prefix = QString("Debug");
        break;
    case QtWarningMsg:
        text = QString("Warning:");
        prefix = QString("Warning");
        break;
    case QtInfoMsg:
        text = QString("Info:");
        prefix = QString("Info");
        break;
    case QtCriticalMsg:
        text = QString("Critical:");
        prefix = QString("Critical");
        break;
    case QtFatalMsg:
        text = QString("Fatal:");
        prefix = QString("Fatal");
        break;
    }

    QString context_info = QString("File:(%1) Line:(%2)").arg(QString(context.file)).arg(context.line);
    QString current_date_time = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    QString current_date = QString("(%1)").arg(current_date_time) + "\n";
    QString message = QString("%1 %2 %3 %4").arg(text).arg(context_info).arg(current_date).arg(msg);
    //QString message = QString("%1 %2 %3").arg(text).arg(context_info).arg(msg);
    QDir dir;
    if (dir.mkdir("./log"))
    {

    }

    QString file_name ="log/" + prefix + QDateTime::currentDateTime().toString("yyyy_MM_dd_hh");
    file_name += ".log";
    QFile file(file_name);
    file.open(QIODevice::WriteOnly | QIODevice::Append);
    QTextStream text_stream(&file);
    text_stream << message << "\n";
    file.flush();
    file.close();
    mutex.unlock();
}

void testHdWallet()
{
    HDwallet hd_wallet;
    std::string passwd = "test fast";
    hd_wallet.createMasterKey(passwd);
    HDwallet::HdInfo hd_info;
    hd_wallet.getHdInfo(hd_info);
    hd_wallet.createChildKey(1);
    std::cout << hd_info.mnemonic << std::endl;
    std::cout << hd_info.passwd << std::endl;
    std::cout << hd_info.seed << std::endl;


    HDwallet hd_wallet_bak;
    hd_info.mnemonic = "abandon amount liar amount expire adjust cage candy arch gather drum bullet absurd math era live bid rhythm alien crouch range attend journey tomato cancel baby simple engage give quit";
    hd_wallet_bak.importMasterKey(hd_info.mnemonic, hd_info.passwd, true);
    HDwallet::HdInfo hd_info_bak;
    hd_wallet_bak.getHdInfo(hd_info_bak);
    hd_wallet_bak.createChildKey(1);
    std::cout << hd_info_bak.mnemonic << std::endl;
    std::cout << hd_info_bak.passwd << std::endl;
    std::cout << hd_info_bak.seed << std::endl;
}


/*
void testSeed()
{
    using namespace multy_core::internal;
     try
       {
           const EntropySource entropy{nullptr, &feed_silly_entropy};

           ConstCharPtr mnemonic;
           throw_if_error(make_mnemonic(entropy, reset_sp(mnemonic)));
           std::cout << "Generated mnemonic: " << mnemonic.get() << std::endl;

           std::cout << "Enter password: ";
           std::string password;
           password = "test";
          // std::getline(std::cin, password);

           BinaryDataPtr seed;
           throw_if_error(
                   make_seed(mnemonic.get(), password.c_str(), reset_sp(seed)));

           ConstCharPtr seed_string;
           throw_if_error(seed_to_string(seed.get(), reset_sp(seed_string)));
           std::cout << "Seed: " << seed_string.get() << std::endl;
           ExtendedKey parent_key;
           memset(&parent_key.key, 0, sizeof(parent_key.key));

           std::cout << "size: " << seed->len << std::endl;
            bip32_key_from_seed(seed->data,seed->len,BIP32_VER_MAIN_PRIVATE,0,&parent_key.key);
           unsigned char serialized_key[BIP32_SERIALIZED_LEN] = {'\0'};

           bip32_key_serialize(&parent_key.key, 0, serialized_key, sizeof(serialized_key));
           std::cout << "parent key: " << parent_key.to_string() << std::endl;
           ExtendedKeyPtr child_key1 = make_child_key(parent_key, 1);
           ExtendedKeyPtr child_key2 = make_child_key(parent_key, 2);
           ExtendedKeyPtr child_key3 = make_child_key(parent_key, 3);
           ExtendedKeyPtr child_key4 = make_child_key(parent_key, 4);
           ExtendedKeyPtr child_key5 = make_child_key(parent_key, 5);
           ExtendedKeyPtr child_key6 = make_child_key(parent_key, 6);
           std::cout << "child 1 key: " << child_key1->to_string() << std::endl;
           std::cout << "child 2 key: " << child_key2->to_string() << std::endl;
           std::cout << "child 3 key: " << child_key3->to_string() << std::endl;
           std::cout << "child 4 key: " << child_key4->to_string() << std::endl;
           std::cout << "child 5 key: " << child_key5->to_string() << std::endl;
           std::cout << "child 6 key: " << child_key6->to_string() << std::endl;

       }
       catch (Error* e)
       {
           std::cerr << "Got error: " << e->message << std::endl;
           free_error(e);
       }
}


void testMultyCore ()
{

    //using namespace multy_core::internal;
    const unsigned char data_vals[] = {1U, 2U, 3U, 4U};
    const BinaryData data{data_vals, 3};
    multy_core::internal::ExtendedKeyPtr key;
    make_master_key(&data, reset_sp(key));
    if(key)
        std::cout <<  key->to_string() << std::endl;
    else
        std::cout << "key is null" << std::endl;
    try
    {
        ExtendedKey parent_key;
        memset(&parent_key.key, 0, sizeof(parent_key.key));
        unsigned char serialized_key[BIP32_SERIALIZED_LEN] = {'\0'};
        bip32_key_serialize(&parent_key.key, 0, serialized_key, sizeof(serialized_key));
        std::cout << "parent key: " << parent_key.to_string() << std::endl;
        multy_core::internal::ExtendedKeyPtr child_key;
    }
    catch (Error* e)
    {
        std::cerr << "Got error: " << e->message << std::endl;
        free_error(e);
    }

}*/

void testDb()
{
    QSqlDatabase db_ = QSqlDatabase::addDatabase("QSQLITE");
    QString path_dbname = "test.db";
    db_.setDatabaseName(path_dbname);
    if(!db_.open())
        return ;

    QStringList tables = db_.tables();
    QSqlDatabase::removeDatabase("QSQLITE");
    db_.close();
}

void testLog()
{
    qInfo("info data");
    qDebug("debug data");
    qWarning("warning data");
    qCritical("critical data");

}

int main(int argc, char *argv[])
{

    QApplication a(argc, argv);
    qInstallMessageHandler(outputMessage);
    MainWindow w;
    w.show();
    return a.exec();
}
