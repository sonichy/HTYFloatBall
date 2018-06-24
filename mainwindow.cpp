#include "mainwindow.h"
#include <QApplication>
#include <QDesktopWidget>
#include <QMouseEvent>
#include <QTimer>
#include <QTime>
#include <QDebug>
#include <QProcess>
#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTextBrowser>
#include <QPushButton>
#include <QDate>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    i=db=ub=dbt=ubt=dbt1=ubt1=dbt0=ubt0=0;
    //i=0;db=0;ub=0;dbt=0;ubt=0;dbt1=0;ubt1=0;dbt0=0;ubt0=0;
    setStyleSheet("QLabel { color:white; padding:2px; border-radius:15px; }"
                  "QToolTip { color:white; border-style:none; background-color:black; }");
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::X11BypassWindowManagerHint);
    setAttribute(Qt::WA_TranslucentBackground,true);
    setAutoFillBackground(true);
    setFixedSize(QSize(60,35));
    move(QApplication::desktop()->width()-width()-20, QApplication::desktop()->height()-height()-50);
    QFont font;
    font.setPointSize(7);
    label = new QLabel(this);
    label->setText("↑ 0.00 K/s\n↓ 0.00 K/s");
    label->setFont(font);
    //label->setAlignment(Qt::AlignCenter);
    setCentralWidget(label);
    QTimer *timer = new QTimer;
    timer->setInterval(1000);
    timer->start();
    connect(timer, SIGNAL(timeout()), this, SLOT(refresh()));

    labelFloat = new QLabel;
    labelFloat->setFixedSize(QSize(155,110));
    labelFloat->setWindowFlags(Qt::FramelessWindowHint | Qt::X11BypassWindowManagerHint);
    font.setPointSize(8);
    labelFloat->setFont(font);
    labelFloat->setAlignment(Qt::AlignVCenter);
    labelFloat->setStyleSheet("QLabel { padding:2px; color:white; background-color:#000000; border-radius:15px; }");

    menu = new QMenu;
    menu->setStyleSheet("QMenu { color:white; background:rgba(0,0,0,200); }"
                        //"QMenu::item { background:rgba(0,0,0,200);}"
                        "QMenu::item:selected { background:rgba(100,100,100,100); }");
    //menu->setAttribute(Qt::WA_TranslucentBackground,true);
    //menu->setAutoFillBackground(true);
    action_boot_analyze = new QAction("启动分析", menu);
    action_boot_record = new QAction("开机记录", menu);
    action_quit = new QAction("退出", menu);
    menu->addAction(action_boot_analyze);
    menu->addAction(action_boot_record);
    menu->addAction(action_quit);
    connect(action_boot_analyze, SIGNAL(triggered()), this, SLOT(bootAnalyze()));
    connect(action_boot_record, SIGNAL(triggered()), this, SLOT(bootRecord()));
    connect(action_quit, SIGNAL(triggered()), qApp, SLOT(quit()));

    // 开机时长
    QProcess *process = new QProcess;
    process->start("systemd-analyze");
    process->waitForFinished();
    QString PO = process->readAllStandardOutput();
    QString SD = PO.mid(PO.indexOf("=") + 1, PO.indexOf("\n") - PO.indexOf("=") - 1);
    SD.replace("min"," 分");
    SD.replace("ms"," 毫秒");
    SD.replace("s"," 秒");
    startup = "启动: " + SD;
}

MainWindow::~MainWindow()
{

}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton){
        relativePos = pos()- event->globalPos();
    }
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    setCursor(Qt::ClosedHandCursor);
    move(event->globalPos() + relativePos);
    labelFloat->move(x()-labelFloat->width()/2, y()-labelFloat->height()-5);
}

void MainWindow::mouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    setCursor(Qt::ArrowCursor);
}

