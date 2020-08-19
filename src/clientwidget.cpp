#include "clientwidget.h"
#include "ui_clientwidget.h"
#include <QHostAddress>
#include <QFileDialog>
#include <QFileInfo>
#include <QDebug>
#include <QLayout>
#include <QMessageBox>

ClientWidget::ClientWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ClientWidget)
{
    ui->setupUi(this);

    tcpSocket = new QTcpSocket(this);       // 实例化

    isStart = true;                         // 默认有头部信息

    ui->progressBar->setValue(0);           // 默认值0

    setWindowTitle("Gateway_Tools_Client"); // 设置窗口Title

    // 默认两个按钮都不可按
    ui->pushButtonOpen->setEnabled(false);
    ui->pushButtonSend->setEnabled(false);

    // 设置默认勾选状态
    ui->checkBoxUser->setChecked(false);
    ui->checkBoxPassword->setChecked(false);
    ui->checkBoxApn->setChecked(false);

    boolCheckGSMUser = false;
    boolCheckGSMPassword = false;
    boolCheckGSMAPN = false;

    // 默认情况下读取按钮不可按
    ui->pushButtonReadGsm->setEnabled(false);
    ui->pushButtonSetGsm->setEnabled(false);
    ui->pushButtonResetGSM->setEnabled(false);

    connect(tcpSocket, &QTcpSocket::connected,
    [=]()
    {
        // 提示连接成功
        ui->textEdit->clear();
        ui->textEdit->append("Connect to the server successfully, wait for the file package to be sent...");

        QMessageBox::information(NULL, "Message Notification", "Connect to the server successfully.");
        // 连接服务器成功,才能选择文件
        ui->pushButtonOpen->setEnabled(true);
    }
    );

    connect(&timer, &QTimer::timeout,
    [=]()
    {
        // 关闭定时器
        timer.stop();

        // 发送文件
        sendData();
    }

            );


    connect(&timerRestartServer, &QTimer::timeout,
    [=]()
    {
        // 关闭定时器
        timerRestartServer.stop();

        // 发送文件
        showRestartServer();
    }

            );


    connect(&timerSendGSMUser, &QTimer::timeout,
    [=]()
    {
        // 关闭定时器
        timerSendGSMUser.stop();

        // 发送
        sendGSMUser();
    }

            );

    connect(&timerSendGSMPassword, &QTimer::timeout,
    [=]()
    {
        // 关闭定时器
        timerSendGSMPassword.stop();

        // 发送
        sendGSMPassword();
    }

            );

    connect(&timerSendGSMApn, &QTimer::timeout,
    [=]()
    {
        // 关闭定时器
        timerSendGSMApn.stop();

        // 发送
        sendGSMApn();
    }

            );


}

ClientWidget::~ClientWidget()
{
    delete ui;
}

void ClientWidget::on_pushButtonConnnect_clicked()
{
    // 获取服务器的IP和Port
    QString ip = ui->lineEditIP->text();
    qint64 port = ui->lineEditPort->text().toInt();

    // 主动和服务器连接
    tcpSocket->connectToHost(QHostAddress(ip), port);

    isStart = true;

    ui->progressBar->setValue(0);       // 设置进度条
}

void ClientWidget::on_pushButtonOpen_clicked()
{
    QString filePath = QFileDialog::getOpenFileName(this, "open", "../");
    if(false == filePath.isEmpty())     // 如果选择文件路径有效
    {
        fileName.clear();
        fileSize = 0;

        // 获取文件信息
        QFileInfo info(filePath);
        fileName = info.fileName();     // 获取文件名字
        fileSize = info.size();         // 获取文件大小

        sendSize = 0;                   // 发送文件的大小

        // 只读方式打开文件
        // 指定文件的名字
        file.setFileName(filePath);

        // 打开文件
        bool isOk = file.open(QIODevice::ReadOnly);
        if(false == isOk)
        {
            qDebug() << "只读方式打开文件失败 80";
        }

        // 提示文件打开的路径
        ui->textEdit->append(filePath);

        ui->pushButtonOpen->setEnabled(false);
        ui->pushButtonSend->setEnabled(true);
    }
}

