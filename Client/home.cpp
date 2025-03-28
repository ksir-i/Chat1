#include "home.h"
#include "ui_home.h"
#include<QPainter>
#include "myinformation.h"
#include "frienditem.h"
#include<QMessageBox>
#include "addfriend.h"
#include "isfriend.h"
#include "attnitem.h"
#include<QGraphicsDropShadowEffect>
#include<QDateTime>
#include "mysendfile.h"
#include<QFileDialog>
#include<QScrollBar>



Home::Home(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Home)
{
    ui->setupUi(this);
    setWindowFlags(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    ui->widget_3->resize(QSize(300,ui->widget_3->height()));
    ui->widget_7->setObjectName("parentWidget");
    ui->widget_7->setStyleSheet(
        "#parentWidget {"
        "background-color: rgba(255, 255, 255, 0);"
        "border-radius: 15px;"
        "}"
        );
    ui->widget_8->setObjectName("parentWidget");
    ui->widget_8->setStyleSheet(
        "#parentWidget {"
        "background-color: rgba(240, 240, 240, 255);"
        "border-top-right-radius: 15px;"
        "border-bottom-right-radius: 0px;"
        "border-top-left-radius: 0px;"
        "border-bottom-left-radius: 0px;"
        "}"
        );
    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(this);
    shadow->setOffset(0, 0);
    shadow->setColor(QColor("#444444"));
    shadow->setBlurRadius(30);
    ui->widget_7->setGraphicsEffect(shadow);

    chattext=new ChatPlainText(this);
    connect(chattext,&ChatPlainText::sendmsg,this,&Home::on_sendmsg);
    connect(chattext,&ChatPlainText::sendfile,this,&Home::on_sendfile);
    connect(chattext,&ChatPlainText::sendpicture,this,&Home::on_sendpicture);
    chattext->hide();
    ui->pushButton_2->setStyleSheet(
        "QPushButton{"
        "background-color: rgba(255, 255, 255, 0);}"
        "QPushButton:hover {"
        "background-color: rgba(202, 0, 0, 255);"
        "}"
        );
    ui->pushButton_3->setStyleSheet(
        "QPushButton{"
        "background-color: rgba(255, 255, 255, 0);}"
        "QPushButton:hover {"
        "background-color: rgba(202, 0, 0, 255);"
        "}"
        );
    ui->pushButton_4->setStyleSheet(
        "QPushButton{"
        "background-color: rgba(255, 255, 255, 0);}"
        "QPushButton:hover {"
        "background-color: rgba(202, 0, 0, 255);"
        "}"
        );
    prompt=new Prompt;
    prompt->hide();
    connect(&timer,&QTimer::timeout,this,[=](){
        timer.stop();
        prompt->hide();
    });
    friendwidget=new QListWidget(ui->widget_6);
    connect(friendwidget,&QListWidget::itemClicked,this,&Home::widget_5addwidget);
    friendwidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    friendwidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    friendwidget->hide();
    friendwidget->setStyleSheet(
        "border: none;"
        "background-color: rgba(255, 255, 255, 255);"
        );
    attnwidget=new QListWidget(ui->widget_6);
    attnwidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    attnwidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    attnwidget->hide();
    attnwidget->setStyleSheet(
        "border: none;"
        "background-color: rgba(255, 255, 255, 255);"
        );

    selectfriend=new QListWidget(ui->widget_6);
    selectfriend->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    selectfriend->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    selectfriend->hide();
    selectfriend->setStyleSheet(
        "border: none;"
        "background-color: rgba(255, 255, 255, 255);"
        );

    QVBoxLayout *layout = new QVBoxLayout;
    ui->widget_6->setLayout(layout);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout1=new QVBoxLayout;
    ui->widget_5->setLayout(layout1);
    layout1->setContentsMargins(0,0,0,0);
    layout1->setSpacing(0);
    action=new QAction(QIcon(":/icon/13.png"), QString("搜索"), ui->lineEdit);
    ui->lineEdit->addAction(action, QLineEdit::TrailingPosition);
    connect(action,&QAction::triggered,this,[=](){//点击搜索按钮
        pushbuttoninit();
        QString s=ui->lineEdit->text();
        if(s.isEmpty()){
            return;
        }
        if(s.toInt()==id){
            QMessageBox::information(this,"查找用户","输入的是自己的id");
            return;
        }
        int id=s.toInt();
        selectfriend->clear();
        emit selectfriendid(id);
        if(curlistwidget){
            ui->widget_6->layout()->removeWidget(curlistwidget);
            curlistwidget->hide();
        }
        curlistwidget=selectfriend;
        ui->widget_6->layout()->addWidget(selectfriend);
        selectfriend->show();
    });
    on_pushButton_5_clicked();
}

Home::~Home()
{
    delete ui;
}

void Home::on_logonsuccsee(int m_id, QString m_name)
{
    this->id=m_id;
    this->name=m_name;
}

void Home::on_logonsuccseeicon(QByteArray data)
{
    pix.loadFromData(data);
    QPixmap pixmap = pix.scaled(ui->pushButton->size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
    QPixmap circularPixmap(ui->pushButton->size());
    circularPixmap.fill(Qt::transparent);
    QPainter painter(&circularPixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setBrush(QBrush(pixmap));
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(0, 0, ui->pushButton->width(), ui->pushButton->height());
    ui->pushButton->setIcon(circularPixmap);
}


void Home::mousePressEvent(QMouseEvent *event)
{
    if(event->button()==Qt::LeftButton){
        isleft=1;
        point=event->globalPos()-this->pos();//用鼠标点击位置到屏幕左上角的向量减去窗口到屏幕左上角的向量
    }else{
        return;
    }
}

void Home::mouseMoveEvent(QMouseEvent *event)
{
    if(isleft==0){
        return;
    }
    this->move(event->globalPos()-point);
}

void Home::mouseReleaseEvent(QMouseEvent *event)
{
    isleft=0;
    QWidget::mouseReleaseEvent(event);
}



void Home::widget_5addwidget(QListWidgetItem *item)
{
    widget_5clear();
    ui->label->setStyleSheet(
        "color: rgb(0, 0, 0);"
        "background-color: rgba(240, 240, 240, 255);"
        "font: 16pt \"Microsoft YaHei UI\";"
        "border: 0px;"
        "border-bottom: 1px dotted #bebebe;"
        );
    QWidget *widget=friendwidget->itemWidget(item);
    FriendItem *frienditem=qobject_cast<FriendItem*>(widget);
    frienditem->havemsg(0);
    int friendid=frienditem->getid();
    curfriendid=friendid;
    QString friendname=frienditem->getname();
    ui->label->setText("      "+friendname);
    QScrollArea *chatitem=chatmap[friendid];
    ui->widget_5->layout()->addWidget(chatitem);
    ui->widget_5->layout()->addWidget(chattext);
    chatitem->show();
    chattext->show();
    layout1->setStretch(0,2);
    layout1->setStretch(1,1);
    curchatscroll=chatitem;
}

void Home::pushbuttoninit()
{
    ui->pushButton_5->setStyleSheet(
        "background-color: rgba(138, 138, 138,0);"
        "border-radius:4px;"
        );
    ui->pushButton_6->setStyleSheet(
        "background-color: rgba(138, 138, 138,0);"
        "border-radius:4px;"
        );
}


void Home::on_pushButton_4_clicked()
{
    this->close();
}


void Home::on_pushButton_2_clicked()
{
    this->showMinimized();
}


void Home::on_pushButton_clicked()
{
    QWidget *m_pMaskLayer = new QWidget(this);
    m_pMaskLayer->setStyleSheet("background-color: rgba(0, 0, 0, 150);");
    m_pMaskLayer->setGeometry(rect());
    m_pMaskLayer->raise();
    m_pMaskLayer->show();
    Myinformation *myinfor=new Myinformation;
    myinfor->setWindowFlags(myinfor->windowFlags() | Qt::WindowStaysOnTopHint);
    connect(myinfor,&Myinformation::cancelclicked,this,[=](){
        m_pMaskLayer->deleteLater();
        myinfor->deleteLater();
    });
    connect(myinfor,&Myinformation::confirmclicked,this,[=](QString newname){
        m_pMaskLayer->deleteLater();
        myinfor->deleteLater();
        this->name=newname;
        if(m_path.isEmpty()){
            emit updatenameonly(this->id,newname);
        }else{
            emit updateinformation(this->id,newname,m_path);
        }

        m_path="";
    });
    connect(myinfor,&Myinformation::tohomeicon,this,&Home::on_logonsuccseeicon);
    connect(myinfor,&Myinformation::iconchanged,this,[=](QString newpath){
        m_path=newpath;
    });
    myinfor->init(id,name,pix);
    QRect parentGeometry = geometry();
    QSize childSize = myinfor->size();
    int x = parentGeometry.x() + (parentGeometry.width() - childSize.width()) / 2;
    int y = parentGeometry.y() + (parentGeometry.height() - childSize.height()) / 2;
    myinfor->move(x, y);
    myinfor->show();
}


void Home::on_pushButton_5_clicked()
{
    pushbuttoninit();
    ui->pushButton_5->setStyleSheet(
        "background-color: rgba(138, 138, 138,60);"
        "border-radius:4px;"
        );
    if(curlistwidget){
        ui->widget_6->layout()->removeWidget(curlistwidget);
        curlistwidget->hide();
    }
    curlistwidget=friendwidget;
    ui->widget_6->layout()->addWidget(friendwidget);
    friendwidget->show();
}


void Home::on_pushButton_6_clicked()
{
    pushbuttoninit();
    widget_5clear();
    ui->label->setText("");
    ui->pushButton_6->setStyleSheet(
        "background-color: rgba(138, 138, 138,60);"
        "border-radius:4px;"
        );
    if(curlistwidget){
        ui->widget_6->layout()->removeWidget(curlistwidget);
        curlistwidget->hide();
    }
    curlistwidget=attnwidget;
    ui->widget_6->layout()->addWidget(attnwidget);
    attnwidget->show();
}



void Home::friendwidgetadd(int id, QString name, QPixmap pix)//把好友添加到好友列表
{
    if(promptdup(&friendvector,id)){
        return;
    }
    QScrollArea *chatscroll=new QScrollArea(ui->widget_5);
    chatscroll->setStyleSheet(
        "border: none;"
        );
    QWidget *scrwidget=new QWidget;
    chatscroll->setWidgetResizable(true);
    chatscroll->setWidget(scrwidget);
    QVBoxLayout *layout=new QVBoxLayout;
    layout->setSpacing(5);
    scrwidget->setLayout(layout);
    layout->addStretch();
    chatscroll->setWidgetResizable(true);
    chatscroll->hide();
    chatscroll->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    chatscroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    chatmap[id]=chatscroll;
    friendvector.push_back(id);
    FriendItem *frienditem=new FriendItem(this);
    frienditem->init(id,name,pix);
    QListWidgetItem *item=new QListWidgetItem(friendwidget);
    friendwidget->setItemWidget(item,frienditem);
    item->setSizeHint(QSize(300,80));
}

void Home::on_isnullid()
{
    QMessageBox::information(this,"提示","查询的是空id");
}



void Home::selectlistadd(int id, QString name, QPixmap pixmap)
{
    if(promptdup(&friendvector,id)){//如果已经是好友
        IsFriend *isfrienditem=new IsFriend(this);
        isfrienditem->init(id,name,pixmap);
        QListWidgetItem *item=new QListWidgetItem(selectfriend);
        selectfriend->setItemWidget(item,isfrienditem);
        item->setSizeHint(QSize(300,80));
        return;
    }
    AddFriend *addfriend=new AddFriend(this);
    connect(addfriend,&AddFriend::addfriend,this,&Home::on_addfriend);//发出添加好友
    addfriend->initwidget(id,name,pixmap);
    QListWidgetItem *item=new QListWidgetItem(selectfriend);
    selectfriend->setItemWidget(item,addfriend);
    item->setSizeHint(QSize(300,80));
}



bool Home::promptdup(QVector<int> *vec, int dupid)
{
    foreach (auto &it, *vec) {
        if(it==dupid){
            return 1;
        }
    }
    return 0;
}



void Home::on_addfriend(int toid)
{
    addfriendrequest(toid);
}

void Home::on_sendmsg(QString msg)
{
    QScrollArea *scroll = chatmap[curfriendid];
    QWidget *widget = scroll->widget();
    QLabel *label_1=new QLabel(widget);
    label_1->setMinimumSize(QSize(40,40));
    label_1->setMaximumSize(QSize(40,40));
    QPixmap pix = this->pix.scaled(label_1->size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);  // 缩放图片以适应标签大小
    QPixmap circularPixmap(label_1->size());
    circularPixmap.fill(Qt::transparent);
    QPainter painter(&circularPixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setBrush(QBrush(pix));
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(0, 0, label_1->width(), label_1->height());
    label_1->setPixmap(circularPixmap);
    label_1->setAlignment(Qt::AlignCenter);

    QLabel *msglabel=new QLabel(widget);
    msglabel->setTextFormat(Qt::RichText);
    msglabel->setStyleSheet(
        "background-color: #0099ff;"
        "color: white;"
        "border-radius: 8px;"
        "font: 12pt \"Microsoft YaHei UI\";"
        "padding: 8px;"
        );
    msglabel->setMaximumWidth(600);
    msglabel->setWordWrap(1);
    msglabel->setText(replaceEmojisWithImages(msg));
    QHBoxLayout *layout=new QHBoxLayout;
    layout->addStretch();
    layout->addWidget(msglabel);
    layout->addWidget(label_1);
    QVBoxLayout *mainLayout = qobject_cast<QVBoxLayout*>(widget->layout());
    mainLayout->insertLayout(mainLayout->count() - 1,layout);
    emit sendtofriend(curfriendid,msg,this->id);
    for (int i = 0; i < friendwidget->count(); ++i) {
        QListWidgetItem* item = friendwidget->item(i);
        if (!item) continue;
        QWidget* widget = friendwidget->itemWidget(item);
        FriendItem* friendItem = qobject_cast<FriendItem*>(widget);
        if (friendItem->getid()==curfriendid) {
            friendItem->setmsg(msg,QDateTime::currentDateTime().toString("HH:mm"));
            break;
        }
    }
    QTimer::singleShot(20, [scroll]() {
        QScrollBar *scrollBar = scroll->verticalScrollBar();
        scrollBar->setValue(scrollBar->maximum());
    });
}



void Home::on_addfriendrequest(int id, QString name, QPixmap pix)
{
    if(promptdup(&promptvector,id)){
        return;
    }
    promptvector.push_back(id);
    timer.start(3000);
    prompt->setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    prompt->resize(300, 150);
    QScreen* screen = prompt->screen();
    QRect screenGeometry = screen->availableGeometry();
    int x = screenGeometry.right() - prompt->width() - 10;
    int y = screenGeometry.bottom() - prompt->height() - 10;
    prompt->move(x, y);
    prompt->show();
    prompt->activateWindow();
    AttnItem *additem=new AttnItem(this);
    connect(additem,&AttnItem::agreeclicked,this,[=](int addid,QString addname,QPixmap addpix){//同意添加好友
        deleteid(addid);
        emit agreeaddfriend(addid);
        friendwidgetadd(addid,addname,addpix);
        QListWidgetItem* item = findItemByAttnItem(attnwidget, additem);
        if (item) {
            int row = attnwidget->row(item);
            QListWidgetItem* takenItem = attnwidget->takeItem(row);

            if (takenItem) {
                delete attnwidget->itemWidget(takenItem);
                delete takenItem;
            }
        }
    });
    connect(additem,&AttnItem::disagree,this,[=](int userid){//不同意好友申请
        deleteid(userid);
        QListWidgetItem* item = findItemByAttnItem(attnwidget, additem);
        if (item) {
            int row = attnwidget->row(item);
            QListWidgetItem* takenItem = attnwidget->takeItem(row);

            if (takenItem) {
                delete attnwidget->itemWidget(takenItem);
                delete takenItem;
            }
        }
    });
    additem->init(id,name,pix);
    QListWidgetItem *item=new QListWidgetItem(attnwidget);
    attnwidget->setItemWidget(item,additem);
    item->setSizeHint(QSize(300,80));
}

void Home::on_receivesendmsg(int fromid, QString msg)
{
    QPixmap friendpix;
    for (int i = 0; i < friendwidget->count(); ++i) {
        QListWidgetItem* item = friendwidget->item(i);
        if (!item) continue;
        QWidget* widget = friendwidget->itemWidget(item);
        FriendItem* friendItem = qobject_cast<FriendItem*>(widget);
        if (friendItem->getid()==fromid) {
            friendpix=friendItem->getpixmap();
            friendItem->setmsg(msg,QDateTime::currentDateTime().toString("HH:mm"));
            if(curfriendid!=fromid){
                friendItem->havemsg(1);
            }
            break;
        }
    }
    QScrollArea *scroll = chatmap[fromid];
    QWidget *widget = scroll->widget();
    QLabel *label_1=new QLabel(widget);

    label_1->setMinimumSize(QSize(40,40));
    label_1->setMaximumSize(QSize(40,40));
    QPixmap pix = friendpix.scaled(label_1->size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);  // 缩放图片以适应标签大小
    QPixmap circularPixmap(label_1->size());
    circularPixmap.fill(Qt::transparent);
    QPainter painter(&circularPixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setBrush(QBrush(pix));
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(0, 0, label_1->width(), label_1->height());
    label_1->setPixmap(circularPixmap);
    label_1->setAlignment(Qt::AlignCenter);

    QLabel *msglabel=new QLabel(widget);
    msglabel->setTextFormat(Qt::RichText);
    msglabel->setStyleSheet(
        "background-color: rgb(255,255,255);"
        "color: black;"
        "border-radius: 8px;"
        "font: 12pt \"Microsoft YaHei UI\";"
        "padding: 8px;"
        );
    msglabel->setMaximumWidth(600);
    msglabel->setWordWrap(1);
    msglabel->setText(replaceEmojisWithImages(msg));
    QHBoxLayout *layout=new QHBoxLayout;
    layout->addWidget(label_1);
    layout->addWidget(msglabel);
    layout->addStretch();
    QVBoxLayout *mainLayout = qobject_cast<QVBoxLayout*>(widget->layout());
    mainLayout->insertLayout(mainLayout->count() - 1,layout);
    QTimer::singleShot(20, [scroll]() {
        QScrollBar *scrollBar = scroll->verticalScrollBar();
        scrollBar->setValue(scrollBar->maximum());
    });
}

void Home::on_mysendsize(int value)
{
    if(sendingfile!=NULL){
        sendingfile->setvalue(value);
    }
}

void Home::on_friendsendfilesize(int value)
{
    receivingfile->setvalue(value);
}

void Home::on_frinedsendfile(qint64 size, QString filename, QString suffix,int fromid)
{
    QPixmap friendpix;
    for (int i = 0; i < friendwidget->count(); ++i) {
        QListWidgetItem* item = friendwidget->item(i);
        if (!item) continue;
        QWidget* widget = friendwidget->itemWidget(item);
        FriendItem* friendItem = qobject_cast<FriendItem*>(widget);
        if (friendItem->getid()==fromid) {
            friendpix=friendItem->getpixmap();
            friendItem->setmsg("[文件]",QDateTime::currentDateTime().toString("HH:mm"));
            if(curfriendid!=fromid){
                friendItem->havemsg(1);
            }
            break;
        }
    }
    QScrollArea *scroll = chatmap[fromid];
    QWidget *widget = scroll->widget();
    FriendSendfile *friendsendfile=new FriendSendfile(widget);
    receivingfile=friendsendfile;
    friendsendfile->setfile(filename,size,suffix);
    QLabel *label_1=new QLabel(widget);
    label_1->setMinimumSize(QSize(40,40));
    label_1->setMaximumSize(QSize(40,40));
    QPixmap pix = friendpix.scaled(label_1->size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);  // 缩放图片以适应标签大小
    QPixmap circularPixmap(label_1->size());
    circularPixmap.fill(Qt::transparent);
    QPainter painter(&circularPixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setBrush(QBrush(pix));
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(0, 0, label_1->width(), label_1->height());
    label_1->setPixmap(circularPixmap);
    label_1->setAlignment(Qt::AlignCenter);

    QHBoxLayout *layout=new QHBoxLayout;
    layout->addWidget(label_1);
    layout->addWidget(friendsendfile);
    layout->addStretch();
    QVBoxLayout *mainLayout = qobject_cast<QVBoxLayout*>(widget->layout());
    mainLayout->insertLayout(mainLayout->count() - 1,layout);
}

void Home::on_friendsendpicture(int fromid, QByteArray data,int h,int w)
{
    QPixmap pix;
    pix.loadFromData(data);
    QPixmap friendpix;
    for (int i = 0; i < friendwidget->count(); ++i) {
        QListWidgetItem* item = friendwidget->item(i);
        if (!item) continue;
        QWidget* widget = friendwidget->itemWidget(item);
        FriendItem* friendItem = qobject_cast<FriendItem*>(widget);
        if (friendItem->getid()==fromid) {
            friendpix=friendItem->getpixmap();
            friendItem->setmsg("[图片]",QDateTime::currentDateTime().toString("HH:mm"));
            if(curfriendid!=fromid){
                friendItem->havemsg(1);
            }
            break;
        }
    }
    QScrollArea *scroll = chatmap[fromid];
    QWidget *widget = scroll->widget();
    QLabel *label=new QLabel(widget);
    const int maxDisplayWidth = 200;
    if (w > maxDisplayWidth) {
        label->setFixedWidth(maxDisplayWidth);
        label->setFixedHeight(h * maxDisplayWidth / w);
    } else {
        label->setFixedWidth(w);
        label->setFixedHeight(h);
    }
    label->setStyleSheet(
        "background-color:rgba(255,255,255,0);"
        );
    label->setScaledContents(1);
    label->setPixmap(pix);
    QLabel *label_1=new QLabel(widget);
    label_1->setMinimumSize(QSize(40,40));
    label_1->setMaximumSize(QSize(40,40));
    QPixmap pix1 = friendpix.scaled(label_1->size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);  // 缩放图片以适应标签大小
    QPixmap circularPixmap1(label_1->size());
    circularPixmap1.fill(Qt::transparent);
    QPainter painter1(&circularPixmap1);
    painter1.setRenderHint(QPainter::Antialiasing);
    painter1.setBrush(QBrush(pix1));
    painter1.setPen(Qt::NoPen);
    painter1.drawEllipse(0, 0, label_1->width(), label_1->height());
    label_1->setPixmap(circularPixmap1);
    label_1->setAlignment(Qt::AlignCenter);
    QHBoxLayout *layout=new QHBoxLayout;
    layout->addWidget(label_1);
    layout->addWidget(label);
    layout->addStretch();
    QVBoxLayout *mainLayout = qobject_cast<QVBoxLayout*>(widget->layout());
    mainLayout->insertLayout(mainLayout->count() - 1,layout);
}

void Home::on_friendpicturechanged(int friendid, QByteArray data)
{
    QPixmap pixmap;
    pixmap.loadFromData(data);
    for (int i = 0; i < friendwidget->count(); ++i) {
        QListWidgetItem* item = friendwidget->item(i);
        if (!item) continue;
        QWidget* widget = friendwidget->itemWidget(item);
        FriendItem* friendItem = qobject_cast<FriendItem*>(widget);
        if (friendItem->getid()==friendid) {
            friendItem->picturechanged(pixmap);
            break;
        }
    }
}

void Home::on_friendnamechanged(int friendid, QString friendname)
{
    for (int i = 0; i < friendwidget->count(); ++i) {
        QListWidgetItem* item = friendwidget->item(i);
        if (!item) continue;
        QWidget* widget = friendwidget->itemWidget(item);
        FriendItem* friendItem = qobject_cast<FriendItem*>(widget);
        if (friendItem->getid()==friendid) {
            friendItem->namechanged(friendname);
            break;
        }
    }
}



void Home::widget_5clear()
{
    ui->label->setStyleSheet(
        "color: rgb(0, 0, 0);"
        "background-color: rgba(240, 240, 240, 255);"
        "font: 16pt \"Microsoft YaHei UI\";"
        );
    if(curchatscroll){
        ui->widget_5->layout()->removeWidget(curchatscroll);
        ui->widget_5->layout()->removeWidget(chattext);
        chattext->hide();
        curchatscroll->hide();
    }
}


void Home::closeEvent(QCloseEvent *event)
{
    emit closehome();
    QWidget::closeEvent(event);
}



void Home::deleteid(int id)
{
    for(int i=0;i<promptvector.size();i++){
        if(promptvector[i]==id){
            promptvector.removeAt(i);
            return;
        }
    }
}



QListWidgetItem *Home::findItemByAttnItem(QListWidget *listWidget, AttnItem *targetAttnItem)
{
    if (!listWidget || !targetAttnItem) return NULL;
    for (int i = 0; i < listWidget->count(); ++i) {
        QListWidgetItem* item = listWidget->item(i);
        if (!item) continue;
        QWidget* widget = listWidget->itemWidget(item);
        AttnItem* attnItem = qobject_cast<AttnItem*>(widget);
        if (attnItem == targetAttnItem) {
            return item;
        }
    }
    return NULL;
}

QString Home::replaceEmojisWithImages(const QString &msg)
{
    QString richText = msg.toHtmlEscaped();
    richText.replace("\n", "<br>");
    for (auto it = EMOJI_MAP.constBegin(); it != EMOJI_MAP.constEnd(); ++it) {
        const QString& emoji = it.key();
        const QString& imagePath = it.value();
        richText.replace(emoji, QString("<img src='%1' width='20' height='20'/>").arg(imagePath));
    }

    return richText;
}



void Home::on_sendfile()
{
    QString filepath=QFileDialog::getOpenFileName(this);
    if(filepath.isEmpty()){
        return;
    }
    emit sendfiletofriend(curfriendid,filepath);
    QScrollArea *scroll = chatmap[curfriendid];
    QWidget *widget = scroll->widget();
    MySendfile *mysendfile=new MySendfile(widget);
    sendingfile=mysendfile;
    mysendfile->setfileicon(filepath);
    QLabel *label_1=new QLabel(widget);
    label_1->setMinimumSize(QSize(40,40));
    label_1->setMaximumSize(QSize(40,40));
    QPixmap pix = this->pix.scaled(label_1->size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);  // 缩放图片以适应标签大小
    QPixmap circularPixmap(label_1->size());
    circularPixmap.fill(Qt::transparent);
    QPainter painter(&circularPixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setBrush(QBrush(pix));
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(0, 0, label_1->width(), label_1->height());
    label_1->setPixmap(circularPixmap);
    label_1->setAlignment(Qt::AlignCenter);

    QHBoxLayout *layout=new QHBoxLayout;
    layout->addStretch();
    layout->addWidget(mysendfile);
    layout->addWidget(label_1);
    QVBoxLayout *mainLayout = qobject_cast<QVBoxLayout*>(widget->layout());
    mainLayout->insertLayout(mainLayout->count() - 1,layout);
    for (int i = 0; i < friendwidget->count(); ++i) {
        QListWidgetItem* item = friendwidget->item(i);
        if (!item) continue;
        QWidget* widget = friendwidget->itemWidget(item);
        FriendItem* friendItem = qobject_cast<FriendItem*>(widget);
        if (friendItem->getid()==curfriendid) {
            friendItem->setmsg("[文件]",QDateTime::currentDateTime().toString("HH:mm"));
            break;
        }
    }
}

void Home::on_sendpicture()
{
    QString path=QFileDialog::getOpenFileName(this,"选择图片",QDir::homePath(),"图片文件 (*.png *.jpg *.jpeg *.bmp *.gif)");
    if (path.isEmpty()) {
        return;
    }
    emit sendpicture(path,curfriendid);
    QScrollArea *scroll = chatmap[curfriendid];
    QWidget *widget = scroll->widget();
    QLabel *label=new QLabel(widget);
    const int maxDisplayWidth = 200;
    QPixmap pixmap(path);
    if (!pixmap.isNull()) {
        if (pixmap.width() > maxDisplayWidth) {
            label->setFixedWidth(maxDisplayWidth);
            label->setFixedHeight(pixmap.height() * maxDisplayWidth / pixmap.width());
        } else {
            label->setFixedWidth(pixmap.width());
            label->setFixedHeight(pixmap.height());
        }
    }
    label->setStyleSheet(
        "background-color:rgba(255,255,255,0);"
        );
    label->setScaledContents(1);
    label->setPixmap(pixmap);
    QLabel *label_1=new QLabel(widget);
    label_1->setMinimumSize(QSize(40,40));
    label_1->setMaximumSize(QSize(40,40));
    QPixmap pix1 = this->pix.scaled(label_1->size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);  // 缩放图片以适应标签大小
    QPixmap circularPixmap1(label_1->size());
    circularPixmap1.fill(Qt::transparent);
    QPainter painter1(&circularPixmap1);
    painter1.setRenderHint(QPainter::Antialiasing);
    painter1.setBrush(QBrush(pix1));
    painter1.setPen(Qt::NoPen);
    painter1.drawEllipse(0, 0, label_1->width(), label_1->height());
    label_1->setPixmap(circularPixmap1);
    label_1->setAlignment(Qt::AlignCenter);
    QHBoxLayout *layout=new QHBoxLayout;
    layout->addStretch();
    layout->addWidget(label);
    layout->addWidget(label_1);
    QVBoxLayout *mainLayout = qobject_cast<QVBoxLayout*>(widget->layout());
    mainLayout->insertLayout(mainLayout->count() - 1,layout);
    for (int i = 0; i < friendwidget->count(); ++i) {
        QListWidgetItem* item = friendwidget->item(i);
        if (!item) continue;
        QWidget* widget = friendwidget->itemWidget(item);
        FriendItem* friendItem = qobject_cast<FriendItem*>(widget);
        if (friendItem->getid()==curfriendid) {
            friendItem->setmsg("[图片]",QDateTime::currentDateTime().toString("HH:mm"));
            break;
        }
    }
}



