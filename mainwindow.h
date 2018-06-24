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
    long int i, db, ub, dbt, ubt, dbt1, ubt1, dbt0, ubt0, tt0, idle0;
    QString startup;
    QLabel *label, *labelFloat;
    QPoint relativePos;
    QString KB(long k);
    QString BS(long b);
    QMenu *menu;
    QAction *action_quit, *action_boot_record, *action_boot_duration, *action_boot_analyze;
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

};

#endif // MAINWINDOW_H
