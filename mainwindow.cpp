#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      settings(QApplication::organizationName(), QApplication::applicationName())
{
    i=db=ub=dbt=ubt=dbt1=ubt1=dbt0=ubt0=0;
    setStyleSheet("QLabel { color:white; padding:2px; border:1px solid white; border-radius:15px; }");
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::X11BypassWindowManagerHint);
    setAttribute(Qt::WA_TranslucentBackground, true);
    setAutoFillBackground(true);
    setFixedSize(QSize(60,35));
    int x0 = QApplication::desktop()->width() - width() - 20;
    int y0 = QApplication::desktop()->height() - height() - 50;
    int x = settings.value("X", x0).toInt();
    int y = settings.value("Y", y0).toInt();
    if (x >= QApplication::desktop()->width())
        x = x0;
    if (y >= QApplication::desktop()->height())
        y = y0;
    move(x, y);
    QFont font;
    font.setPointSize(7);
    label = new QLabel(this);
    label->setText("↑ 0.00 K/s\n↓ 0.00 K/s");
    label->setFont(font);
    //label->setAlignment(Qt::AlignCenter);
    setCentralWidget(label);
    bool b = settings.value("isShow", true).toBool();
    qDebug() << b;
    if (!b)
        this->hide();
    QTimer *timer = new QTimer;
    timer->setInterval(1000);
    timer->start();
    connect(timer, SIGNAL(timeout()), this, SLOT(refresh()));

    label_float = new QLabel;
    label_float->setFixedSize(QSize(155,110));
    label_float->setWindowFlags(Qt::FramelessWindowHint | Qt::X11BypassWindowManagerHint);
    font.setPointSize(8);
    label_float->setFont(font);
    label_float->setAlignment(Qt::AlignVCenter);
    label_float->setStyleSheet("QLabel { padding:2px; color:white; background-color:rgba(0,0,0,200); border-radius:15px; }");//圆角无效

    //托盘
    trayStyle = settings.value("TrayStyle", 0).toInt();
    systray = new QSystemTrayIcon(this);
    systray->setToolTip("系统监控");
    systray->setIcon(QIcon(":/icon.png"));
    systray->setVisible(true);
    QMenu *traymenu = new QMenu(this);
    QAction *action_showhide = new QAction("显示", traymenu);
    if (!b)
        action_showhide->setText("隐藏");    
    action_showhide->setIcon(QIcon::fromTheme("computer"));
    QAction *action_about = new QAction("关于", traymenu);    
    action_about->setIcon(QIcon::fromTheme("about"));
    QAction *action_set = new QAction("设置", traymenu);
    action_set->setIcon(QIcon::fromTheme("set"));
    QAction *action_quit = new QAction("退出", traymenu);    
    action_quit->setIcon(QIcon::fromTheme("quit"));
    traymenu->addAction(action_showhide);
    traymenu->addAction(action_about);
    traymenu->addAction(action_set);
    traymenu->addAction(action_quit);
    systray->setContextMenu(traymenu);
    systray->show();
    connect(systray, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(trayActivated(QSystemTrayIcon::ActivationReason)));
    connect(action_showhide, &QAction::triggered, [=](){
        if (label->isVisible()) {
            this->hide();
            action_showhide->setText("显示");
            settings.setValue("isShow", false);
        } else {
            this->show();
            action_showhide->setText("隐藏");
            settings.setValue("isShow", true);
        }
    });
    connect(action_about, &QAction::triggered, [](){
        QMessageBox MB(QMessageBox::NoIcon, "关于", "海天鹰浮球 2.3\n一款基于Qt的内存使用率托盘和网速浮窗。\n作者：海天鹰\nE-mail: sonichy@163.com\n主页：https://github.com/sonichy");
        MB.setIconPixmap(QPixmap(":/HTYFB.png"));
        MB.exec();
    });
    connect(action_set, &QAction::triggered, [=](){
        QDialog *dialog = new QDialog;
        dialog->setWindowTitle("设置");
        dialog->setFixedWidth(300);
        QVBoxLayout *vbox = new QVBoxLayout;
        QHBoxLayout *hbox = new QHBoxLayout;
        QLabel *label = new QLabel("系统监视器");
        hbox->addWidget(label);
        QLineEdit *lineEdit = new QLineEdit;
        lineEdit->setText(settings.value("system-monitor", "deepin-system-monitor").toString());
        hbox->addWidget(lineEdit);
        vbox->addLayout(hbox);

        hbox = new QHBoxLayout;
        label = new QLabel("托盘样式");
        hbox->addWidget(label);
        QComboBox *comboBox_trayStyle = new QComboBox;
        QStringList SL;
        SL << "圆弧" << "双竖线";
        comboBox_trayStyle->addItems(SL);
        comboBox_trayStyle->setCurrentIndex(settings.value("TrayStyle", 0).toInt());
        hbox->addWidget(comboBox_trayStyle);
        vbox->addLayout(hbox);

        hbox = new QHBoxLayout;
        hbox->addStretch();
        QPushButton *pushButton_confirm = new QPushButton;
        pushButton_confirm->setText("确定");
        hbox->addWidget(pushButton_confirm);
        QPushButton *pushButton_cancel = new QPushButton;
        pushButton_cancel->setText("取消");
        hbox->addWidget(pushButton_cancel);
        hbox->addStretch();
        vbox->addLayout(hbox);
        dialog->setLayout(vbox);
        connect(pushButton_confirm, SIGNAL(pressed()), dialog, SLOT(accept()));
        connect(pushButton_cancel, SIGNAL(pressed()), dialog, SLOT(reject()));
        if (dialog->exec() == QDialog::Accepted) {
            settings.setValue("system-monitor", lineEdit->text());
            trayStyle = comboBox_trayStyle->currentIndex();
            settings.setValue("TrayStyle", trayStyle);
            dialog->close();
        }
    });
    connect(action_quit, SIGNAL(triggered()), qApp, SLOT(quit()));

    //右键
    menu = new QMenu;
    menu->setStyleSheet("QMenu { color:white; background:rgba(0,0,0,200); }"
                        //"QMenu::item { background:rgba(0,0,0,200);}"
                        "QMenu::item:selected { background:rgba(100,100,100,100); }");//无效
    //menu->setAttribute(Qt::WA_TranslucentBackground,true);
    //menu->setAutoFillBackground(true);
    QAction *action_hide = new QAction("隐藏", menu);
    QAction *action_boot_analyze = new QAction("启动分析", menu);
    QAction *action_boot_record = new QAction("开机记录", menu);
    action_quit = new QAction("退出", menu);
    menu->addAction(action_hide);
    menu->addAction(action_boot_analyze);
    menu->addAction(action_boot_record);
    menu->addAction(action_quit);
    connect(action_hide, &QAction::triggered, [=](){
        this->hide();
        action_showhide->setText("显示");
        settings.setValue("isShow", false);
    });
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
    label_float->move(x() - label_float->width()/2, y() - label_float->height() - 5);
}

