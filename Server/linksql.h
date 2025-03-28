#ifndef LINKSQL_H
#define LINKSQL_H
#include<QtSql>
#include <QObject>
#include <QMutex>

class LinkSQL : public QObject
{
    Q_OBJECT
public:
    explicit LinkSQL(QObject *parent = nullptr);

private:
    QSqlDatabase sql;

private:
    static QSqlDatabase createConnection();

public:
    bool selectuser(int id,QString password);
    QString selectname(int id);
    QString selecticon(int id);
    int insertuser(QString name,QString password);
    void updateinformation(int id,QString name,QString path);
    void updatenameonly(int id,QString name);
    void selectfriend(int id);
    void addfriend(int userid, int friendid);

public:signals:
    void frienddata(int friendid,QString friendname);
};

#endif // LINKSQL_H
