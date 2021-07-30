#ifndef DIALOGBALANCE_H
#define DIALOGBALANCE_H

#include <QDialog>
#include <QtSql>
namespace Ui {
class DialogBalance;
}

class DialogBalance : public QDialog
{
    Q_OBJECT

public:
    explicit DialogBalance(QWidget *parent = nullptr);
    ~DialogBalance();

    void resetModel();

signals:
    void sendAddress(const QString& address, const QString& coin);

private slots:
    void on_buttonBox_accepted();
    void on_tableView_doubleClicked(const QModelIndex &index);

protected:
    void updateBalance();

    void updateBtcBalance();

    void updateEthBalance();

    void updateAitdBalance();
private:
    Ui::DialogBalance *ui;
    QSqlTableModel* model_;
};

#endif // DIALOGBALANCE_H
