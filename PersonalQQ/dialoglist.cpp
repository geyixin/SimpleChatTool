#include "dialoglist.h"
#include "ui_dialoglist.h"
#include <QToolButton>
#include "widget.h"
#include <QMessageBox>
#include <QDebug>

Dialoglist::Dialoglist(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Dialoglist)
{
    ui->setupUi(this);

    // Set ui Title
    setWindowTitle("PersonalQQ 2019");
    // Set ui Icon
    setWindowIcon(QPixmap(":/images/qq.png"));

    // Prepare Title
    QStringList titleNameList;
    titleNameList << "spqy" << "ymrl" << "qq" << "Cherry" << "dr" << "jj" << "lswh" << "qmnn" << "wy";
    // Prepare Icon
    QList<QString> iconList;
    iconList << "spqy" << "ymrl" << "qq" << "Cherry" << "dr" << "jj" << "lswh" << "qmnn" << "wy";

    QVector<QToolButton *> vTb;

    for(int i=0; i <9; i++)
    {
        QToolButton * btn = new QToolButton;

        // Set people image
        QString str = QString(":/images/%1.png").arg(iconList.at(i));
        btn->setIcon(QPixmap(str));

        // Set people name
        btn->setText(titleNameList.at(i));

        btn->setIconSize(QPixmap(str).size());
        btn->setAutoRaise(true);    // Set btn style: transparency
        btn->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);  // show picture and text together
        ui->vLayout->addWidget(btn);    // add btn to ui
        vTb.push_back(btn);
        isShow.push_back(false);
    }

    // Signal Slot
    for(int i=0; i<vTb.size(); i++)
    {
        // 想在lambda中修改内容，可能需要加mutable。也有可能是需要在类中维护一个全局变量
        connect(vTb.at(i), &QToolButton::clicked, [=](){
            if(isShow[i])   // 控制每个用户的界面只打开一个
            {
                QString str = QString("%1窗口已被打开").arg(vTb.at(i)->text());
                QMessageBox::warning(this,"Warning..",str);
                return ;
            }

            isShow[i] = true;
            // (参数1，参数2)，参数1为0表示以顶层方式弹出，
            // qDebug() << vTb.at(i)->text() << "Connected...";
            Widget * w = new Widget(0,vTb[i]->text());
            w->setWindowTitle(vTb.at(i)->text());   // Set Title
            w->setWindowIcon(vTb.at(i)->icon());
            w->show();

            connect(w, &Widget::closeWidget,[=](){
                isShow[i] = false;
            });
        });
    }



}

Dialoglist::~Dialoglist()
{
    delete ui;
}
