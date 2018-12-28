#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "itemselectionwindow.h"
#include <QDebug>
#include <QMessageBox>
#include <QtNetwork/QNetworkAccessManager>
#include <QThreadPool>
#include <QSslConfiguration>
#include <QNetworkReply>
#include <QNetworkCookieJar>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow) {
    ui->setupUi(this);
    setFixedSize(width(), height());
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::on_pushButton_pressed() {
    ui->informationLabel->setStyleSheet("QLabel {color: #000000;}");
    ui->informationLabel->setText("Connexion...");
    ui->progressBar->setValue(0);
    QString username = ui->usernameTextField->text();
    QString password = ui->passwordTextField->text();
    if(username.size() > 0 && password.size() > 0) {
        connectPlayer(username, password);
    }else{
        QMessageBox::critical(this, "Erreur", "Pseudonyme ou mot de passe incorrect");
    }
}

void MainWindow::connectPlayer(QString &username, QString &password){
    m_currentState = TOKEN;
    m_username = username;
    m_password = password;
    m_manager = new QNetworkAccessManager(this);
    m_manager->setCookieJar(new QNetworkCookieJar(m_manager));

    connect(m_manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(dataReady(QNetworkReply*)));

    QNetworkRequest request(QUrl("https://www.darkorbit.fr"));
    request.setSslConfiguration(QSslConfiguration::defaultConfiguration());

    m_manager->get(request);
}

void MainWindow::connectionFinished(bool isSuccess){
    if(isSuccess){
        ui->progressBar->setValue(80);
        ui->informationLabel->setStyleSheet("QLabel {color: #4CAF50;}");
        ui->informationLabel->setText("Connecté, chargement des objets...");
        ItemSelectionWindow *window = new ItemSelectionWindow(nullptr, m_manager, m_serverID);
        connect(window, SIGNAL(initializationFinished()), this, SLOT(hide()));
    }else{
        ui->progressBar->setValue(100);
        ui->informationLabel->setStyleSheet("QLabel {color: #F44336;}");
        ui->informationLabel->setText("Identifiants invalides");
    }
}

void MainWindow::dataReady(QNetworkReply *reply){

    int code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    ui->progressBar->setValue(ui->progressBar->value()+20);

    if(reply->url().toString().contains("darkorbit.com"))
        m_serverID = reply->url().toString().split("//")[1].split(".")[0];

    if(m_currentState == TOKEN){

        m_currentState = CONNEXION;

        QString token;
        char buf[2048];
        while(reply->readLine(buf, sizeof(buf)) != -1){
            QString line = QString(buf);
            if(line.contains("<form n")){
                QStringList parts = line.split("token=");
                token = parts[1].split("\"")[0];
                break;
            }
        }

        QNetworkRequest request(QUrl("https://sas.bpsecure.com/Sas/Authentication/Bigpoint?authUser=22&token=" + token));
        request.setSslConfiguration(QSslConfiguration::defaultConfiguration());

        QByteArray data = QString("username="+m_username+"&password="+m_password).toUtf8();

        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
        request.setHeader(QNetworkRequest::ContentLengthHeader, data.size());
        m_manager->post(request, data);

    }else if(m_currentState == CONNEXION){
        if(code == 302){
            QNetworkRequest request(QUrl(reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toString()));
            request.setSslConfiguration(QSslConfiguration::defaultConfiguration());
            m_manager->get(request);
        }else{
            m_currentState = FINISHED;
            connectionFinished(reply->url().toString().contains("indexInternal.es"));
        }
        return;
    }

}
