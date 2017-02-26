#include "oauth2pininputdialog.h"
#include "ui_oauth2pininputdialog.h"
#include <QPushButton>

OAuth2PinInputDialog::OAuth2PinInputDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::OAuth2PinInputDialog)
{
    ui->setupUi(this);

    Q_EMIT on_pinCode_textChanged("");
}

OAuth2PinInputDialog::~OAuth2PinInputDialog()
{
    delete ui;
}

const QString& OAuth2PinInputDialog::getPinCode() const
{
    return pinCode;
}

void OAuth2PinInputDialog::on_pinCode_textChanged(const QString &arg1)
{
    QPushButton *okButton = ui->buttonBox->button(QDialogButtonBox::Ok);
    if (arg1.isEmpty() == okButton->isEnabled()) {
        okButton->setEnabled(!arg1.isEmpty());
    }
}

void OAuth2PinInputDialog::on_buttonBox_accepted()
{
    pinCode = ui->pinCode->text();
    accept();
}

void OAuth2PinInputDialog::on_buttonBox_rejected()
{
    reject();
}
