#include "botwindow.h"
#include "ui_botwindow.h"
#include <QTime>

BotWindow::BotWindow(ItemSelectionWindow *parent, QString itemName, int itemIndex, QPixmap image) :
    QMainWindow(parent),
    m_parent(parent),
    m_itemIndex(itemIndex),
    m_amount(10000),
    m_bidPlaced(false),
    m_timer(new QTimer(this)),
    ui(new Ui::BotWindow),
    m_timeBeforeAuction(0,0,20)
{
    setAttribute(Qt::WA_DeleteOnClose);
    ui->setupUi(this);
    ui->creditsTextField->setValidator(new QIntValidator(10000, 100000000000));
    ui->itemImageLabel->setScaledContents(true);
    m_timer->setInterval(1000);
    m_timer->start();
    refresh();
    connect(m_timer, SIGNAL(timeout()), this, SLOT(refresh()));
    ui->itemNameLabel->setText(itemName);
    ui->itemImageLabel->setPixmap(image);
    setFixedSize(width(), height());
}

BotWindow::~BotWindow() {
    delete m_timer;
    delete ui;
}
void BotWindow::refresh() {
    int secondRemaining = 60 - QTime::currentTime().second() - m_timeBeforeAuction.second();
    int minuteRemaining = ((QTime::currentTime().minute() < 25) ? 24 : 84) - QTime::currentTime().minute() - m_timeBeforeAuction.minute();

    if(secondRemaining < 0){
        secondRemaining += 60;
        minuteRemaining--;
    }else if(secondRemaining > 60){
        secondRemaining -=60;
        minuteRemaining++;
    }
    if(minuteRemaining < 0)
        minuteRemaining += 60;
    else if(minuteRemaining > 60)
        minuteRemaining -= 60;
    QTime timeRemaining(0, minuteRemaining, secondRemaining);
    if(minuteRemaining == 0 && secondRemaining == 0){
        m_parent->placeBid(m_amount, m_itemIndex);
    }
    ui->timeLabel->setText(timeRemaining.toString("mm:ss"));
    ui->progressBar->setValue(60-minuteRemaining);
}

void BotWindow::on_timeEdit_timeChanged(const QTime &time) {
    m_timeBeforeAuction = time;
    refresh();
}

void BotWindow::on_creditsTextField_textEdited(const QString &arg1) {
    m_amount = arg1.toInt();
}
