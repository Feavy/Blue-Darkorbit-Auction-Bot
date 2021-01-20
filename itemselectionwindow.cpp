#include "itemselectionwindow.h"
#include "ui_itemselectionwindow.h"
#include "botwindow.h"
#include <QDebug>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <random>
#include <ctime>
#include <QMessageBox>

ItemSelectionWindow::ItemSelectionWindow(QMainWindow *parent, QNetworkAccessManager *manager, QString serverID) :
    QMainWindow(parent),
    ui(new Ui::ItemSelectionWindow),
    m_timer(new QTimer(this)),
    m_refreshTimer(new QTimer(this)),
    m_manager(manager),
    m_serverID(serverID),
    m_placeBidRequest(),
    m_comboBoxImagesReady(false),
    m_comboBoxInformationsReady(false),
    m_initialized(false),
    m_iconsInitialized(false)
{
    srand(time(0));
    ui->setupUi(this);
    ui->serverLabel->setText("Serveur : "+serverID);
    ui->label_2->setScaledContents(true);
    setFixedSize(width(), height());
    connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(dataReceived(QNetworkReply*)));
    connect(m_refreshTimer, SIGNAL(timeout()), this, SLOT(refreshTimerTimeout()));
    connect(m_timer, SIGNAL(timeout()), this, SLOT(timerTimeout()));
    refreshInformations();
}

ItemSelectionWindow::~ItemSelectionWindow() {
    delete ui;
}

void ItemSelectionWindow::dataReceived(QNetworkReply *reply){
    bool terminated{false};
    char buf[2048];
    int size = 0;
    int i = 0;

    qDebug() << reply->url();

    if(reply->url().toString().contains("png")){
        QByteArray data;
        while((size = reply->readLine(buf, sizeof(buf))) != 0){
            data.append(buf, size);
        }
        m_icons.insert(m_currentRequests.find(reply).value(), QPixmap::fromImage(QImage::fromData(data, "png")));
        if(reply->url().toString() == m_items.at(m_items.size()-1).imageUrl.toString()){
            m_iconsInitialized = true;
            on_comboBox_currentIndexChanged(ui->comboBox->currentIndex());
        }
        return;
    }else if(reply->url().toString().contains("Error")){
        QMessageBox::critical(this, "Erreur", "Impossible de surenchérir : La session a expiré.");
        return;
    }

    while((size = reply->readLine(buf, sizeof(buf))) != 0){
        QString line{buf};
        if(line.toLower().contains("captcha") && line.contains("=")){
            if(line.trimmed().split("=")[1].contains("true")){
                QMessageBox::critical(this, "Erreur", "Impossible de surenchérir : Vous devez compléter un captcha.");
            }
        }
        if(terminated){
            if(line.contains("placeBid")){
                QString path{line.split("\"")[5]};
                m_placeBidRequest.requestUrl = QUrl("https://"+m_serverID+".darkorbit.com"+path);
                m_placeBidRequest.reloadToken = line.split("\"")[13];
                break;
            }
            continue;
        }
        if(line.contains("header_credits")){
            reply->readLine(buf, sizeof(buf));
            line = QString(buf);
            ui->creditsAmountLabel->setText("Crédits : "+line.trimmed());
        }else if(line.contains("firstColumn")){
           reply->readLine(buf, sizeof(buf));
           line = QString{buf};
           QUrl imageUrl = QUrl{line.split("\"")[1]};
           if(i == m_items.size()){
                AuctionItem item;
                item.imageUrl = imageUrl;
                m_items.push_back(item);
           }else
                m_items[i].imageUrl = imageUrl;
        }else if(line.contains("auction_item_name_col")){
            reply->readLine(buf, sizeof(buf));
            line = QString{buf};
            m_items[i].name = line.trimmed();
        }else if(line.contains("td class=\"auction_item_highest\"")){
            reply->readLine(buf, sizeof(buf));
            reply->readLine(buf, sizeof(buf));
            reply->readLine(buf, sizeof(buf));
            line = QString{buf};
            m_items[i].owner = line.trimmed();
        }else if(line.contains("<td class=\"auction_item_current\"")){
            reply->readLine(buf, sizeof(buf));
            line = QString{buf};
            m_items[i].bid = line.trimmed();
        }else if(line.contains("lootId")){
            m_items[i].lootID = line.split("\"")[5];
            i++;
        }else if(line.contains("auction_list_history")){
            terminated = true;
        }
    }
    refreshInformationsEnd(reply->url().toString().contains("reloadToken"));
}

void ItemSelectionWindow::refreshInformations(){
    ui->refreshButton->setEnabled(false);
    ui->startButton->setEnabled(false);
    QNetworkRequest request{QUrl("https://"+m_serverID+".darkorbit.com/indexInternal.es?action=internalAuction")};
    request.setSslConfiguration(QSslConfiguration::defaultConfiguration());
    m_manager->get(request);
}

void ItemSelectionWindow::refreshInformationsEnd(bool wasBidRequest){
    if(!m_initialized){
        ui->refreshButton->setEnabled(true);
        ui->startButton->setEnabled(true);
        m_refreshTimer->setInterval(60000);
        m_refreshTimer->start();
        m_initialized = true;
        for(int i = 0; i < m_items.size(); i++){
            AuctionItem item = m_items.at(i);
            ui->comboBox->addItem(item.name);
            QNetworkRequest request{item.imageUrl};
            request.setSslConfiguration(QSslConfiguration::defaultConfiguration());
            QNetworkReply *reply = m_manager->get(request);
            m_currentRequests.insert(reply, i);
        }
        showNormal();
        emit initializationFinished();
    }else if(!wasBidRequest){
        ui->refreshButton->setEnabled(true);
        ui->startButton->setEnabled(true);
    }
    on_comboBox_currentIndexChanged(ui->comboBox->currentIndex());
}

void ItemSelectionWindow::on_refreshButton_clicked() {
    refreshInformations();
}

void ItemSelectionWindow::on_comboBox_currentIndexChanged(int index) {
    if(m_initialized){
        AuctionItem item = m_items.at(index);
        ui->ownerLabel->setText(item.owner);
        ui->bidLabel->setText(item.bid);
    }
    if(m_iconsInitialized){
        ui->label_2->setPixmap(m_icons.find(index).value());
    }
}

void ItemSelectionWindow::placeBid(int amount, int itemIndex){
    AuctionItem item = m_items.at(itemIndex);
    QNetworkRequest request{m_placeBidRequest.requestUrl};
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    request.setSslConfiguration(QSslConfiguration::defaultConfiguration());
    QByteArray data = QString("reloadToken="+m_placeBidRequest.reloadToken+"&auctionType=hour&subAction=bid&lootId="+item.lootID+"&itemId="+item.itemId+"&credits="+QString::number(amount)).toUtf8();
    m_manager->post(request, data);
    m_timer->start(10000);
}

void ItemSelectionWindow::timerTimeout(){
    ui->startButton->setEnabled(true);
    ui->refreshButton->setEnabled(true);
    m_timer->stop();
}

void ItemSelectionWindow::refreshTimerTimeout(){
    if(QTime::currentTime().minute() == 50){
        refreshInformations();
    }
}

void ItemSelectionWindow::on_startButton_clicked() {
    BotWindow *window = new BotWindow(this, ui->comboBox->currentText(), ui->comboBox->currentIndex(), m_icons[ui->comboBox->currentIndex()]);
    window->show();
}