QString MainWindow::KB(long k)
{
    QString s = "";
    if(k > 999999){
        s = QString::number(k/(1024*1024.0),'f',2) + " GB";
    }else{
        if(k > 999){
            s = QString::number(k/1024.0,'f',2) + " MB";
        }else{
            s = QString::number(k/1.0,'f',2) + " KB";
        }
    }
    return s;
}

QString MainWindow::BS(long b)
{
    QString s = "";
    if(b > 999999999){
        s = QString::number(b/(1024*1024*1024.0),'f',2) + " GB";
    }else{
        if(b > 999999){
            s = QString::number(b/(1024*1024.0),'f',2) + " MB";
        }else{
            if(b > 999){
                s = QString::number(b/1024.0,'f',2) + " KB";
            }else{
                s = QString::number(b) + " B";
            }
        }
    }
    return s;
}

void MainWindow::refresh()
{
    // 开机
    QFile file("/proc/uptime");
    file.open(QIODevice::ReadOnly);
    QString l = file.readLine();
    file.close();
    QTime t = QTime(0,0,0);
    t = t.addSecs(l.left(l.indexOf(".")).toInt());
    QString uptime = "开机: " + t.toString("hh:mm:ss");

    // 内存
    file.setFileName("/proc/meminfo");
    file.open(QIODevice::ReadOnly);
    l = file.readLine();
    long mt = l.replace("MemTotal:","").replace("kB","").replace(" ","").toLong();
    l = file.readLine();
    l = file.readLine();
    long ma = l.replace("MemAvailable:","").replace("kB","").replace(" ","").toLong();
    l = file.readLine();
    l = file.readLine();
    file.close();
    long mu = mt - ma;
    int mp = mu*100/mt;
    QString mem = "内存: " + QString("%1/%2=%3").arg(KB(mu)).arg(KB(mt)).arg(QString::number(mp) + "%");

    // CPU
    file.setFileName("/proc/stat");
    file.open(QIODevice::ReadOnly);
    l = file.readLine();
    QByteArray ba;
    ba = l.toLatin1();
    const char *ch;
    ch = ba.constData();
    char cpu[5];
    long int user, nice, sys, idle, iowait, irq, softirq, tt;
    sscanf(ch, "%s%ld%ld%ld%ld%ld%ld%ld", cpu, &user, &nice, &sys, &idle, &iowait, &irq, &softirq);
    tt = user + nice + sys + idle + iowait + irq + softirq;
    file.close();
    int cusage = 0;
    if(i>0) cusage = 100 -(idle-idle0)*100/(tt-tt0);
    idle0 = idle;
    tt0 = tt;

    // 网速
    file.setFileName("/proc/net/dev");
    file.open(QIODevice::ReadOnly);
    l = file.readLine();
    l = file.readLine();
    dbt1=ubt1=0;
    while(!file.atEnd()){
        l = file.readLine();
        QStringList list = l.split(QRegExp("\\s{1,}"));
        db = list.at(1).toLong();
        ub = list.at(9).toLong();
        dbt1 += db;
        ubt1 += ub;
    }
    file.close();
    QString dss = "";
    QString uss = "";
    if(i > 0){
        long ds = dbt1 - dbt0;
        long us = ubt1 - ubt0;
        dss = BS(ds) + "/s";
        uss = BS(us) + "/s";
        dbt0 = dbt1;
        ubt0 = ubt1;
    }
    QString netspeed = "↑ " + uss + "\n↓ " + dss;
    QString net = "上传: " + BS(ubt1) + "  " + uss + "\n下载: " + BS(dbt1) + "  " + dss;

    i++;
    if(i>2)i=2;

    // 绘图
    label->setText(netspeed);
    labelFloat->setText(startup + "\n" + uptime + "\nCPU: " + QString::number(cusage) + "%\n" + mem + "\n"+ net);
    //setToolTip(startup + "\n" + uptime + "\nCPU: " + QString::number(cusage) + "%\n" + mem + "\n"+ net);
    QString SS ="";
    if(mp<90){
        SS = QString("color:white; padding:1px; border-radius:15px; background-color: qlineargradient(spread:pad, x1:0, y1:1, x2:0, y2:0,"
                     "stop:0 rgba(0,200,0,200), stop:%1 rgba(0,200,0,200),"
                     "stop:%2 rgba(0,0,0,200), stop:1 rgba(0,0,0,200));")
                .arg(mp*1.0/100-0.001)
                .arg(mp*1.0/100);
    }else{
        SS = QString("color:white; padding:1px; border-radius:15px; background-color: qlineargradient(spread:pad, x1:0, y1:1, x2:0, y2:0,"
                     "stop:0 rgba(255,0,0,200), stop:%1 rgba(255,0,0,200),"
                     "stop:%2 rgba(0,0,0,200), stop:1 rgba(0,0,0,200));")
                .arg(mp*1.0/100-0.001)
                .arg(mp*1.0/100);
    }
    //qDebug() << SS;
    label->setStyleSheet(SS);

}

