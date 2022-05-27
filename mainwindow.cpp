#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include "DiaryOperator.hpp"
#include "json.hpp"

#include <QMessageBox>
#include <QFileDialog>
#include <QPdfWriter>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->textTipInformation->setReadOnly(true);

    connect(ui->btnLogin,&QPushButton::clicked,[this](){
        this->putTipInfo("尝试登录, 请稍候...");
        this->setEnabled(false);
        do{
            if(DiaryOperator::getInstance().login(this->ui->editEmail->text(), this->ui->editPasswd->text())){
                QMessageBox::information(this, "提示", "登录成功.");
                this->putTipInfo("登录成功.当前登录账号:"+this->ui->editEmail->text());
                break;
            }
            QMessageBox::information(this, "提示", "登录失败或此账号未创建过日记,请重试.");
            this->putTipInfo("登录失败或此账号未创建过日记,请重试.");
        } while (0);
        this->setEnabled(true);
    });

    connect(ui->btnExprotToJson,&QPushButton::clicked,[this]() {
        this->setEnabled(false);
        do{
            if (!DiaryOperator::getInstance().checkLoginStatus()) {
                QMessageBox::information(this, "提示", "还未登录,请先登录.");
                this->putTipInfo("还未登录,请先登录.");
                break;
            }
            QString path = QFileDialog::getExistingDirectory(this, "请选择导出的json文件的保存位置", "/");
            if (path.isEmpty()) {//按了取消
                this->putTipInfo("打开文件失败,请重试.");
                break;
            }
            QString filename = path + "/nideriji_bak_"
                    + ui->editEmail->text() + "_"
                    + QDate::currentDate().toString("yyyy-MM-dd") + ".json";
            scriptExportToJson(filename);
        } while (0);
        this->setEnabled(true);
    });

    connect(ui->btnExprotToPdf,&QPushButton::clicked,[this](){
        this->setEnabled(false);
        do{
            if (!DiaryOperator::getInstance().checkLoginStatus()) {
                QMessageBox::information(this, "提示", "还未登录,请先登录.");
                this->putTipInfo("还未登录,请先登录.");
                break;
            }
            QString path = QFileDialog::getExistingDirectory(this, "请选择导出的pdf文件的保存位置", "/");
            if (path.isEmpty()) {//按了取消
                this->putTipInfo("打开文件失败,请重试.");
                break;
            }
            QString filename = path + "/nideriji_bak_"
                    + ui->editEmail->text() + "_"
                    + QDate::currentDate().toString("yyyy-MM-dd") + ".pdf";
            scriptExportToPdf(filename);
        } while (0);
        this->setEnabled(true);
    });

    connect(ui->btnImportFromJson,&QPushButton::clicked,[this](){
        this->setEnabled(false);
        do{
            if (!DiaryOperator::getInstance().checkLoginStatus()) {
                QMessageBox::information(this, "提示", "还未登录,请先登录.");
                this->putTipInfo("还未登录,请先登录.");
                break;
            }
            auto userSelect = QMessageBox::warning(this, "警告!!!",
                                 "警告!!! \"导入日记\"为实验性功能, 可能导致云端日记丢失. 确定要导入吗?",
                                 QMessageBox::Yes|QMessageBox::Cancel);
            if(QMessageBox::Yes != userSelect)
                break;
            this->putTipInfo("用户选择导入.");
            QString fileName = QFileDialog::getOpenFileName(this,
                     QStringLiteral("选择json文件"),"F:",QStringLiteral("文本文件(*json)"));
            this->putTipInfo("已选择json文件: " + fileName);
            this->scriptImportFromJson(fileName);
        } while (0);
        this->setEnabled(true);
    });

}

MainWindow::~MainWindow()
{
    delete ui;
}




