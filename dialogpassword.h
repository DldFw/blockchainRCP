#ifndef DIALOGPASSWORD_H
#define DIALOGPASSWORD_H

#include <QDialog>

namespace Ui {
class DialogPassword;
}

class DialogPassword : public QDialog
{
    Q_OBJECT

public:
    explicit DialogPassword(QWidget *parent = nullptr);
    ~DialogPassword();

    QString getPassword()
    {
        return  password_;
    }

    void setConfirm(bool need_confirm);

    bool confirmSuccess()
    {
        return confirm_success_;
    }


private slots:
    void on_buttonBox_accepted();

private:
    Ui::DialogPassword *ui;
    QString password_;
    bool need_confirm_ = true;
    bool confirm_success_ = false;
};

#endif // DIALOGPASSWORD_H
