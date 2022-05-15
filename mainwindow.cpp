#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include "DiaryOperator.hpp"

#include <QMessageBox>
#include <QFileDialog>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->btnLogin,&QPushButton::clicked,[this](){
        this->setEnabled(false);
       if(DiaryOperator::getInstance().checkLoginStatus()){
           QMessageBox::information(this, "提示", "已登录过.");
           this->setEnabled(true);
           return ;
       }
       if(DiaryOperator::getInstance().login(this->ui->editEmail->text(), this->ui->editPasswd->text())){
           QMessageBox::information(this, "提示", "登录成功.");
           this->setEnabled(true);
           return ;
       }
        QMessageBox::information(this, "提示", "登录失败或此账号未创建过日记,请重试.");
        this->setEnabled(true);
    });
    connect(ui->btnExprotToJson,&QPushButton::clicked,[this]() {
        this->setEnabled(false);
        if (!DiaryOperator::getInstance().checkLoginStatus()) {
            QMessageBox::information(this, "提示", "还未登录,请先登录.");
            this->setEnabled(true);
            return;
        }
        QString path = QFileDialog::getExistingDirectory(this, "请选择导出的json文件的保存位置", "/");
        if (path.isEmpty()) {//按了取消
            this->setEnabled(true);
            return;
        }
        QString filename = path + "/nideriji.json";
        scriptExportToJson(filename);
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
    if(!file.exists()){
        QMessageBox::information(this, "提示", "打开文件失败,请重试.");
        return;
    }
    QString tipInformation = this->ui->textTipInformation->toPlainText();
    tipInformation += "开始导入\r\n打开文件成功\r\n";
    tipInformation += "开始从服务器获取日记.\r\n";
    this->ui->textTipInformation->setPlainText(tipInformation);
    this->ui->textTipInformation->moveCursor(QTextCursor::End);
    QJsonArray diaries = DiaryOperator::getInstance().getAllDiaryToJson();
    tipInformation += "共获取到" + QString::fromStdString(std::to_string(diaries.size()))+ "篇日记.\r\n";
    tipInformation += "开始写入json文件...\r\n";
    tipInformation += "json文件保存位置: " + filename+"\r\n";
    this->ui->textTipInformation->setPlainText(tipInformation);
    this->ui->textTipInformation->moveCursor(QTextCursor::End);
    int64_t cntWrite = file.write( QJsonDocument(diaries).toJson() );
    if(cntWrite<=0){
        tipInformation += "写入json文件失败.\r\n";
        return;
    }
    file.close();
    tipInformation += "写入json文件成功, 共写入" + QString::fromStdString(std::to_string(cntWrite))+ "字节.\r\n";
    this->ui->textTipInformation->setPlainText(tipInformation);
    this->ui->textTipInformation->moveCursor(QTextCursor::End);
#if 0
    std::string jsonStr;
    while (!file.atEnd()) {
        jsonStr.append(file.read(1));
    }
    qDebug()<<jsonStr.size();
    QJsonDocument diarysJson=QJsonDocument::fromJson(QByteArray::fromStdString( jsonStr) );
//    qDebug()<<diarysJson.array().size() ;
    QJsonArray diarys = diarysJson.array();
    int cntS=0,cntF=0;
    for(int i=0; i<diarys.size(); ++i){
        qDebug()<<QString::fromStdString(std::to_string(i)+"/"+std::to_string(diarys.size()));
        QJsonObject curDiary = diarys.at(i).toObject();
        QString rawContent = curDiary.value("content").toString();
        QDate date = QDate::fromString(curDiary.value("createddate").toString(),"yyyy-MM-dd");
        if(DiaryOperator::getInstance().saveDiaryToDate(QString::fromStdString(chiperContent),date)){
            qDebug()<<"导入成功";
            ++cntS;
        }
        else{
            qDebug()<<"导入失败";
            ++cntF;
        }
        qDebug()<<QStringLiteral("完成导入日记: ")+QString::fromStdString(std::to_string(i)+"/"+std::to_string(diarys.size()));
    }
    qDebug()<<("共"+std::to_string(diarys.size())+"篇日记").data();
    qDebug()<<("导入成功"+std::to_string(cntS)+", 失败"+std::to_string(cntF)).data();
    file.close();
#endif
}
