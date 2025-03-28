#include "frienditem.h"
#include "ui_frienditem.h"
#include<QPainter>

FriendItem::FriendItem(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::FriendItem)
{
    ui->setupUi(this);
}

FriendItem::~FriendItem()
{
    delete ui;
}
void FriendItem::init(int id, QString name, QPixmap pix)
{
    this->id=id;
    this->name=name;
    m_pix=pix;
    ui->label_2->setText(name);
    QPixmap pixmap = pix.scaled(ui->label->size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);  // 缩放图片以适应标签大小
    QPixmap circularPixmap(ui->label->size());
    circularPixmap.fill(Qt::transparent);
    QPainter painter(&circularPixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setBrush(QBrush(pixmap));
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(0, 0, ui->label->width(), ui->label->height());
    ui->label->setPixmap(circularPixmap);
    ui->label->setAlignment(Qt::AlignCenter);

}

int FriendItem::getid()
{
    return this->id;
}

QString FriendItem::getname()
{
    return this->name;
}

QPixmap FriendItem::getpixmap()
{
    return this->m_pix;
}

void FriendItem::havemsg(bool isok)
{
    if(isok){
        ui->label_3->setPixmap(QPixmap(":/icon/19.png"));
    }else{
        QPixmap pix;
        ui->label_3->setPixmap(pix);
    }
}

void FriendItem::setmsg(QString msg, QString curtime)
{
    ui->label_4->setText(msg);
    ui->label_5->setText(curtime);
}

void FriendItem::picturechanged(QPixmap pix)
{
    m_pix=pix;
    QPixmap pixmap = pix.scaled(ui->label->size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);  // 缩放图片以适应标签大小
    QPixmap circularPixmap(ui->label->size());
    circularPixmap.fill(Qt::transparent);
    QPainter painter(&circularPixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setBrush(QBrush(pixmap));
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(0, 0, ui->label->width(), ui->label->height());
    ui->label->setPixmap(circularPixmap);
    ui->label->setAlignment(Qt::AlignCenter);
}

void FriendItem::namechanged(QString name)
{
    ui->label_2->setText(name);
}
