#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QProgressBar>
#include <QNetworkAccessManager>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
private slots:
    void on_pushButton_pressed();
    void dataReady(QNetworkReply *reply);
private:
    enum STATE{TOKEN, CONNEXION, FINISHED};
    void connectPlayer(QString &username, QString &password);
    void connectionFinished(bool isSuccess);
    QString m_username;
    QString m_password;
    QString m_serverID;
    STATE m_currentState;
    QNetworkAccessManager *m_manager;
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
