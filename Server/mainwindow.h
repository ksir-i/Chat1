#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include<QTcpServer>
#include<QTcpSocket>
#include<QUdpSocket>
#include<QMutex>
#include<QMap>
#include<QList>
#include<QTimer>
#include<QJsonObject>
#include<QJsonDocument>
#include<QQueue>
#include<FileTransferContext.h>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    struct A{
        QTcpSocket *msgsocket=NULL;
        QTcpSocket *filesocket=NULL;
    };
    struct B{
        QMutex *mutex=NULL;;
        QQueue<QByteArray>* queue=NULL;

        B(){
            queue=new QQueue<QByteArray>;
            mutex=new QMutex;
        }
        ~B(){
            delete mutex;
            mutex=NULL;
            delete queue;
            queue=NULL;
        }
    };

    unsigned int port=1210;
    QTcpServer *msgserver=NULL;
    QTcpServer *fileserver=NULL;
    QMap<QString,A>socketmap;
    QMap<int,QQueue<FileTransferContext>*>queuemap;
    QMap<int,QMutex*>mutexmap;
    QMap<int,QString>idmarkmap;
    QMutex mutexmutex;
    QMutex idmarkmutex;
    QMutex socketmapmutex;
    QMutex queuemapmutex;
    QString ip;
    QTimer *timer=NULL;
    qint64 m_fileid=0;
    QMutex fileidmutex;
    QMap<int,B*>offlinemsgmap;
    QMap<int,bool>vismap;
    QMutex vismapmutex;
    QMutex offlinemsgmapmutex;

private:
    void managejson(QByteArray data,QTcpSocket *msgtcp);
    void packintjson(const QJsonObject &json,QTcpSocket *msgtcp);
    void packingfile(const QByteArray &chunk,qint64 fileid,QString mark,int toid);
    void addfilequeue(QQueue<FileTransferContext>*queue,qint64 fileid,QString type,qint64 size,int id);
    void addfriend(QString type,QString mark,int userid,int friendid,QString friendname);
    void onlinesendmsg(int id);
    void sendonlinemsg(int id,QByteArray &data);


private slots:
    void do_msgnewConnection();
    void do_filenewConnection();
    void msgreaddata(QTcpSocket *msgtcp);
    void filereaddata(QTcpSocket *filetcp);

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
