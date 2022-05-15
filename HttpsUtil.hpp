#ifndef HTTPSUTIL_H
#define HTTPSUTIL_H

#include <QString>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkAccessManager>
#include <QEventLoop>
#include <QtNetwork/QNetworkReply>

class HttpsUtil{
public:
    static HttpsUtil& getInstance()
    {
        static HttpsUtil instance;
        return instance;
    }
private:
    HttpsUtil() {};                   // Constructor
    HttpsUtil(HttpsUtil const&);              // Don't Implement
    void operator=(HttpsUtil const&); // Don't implement

public:
    QByteArray syncHttpsRequest(QString const &url, QString const &method,
                         QVariant const &cookies=QVariant::Invalid,
                                QByteArray const &auth="",
                                QByteArray const &formData=""){
        QNetworkRequest request;
        request.setUrl(QUrl(url));

        //if params contain extra items, need to set header as follow.
        if(""!=formData){
            request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded; charset=UTF-8");
        }
        if(QVariant::Invalid!=cookies){
            request.setHeader(QNetworkRequest::CookieHeader, cookies);
        }
        if(""!=auth){
            request.setRawHeader("auth", QByteArray::fromStdString(auth.toStdString()));
        }

        QNetworkAccessManager manager;

        QNetworkReply *reply=nullptr;
        if("get"==method)
            reply = manager.get(request);
        else if("post"==method)
            reply = manager.post(request, formData);
        else{
            qDebug()<<"request para error. use default:get";
            reply = manager.get(request);
        }

        //this eventLoop is for is for synchronization;
        QEventLoop eventLoop;
        QObject::connect(reply, &QNetworkReply::finished, &eventLoop, &QEventLoop::quit);
        eventLoop.exec();

        if(reply->error()!=QNetworkReply::NoError){
            qDebug()<<"reply error:"<<reply->errorString();
            reply->deleteLater();
            qDebug()<<reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
            return "";
        }
        QByteArray result=reply->readAll();
        reply->deleteLater();
        return result;
    }

    QByteArray map2formBody(const QMap<QString,QString> &map){
        QByteArray result;
        for(auto it=map.begin(); it!=map.end(); ++it){
            result+=it.key().toUtf8(). toPercentEncoding();
            result+='=';
            result+=it.value().toUtf8(). toPercentEncoding();
            result+='&';
        }
        result.remove(result.size()-1,1);
        return result;
    }
};


#endif // HTTPSUTIL_H
