#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "oauth2pininputdialog.h"
#include "dropbox.h"
#include <QDialogButtonBox>
#include <QDebug>

const QString tokenFileName = "NetworkStorageAccessSample.token";

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , dropbox(new Dropbox(this))
{
    ui->setupUi(this);

    connect(dropbox, &Dropbox::authenticated, [&]() {
        //qDebug() << (int)dropbox->status() << "," << (int)Dropbox::Status::Granted;
        ui->stateDropbox->setText("authenticated!");
    });

    connect(dropbox, &Dropbox::uploaded, [&](const QString& path) {
        appendLog(QString("Dropbox::uploaded >> \"%1\"").arg(path));
        dropbox->download("/12345.txt");
    });

    connect(dropbox, &Dropbox::downloaded, [&](const QString& path, const QByteArray& data) {
        appendLog(QString("Dropbox::downloaded >> \"%1\" -> %2").arg(path).arg(data.size()));
        dropbox->remove("/12345.txt");
    });

    connect(dropbox, &Dropbox::removed, [&](const QString& path) {
        appendLog(QString("Dropbox::removed >> \"%1\"").arg(path));
    });

    tokensLoad();
}

MainWindow::~MainWindow()
{
    tokensSave();
    delete ui;
}

void MainWindow::tokensLoad()
{
    QFile file(QFileInfo(QDir::homePath(), tokenFileName).filePath());
    file.open(QIODevice::ReadOnly);
    QDataStream in(&file);

    QMap<QString, QString> tokenAndService;
    in >> tokenAndService;

    QMap<QString, QString>::const_iterator ite = tokenAndService.constBegin();
    while (ite != tokenAndService.constEnd()) {
        if (ite.key() == "dropbox") {
            dropbox->deserialize(ite.value());
        }
        ++ite;
    }
}

void MainWindow::tokensSave()
{
    QMap<QString, QString> tokenAndService;

    tokenAndService.insert("dropbox", dropbox->serialize());

    QFile file(QFileInfo(QDir::homePath(), tokenFileName).filePath());
    file.open(QIODevice::WriteOnly);
    QDataStream out(&file);
    out << tokenAndService;
}

void MainWindow::appendLog(const QString &newLog)
{
    const auto text = ui->log->toPlainText();
    ui->log->setPlainText(text + QString(text.isEmpty() ? "%1" : "\n%1").arg(newLog));
}

void MainWindow::on_authDropbox_clicked()
{
    ui->stateDropbox->setText("");

    dropbox->authenticate();

    OAuth2PinInputDialog dlg;

    if (QDialog::Accepted == dlg.exec()) {
        dropbox->setPinCode(dlg.getPinCode());
    }
}

void MainWindow::on_runBenchmark_clicked()
{
    QByteArray data;
    data = QString("test").toUtf8();
    dropbox->upload(data, "/12345.txt");
}