void ClientWidget::on_pushButtonSend_clicked()
{
    ui->pushButtonSend->setEnabled(false);

    // 根据文件类型进行发送升级头部信息
    if(QString::compare(update_gateway_name, fileName) == 0)      // 升级网关程序
    {
        // 先发送文件头信息 文件名##文件大小
        head = QString("update_gateway##%1##%2").arg(fileName).arg(fileSize);
    }else if(QString::compare(config_pipes_name, fileName) == 0)  // 配置pipe信息
    {
        // 先发送文件头信息 文件名##文件大小
        head = QString("config_pipe##%1##%2").arg(fileName).arg(fileSize);
    }
    // 先发送文件头信息 文件名##文件大小
//    QString head = QString("update_gateway##%1##%2").arg(fileName).arg(fileSize);

    // 发送头部信息
    qint64 len = tcpSocket->write(head.toUtf8());
    if(len > 0)     // 头部信息发送成功
    {
        // 发送真正的文件信息
        // 防止TCP黏包
        // 需要通过定时器延时20ms
        timer.start(20);
    }else {
        qDebug() << "头部信息发送失败";
        file.close();

        ui->pushButtonOpen->setEnabled(true);
        ui->pushButtonSend->setEnabled(false);
    }

}


void ClientWidget::sendData()
{
    // 根据文件类型进行发送升级头部信息
    if(QString::compare(update_gateway_name, fileName) == 0)      // 升级网关程序
    {
        ui->textEdit->append("Sending Upgrade Package...");
    }else if(QString::compare(config_pipes_name, fileName) == 0)  // 配置pipe信息
    {
        ui->textEdit->append("Sending Configuration Package...");
    }
//    ui->textEdit->append("正在发送文件...");

    ui->progressBar->setMinimum(0);     // 最小值
    ui->progressBar->setMaximum(fileSize/1024);   // 最大值
    ui->progressBar->setValue(0);       // 当前值

    qint64 len = 0;

    do{
        // 每次发送文件的大小
        char buff[1024*4] = {0};
        len = 0;

        // 从文件中读取数据
        len = file.read(buff, sizeof(buff));

        // 发送数据,读取多,发送多少
        len = tcpSocket->write(buff, len);

        // 发送数据需要累计
        sendSize += len;

        // 更新进度条
        ui->progressBar->setValue(sendSize/1024);

    }while (len > 0);


    // 是否发送文件完毕
    if(sendSize == fileSize)
    {
        ui->textEdit->append("Package upgrade successfully.");
        ui->textEdit->append("The Gateway Application Will Restart OK After 15 Seconds.");
        file.close();

        ui->pushButtonOpen->setEnabled(true);

        timerRestartServer.start(20*1000);      // 20s


        // 关闭客户端
//        tcpSocket->disconnectFromHost();
//        tcpSocket->close();


    }
}

void ClientWidget::showRestartServer()
{
    ui->textEdit->append("Now You Can Reconnect To The Server.");
    QMessageBox::information(NULL, "Message Notification", "Now You Can Reconnect To The Server.");
}



void ClientWidget::on_checkBoxUser_clicked()
{

    if(ui->checkBoxUser->isChecked())
    {
        ui->textEdit->append("Now You choose user CheckBox.");
        ui->pushButtonSetGsm->setEnabled(true);
        boolCheckGSMUser = true;
    }else
    {
        ui->textEdit->append("Now You release user CheckBox.");
        boolCheckGSMUser = false;
    }

}

void ClientWidget::on_checkBoxPassword_clicked()
{

    if(ui->checkBoxPassword->isChecked())
    {
        ui->textEdit->append("Now You choose Password CheckBox.");
        ui->pushButtonSetGsm->setEnabled(true);
        boolCheckGSMPassword = true;
    }else{
        ui->textEdit->append("Now You release Password CheckBox.");
        boolCheckGSMPassword = false;
    }

}

void ClientWidget::on_checkBoxApn_clicked()
{

    if(ui->checkBoxApn->isChecked())
    {
        ui->textEdit->append("Now You choose APN CheckBox.");
        ui->pushButtonSetGsm->setEnabled(true);
        boolCheckGSMAPN = true;
    }else{
        ui->textEdit->append("Now You release APN CheckBox.");
        boolCheckGSMAPN = false;
    }

}

// 按下读取GSM按钮
// 1.判断选中状态,如果选中,组帧选中内容项,发送帧数据
// 2.等待状态恢复信息,显示在textEdit
void ClientWidget::on_pushButtonReadGsm_clicked()
{

    // QString("%1, %2").arg(*, *)
    ui->textEdit->append("Now You clicked Read Parameters Button.");


}


