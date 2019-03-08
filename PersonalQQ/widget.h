#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QUdpSocket>

namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT
    enum MsgType{Msg, UsrEnter, UsrLeft};   // 定义枚举，分别表示 聊天信息、新用户加入、用户退出

public:
    //explicit Widget(QWidget *parent = nullptr);
    explicit Widget(QWidget *parent, QString name);
    ~Widget();

private:
    Ui::Widget *ui;

signals:
    // 为了在窗口关闭的时候发送信号，告知外界是适合将isShow变为false了
    void closeWidget();
public:
    // 关闭事件
    void closeEvent(QCloseEvent *);

public:
    void sndMsg(MsgType type);  // 广播UDP消息
    void usrEnter(QString username);    //  处理新用户加入
    void usrLeft(QString username, QString time);   //  处理用户离开
    QString getUsr();   //  获取用户名
    QString getMsg();   //  获取聊天信息

private:
    QUdpSocket * udpSocket; //  UPD套接字
    qint16 port;    //  端口
    QString uName;  //  用户名

    void ReceiveMsg();  // 接收UDP消息
};

#endif // WIDGET_H
