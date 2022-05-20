#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include "DiaryOperator.hpp"

#include <QMessageBox>
#include <QFileDialog>
#include <QPdfWriter>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->btnLogin,&QPushButton::clicked,[this](){
        this->putTipInfo("尝试登录, 请稍候...");
        this->setEnabled(false);
        do{
//            if(DiaryOperator::getInstance().checkLoginStatus()){
//                this->putTipInfo("已登录过.");
//                QMessageBox::information(this, "提示", "已登录过.");
//                break;
//            }
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
            QString filename = path + "/nideriji.json";
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
            QString filename = path + "/nideriji.pdf";
            scriptExportToPdf(filename);
        } while (0);
        this->setEnabled(true);
    });

    connect(ui->btnImportFromJson,&QPushButton::clicked,[this](){
        QMessageBox::information(this, "提示", "该功能暂未实现.");
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
    putTipInfo("开始导出json.");
    putTipInfo("打开文件成功.");
    putTipInfo("开始从服务器获取日记.");
    QJsonArray diaries = DiaryOperator::getInstance().getAllDiaryToJson();
    putTipInfo("共获取到" + QString::fromStdString(std::to_string(diaries.size()))+ "篇日记.");
    putTipInfo("开始写入json文件.");
    int64_t cntWrite = file.write( QJsonDocument(diaries).toJson() );
    if(cntWrite<=0){
        putTipInfo("写入json文件失败.");
        return;
    }
    file.close();
    putTipInfo("写入json文件成功.json文件保存位置:" + filename);
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


void MainWindow::putTipInfo(const QString & info) {
    QString tipInformation = this->ui->textTipInformation->toPlainText();
    tipInformation += QTime::currentTime().toString()+": "+info+"\r\n";
    this->ui->textTipInformation->setPlainText(tipInformation);
    this->ui->textTipInformation->moveCursor(QTextCursor::End);
}
