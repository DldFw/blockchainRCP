#include "dialogmasterkey.h"
#include "ui_dialogmasterkey.h"
#include <iostream>
#include "json.hpp"
#include <QMessageBox>
#include <QDateTime>
#include <fstream>
#include <QFileDialog>
using json = nlohmann::json;

DialogMasterKey::DialogMasterKey(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogMasterKey)
{
    ui->setupUi(this);
}

DialogMasterKey::~DialogMasterKey()
{
    delete ui;
}

void DialogMasterKey::setShowText(const QString &privkey, const QString &pubkey, const QString &mnemonic)
{
    ui->textEdit_privkey->setText(privkey);
    ui->textEdit_pubkey->setText(pubkey);
    ui->textEdit->setText(mnemonic);
}

void DialogMasterKey::on_save_clicked()
{
    QString privkey = ui->textEdit_privkey->toPlainText();
    QString pubkey = ui->textEdit_pubkey->toPlainText();
    QString mnemonic = ui->textEdit->toPlainText();

    QString file_name = QFileDialog::getSaveFileName(this,
            QString::fromLocal8Bit("file save"),
            "",
            tr("master key (*.json)"));

    json json_data;
    json_data["privkey"] = privkey.toStdString();
    json_data["pubkey"] = pubkey.toStdString();
    json_data["mnemonic"] = mnemonic.toStdString();
    std::ofstream save_file(file_name.toStdString().c_str());
    save_file << std::setw(4) << json_data << std::endl;
    save_file.close();
    QMessageBox::warning(this,"save master key", file_name.toStdString().c_str());
}
