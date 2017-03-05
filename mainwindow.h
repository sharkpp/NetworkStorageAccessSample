#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class Dropbox;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    void tokensLoad();
    void tokensSave();

    void appendLog(const QString& newLog);

private slots:
    void on_authDropbox_clicked();
    void on_runBenchmark_clicked();

private:
    Ui::MainWindow *ui;
    Dropbox* dropbox;
};

#endif // MAINWINDOW_H
