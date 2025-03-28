#include "linksql.h"
#include<QMessageBox>

LinkSQL::LinkSQL(QObject *parent)
    : QObject{parent}
{
    sql = createConnection();
    if(!sql.open()){
        QMessageBox::warning(NULL,"注意","数据库连接错误");
        return;
    }else{
        qDebug()<<"打开成功"<<'\n';
    }
}


QSqlDatabase LinkSQL::createConnection()
{

    QString connName = QString("conn_%1").arg(quintptr(QThread::currentThreadId()));//用线程地址生成唯一的连接id

    if (!QSqlDatabase::contains(connName)) {
        QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL", connName);
        db.setHostName("localhost");
        db.setUserName("root");
        db.setPassword("20041203Li?");
        db.setDatabaseName("test");
        if (!db.open()) {
            qCritical() << "Failed to open database:" << db.lastError();
            return QSqlDatabase();
        }
    }
    return QSqlDatabase::database(connName);
}

bool LinkSQL::selectuser(int id, QString password)
{
    QSqlQuery q(sql);
    q.prepare("select password from users where id=?;");
    q.addBindValue(id);
    q.exec();
    if(q.next()){
        QString spassword=q.value(0).toString();
        if(spassword==password){
            return 1;
        }
    }
    return 0;
}



QString LinkSQL::selecticon(int id)
{
    QSqlQuery q(sql);
    q.prepare("select path from users where id=?;");
    q.addBindValue(id);
    q.exec();
    if(q.next()){
        return q.value(0).toString();
    }
    return "";
}

int LinkSQL::insertuser(QString name, QString password)
{
    QSqlQuery q(sql);
    q.prepare("insert into users (username, password) values (?, ?)");
    q.addBindValue(name);
    q.addBindValue(password);

    if (!q.exec()) {
        qCritical() << "Insert failed:" << q.lastError();
        return 0;
    }
    QVariant idVar = q.lastInsertId();
    int newId = idVar.toInt();
    return newId;
}

void LinkSQL::updateinformation(int id, QString name, QString path)
{
    QSqlQuery q(sql);
    q.prepare("update users set username=?,path=? where id=?");
    q.addBindValue(name);
    q.addBindValue(path);
    q.addBindValue(id);
    q.exec();
}

void LinkSQL::updatenameonly(int id, QString name)
{
    QSqlQuery q(sql);
    q.prepare("update users set username=? where id=?");
    q.addBindValue(name);
    q.addBindValue(id);
    q.exec();
}



QString LinkSQL::selectname(int id)
{
    QSqlQuery q(sql);
    q.prepare("select username from users where id=?;");
    q.addBindValue(id);
    q.exec();
    if(q.next()){
        return q.value(0).toString();
    }
    return "";
}



void LinkSQL::selectfriend(int id)
{
    QSqlQuery q(sql);
    q.prepare("select friendid from friendships where userid=?");
    q.addBindValue(id);
    q.exec();
    while(q.next()){
        int friendid=q.value(0).toInt();
        QString friendname=selectname(friendid);
        qDebug()<<"id:"<<friendid<<'\n';
        qDebug()<<"name:"<<friendname<<'\n';
        emit frienddata(friendid,friendname);
    }
}



void LinkSQL::addfriend(int userid, int friendid)
{
    QSqlQuery q(sql);
    q.prepare("insert into friendships(userid,friendid) value (?,?);");
    q.addBindValue(userid);
    q.addBindValue(friendid);
    q.exec();
    q.prepare("insert into friendships(userid,friendid) value (?,?);");
    q.addBindValue(friendid);
    q.addBindValue(userid);
    q.exec();
}