void MainWindow::mouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    setCursor(Qt::ArrowCursor);
    settings.setValue("X", x());
    settings.setValue("Y", y());
}

QString MainWindow::KB(long k)
{
    QString s = "";
    if (k > 999999) {
        s = QString::number(k/(1024*1024.0),'f',2) + " GB";
    } else {
        if (k > 999) {
            s = QString::number(k/1024.0,'f',2) + " MB";
        } else {
            s = QString::number(k/1.0,'f',2) + " KB";
        }
    }
    return s;
}

QString MainWindow::BS(long b)
{
    QString s = "";
    if (b > 999999999) {
        s = QString::number(b/(1024*1024*1024.0),'f',2) + " GB";
    } else {
        if (b > 999999) {
            s = QString::number(b/(1024*1024.0),'f',2) + " MB";
        } else {
            if (b > 999) {
                s = QString::number(b/1024.0,'f',2) + " KB";
            } else {
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
    int mp = static_cast<int>(mu*100/mt);
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
    int cp = 0;
    if (i>0) cp = static_cast<int>(100 -(idle-idle0)*100/(tt-tt0));
    idle0 = idle;
    tt0 = tt;

    // 网速
    file.setFileName("/proc/net/dev");
    file.open(QIODevice::ReadOnly);
    l = file.readLine();
    l = file.readLine();
    dbt1=ubt1=0;
    while (!file.atEnd()) {
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
    text_float = startup + "\n" + uptime + "\nCPU: " + QString::number(cp) + "%\n" + mem + "\n" + net;
    label_float->setText(text_float);
    systray->setToolTip(text_float);

    QString SS = "";
    if (mp < 90) {
        SS = QString("background-color: qlineargradient(spread:pad, x1:0, y1:1, x2:0, y2:0,"
                     "stop:0 rgba(0,200,0,200), stop:%1 rgba(0,200,0,200),"
                     "stop:%2 rgba(0,0,0,200), stop:1 rgba(0,0,0,200));")
                .arg(mp*1.0/100-0.001)
                .arg(mp*1.0/100);
    } else {
        SS = QString("background-color: qlineargradient(spread:pad, x1:0, y1:1, x2:0, y2:0,"
                     "stop:0 rgba(255,0,0,200), stop:%1 rgba(255,0,0,200),"
                     "stop:%2 rgba(0,0,0,200), stop:1 rgba(0,0,0,200));")
                .arg(mp*1.0/100-0.001)
                .arg(mp*1.0/100);
    }
    //qDebug() << SS;
    label->setStyleSheet(SS);

    //修改托盘图标
    QPixmap pixmap(128,128);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing, true);

    if (trayStyle == 0) {
        //环形
        QPen pen;
        if (mp<90)
            pen.setColor(Qt::green);
        else
            pen.setColor(Qt::red);
        pen.setWidth(10);
        painter.setPen(pen);
        painter.drawArc(pixmap.rect().adjusted(10,10,-10,-10), 0, 360*16*mp/100);
        QFont font;
        font.setPointSize(50);
        painter.setFont(font);
        painter.drawText(pixmap.rect(), Qt::AlignCenter, QString::number(mp));
    } else if (trayStyle == 1) {
        //双竖线
        QColor color;
        if (mp < 90)
            color = Qt::green;
        else
            color = Qt::red;
        painter.fillRect(0, pixmap.height() - mp*pixmap.height()/100, pixmap.width()/2-5, mp*pixmap.height()/100, color);
        if (cp < 90)
            color = Qt::green;
        else
            color = Qt::red;
        painter.fillRect(pixmap.width()/2+5, pixmap.height() - cp*pixmap.height()/100, pixmap.width()/2-1, cp*pixmap.height()/100, color);
    }
    systray->setIcon(QIcon(pixmap));
}

void MainWindow::enterEvent(QEvent *event)
{
    Q_UNUSED(event);
    label_float->move(x() - label_float->width()/2, y()-label_float->height() - 5);
    label_float->show();
}

void MainWindow::leaveEvent(QEvent *event)
{
    Q_UNUSED(event);
    label_float->hide();
}

void MainWindow::contextMenuEvent(QContextMenuEvent *event)
{
    menu->exec(QCursor::pos());
    event->accept();
}

void MainWindow::mouseDoubleClickEvent(QMouseEvent* event)
{
    Q_UNUSED(event);
    QProcess::startDetached(settings.value("system-monitor","deepin-system-monitor").toString());
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
    QPushButton *pushButton_confirm = new QPushButton("确定");
    hbox->addStretch();
    hbox->addWidget(pushButton_confirm);
    hbox->addStretch();
    vbox->addLayout(hbox);
    dialog->setLayout(vbox);
    dialog->show();
    connect(pushButton_confirm, SIGNAL(clicked()), dialog, SLOT(accept()));
    if (dialog->exec() == QDialog::Accepted) {
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
    QPushButton *pushButton_confirm = new QPushButton("确定");
    hbox->addStretch();
    hbox->addWidget(pushButton_confirm);
    hbox->addStretch();
    vbox->addLayout(hbox);
    dialog->setLayout(vbox);
    dialog->show();
    connect(pushButton_confirm, SIGNAL(clicked()), dialog, SLOT(accept()));
    if (dialog->exec() == QDialog::Accepted) {
        dialog->close();
    }
}

void MainWindow::trayActivated(QSystemTrayIcon::ActivationReason reason)
{
    qDebug() << "QSystemTrayIcon::ActivationReason" << reason;
    switch (reason) {
    case QSystemTrayIcon::Trigger:
        systray->showMessage("实时监控", text_float, QSystemTrayIcon::MessageIcon::Information, 9000);
        break;
    case QSystemTrayIcon::DoubleClick: //Deepin不支持
        QProcess::startDetached(settings.value("system-monitor","deepin-system-monitor").toString());
        break;
    case QSystemTrayIcon::MiddleClick:
        QProcess::startDetached(settings.value("system-monitor","deepin-system-monitor").toString());
        break;
    default:
        break;
    }
}