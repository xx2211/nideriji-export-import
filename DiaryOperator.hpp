#ifndef DIARYOPERATOR_H
#define DIARYOPERATOR_H

#include "HttpsUtil.hpp"
#include <QString>
#include <QSettings>
#include <QtNetwork/QNetworkCookie>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

class DiaryOperator{
public: // singleton
    static DiaryOperator& getInstance()
    {
        static DiaryOperator instance;

        return instance;
    }
private: // singleton
    DiaryOperator() {
//        initThisObject();
    };                   // Constructor
    DiaryOperator(DiaryOperator const&);              // Don't Implement
    void operator=(DiaryOperator const&); // Don't implement

public: // data member
    QByteArray token = "hello";
    QByteArray csrfmiddlewaretoken = "YYCRw31cBrt3MW8cS6BbWsaESks1n0s1";
    QJsonObject curDiaryJsonObject;

public: // public func member
    bool login(const QString &email, const QString &passwd){
        QMap<QString,QString> formMap;
        formMap["csrfmiddlewaretoken"]=getInstance().csrfmiddlewaretoken;
        formMap["email"]=email;
        formMap["password"]=passwd;
        QByteArray response = HttpsUtil::getInstance().syncHttpsRequest("https://nideriji.cn/api/login/","post",
                                                  QVariant::Invalid,"",HttpsUtil::getInstance().map2formBody(formMap));
        if(!checkResponesNoError(response)){
            return false;
        }
        token = QJsonDocument::fromJson(response).object().value("token").toString().toUtf8();
        return true;
    }

    bool saveDiaryToToday(const QString &content){
        return saveDiaryToDate(content, QDate::currentDate());
    }

    bool saveDiaryToDate(const QString &content, const QDate &date){
        QMap<QString,QString> form;
        if(""==(form["content"]=content))
            form["content"]="\n";
        form["csrfmiddlewaretoken"]=getInstance().csrfmiddlewaretoken;
        form["date"]=date.toString("yyyy-MM-dd");

        QByteArray reveiveData =  HttpsUtil::getInstance().syncHttpsRequest(
                                   "https://nideriji.cn/api/write/","post",
                                   QVariant::Invalid,
                                   "token "+getInstance().token,
                                   HttpsUtil::getInstance().map2formBody(form));

        if(!checkResponseDoesHavDiary(reveiveData)){
            qDebug()<<"has not more diary.";
            return false;
        }
        updateCurDiaryJsonObjectByResponeseData(reveiveData);
        return true;
    }

    bool saveCurDiary(const QString &content){
        return saveDiaryToDate(content,
             QDate::fromString(curDiaryJsonObject.value("createddate").toString(),"yyyy-MM-dd"));
    }

    bool checkLoginStatus(){
        QList<QNetworkCookie> cookies;
        cookies.append(QNetworkCookie("csrftoken", getInstance().csrfmiddlewaretoken));
        cookies.append(QNetworkCookie("token",getInstance().token));
        for(int i=0; i<cookies.length(); ++i){
            cookies[i].setPath("/");
            cookies[i].setDomain("nideriji.cn");
        }
        QVariant variant = QVariant::fromValue(cookies);
        QByteArray receiveData = HttpsUtil::getInstance().syncHttpsRequest("https://nideriji.cn/api/diary/latest/","get",
                                   variant,
                                   "token "+getInstance().token);
        return checkResponesNoError(receiveData);
    }

    bool getLatestDiary(){
        QString url = "https://nideriji.cn/api/diary/latest/";
        QByteArray receiveData = getDiaryRawRespByUrl(url);
        if(!checkResponseDoesHavDiary(receiveData)){
            qDebug()<<"has not more diary.";
            return false;
        }
        updateCurDiaryJsonObjectByResponeseData(receiveData);
        return true;
    }

    bool getPrevDiary(){
        QByteArray receiveData = getDiaryRawRespByUrl(
                                   "https://nideriji.cn/api/diary/prev/"+
                                  getInstance().curDiaryJsonObject.value("id").toString()
                                    +"/");

        if(!checkResponseDoesHavDiary(receiveData)){
            qDebug()<<"has not more diary.";
            return false;
        }
        updateCurDiaryJsonObjectByResponeseData(receiveData);
        return true;
    }

    bool getNextDiary(){
        QByteArray receiveData = getDiaryRawRespByUrl(
                    "https://nideriji.cn/api/diary/next/"+
                    getInstance().curDiaryJsonObject.value("id").toString() +"/");
        if(!checkResponseDoesHavDiary(receiveData)){
            qDebug()<<"has not more diary.";
            return false;
        }
        updateCurDiaryJsonObjectByResponeseData(receiveData);
        return true;
    }

