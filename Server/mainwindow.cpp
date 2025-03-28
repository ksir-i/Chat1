#include "mainwindow.h"
#include "ui_mainwindow.h"
#include<QNetworkInterface>
#include<QMessageBox>
#include<QtEndian>
#include<linksql.h>
#include<QDateTime>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QList<QHostAddress>addlist=QNetworkInterface::allAddresses();//获取局域网中的IPV4地址
    foreach (auto &var, addlist) {
        if( var.protocol()==QAbstractSocket::IPv4Protocol){
            if(var.toString().startsWith("192.168.")){
                ip=var.toString();
                break;
            }
        }
    }

    if(ip.isEmpty()){
        QMessageBox::warning(this,"注意","没有提取到服务器在局域网中的IPV4地址");
    }

    msgserver=new QTcpServer(this);
    msgserver->listen(QHostAddress::Any,port);
    connect(msgserver,&QTcpServer::newConnection,this,&MainWindow::do_msgnewConnection);
    fileserver=new QTcpServer(this);
    fileserver->listen(QHostAddress::Any,port+1);
    connect(fileserver,&QTcpServer::newConnection,this,&MainWindow::do_filenewConnection);

    timer=new QTimer(this);
    connect(timer,&QTimer::timeout,this,[=](){//通过udp广播服务器ip地址和端口号
        QByteArray data=QString("SERVER:%1:%2").arg(ip).arg(msgserver->serverPort()).toUtf8();
        QUdpSocket udpSocket;
        udpSocket.writeDatagram(data, QHostAddress::Broadcast, 45454);
    });
    timer->start(100);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::managejson(QByteArray data,QTcpSocket *msgtcp)
{
    QJsonDocument doc=QJsonDocument::fromJson(data);
    QJsonObject request = doc.object();
    QString type = request["type"].toString();
    if(type=="connect"){
        QString mark=request["mark"].toString();
        QMutexLocker locker(&socketmapmutex);
        socketmap[mark].msgsocket=msgtcp;
    }else if(type=="logon"){//登录
        QString mark=request["mark"].toString();
        int userid=request["id"].toInt();
        QString userpassword=request["password"].toString();
        LinkSQL sql;
        connect(&sql,&LinkSQL::frienddata,this,[=](int friendid,QString friendname){
            addfriend("initfriend",mark,userid,friendid,friendname);
        });
        QJsonObject json;
        json["type"]="logon";
        queuemapmutex.lock();
        if(queuemap[userid]!=NULL){
            json["num"]=2;
            packintjson(json,msgtcp);
            qDebug()<<"已登陆"<<'\n';
            return;
        }
        queuemapmutex.unlock();
        if(sql.selectuser(userid,userpassword)){
            json["num"]=1;
            json["id"]=userid;
            QString username=sql.selectname(userid);
            json["name"]=username;
            QString path=sql.selecticon(userid);
            qint64 filesize;
            QString iconpath="";
            if(path==""){
                iconpath="usericon/1.png";
            }else{
                iconpath="usericon/"+QString::number(userid)+".png";
            }
            QFile file(iconpath);
            file.open(QIODevice::ReadOnly);
            filesize=file.size();
            fileidmutex.lock();
            qint64 fileId = m_fileid++;
            if(m_fileid>1e15){
                m_fileid=0;
            }
            fileidmutex.unlock();
            json["fileid"]=fileId;
            json["size"]=filesize;
            packintjson(json,msgtcp);

            const qint64 CHUNK_SIZE = 1024 * 50;
            while (!file.atEnd()) {
                QByteArray chunk = file.read(CHUNK_SIZE);
                packingfile(chunk,fileId,mark,userid);
            }
            file.close();
            queuemapmutex.lock();
            QQueue<FileTransferContext> *newqueue=new QQueue<FileTransferContext>;
            queuemap[userid]=newqueue;
            queuemapmutex.unlock();
            idmarkmutex.lock();
            idmarkmap[userid]=mark;
            idmarkmutex.unlock();
            QMutex *newmutex=new QMutex;
            mutexmutex.lock();
            mutexmap[userid]=newmutex;
            mutexmutex.unlock();
            sql.selectfriend(userid);
            vismapmutex.lock();
            if(vismap[userid]){
                QTimer::singleShot(300, [this,userid]() {
                    onlinesendmsg(userid);
                });
            }else{
                qDebug()<<"没有离线消息"<<'\n';
            }
            vismapmutex.unlock();
        }else{
            json["num"]=0;
            packintjson(json,msgtcp);
        }
    }else if(type=="login"){
        QString mark=request["mark"].toString();
        QString name=request["name"].toString();
        QString password=request["password"].toString();
        LinkSQL sql;
        int id=sql.insertuser(name,password);
        QJsonObject json;
        json["type"]="login";
        if(id){
            json["num"]=1;
            json["id"]=id;
            json["name"]=name;
            qint64 filesize;
            QFile file("usericon/1.png");
            file.open(QIODevice::ReadOnly);
            filesize=file.size();
            fileidmutex.lock();
            qint64 fileId = m_fileid++;
            if(m_fileid>1e15){
                m_fileid=0;
            }
            fileidmutex.unlock();
            json["fileid"]=fileId;
            json["size"]=filesize;
            packintjson(json,msgtcp);

            const qint64 CHUNK_SIZE = 1024 * 50;
            while (!file.atEnd()) {
                QByteArray chunk = file.read(CHUNK_SIZE);
                packingfile(chunk,fileId,mark,id);
            }
            file.close();
            queuemapmutex.lock();
            QQueue<FileTransferContext> *newqueue=new QQueue<FileTransferContext>;
            queuemap[id]=newqueue;
            queuemapmutex.unlock();
            idmarkmutex.lock();
            idmarkmap[id]=mark;
            idmarkmutex.unlock();
            QMutex *newmutex=new QMutex;
            mutexmutex.lock();
            mutexmap[id]=newmutex;
            mutexmutex.unlock();
        }else{
            json["num"]=0;
            packintjson(json,msgtcp);
        }
    }else if(type=="updateinformation"){
        QString name=request["name"].toString();
        int id=request["id"].toInt();
        QString path="usericon/"+QString::number(id)+".png";
        LinkSQL sql;
        connect(&sql,&LinkSQL::frienddata,this,[=](int friendid,QString friendname){
            QJsonObject json;
            json["type"]="friendnamechanged";
            json["friendid"]=id;
            json["friendnewname"]=name;
            QTcpSocket *msgtcp=socketmap[idmarkmap[friendid]].msgsocket;
            packintjson(json,msgtcp);
        });
        sql.selectfriend(id);
        sql.updateinformation(id,name,path);
        qint64 size=static_cast<qint64>(request["size"].toDouble());
        qint64 fileid=static_cast<qint64>(request["fileid"].toDouble());
        queuemapmutex.lock();
        QQueue<FileTransferContext>*queue=queuemap[id];
        queuemapmutex.unlock();
        addfilequeue(queue,fileid,type,size,id);
    }else if(type=="updatenameonly"){
        QString name=request["name"].toString();
        int id=request["id"].toInt();
        LinkSQL sql;
        connect(&sql,&LinkSQL::frienddata,this,[=](int friendid,QString friendname){
            QJsonObject json;
            json["type"]="friendnamechanged";
            json["friendid"]=id;
            json["friendnewname"]=name;
            QTcpSocket *msgtcp=socketmap[idmarkmap[friendid]].msgsocket;
            packintjson(json,msgtcp);
        });
        sql.updatenameonly(id,name);
        sql.selectfriend(id);
    }else if(type=="selectuser"){
        QString mark=request["mark"].toString();
        int id=request["id"].toInt();
        int selectid=request["selectid"].toInt();
        LinkSQL sql;
        QString selectname=sql.selectname(selectid);
        if(selectname.isEmpty()){
            QJsonObject json;
            json["type"]="selectuser";
            json["num"]=0;
            packintjson(json,msgtcp);
        }else{
            addfriend(type,mark,id,selectid,selectname);
        }
    }else if(type=="friendrequest"){
        int fromid=request["fromid"].toInt();
        QString fromname=request["fromname"].toString();
        int toid=request["toid"].toInt();
        QString mark=idmarkmap[toid];
        addfriend(type,mark,toid,fromid,fromname);
    }else if(type=="agreeadd"){
        int fromid=request["fromid"].toInt();
        QString fromname=request["fromname"].toString();
        int toid=request["toid"].toInt();
        QString mark=idmarkmap[toid];
        LinkSQL sql;
        sql.addfriend(fromid,toid);
        addfriend(type,mark,toid,fromid,fromname);
    }else if(type=="sendmsg"){
        int fromid=request["fromid"].toInt();
        QString msg=request["msg"].toString();
        int toid=request["toid"].toInt();
        QString mark=idmarkmap[toid];
        QJsonObject json;
        json["type"]=type;
        json["fromid"]=fromid;
        json["msg"]=msg;
        QTcpSocket *msgtcp=socketmap[mark].msgsocket;
        if(msgtcp==NULL){//如果用户没有登录把消息添加到记录队列中
            offlinemsgmapmutex.lock();
            vismapmutex.lock();
            if(vismap[toid]==0){
                B *b=new B;
                offlinemsgmap.insert(toid,b);
            }
            vismap[toid]=1;
            vismapmutex.unlock();
            offlinemsgmap[toid]->mutex->lock();
            QByteArray newdata;
            int p=1;
            newdata.append(reinterpret_cast<const char*>(&p), sizeof(int));
            newdata.append(data);
            offlinemsgmap[toid]->queue->enqueue(newdata);
            offlinemsgmap[toid]->mutex->unlock();
            offlinemsgmapmutex.unlock();
        }else{
            packintjson(json,msgtcp);
        }

    }else if(type=="sendfile"||type=="sendpicture"){
        int toid=request["toid"].toInt();
        QString mark=idmarkmap[toid];
        QTcpSocket *msgtosocket=socketmap[mark].msgsocket;
        if(msgtosocket!=NULL){
            packintjson(request,msgtosocket);
        }else{
            offlinemsgmapmutex.lock();
            vismapmutex.lock();
            if(vismap[toid]==0){
                B *b=new B;
                offlinemsgmap.insert(toid,b);
            }
            vismap[toid]=1;
            vismapmutex.unlock();
            offlinemsgmap[toid]->mutex->lock();
            QByteArray newdata;
            int p=1;
            newdata.append(reinterpret_cast<const char*>(&p), sizeof(int));
            newdata.append(data);
            offlinemsgmap[toid]->queue->enqueue(newdata);
            offlinemsgmap[toid]->mutex->unlock();
            offlinemsgmapmutex.unlock();
        }
    }else if(type=="close"){
        qDebug()<<"清理内存"<<'\n';
        QString mark=request["mark"].toString();
        int id=request["id"].toInt();
        socketmapmutex.lock();
        socketmap[mark].msgsocket->disconnect();
        socketmap[mark].msgsocket->deleteLater();
        socketmap[mark].msgsocket=NULL;
        socketmap[mark].filesocket->disconnect();
        socketmap[mark].filesocket->deleteLater();
        socketmap[mark].filesocket=NULL;
        socketmap.remove(mark);
        socketmapmutex.unlock();
        queuemapmutex.lock();
        delete queuemap[id];
        queuemap[id]=NULL;
        queuemap.remove(id);
        queuemapmutex.unlock();
        idmarkmutex.lock();
        idmarkmap.remove(id);
        idmarkmutex.unlock();
        mutexmutex.lock();
        mutexmap.remove(id);
        mutexmutex.unlock();
    }
}