void MainWindow::scriptExportToJson(const QString &filename){
    QFile file(filename);
    file.open(QIODevice::ReadWrite|QIODevice::Text);

    do{
        if(!file.exists()){
            QMessageBox::information(this, "提示", "打开文件失败,请重试.");
            break;
        }
        putTipInfo("开始导出json.");
        putTipInfo("打开文件成功.");
        putTipInfo("开始从服务器获取日记.");
        QJsonArray diaries = DiaryOperator::getInstance().getAllDiaryToJson();
        putTipInfo("共获取到" + QString::fromStdString(std::to_string(diaries.size()))+ "篇日记.");
        putTipInfo("开始写入json文件.");
        int64_t cntWrite = file.write( QJsonDocument(diaries).toJson() );
        if(cntWrite<=0){
            putTipInfo("写入json文件失败.");
            break;
        }
        putTipInfo("写入json文件成功.json文件保存位置:" + filename);
    }while(0);

    file.close();
}

void MainWindow::scriptExportToPdf(const QString &filename){
    //通过文件路径构造pdfWriter，并初始pdfWriter的一些设置
    QPdfWriter writer(filename);
    writer.setPageSize(QPdfWriter::A4);
    writer.setPageMargins(QMargins(00, 0, 0, 0));
    writer.setResolution(300);

    putTipInfo("开始导出pdf.");
    putTipInfo("打开文件成功.");
    putTipInfo("开始从服务器获取日记.");
    QJsonArray diaries = DiaryOperator::getInstance().getAllDiaryToJson();
    putTipInfo("共获取到" + QString::fromStdString(std::to_string(diaries.size()))+ "篇日记.");
    putTipInfo("开始写入pdf文件.");

    QString htmlStr;
    for(int i=0; i<diaries.size(); ++i){
        QJsonObject curDiary = diaries[i].toObject();
        if(!curDiary.value("title").isString()
                || !curDiary.value("createddate").isString()
                || !curDiary.value("content").isString()
                ){
            continue;
        }
        QString curTitle = curDiary.value("title").toString();
        QString curDate = curDiary.value("createddate").toString();
        QString curContent = curDiary.value("content").toString();
        htmlStr += "<hr>";
        htmlStr += "<h3>"+curTitle+"</h3>";
        htmlStr += "<h3>"+curDate+"</h3>";
        htmlStr += "<p>"+curContent+"</p>";
    }
    QTextDocument textDocument;
    textDocument.setHtml(htmlStr);
    textDocument.print(&writer);
    putTipInfo("生成pdf文件成功.pdf文件保存位置: " + filename);
}

void MainWindow::scriptImportFromJson(const QString &filename){
    QFile file(filename);
    file.open(QIODevice::ReadOnly|QIODevice::Text);

    do{
        if(!file.exists()){
            this->putTipInfo("打开文件失败. 请重试. ");
            break;
        }
        putTipInfo("打开文件成功.");
        std::string jsonStr;
        while(!file.atEnd()){
            jsonStr.append(file.read(1));
        }
        auto diariesJson = nlohmann::json::parse(jsonStr);
        if(!diariesJson.is_array()){
            qDebug()<<"json parse error. in import diary from json.";
            break;
        }
        int cntDiaryNum=0,cntSuccess=0,cntFail=0;
        for(auto it=diariesJson.begin(); it!=diariesJson.end(); ++it){
            QString curContent = QString::fromStdString( it->at("content").get<std::string>() );
            QString curDate = QString::fromStdString( it->at("createddate").get<std::string>() );
            bool curRes = DiaryOperator::getInstance().saveDiaryToDate(curContent, QDate::fromString(curDate, "yyyy-MM-dd"));
            QString curResDes = curRes?"成功":"失败";
            putTipInfo("导入第" + QString::number(++cntDiaryNum) + "篇日记" + curResDes);
            if(curRes)
                ++cntSuccess;
            else
                ++cntFail;
        }
        putTipInfo("json文件中共"+QString::number(cntDiaryNum)+"篇日记");
        putTipInfo(QString::number(cntSuccess)+"篇日记导入成功");
        putTipInfo(QString::number(cntFail)+"篇日记导入失败");
    }while(0);

    file.close();


}

void MainWindow::putTipInfo(const QString & info) {
    QString tipInformation = this->ui->textTipInformation->toPlainText();
    tipInformation += QTime::currentTime().toString()+": "+info+"\r\n";
    this->ui->textTipInformation->setPlainText(tipInformation);
    this->ui->textTipInformation->moveCursor(QTextCursor::End);
}
