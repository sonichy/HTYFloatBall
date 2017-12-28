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
    i = 0;
    setStyleSheet("QLabel { color:white; padding:1px; border-radius:15px; }");
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::X11BypassWindowManagerHint);
    setAttribute(Qt::WA_TranslucentBackground, true);
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
    label->hide();

    labelFloat = new QLabel;
    labelFloat->setFixedSize(QSize(155,90));
    labelFloat->setWindowFlags(Qt::FramelessWindowHint | Qt::X11BypassWindowManagerHint);
    font.setPointSize(8);
    labelFloat->setFont(font);    
    labelFloat->setAlignment(Qt::AlignVCenter);
    labelFloat->setStyleSheet("QLabel { padding:2px; color:white; background-color:#000000; border-radius:15px; }");

    menu = new QMenu;
    action_boot_duration = new QAction("启动时间", menu);
    action_boot_analyze = new QAction("启动分析", menu);
    action_boot_record = new QAction("开机记录", menu);
    action_quit = new QAction("退出", menu);
    menu->addAction(action_boot_duration);
    menu->addAction(action_boot_analyze);
    menu->addAction(action_boot_record);
    menu->addAction(action_quit);
    connect(action_boot_duration, SIGNAL(triggered()), this, SLOT(showBootDuration()));
    connect(action_boot_analyze, SIGNAL(triggered()), this, SLOT(bootAnalyze()));
    connect(action_boot_record, SIGNAL(triggered()), this, SLOT(bootRecord()));
    connect(action_quit, SIGNAL(triggered()), qApp, SLOT(quit()));

    // 开机时长
    QProcess *process = new QProcess;
    process->start("systemd-analyze");
    process->waitForFinished();
    QString PO = process->readAllStandardOutput();
    QStringList SLSA = PO.split(" = ");
    QString SD = SLSA.at(1);
    if(SD.contains("min"))SD.replace("min","分");
    labelStartupDuration = new QLabel;
    labelStartupDuration->setText(SD.mid(0,SD.indexOf(".")) + "秒");
    labelStartupDuration->setFixedSize(QSize(200,150));
    labelStartupDuration->setWindowFlags(Qt::FramelessWindowHint | Qt::X11BypassWindowManagerHint);
    font.setPointSize(30);
    labelStartupDuration->setFont(font);
    labelStartupDuration->setAlignment(Qt::AlignCenter);
    labelStartupDuration->setStyleSheet("QLabel { padding:2px; color:white; background-color:#00FF00;}");
    labelStartupDuration->adjustSize();
    labelStartupDuration->move(QApplication::desktop()->width()-labelStartupDuration->width()-10, QApplication::desktop()->height()-labelStartupDuration->height()-50);
    labelStartupDuration->show();
    QTimer::singleShot(5000, this, SLOT(HSDSNS()));

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
    move(event->globalPos() + relativePos);
    labelFloat->move(x()-labelFloat->width()/2, y()-labelFloat->height()-5);
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
    QFile file("/proc/uptime");
    file.open(QIODevice::ReadOnly);
    QString l = file.readLine();
    file.close();
    QTime t = QTime(0,0,0);
    t = t.addSecs(l.left(l.indexOf(".")).toInt());
    QString uptime = "开机时长: " + t.toString("hh:mm:ss");

    file.setFileName("/proc/meminfo");
    file.open(QIODevice::ReadOnly);
    l = file.readLine();
    long int mt = l.mid(l.indexOf(":")+1, l.length()-13).replace(" ","").toInt();
    l = file.readLine();
    file.close();
    long int mf = l.mid(l.indexOf(":")+1, l.length()-11).replace(" ","").toInt();
    long int mu = mt - mf;
    QString musage = QString::number(mu*100/mt) + "%";
    QString mem = "内存: " + QString("%1 / %2 = %3").arg(KB(mu)).arg(KB(mt)).arg(musage);

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
    i++;
    if(i>2)i=2;

    file.setFileName("/proc/net/dev");
    file.open(QIODevice::ReadOnly);
    l = file.readLine();
    l = file.readLine();
    l = file.readLine();
    if(l.contains("lo",Qt::CaseInsensitive))l = file.readLine();
    file.close();
    QStringList list = l.split(QRegExp("\\s{1,}"));
    QString dss = "";
    QString uss = "";
    if(i > 0){
        long ds = list.at(1).toLong() - db;
        long us = list.at(9).toLong() - ub;
        dss = BS(ds) + "/s";
        uss = BS(us) + "/s";

        db = list.at(1).toLong();
        ub = list.at(9).toLong();
    }
    QString netspeed = "↑ " + uss + "\n↓ " + dss;
    QString net = "上传: " + BS(ub) + "  " + uss + "\n下载: " + BS(db) + "  " + dss;

    label->setText(netspeed);
    labelFloat->setText(uptime + "\nCPU: " + QString::number(cusage) + "%\n" + mem + "\n"+ net);
    QString SS ="";
    if(cusage<80){
        SS = QString("QLabel{ color:white; padding:1px; border-radius:15px; background-color: qlineargradient(spread:pad, x1:0, y1:1, x2:0, y2:0,"
                     "stop:0 rgba(0, 255, 0, 255), stop:%1 rgba(0, 255, 0, 255),"
                     "stop:%2 rgba(0, 0, 0, 255), stop:1 rgba(0, 0, 0, 255));}")
                .arg(cusage*1.0/100-0.001)
                .arg(cusage*1.0/100);
    }else{
        SS = QString("QLabel{ color:white; padding:1px; border-radius:15px; background-color: qlineargradient(spread:pad, x1:0, y1:1, x2:0, y2:0,"
                     "stop:0 rgba(255, 0, 0, 255), stop:%1 rgba(255, 0, 0, 255),"
                     "stop:%2 rgba(0, 0, 0, 255), stop:1 rgba(0, 0, 0, 255));}")
                .arg(cusage*1.0/100-0.001)
                .arg(cusage*1.0/100);
    }
    //qDebug() << SS;
    setStyleSheet(SS);

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

void MainWindow::HSDSNS()
{
    labelStartupDuration->hide();
    label->show();
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

void MainWindow::showBootDuration()
{
    labelStartupDuration->show();
    QTimer::singleShot(5000, labelStartupDuration, SLOT(hide()));
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