void MainWindow::packintjson(const QJsonObject &json, QTcpSocket *msgtcp)//打包并发送json
{
    QJsonDocument doc(json);
    QByteArray m_data;
    m_data = doc.toJson(QJsonDocument::Compact);
    qint64 len = qToBigEndian<qint64>(m_data.size());
    QByteArray newdata(reinterpret_cast<const char*>(&len), sizeof(len));
    newdata.append(m_data);
    msgtcp->write(newdata);
    msgtcp->flush();
    qDebug()<<"json发送完成"<<'\n';
}

void MainWindow::packingfile(const QByteArray &chunk, qint64 fileid, QString mark,int toid)//打包并发送文件块
{
    QByteArray header;
    qint64 m_fileid=qToBigEndian<qint64>(fileid);
    qint64 len=qToBigEndian<qint64>(chunk.size());
    header.append(reinterpret_cast<const char*>(&m_fileid), sizeof(m_fileid));
    header.append(reinterpret_cast<const char*>(&len), sizeof(len));
    header.append(chunk);
    QTcpSocket *filetcp=socketmap[mark].filesocket;
    if(filetcp!=NULL){
        filetcp->write(header);
    }else{
        qDebug()<<"文件加入队列"<<'\n';
        vismapmutex.lock();
        if(!vismap[toid]){
            B *b=new B;
            offlinemsgmap.insert(toid,b);
        }
        vismap[toid]=1;
        vismapmutex.unlock();
        offlinemsgmapmutex.lock();
        offlinemsgmap[toid]->mutex->lock();
        if(offlinemsgmap[toid]->queue==NULL){
            QQueue<QByteArray>* newqueue=new QQueue<QByteArray>;
            offlinemsgmap[toid]->queue=newqueue;
        }
        QByteArray newdata;
        int p=2;
        newdata.append(reinterpret_cast<const char*>(&p), sizeof(int));
        newdata.append(header);
        offlinemsgmap[toid]->queue->enqueue(newdata);
        offlinemsgmap[toid]->mutex->unlock();
        offlinemsgmapmutex.unlock();

    }
}

