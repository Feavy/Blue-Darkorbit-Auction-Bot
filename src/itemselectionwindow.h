#ifndef ITEMSELECTIONWINDOW_H
#define ITEMSELECTIONWINDOW_H

#include <QMainWindow>
#include <QList>
#include <QImage>
#include <QPixmap>
#include <QNetworkAccessManager>
#include <QTimer>

namespace Ui {
class ItemSelectionWindow;
}

class ItemSelectionWindow : public QMainWindow
{
    Q_OBJECT
public:
    struct AuctionItem{
        QString name;
        QString owner;
        QString lootID;
        QString itemId;
        QString bid;
        QUrl imageUrl;
    };
    struct PlaceBidRequest {
        QUrl requestUrl;
        QString reloadToken;
        QString lootId;
        QString itemId;
        int credits;
    };
    explicit ItemSelectionWindow(QMainWindow *parent, QNetworkAccessManager *manager, QString serverID);
    ~ItemSelectionWindow();
signals:
    void initializationFinished();
public slots:
    void dataReceived(QNetworkReply *reply);
    void placeBid(int amount, int itemIndex);
private slots:
    void on_comboBox_currentIndexChanged(int index);
    void on_refreshButton_clicked();
    void timerTimeout();
    void refreshTimerTimeout();
    void on_startButton_clicked();

private:
    Ui::ItemSelectionWindow *ui;
    QNetworkAccessManager *m_manager;
    QTimer *m_timer;
    QTimer *m_refreshTimer;

    QString m_serverID;

    PlaceBidRequest m_placeBidRequest;

    bool m_comboBoxImagesReady;
    bool m_comboBoxInformationsReady;
    bool m_initialized;
    bool m_iconsInitialized;
    QList<AuctionItem> m_items;
    QMap<int, QPixmap> m_icons;

    QMap<QNetworkReply*, int> m_currentRequests;

    void refreshInformations();
    void refreshInformationsEnd(bool wasBidRequest);
};

#endif // ITEMSELECTIONWINDOW_H
