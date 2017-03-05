#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "oauth2pininputdialog.h"
#include "dropbox.h"
#include <QDialogButtonBox>
#include <QElapsedTimer>
#include <QDebug>
#include <random> //

const QString tokenFileName = "NetworkStorageAccessSample.token";

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , currentBenchmarkState(BenchmarkStandby)
    , benchmarkWorkingTime(new QElapsedTimer())
    , random10kB(new QByteArray())
    , dropbox(new Dropbox(this))
{
    ui->setupUi(this);

    std::mt19937 myrand((uint)(QTime::currentTime()).msec());
    random10kB->fill('*', 10 * 1024);
    for (int i = 0; i < random10kB->size(); ++i) {
        (*random10kB)[i] = (qint8)myrand();
    }

    connect(dropbox, &Dropbox::authenticated, [&]() {
        //qDebug() << (int)dropbox->status() << "," << (int)Dropbox::Status::Granted;
        ui->stateDropbox->setText("authenticated!");
    });

    connect(dropbox, &Dropbox::uploaded, [&](const QString& path) {
        //appendLog(QString("Dropbox::uploaded >> \"%1\"").arg(path));
        nextBenchmark();
    });

    connect(dropbox, &Dropbox::downloaded, [&](const QString& path, const QByteArray& data) {
        //appendLog(QString("Dropbox::downloaded >> \"%1\" -> %2").arg(path).arg(data.size()));
        nextBenchmark();
    });

    connect(dropbox, &Dropbox::removed, [&](const QString& path) {
        //appendLog(QString("Dropbox::removed >> \"%1\"").arg(path));
        nextBenchmark();
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

void MainWindow::startBenchmark()
{
    currentBenchmarkState = BenchmarkDropboxUpload10kFromMemory;
    processBenchmark();
}

void MainWindow::nextBenchmark()
{
    qint64 t = benchmarkWorkingTime->elapsed();
    double bytesPerSec = 0;
    QString benchmarkName;

    switch (currentBenchmarkState)
    {
    case BenchmarkDropboxUpload10kFromMemory: {
        benchmarkName = QString("Dropbox 10kB upload from memory");
        bytesPerSec = 10.0 * 1024.0 * 1000.0 / (double)t;
        break; }
    case BenchmarkDropboxDownload10kFromMemory:
        benchmarkName = QString("Dropbox 10kB download to memory");
        bytesPerSec = 10.0 * 1024.0 * 1000.0 / (double)t;
        break;
    case BenchmarkDropboxDelete10kFromMemory:
        benchmarkName = QString("Dropbox 10kB delete");
        bytesPerSec = 10.0 * 1024.0 * 1000.0 / (double)t;
        break;
    }

    appendLog(QString("%1: %2 ms, %3 bytes/s").arg(benchmarkName).arg(t).arg(bytesPerSec));

    currentBenchmarkState = (BenchmarkState)((int)currentBenchmarkState + 1);

    processBenchmark();
}

void MainWindow::processBenchmark()
{
    switch (currentBenchmarkState)
    {
    case BenchmarkFinish:
        appendLog("benchmark complete!");
    case BenchmarkStandby:
        currentBenchmarkState = BenchmarkStandby;
        return;
    case BenchmarkError:
        appendLog("benchmark error!");
        return;

    case BenchmarkDropboxUpload10kFromMemory: {
        dropbox->upload(*random10kB, "/12345.txt");
        break; }
    case BenchmarkDropboxDownload10kFromMemory:
        dropbox->download("/12345.txt");
        break;
    case BenchmarkDropboxDelete10kFromMemory:
        //currentBenchmarkState = BenchmarkStandby;
        dropbox->remove("/12345.txt");
        break;
    }

    benchmarkWorkingTime->start();
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
    startBenchmark();
}
