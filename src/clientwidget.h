#ifndef CLIENTWIDGET_H
#define CLIENTWIDGET_H

#include <QWidget>
#include <QTcpSocket>
#include <QFile>
#include <QTimer>

namespace Ui {
class ClientWidget;
}

class ClientWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ClientWidget(QWidget *parent = 0);
    ~ClientWidget();

    void sendData();        // 发送文件信息
    void showRestartServer();   // 显示重连服务端提示
    void sendGSMUser();     // 发送GSMUser数据
    void sendGSMPassword(); // 发送GSMPassword数据
    void sendGSMApn();      // 发送GSMApn数据

private slots:
    void on_pushButtonConnnect_clicked();

    void on_pushButtonOpen_clicked();

    void on_pushButtonSend_clicked();

    void on_checkBoxUser_clicked();

    void on_checkBoxPassword_clicked();

    void on_checkBoxApn_clicked();

    void on_pushButtonReadGsm_clicked();

    void on_pushButtonSetGsm_clicked();

    void on_pushButtonResetGSM_clicked();

private:
    Ui::ClientWidget *ui;

    QTcpSocket *tcpSocket;  // socket实例化

    QFile file;         // 文件对象
    QString fileName;   // 文件名字
    qint64 fileSize;    // 文件大小
    qint64 recvSize;    // 已经接收文件的大小
    qint64 sendSize;    // 已经发送文件的大小

    bool isStart;       // 标志位,是否为头部信息

    QTimer timer;       // 定时器
    QTimer timerRestartServer;    // 重连服务端提示

    QTimer timerSendGSMUser;
    QTimer timerSendGSMPassword;
    QTimer timerSendGSMApn;
    QTimer timerSendResetGSMConfig;       // 重启GSM配置

    QString update_gateway_name = "Lowan_Gateway";      // 升级包名字
    QString config_pipes_name = "pipes.json";           // 配置pipe文件
    QString head;                                       // 文件头部信息

    QString GSMUser;            // GSM用户信息
    QString GSMPassword;        // GSM用户密码
    QString GSMApn;             // GSMAPN

    bool boolCheckGSMUser;           //
    bool boolCheckGSMPassword;
    bool boolCheckGSMAPN;

};

#endif // CLIENTWIDGET_H
