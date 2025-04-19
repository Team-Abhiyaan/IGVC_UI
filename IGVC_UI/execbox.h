#ifndef EXECBOX_H
#define EXECBOX_H

#include <QWidget>
#include <QProcess>
#include <QMap>
#include <QFile>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonParseError>
#include <QDebug>
#include <QCheckBox>
#include <QScrollArea>


#include "./ui_mainwindow.h"

class ExecBox : public QWidget
{
    Q_OBJECT

private:
    Ui::MainWindow* m_ui;

public:
    explicit ExecBox(QWidget *parent = nullptr, Ui::MainWindow* ui  = nullptr);

    QMap <QCheckBox*, QProcess*> CheckBoxProcessMap;
    QVector<QCheckBox* > checkBoxes;
    QVector<QString> commands;

    void ReadFile();
    void SetupUI(QCheckBox* checkbox);
    void StartSession(QProcess* process, const QString& cmd);
    void StopSession(QProcess* process);
};

#endif // EXECBOX_H