void MainWindow::addfilequeue(QQueue<FileTransferContext> *queue, qint64 fileid,QString type,qint64 size,int id)
{
    FileTransferContext *ctx=NULL;
    for(auto &them:*queue){
        if(them.fileId==fileid){
            ctx=&them;
            break;
        }
    }
    if(ctx==NULL){
        FileTransferContext newctx;
        newctx.type=type;
        newctx.fileId=fileid;
        newctx.totalSize=size;
        newctx.data.reserve(size);
        newctx.receivedSize=0;
        mutexmutex.lock();
        QMutex *usermutex=mutexmap[id];
        mutexmutex.unlock();
        usermutex->lock();
        queue->enqueue(newctx);
        usermutex->unlock();
    }else{
        ctx->totalSize=size;
        ctx->type=type;
        ctx->data.reserve(size);
        if(ctx->receivedSize>=ctx->totalSize){
            mutexmutex.lock();
            QMutex *usermutex=mutexmap[id];
            mutexmutex.unlock();
            usermutex->lock();
            if(ctx->type=="updateinformation"){
                LinkSQL sql;
                connect(&sql,&LinkSQL::frienddata,this,[=](int friendid,QString friendname){
                    qDebug()<<"发送头像"<<'\n';
                    QByteArray ctxdata=ctx->data;
                    QString friendmark=idmarkmap[friendid];
                    QTcpSocket *msgtcp=socketmap[friendmark].msgsocket;
                    QJsonObject json;
                    json["type"]="friendpicturechanged";
                    json["friendid"]=id;
                    json["fileid"]=ctx->fileId;
                    json["size"]=ctx->totalSize;
                    if(msgtcp!=NULL){
                        packintjson(json,msgtcp);
                    }


                    QDataStream stream(&ctxdata, QIODevice::ReadOnly);
                    const qint64 CHUNK_SIZE = 1024 * 50;
                    qDebug()<<"发送头像文件"<<'\n';
                    while(!stream.atEnd()){
                        QByteArray newdata(CHUNK_SIZE, 0);
                        qint64 bytesRead = stream.readRawData(newdata.data(), CHUNK_SIZE);

                        if (bytesRead < CHUNK_SIZE) {
                            newdata.resize(bytesRead);
                        }
                        packingfile(newdata,ctx->fileId,idmarkmap[friendid],friendid);
                    }
                    qDebug()<<"发送完毕"<<'\n';
                });
                sql.selectfriend(id);
            }
            queue->removeAll(*ctx);
            usermutex->unlock();
        }
    }
}

