#ifndef SENDDATA_H
#define SENDDATA_H

#include <QObject>
#include<QTcpSocket>
#include<QUdpSocket>
#include<QQueue>
#include<QMutex>
#include<QJsonDocument>
#include<QJsonObject>
#include<QPixmap>
#include<FileTransferContext.h>
#include<QFile>

class SendData : public QObject
{
    Q_OBJECT
public:
    explicit SendData(QObject *parent = nullptr);

private:
    QUdpSocket *udpsocket=NULL;
    QTcpSocket *msgsocket=NULL;
    QTcpSocket *filesocket=NULL;
    QQueue<QByteArray>msgqueue;
    QMutex msgmutex;
    QQueue<FileTransferContext> filequeue;
    QQueue<BeforFiledata>beforfilequeue;
    QMutex beforfilemutex;
    QMutex filemutex;
    QString mark;
    int id;
    QString name;
    QPixmap profilephoto;

private:
    void getudpsocket();
    void connectToServer(QString ip,int port);
    void packingjson(const QByteArray &m_data);
Q_INVOKABLE void handleMessages();
    void processJson(const QJsonObject &json);
void addfilequeue(qint64 fileid,QString type,qint64 size,int friendid=0,QString friendname="",QString path="",int h=0,int w=0);
    BeforFiledata* selectfileid(qint64 fileid);

public:
    void init();

private slots:
    void on_msgreadyRead();
    void on_filereadyRead();


public slots:
    void on_userlogon(int id,QString password);
    void on_login(QString name,QString password);
    void on_updateinformation(int id,QString name,QString path);
    void on_updatenameonly(int id,QString name);
    void on_selectfriendid(int id);
    void on_addfriendrequest(int toid);
    void on_agreeaddfriend(int addid);
    void on_sendtofriend(int toid,QString msg,int fromid);
    void on_sendfiletofriend(int toid,QString path);
    void on_sendpicture(QString path,int toid);
    void on_closehome();

public:signals:
    void logonsuccsee(int id,QString name);
    void logonfailure();
    void logonsuccseeicon(QByteArray icon);
    void loginsuccsee(int id,QString name);
    void loginfailure();
    void loginsuccseeicon(QByteArray icon);
    void initfriend(int id,QString name,QByteArray data);
    void isnullid();
    void showselectuser(int id,QString name,QByteArray data);
    void showfriendrequest(int id,QString name,QByteArray data);
    void receivesendmsg(int fromid,QString msg);
    void mysendsize(int value);
    void friendsendfilesize(int value);
    void frinedsendfile(qint64 size,QString filename,QString suffix,int fromid);
    void friendsendpicture(int fromid,QByteArray data,int h,int w);
    void deletesocket();
    void logoning();
    void friendpicturechanged(int friendid,QByteArray data);
    void friendnamechanged(int friendid,QString friendname);
};

#endif // SENDDATA_H
