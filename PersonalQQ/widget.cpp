#include "widget.h"
#include "ui_widget.h"
#include <QMessageBox>
#include <QDateTime>
#include <QColorDialog>
#include <QFileDialog>
#include <QFile>

Widget::Widget(QWidget *parent, QString name) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);

    udpSocket = new QUdpSocket(this);
    this->uName = name;
    this->port = 9999;

    //   绑定端口号  绑定模式  共享地址，断线重连
    udpSocket->bind(this->port,QUdpSocket::ShareAddress |QUdpSocket::ReuseAddressHint);

    sndMsg(UsrEnter);   //发送新用户进入

    //点击发送按钮发送消息
    connect(ui->pbSend,&QPushButton::clicked,[=](){
        sndMsg(Msg);
    });

    connect(udpSocket,&QUdpSocket::readyRead, this, &Widget::ReceiveMsg);   //监听别人发送的数据

    connect(ui->pbExit,&QPushButton::clicked, [=](){
        this->close();
    });

    // set font style
    connect(ui->fontCbs,&QFontComboBox::currentFontChanged,[=](const QFont &font){
         ui->msgTxtEdit->setCurrentFont(font);
         ui->msgTxtEdit->setFocus();
    });

    // set font size
    // 因为currentIndexChanged被重载了，所以要用函数指针去明确的指向某一个。
    void(QComboBox::* sig)(const QString &text) = &QComboBox::currentIndexChanged;
    connect(ui->sizeCbx,sig,[=](const QString &text){
        ui->msgTxtEdit->setFontPointSize(text.toDouble());
        ui->msgTxtEdit->setFocus();
    });

    // set bold
    connect(ui->btnBold,&QToolButton::clicked,[=](bool ischecked){
        if(ischecked)
        {
            ui->msgTxtEdit->setFontWeight(QFont::Bold);
        }
        else
        {
            ui->msgTxtEdit->setFontWeight(QFont::Normal);
        }

    });

    // set Italic
    connect(ui->btnItalic, &QToolButton::clicked,[=](bool ischecked){
        ui->msgTxtEdit->setFontItalic(ischecked);
    });

    // set Underline
    connect(ui->btnUnderline,&QToolButton::clicked,[=](bool check){
        ui->msgTxtEdit->setFontUnderline(check);
    });

    // set font color
    connect(ui->btnColor,&QToolButton::clicked,[=](){
        QColor col = QColorDialog::getColor(Qt::red);   // 使其一打开选择的是红色
        ui->msgTxtEdit->setTextColor(col);
    });

    // clear record of chat
    connect(ui->btnClear,&QToolButton::clicked,[=](){
        ui->msgBrower->clear();
    });

    // download record of chat
    connect(ui->btnDownload,&QToolButton::clicked,[=](){
        if(ui->msgBrower->document()->isEmpty())
        {
            QMessageBox::warning(this,"Warning..","Content cannot be empty");
            return;

        }
        else
        {
            QString time = QDateTime::currentDateTime().toString("yyyy-MM-dd-hh-mm-ss");
            QString fileName = QString("ChatRecord") + "-" + this->uName + "-"  + time;
            QString path = QFileDialog::getSaveFileName(this,"Save",fileName,"(*.txt)");
            QFile file(path);
            file.open(QIODevice::WriteOnly);
            QTextStream stream(&file);
            stream << ui->msgBrower ->toPlainText();
            file.close();
        }
    });

}

Widget::~Widget()
{
    delete ui;
}


void Widget::closeEvent(QCloseEvent *e) // 点叉号会自动触发，执行this->close()也会触发
{
    emit this->closeWidget();

    sndMsg(UsrLeft);

    // Close socket
    udpSocket->close();
    udpSocket->destroyed();

    QWidget::closeEvent(e); // 让父类接着处理可能的事情
}


void Widget::sndMsg(MsgType type)  // 广播UDP消息
{
    // 消息分三样：聊天信息、新用户加入、用户退出
    QByteArray arr;
    QDataStream stream(&arr, QIODevice::WriteOnly); // 设置为写模式，并将写的内容放到arr中

    stream << type << getUsr();
    switch (type) {
    case Msg:   // 普通消息发送
        if(ui->msgTxtEdit->toPlainText() == "")
        {
            QMessageBox::warning(this, "Warning..", "The sent content cannot be empty");
            return ;
        }
        stream << getMsg();
        break;
    case UsrEnter:  // 新用户进入
        break;
    case UsrLeft:   // 用户离开
        break;
    default:
        break;
    }

    //书写报文  广播发送
    udpSocket->writeDatagram(arr, QHostAddress::Broadcast, port);   // 要发送的内容，对方IP，端口号

}

void Widget::ReceiveMsg()
{
    qint64 size = udpSocket->pendingDatagramSize(); //获取接收到的报文的长度
    QByteArray arr  = QByteArray(size, 0);  //新建一个空间用于接收报文
    udpSocket->readDatagram(arr.data(), size); //拿到数据报文

    QDataStream stream(&arr, QIODevice::ReadOnly);  // 数据流形式读取

    int msgType;
    QString usrname;
    QString msg;


    QString time = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");    //获取时间

    stream >> msgType;

    switch (msgType) {
    case Msg:   // 普通聊天
        stream >> usrname >> msg;
        ui->msgBrower->setTextColor(Qt::blue);
        ui->msgBrower->append("[" + usrname + "]" + time);
        ui->msgBrower->append(msg);
        break;
    case UsrEnter:  // 用户进入
        stream >> usrname;
        usrEnter(usrname);
        break;
    case UsrLeft:   // 用户离开
        stream >> usrname;
        usrLeft(usrname, time);
        break;
    default:
        break;
    }

}


QString Widget::getMsg()   //  获取聊天信息
{
    QString str = ui->msgTxtEdit->toHtml(); // 获取msgTxtEdit中输入的内容

    ui->msgTxtEdit->clear();    //  清空输入页面的内容
    ui->msgTxtEdit->setFocus(); //  将光标移到开头

    return str;
}

QString Widget::getUsr()
{
    return this->uName;
}


void Widget::usrEnter(QString username)    //  处理新用户加入
{
    bool isempty = ui->userTWidget->findItems(username, Qt::MatchExactly).isEmpty();
    if(isempty)
    {
        // Update TableWidget
        QTableWidgetItem * usr = new QTableWidgetItem(username);
        ui->userTWidget->insertRow(0);
        ui->userTWidget->setItem(0,0,usr);

        // Update msgBrower
        ui->msgBrower->setTextColor(Qt::gray);
        ui->msgBrower->append(QString("%1 is online Now.").arg(username));

        // Update person onlie
        ui->labelNumOnline->setText(QString("Person online: %1").arg(ui->userTWidget->rowCount()));

        // Broadcast self information
        sndMsg(UsrEnter);
    }
}

void Widget::usrLeft(QString username, QString time)   //  处理用户离开
{
    // Update TableWidget
    bool isempty = ui->userTWidget->findItems(username, Qt::MatchExactly).isEmpty();

    if(!isempty)
    {
        // Update TableWidget by remove row
        int row = ui->userTWidget->findItems(username, Qt::MatchExactly).first()->row();    // find row
        ui->userTWidget->removeRow(row);

        // Update msgBrower
        ui->msgBrower->setTextColor(Qt::gray);
        ui->msgBrower->append(QString("%1 leave on %2").arg(username).arg(time));

        // Update person onlie
        ui->labelNumOnline->setText(QString("Person online: %1").arg(ui->userTWidget->rowCount()));
    }
}





























