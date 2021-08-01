#ifndef CONFIGDIALOG_H
#define CONFIGDIALOG_H

#include <QDialog>
#include <QtSql>

namespace Ui {
class ConfigDialog;
}

class ConfigDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ConfigDialog(QWidget *parent = nullptr);
    ~ConfigDialog();

    void setUrl(const QString&nodeurl);
    struct HDInfo
    {
        std::string maskey_mnemonic;
        int child_code;
        int index;
    };

    bool getHDInfo(const QString& file, HDInfo& hdinfo);

private slots:

    void on_btn_show_clicked();

    void on_buttonBox_accepted();

    void on_btn_import_clicked();

    void on_btn_generate_clicked();

    void on_btn_aitd_clicked();

private:
    Ui::ConfigDialog *ui;
    QSqlTableModel* model_;
};

#endif // CONFIGDIALOG_H