// 按下设置GSM按钮
// 1.判断选中状态,如果选中,组帧选中内容项,发送帧数据
// 2.等待状态恢复信息,显示在textEdit
void ClientWidget::on_pushButtonSetGsm_clicked()
{
    ui->textEdit->append("Now You clicked Set Parameters Button.");



    if(boolCheckGSMUser)
    {
        GSMUser = ui->lineEditUser->text();
//        if(QString::compare(GSMUser, "") == 0)
//        {
//            ui->textEdit->append("Please Enter the Parameters First");
//        }else{
//            timerSendGSMUser.start(10);
//        }
        timerSendGSMUser.start(10);



    }
    if(boolCheckGSMPassword)
    {
        GSMPassword  = ui->lineEditPassword->text();
//        if(QString::compare(GSMPassword, "") == 0)
//        {
//            ui->textEdit->append("Please Enter the Parameters First");
//        }else{
//            timerSendGSMPassword.start(100);
//        }
        timerSendGSMPassword.start(100);

    }
    if(boolCheckGSMAPN)
    {
        GSMApn  = ui->lineEditApn->text();
//        if(QString::compare(GSMApn, "") == 0)
//        {
//            ui->textEdit->append("Please Enter the Parameters First");
//        }else{
//            timerSendGSMApn.start(200);
//        }
        timerSendGSMApn.start(200);


    }
    ui->pushButtonResetGSM->setEnabled(true);
}

void ClientWidget::sendGSMUser()
{

    QString headUser = NULL;
    headUser = QString("config_gsm##user##%1##%2").arg(GSMUser).arg(GSMUser.length());
    ui->textEdit->append(headUser);
    // 发送信息
    qint64 lenUser = tcpSocket->write(headUser.toUtf8());
    if(lenUser > 0)     // 信息发送成功
    {
        ui->textEdit->append("User Data Send Successfully.");
    }else{
        ui->textEdit->append("User Data Send Failed.");
    }
    GSMUser.clear();
}

void ClientWidget::sendGSMPassword()
{
    QString headPasword = NULL;
    headPasword = QString("config_gsm##password##%1##%2").arg(GSMPassword).arg(GSMPassword.length());
    ui->textEdit->append(headPasword);
    qint64 lenPassword = tcpSocket->write(headPasword.toUtf8());
    if(lenPassword > 0)     // 信息发送成功
    {

        ui->textEdit->append("Password Data Send Successfully.");
    }else{
        ui->textEdit->append("Password Data Send Failed.");
    }
    GSMPassword.clear();
}

void ClientWidget::sendGSMApn()
{
    QString headDataApn = NULL;
    headDataApn = QString("config_gsm##apn##%1##%2").arg(GSMApn).arg(GSMApn.length());
    ui->textEdit->append(headDataApn);
    qint64 lenApn = tcpSocket->write(headDataApn.toUtf8());
    if(lenApn > 0)     // 信息发送成功
    {

        ui->textEdit->append("Apn Data Send Successfully.");
    }else{
        ui->textEdit->append("Apn Data Send Failed.");
    }
    GSMApn.clear();
}

void ClientWidget::on_pushButtonResetGSM_clicked()
{
    QString headDataResetGSMConfig = NULL;
    qint64 lenResetGSMConfig = 0;
    headDataResetGSMConfig = QString("reset_gsm##reboot##6");
    QMessageBox:: StandardButton result= QMessageBox::information(NULL, "Message Notification", "Confirm To Restart The GSM Config?",QMessageBox::Yes|QMessageBox::No);
    switch (result)
    {
    case QMessageBox::Yes:
        qDebug()<<"Yes";
        ui->textEdit->append(headDataResetGSMConfig);
        lenResetGSMConfig = tcpSocket->write(headDataResetGSMConfig.toUtf8());
        if(lenResetGSMConfig > 0)     // 信息发送成功
        {
            ui->textEdit->append("ResetGSMConfig Data Send Successfully.");
        }else{
            ui->textEdit->append("ResetGSMConfig Data Send Failed.");
        }
        break;
    case QMessageBox::No:
        qDebug()<<"NO";
        break;
    default:
        break;
    }
}
