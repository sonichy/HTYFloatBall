#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QMenu>
#include <QAction>
#include <QContextMenuEvent>
#include <QSystemTrayIcon>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    long int i, db, ub, dbt, ubt, dbt1, ubt1, dbt0, ubt0, tt0, idle0;
    QString startup, text_float;
    QLabel *label, *label_float;
    QPoint relativePos;
    QString KB(long k);
    QString BS(long b);
    QMenu *menu;
    QSystemTrayIcon *systray;

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