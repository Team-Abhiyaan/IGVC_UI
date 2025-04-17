#ifndef EXECBOX_H
#define EXECBOX_H

#include <QWidget>
#include <QProcess>
#include "./ui_mainwindow.h"

class ExecBox : public QWidget
{
    Q_OBJECT

private:
    QProcess* process = nullptr;

public:
    explicit ExecBox(QWidget *parent = nullptr, Ui::MainWindow* ui  = nullptr);
    QProcess *electrical_process = nullptr
            , *zed_process = nullptr
            , *gps_process = nullptr
            , *waypoint_following_process = nullptr
            , *navigation_process = nullptr
            , *lane_detection_process = nullptr
            , *lane_following_process = nullptr
            , *idk_one_extra_process = nullptr;
    void StartSession(QProcess* process, const QString cmd);
    void StopSession(QProcess* process);
};

#endif // EXECBOX_H
