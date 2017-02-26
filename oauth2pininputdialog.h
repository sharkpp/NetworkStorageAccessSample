#ifndef OAUTH2PININPUTDIALOG_H
#define OAUTH2PININPUTDIALOG_H

#include <QDialog>

namespace Ui {
class OAuth2PinInputDialog;
}

class OAuth2PinInputDialog : public QDialog
{
    Q_OBJECT

public:
    explicit OAuth2PinInputDialog(QWidget *parent = 0);
    ~OAuth2PinInputDialog();

    const QString& getPinCode() const;

private slots:
    void on_pinCode_textChanged(const QString &arg1);

    void on_buttonBox_accepted();

    void on_buttonBox_rejected();

private:
    Ui::OAuth2PinInputDialog *ui;
    QString pinCode;
};

#endif // OAUTH2PININPUTDIALOG_H
