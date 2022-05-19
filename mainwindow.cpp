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
        this->putTipInfo("尝试登录, 请稍候...");
        this->setEnabled(false);
       if(DiaryOperator::getInstance().checkLoginStatus()){
           this->putTipInfo("已登录过.");
           QMessageBox::information(this, "提示", "已登录过.");
           this->setEnabled(true);
           return ;
       }
       if(DiaryOperator::getInstance().login(this->ui->editEmail->text(), this->ui->editPasswd->text())){
           QMessageBox::information(this, "提示", "登录成功.");
           this->putTipInfo("登录成功.");
           this->setEnabled(true);
           return ;
       }
        QMessageBox::information(this, "提示", "登录失败或此账号未创建过日记,请重试.");
        this->putTipInfo("登录失败或此账号未创建过日记,请重试.");
        this->setEnabled(true);
    });

    connect(ui->btnExprotToJson,&QPushButton::clicked,[this]() {
        this->setEnabled(false);
        if (!DiaryOperator::getInstance().checkLoginStatus()) {
            QMessageBox::information(this, "提示", "还未登录,请先登录.");
            this->putTipInfo("还未登录,请先登录.");
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

    connect(ui->btnExprotToPdf,&QPushButton::clicked,[this](){
        QMessageBox::information(this, "提示", "该功能暂未实现.");
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
    putTipInfo("开始导入.");
    putTipInfo("打开文件成功.");
    putTipInfo("开始从服务器获取日记.");
    QJsonArray diaries = DiaryOperator::getInstance().getAllDiaryToJson();
    putTipInfo("共获取到" + QString::fromStdString(std::to_string(diaries.size()))+ "篇日记.");
    putTipInfo("开始写入json文件.");
    putTipInfo("json文件保存位置: " + filename);
    int64_t cntWrite = file.write( QJsonDocument(diaries).toJson() );
    if(cntWrite<=0){
        putTipInfo("写入json文件失败.");
        return;
    }
    file.close();
    putTipInfo("写入json文件成功, 共写入" + QString::fromStdString(std::to_string(cntWrite))+ "字节.");
}

void MainWindow::putTipInfo(const QString & info) {
    QString tipInformation = this->ui->textTipInformation->toPlainText();
    tipInformation += QTime::currentTime().toString()+": "+info+"\r\n";
    this->ui->textTipInformation->setPlainText(tipInformation);
    this->ui->textTipInformation->moveCursor(QTextCursor::End);
}
