#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QMenu>
#include <QAction>
#include <QContextMenuEvent>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    long int i, db, ub, tt0, idle0;
    QLabel *label, *labelFloat, *labelStartupDuration;
    QPoint relativePos;
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    QString KB(long k);
    QString BS(long b);
    void enterEvent(QEvent *event);
    void leaveEvent(QEvent *event);
    void contextMenuEvent(QContextMenuEvent *event);
    QMenu *menu;
    QAction *action_quit, *action_boot_record;
    void mouseDoubleClickEvent(QMouseEvent* event);

private slots:
    void refresh();
    void HSDSNS();
    void bootRecord();
};

#endif // MAINWINDOW_H
