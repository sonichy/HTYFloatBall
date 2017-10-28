#include "mainwindow.h"
#include <QApplication>
#include <QDesktopWidget>
#include <QMouseEvent>
#include <QTimer>
#include <QTime>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    i=0;
    setStyleSheet("QLabel {padding:1px;}");
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::X11BypassWindowManagerHint);
    setFixedSize(QSize(60,35));
    move(QApplication::desktop()->width()-width()-10, QApplication::desktop()->height()-height()-50);
    QFont font;
    font.setPointSize(7);
    label = new QLabel(this);
    label->setText("↑ 0.00 K/s\n↓ 0.00 K/s");
    label->setFont(font);
    //label->setAlignment(Qt::AlignCenter);
    QTimer *timer = new QTimer;
    timer->setInterval(1000);
    timer->start();
    connect(timer, SIGNAL(timeout()), this, SLOT(refresh()));

    labelFloat = new QLabel;
    labelFloat->setFixedSize(QSize(150,90));
    labelFloat->setWindowFlags(Qt::FramelessWindowHint);
    font.setPointSize(8);
    labelFloat->setFont(font);
    labelFloat->setStyleSheet("padding:2px;");

    menu = new QMenu;
    action_quit = new QAction("退出",menu);
    menu->addAction(action_quit);
    connect(action_quit, SIGNAL(triggered()), qApp, SLOT(quit()));
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
    labelFloat->move(x(), y()-labelFloat->height()-5);
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
                s = QString::number(b/1.0,'f',2) + " B";
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
    long int mt = l.mid(l.indexOf(":")+1,l.length()-13).replace(" ","").toInt();
    l = file.readLine();
    file.close();
    long int mf = l.mid(l.indexOf(":")+1, l.length()-11).replace(" ","").toInt();
    long int mu = mt-mf;
    QString musage = QString::number(mu*100/mt) + "%";
    QString mem = "内存: "+QString("%1 / %2 = %3").arg(KB(mu)).arg(KB(mt)).arg(musage);

    file.setFileName("/proc/stat");
    file.open(QIODevice::ReadOnly);
    l = file.readLine();
    QByteArray ba;
    ba = l.toLatin1();
    const char *ch;
    ch = ba.constData();
    char cpu[5];
    long int user,nice,sys,idle,iowait,irq,softirq,tt;
    sscanf(ch,"%s%ld%ld%ld%ld%ld%ld%ld",cpu,&user,&nice,&sys,&idle,&iowait,&irq,&softirq);
    tt = user + nice + sys + idle + iowait + irq + softirq;
    file.close();
    QString cusage = "";
    if(i>0) cusage = QString::number(((tt-tt0)-(idle-idle0))*100/(tt-tt0)) + "%";
    idle0 = idle;
    tt0 = tt;
    i++;

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
    }
    db = list.at(1).toLong();
    ub = list.at(9).toLong();
    QString netspeed = "↑ " + uss + "\n↓ " + dss;
    QString net = "上传: " + BS(ub) + "  " + uss + "\n下载: " + BS(db) + "  " + dss;

    label->setText(netspeed);
    labelFloat->setText(uptime + "\nCPU: " + cusage + "\n" + mem + "\n"+ net);
}

void MainWindow::enterEvent(QEvent *event)
{
    Q_UNUSED(event);
    labelFloat->move(x(), y()-labelFloat->height()-5);
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