void MainWindow::do_msgnewConnection()//处理客户端msgsocket的连接
{
    QTcpSocket *msgsocket=msgserver->nextPendingConnection();
    connect(msgsocket,&QTcpSocket::readyRead,this,[=](){//有数据要接受
        msgreaddata(msgsocket);
    });
}

void MainWindow::do_filenewConnection()//处理客户端filesocket的连接
{
    QTcpSocket *filesocket=fileserver->nextPendingConnection();
    connect(filesocket,&QTcpSocket::readyRead,this,[=](){//有数据要接受
        filereaddata(filesocket);
    });
}

void MainWindow::msgreaddata(QTcpSocket *msgtcp)//处理客户端msgsocket发送的数据
{
    while(msgtcp->bytesAvailable()>0){
        if(msgtcp->bytesAvailable()<sizeof(qint64)){
            return;
        }
        qint64 len;
        msgtcp->read(reinterpret_cast<char*>(&len), sizeof(len));
        len = qFromBigEndian(len);
        while (msgtcp->bytesAvailable() < len) {
            if (!msgtcp->waitForReadyRead(1000)) break;
        }
        QByteArray data = msgtcp->read(len);
        managejson(data,msgtcp);
    }
}

void MainWindow::filereaddata(QTcpSocket *filetcp)//处理客户端filesocket发送的数据
{
    while(filetcp->bytesAvailable()>0){
        if(filetcp->bytesAvailable()<(sizeof(qint64)*2)){
            return;
        }
        qint64 mark;
        filetcp->read(reinterpret_cast<char*>(&mark), sizeof(mark));
        mark = qFromBigEndian(mark);
        if(mark==0){
            qint64 len;
            filetcp->read(reinterpret_cast<char*>(&len), sizeof(len));
            len = qFromBigEndian(len);
            while (filetcp->bytesAvailable() < len) {
                if (!filetcp->waitForReadyRead(1000)) break;
            }
            QByteArray data = filetcp->read(len);
            QJsonDocument doc=QJsonDocument::fromJson(data);
            QJsonObject request = doc.object();
            QString filemark = request["mark"].toString();
            QMutexLocker locker(&socketmapmutex);
            socketmap[filemark].filesocket=filetcp;
        }else if(mark==1){
            qint64 fileid;
            filetcp->read(reinterpret_cast<char*>(&fileid), sizeof(fileid));
            fileid=qFromBigEndian(fileid);
            qint64 id;
            filetcp->read(reinterpret_cast<char*>(&id), sizeof(id));
            id=qFromBigEndian(id);
            queuemapmutex.lock();
            QQueue<FileTransferContext>*queue=queuemap[id];
            queuemapmutex.unlock();
            qint64 len;
            filetcp->read(reinterpret_cast<char*>(&len), sizeof(len));
            len=qFromBigEndian(len);
            while (filetcp->bytesAvailable() < len) {
                if (!filetcp->waitForReadyRead(1000)) break;
            }
            QByteArray data = filetcp->read(len);

            FileTransferContext *ctx=NULL;
            for(auto &task:*queue){
                if(fileid==task.fileId){
                    ctx=&task;
                    break;
                }
            }
            if(ctx==NULL){//没有找到任务，在此创建任务
                FileTransferContext newctx;
                newctx.fileId=fileid;
                newctx.type="unknow";
                newctx.totalSize=-1;
                newctx.receivedSize=0;
                mutexmutex.lock();
                QMutex *usermutes=mutexmap[id];
                mutexmutex.unlock();
                usermutes->lock();
                queue->enqueue(newctx);
                ctx=&queue->last();
                usermutes->unlock();
            }
            QFile file("usericon/"+QString::number(id)+".png");
            if(ctx->receivedSize==0){
                file.open(QIODevice::WriteOnly);
            }else{
                file.open(QIODevice::WriteOnly|QIODevice::Append);
            }
            file.write(data);
            file.close();
            ctx->receivedSize+=data.size();
            ctx->data.append(data);
            if(ctx->totalSize>0&&ctx->receivedSize>=ctx->totalSize){
                mutexmutex.lock();
                QMutex *usermutes=mutexmap[id];
                mutexmutex.unlock();
                usermutes->lock();
                if(ctx->type=="updateinformation"){
                    LinkSQL sql;
                    connect(&sql,&LinkSQL::frienddata,this,[=](int friendid,QString friendname){
                        qDebug()<<"发送头像"<<'\n';
                        QByteArray ctxdata=ctx->data;
                        QString friendmark=idmarkmap[friendid];
                        QTcpSocket *msgtcp=socketmap[friendmark].msgsocket;
                        QJsonObject json;
                        json["type"]="friendpicturechanged";
                        json["friendid"]=id;
                        json["fileid"]=ctx->fileId;
                        json["size"]=ctx->totalSize;
                        if(msgtcp!=NULL){
                            packintjson(json,msgtcp);
                        }


                        QDataStream stream(&ctxdata, QIODevice::ReadOnly);
                        const qint64 CHUNK_SIZE = 1024 * 50;
                        qDebug()<<"发送头像文件"<<'\n';
                        while(!stream.atEnd()){
                            QByteArray newdata(CHUNK_SIZE, 0);
                            qint64 bytesRead = stream.readRawData(newdata.data(), CHUNK_SIZE);

                            if (bytesRead < CHUNK_SIZE) {
                                newdata.resize(bytesRead);
                            }
                            packingfile(newdata,ctx->fileId,idmarkmap[friendid],friendid);
                        }
                        qDebug()<<"发送完毕"<<'\n';
                    });
                    sql.selectfriend(id);
                }
                queue->removeAll(*ctx);
                usermutes->unlock();
            }
        }else if(mark==2){
            qint64 toid;
            filetcp->read(reinterpret_cast<char*>(&toid), sizeof(toid));
            toid = qFromBigEndian(toid);
            qint64 fileid;
            filetcp->read(reinterpret_cast<char*>(&fileid), sizeof(fileid));
            fileid=qFromBigEndian(fileid);
            qint64 len;
            filetcp->read(reinterpret_cast<char*>(&len), sizeof(len));
            len=qFromBigEndian(len);
            while (filetcp->bytesAvailable() < len) {
                if (!filetcp->waitForReadyRead(1000)) break;
            }
            QByteArray data = filetcp->read(len);
            QString filemark=idmarkmap[toid];
            packingfile(data,fileid,filemark,toid);
        }
    }
}


