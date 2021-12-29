#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QMenu>
#include <QAction>
#include <QContextMenuEvent>
#include <QSystemTrayIcon>
#include <QSettings>
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
#include <QMessageBox>
#include <QPainter>
#include <QLineEdit>
#include <QComboBox>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    long int i, db, ub, dbt, ubt, dbt1, ubt1, dbt0, ubt0, tt0, idle0;
    int trayStyle=0;
    QString startup, text_float;
    QLabel *label, *label_float;
    QPoint relativePos;
    QString KB(long k);
    QString BS(long b);
    QMenu *menu;
    QSystemTrayIcon *systray;
    QSettings settings;

protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void enterEvent(QEvent *event);
    void leaveEvent(QEvent *event);
    void contextMenuEvent(QContextMenuEvent *event);
    void mouseDoubleClickEvent(QMouseEvent* event);

private slots:
    void refresh();
    void bootRecord();
    void bootAnalyze();
    void trayActivated(QSystemTrayIcon::ActivationReason reason);

};

#endif // MAINWINDOW_H