    bool getTodayDiary(){
        return getDiaryByDate(QDate::currentDate());
    }

    bool getDiaryByDate(const QDate &date){
        QByteArray receiveData = getDiaryRawRespByUrl(
                    "https://nideriji.cn/api/diary/?date="+
                    date.toString("yyyy-MM-dd") +"");
        qDebug()<<receiveData;
        if(!checkResponseDoesHavDiary(receiveData)){
            qDebug()<<"has not more diary.";
            return false;
        }
        updateCurDiaryJsonObjectByResponeseData(receiveData);
        return true;
    }

    QJsonArray getAllDiaryToJson(){
        QJsonArray diaries;
        QByteArray response = getDiaryRawRespByUrl("https://nideriji.cn/api/diary/latest/");
        if(!checkResponseDoesHavDiary(response)){
            return diaries;
        }
        diaries.append(
                QJsonDocument::fromJson(response).object().value("diary")
        );
        bool hasMore = false;
        do{
            QString lastDiaryDate = diaries.last().toObject().value("createddate").toString();
            QByteArray temp = getDiaryRawRespByUrl(
                    "https://ohshenghuo.com/api/diary/all/?latest_date=" + lastDiaryDate
                    );
            QJsonObject tempJson = QJsonDocument::fromJson(temp).object();
            hasMore = tempJson.value("has_more").toBool();
            QJsonArray tempDiaries = tempJson.value("diaries").toArray();
            for(int i=0; i<tempDiaries.size(); ++i){
                diaries.push_back(tempDiaries.at(i));
            }
        } while (hasMore);
        return  diaries;
    }

private: // private func member
    //return true if no error. else return false.
    bool checkResponesNoError(const QByteArray &receiveData){
        QJsonParseError err_rpt;
        QJsonDocument jsonDocument = QJsonDocument::fromJson(receiveData, &err_rpt);
        if(err_rpt.error != QJsonParseError::NoError)
        {
            qDebug() << "JSON format error.";
            return false;
        }
        if(!jsonDocument.object().value("error").isDouble())
            return false;
        if(0!=jsonDocument.object().value("error").toDouble())
            return false;
        return true;
    }

    //return true if has more diary. else return false.
    bool checkResponseDoesHavDiary(const QByteArray &receiveData){
        QJsonParseError err_rpt;
        QJsonDocument jsonDocument = QJsonDocument::fromJson(receiveData, &err_rpt);
        if(err_rpt.error != QJsonParseError::NoError)
        {
            qDebug() << "JSON format error.";
            return false;
        }
        if(!jsonDocument.object().value("error").isDouble())
            return false;
        if(jsonDocument.object().value("error").toDouble()!=0){
            qDebug()<<"err code:" <<jsonDocument.object().value("error");
            return false;
        }
        if(!jsonDocument.object().value("diary").isObject()){
            return false;
        }
        return true;
    }

    void updateCurDiaryJsonObjectByResponeseData(const QByteArray &receiveData){
        QJsonDocument jsonDocument = QJsonDocument::fromJson(receiveData);
        QJsonObject diaryJsonObject = jsonDocument.object().value("diary").toObject();
        qDebug()<<diaryJsonObject.value("id").toString();
        getInstance().curDiaryJsonObject = diaryJsonObject;
    }

    QByteArray getDiaryRawRespByUrl(const QString &url){
        QList<QNetworkCookie> cookies;
        cookies.append(QNetworkCookie("csrftoken", getInstance().csrfmiddlewaretoken));
        cookies.append(QNetworkCookie("token",getInstance().token));
        for(int i=0; i<cookies.length(); ++i){
            cookies[i].setPath("/");
            cookies[i].setDomain("nideriji.cn");
        }
        QVariant variant = QVariant::fromValue(cookies);
        return  HttpsUtil::getInstance().syncHttpsRequest(
                                   url,"get",
                                   variant,
                                   "token "+getInstance().token);
    }

//    void initThisObject(){
//        QSettings settings("config.ini",QSettings::IniFormat);
//        settings.beginGroup("login");
//        csrfmiddlewaretoken = settings.value("csrfmiddlewaretoken").toByteArray();
//        token = settings.value("token").toByteArray();
//        settings.endGroup();
//    }

};


#endif // DIARYOPERATOR_H