void MainWindow::addfriend(QString type,QString mark, int userid, int friendid, QString friendname)
{
    QTcpSocket *msgtcp=socketmap[mark].msgsocket;
    QJsonObject json;
    json["type"]=type;
    if(type=="selectuser"){
        json["num"]=1;
    }
    json["id"]=userid;
    json["friendid"]=friendid;
    json["friendname"]=friendname;
    LinkSQL sql;
    QString path=sql.selecticon(friendid);
    qint64 filesize;
    QString iconpath="";
    if(path==""){
        iconpath="usericon/1.png";
    }else{
        iconpath="usericon/"+QString::number(friendid)+".png";
    }
    QFile file(iconpath);
    file.open(QIODevice::ReadOnly);
    filesize=file.size();
    fileidmutex.lock();
    qint64 fileId = m_fileid++;
    if(m_fileid>1e15){
        m_fileid=0;
    }
    fileidmutex.unlock();
    json["fileid"]=fileId;
    json["size"]=filesize;
    if(msgtcp==NULL){
        qDebug()<<"id:"<<userid<<'\n';
        QJsonDocument doc(json);
        QByteArray data;
        data = doc.toJson(QJsonDocument::Compact);
        offlinemsgmapmutex.lock();
        vismapmutex.lock();
        if(vismap[userid]==0){
            B *b=new B;
            offlinemsgmap.insert(userid,b);
        }
        vismap[userid]=1;
        vismapmutex.unlock();
        offlinemsgmap[userid]->mutex->lock();
        QByteArray newdata;
        int p=1;
        newdata.append(reinterpret_cast<const char*>(&p), sizeof(int));
        newdata.append(data);
        offlinemsgmap[userid]->queue->enqueue(newdata);
        offlinemsgmap[userid]->mutex->unlock();
        offlinemsgmapmutex.unlock();
    }else{
       packintjson(json,msgtcp);
    }


    const qint64 CHUNK_SIZE = 1024 * 50;
    while (!file.atEnd()) {
        QByteArray chunk = file.read(CHUNK_SIZE);
        packingfile(chunk,fileId,mark,userid);
    }
    file.close();
}

