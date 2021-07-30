#include "coinview.h"
#include "ui_coinview.h"

coinview::coinview(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::coinview)
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    setWindowModality(Qt::WindowModal);
    ui->setupUi(this);
    model_ = nullptr;
}

coinview::~coinview()
{
    delete ui;
}

void coinview::on_btn_search_clicked()
{
    QString address = ui->lineEdit_address->text();
    QString txid = ui->lineEdit_txid->text();
    model_ = new QSqlTableModel(ui->tableView);
   // model_->setEditStrategy(QSqlTableModel::OnManualSubmit);
    model_->setTable("tx");

    // Set the localized header captions:
     model_->setHeaderData(model_->fieldIndex("vin"),
                          Qt::Horizontal, tr("vin"));
     model_->setHeaderData(model_->fieldIndex("vout"),
                          Qt::Horizontal, tr("vout"));
     model_->setHeaderData(model_->fieldIndex("txid"),
                          Qt::Horizontal, tr("txid"));
     model_->setHeaderData(model_->fieldIndex("amount"),
                          Qt::Horizontal, tr("amount"));
     model_->setHeaderData(model_->fieldIndex("height"),
                          Qt::Horizontal, tr("height"));
     model_->setHeaderData(model_->fieldIndex("coin"),
                          Qt::Horizontal, tr("coin"));

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
     ui->tableView->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
     ui->tableView->horizontalHeader()->setSectionResizeMode(4, QHeaderView::Stretch);
     ui->tableView->horizontalHeader()->setSectionResizeMode(5, QHeaderView::Stretch);

}

void coinview::on_btn_close_clicked()
{
    if (model_)
    {
        model_->clear();
        delete  model_;
        model_ = nullptr;
    }
    this->close();
}
