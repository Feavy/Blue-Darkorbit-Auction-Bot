#ifndef BOTWINDOW_H
#define BOTWINDOW_H

#include "itemselectionwindow.h"
#include <QMainWindow>
#include <QTimer>

namespace Ui {
class BotWindow;
}

class BotWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit BotWindow(ItemSelectionWindow *parent, QString itemName, int itemIndex, QPixmap image);
    ~BotWindow();
private slots:
    void refresh();
    void on_timeEdit_timeChanged(const QTime &time);
    void on_creditsTextField_textEdited(const QString &arg1);
private:
    ItemSelectionWindow *m_parent;
    Ui::BotWindow *ui;
    QTimer *m_timer;

    QTime m_timeBeforeAuction;

    int m_itemIndex;

    int m_amount;
    bool m_bidPlaced;
};

#endif // BOTWINDOW_H