void MainWindow::onlinesendmsg(int id)
{
    offlinemsgmapmutex.lock();
    QQueue<QByteArray>*idqueue=offlinemsgmap[id]->queue;
    if(idqueue==NULL){
        return;
    }
    offlinemsgmap[id]->mutex->lock();
    while(!idqueue->isEmpty()){
        QByteArray idarray=idqueue->front();
        idqueue->pop_front();
        QTcpSocket *socket=NULL;
        int p=0;
        p = *reinterpret_cast<const int*>(idarray.constData());
        qDebug()<<"p:"<<p<<'\n';
        QByteArray chunk = idarray.mid(sizeof(int));
        if(p==1){
            socket=socketmap[idmarkmap[id]].msgsocket;
            QJsonDocument doc=QJsonDocument::fromJson(chunk);
            QJsonObject request = doc.object();
            packintjson(request,socket);
        }else if(p==2){
            sendonlinemsg(id,chunk);
        }
    }
    vismapmutex.lock();
    vismap[id]=0;
    vismapmutex.unlock();
    offlinemsgmap[id]->mutex->unlock();
    offlinemsgmap.remove(id);
    offlinemsgmapmutex.unlock();
}

void MainWindow::sendonlinemsg(int id,QByteArray &data)
{
    QTcpSocket *socket=socketmap[idmarkmap[id]].filesocket;
    if(socket!=NULL){
        socket->write(data);
    }else{
        qDebug()<<"离线消息没有拿到socket"<<'\n';
    }
}