void MainWindow::enterEvent(QEvent *event)
{
    Q_UNUSED(event);
    labelFloat->move(x()-labelFloat->width()/2, y()-labelFloat->height()-5);
    labelFloat->show();
}

void MainWindow::leaveEvent(QEvent *event)
{
    Q_UNUSED(event);
    labelFloat->hide();
}

void MainWindow::contextMenuEvent(QContextMenuEvent *event)
{
    menu->exec(QCursor::pos());
    event->accept();
}

void MainWindow::mouseDoubleClickEvent(QMouseEvent* event)
{
    Q_UNUSED(event);
    QProcess *process = new QProcess;
    process->start("deepin-system-monitor");
    //process->start("gnome-system-monitor");
}

void MainWindow::bootRecord()
{
    QProcess *process = new QProcess;
    process->start("last reboot");
    process->waitForFinished();
    QString PO = process->readAllStandardOutput();
    QDialog *dialog = new QDialog;
    dialog->setWindowTitle("开机记录");
    dialog->setFixedSize(500,400);
    QVBoxLayout *vbox = new QVBoxLayout;
    QTextBrowser *textBrowser = new QTextBrowser;
    textBrowser->setText(PO);
    textBrowser->zoomIn();
    vbox->addWidget(textBrowser);
    QHBoxLayout *hbox = new QHBoxLayout;
    QPushButton *btnConfirm = new QPushButton("确定");
    hbox->addStretch();
    hbox->addWidget(btnConfirm);
    hbox->addStretch();
    vbox->addLayout(hbox);
    dialog->setLayout(vbox);
    dialog->show();
    connect(btnConfirm, SIGNAL(clicked()), dialog, SLOT(accept()));
    if(dialog->exec() == QDialog::Accepted){
        dialog->close();
    }
}

void MainWindow::bootAnalyze()
{
    QProcess *process = new QProcess;
    process->start("systemd-analyze blame");
    process->waitForFinished();
    QString PO = process->readAllStandardOutput();
    QDialog *dialog = new QDialog;
    dialog->setWindowTitle("启动进程耗时");
    dialog->setFixedSize(500,400);
    QVBoxLayout *vbox = new QVBoxLayout;
    QTextBrowser *textBrowser = new QTextBrowser;
    textBrowser->setText(PO);
    textBrowser->zoomIn();
    vbox->addWidget(textBrowser);
    QHBoxLayout *hbox = new QHBoxLayout;
    QPushButton *btnConfirm = new QPushButton("确定");
    hbox->addStretch();
    hbox->addWidget(btnConfirm);
    hbox->addStretch();
    vbox->addLayout(hbox);
    dialog->setLayout(vbox);
    dialog->show();
    connect(btnConfirm, SIGNAL(clicked()), dialog, SLOT(accept()));
    if(dialog->exec() == QDialog::Accepted){
        dialog->close();
    }
}
