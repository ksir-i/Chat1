#ifndef FRIENDITEM_H
#define FRIENDITEM_H

#include <QWidget>

namespace Ui {
class FriendItem;
}

class FriendItem : public QWidget
{
    Q_OBJECT

public:
    explicit FriendItem(QWidget *parent = nullptr);
    ~FriendItem();
    int id;
    QString name;
    QPixmap m_pix;

public:
    void init(int id,QString name,QPixmap pix);
    int getid();
    QString getname();
    QPixmap getpixmap();
    void havemsg(bool isok);
    void setmsg(QString msg,QString curtime);
    void picturechanged(QPixmap pix);
    void namechanged(QString name);
private:
    Ui::FriendItem *ui;
};

#endif // FRIENDITEM_H
