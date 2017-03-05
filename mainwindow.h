#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class QElapsedTimer;

class Dropbox;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

    enum BenchmarkState {
        BenchmarkStandby,
        BenchmarkDropboxUpload10kFromMemory,
        BenchmarkDropboxDownload10kFromMemory,
        BenchmarkDropboxDelete10kFromMemory,
        BenchmarkFinish,
        BenchmarkError,
    };

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    void tokensLoad();
    void tokensSave();

    void appendLog(const QString& newLog);

    void startBenchmark();
    void nextBenchmark();
    void processBenchmark();

private slots:
    void on_authDropbox_clicked();
    void on_runBenchmark_clicked();

private:
    Ui::MainWindow *ui;

    BenchmarkState currentBenchmarkState;
    QElapsedTimer* benchmarkWorkingTime;
    QByteArray* random10kB;

    Dropbox* dropbox;
};

#endif // MAINWINDOW_H
