#ifndef DIALOGMASTERKEY_H
#define DIALOGMASTERKEY_H

#include <QDialog>

namespace Ui {
class DialogMasterKey;
}

class DialogMasterKey : public QDialog
{
    Q_OBJECT

public:
    explicit DialogMasterKey(QWidget *parent = nullptr);
    ~DialogMasterKey();

    void setShowText(const QString& privkey, const QString& pubkey, const QString& mnemonic);

private slots:
    void on_save_clicked();

private:
    Ui::DialogMasterKey *ui;
};

#endif // DIALOGMASTERKEY_H
