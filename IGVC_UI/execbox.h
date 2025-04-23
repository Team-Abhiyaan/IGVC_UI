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
#include <QDebug>
#include <QTimer>
#include <QListWidgetItem>
#include <QCoreApplication>
#include <QDebug>
#include <yaml-cpp/yaml.h>
#include <iostream>
#include <fstream>


#include <typeinfo> //remove it if not used

#include "parameter.h"
#include "./ui_mainwindow.h"

class ExecBox : public QWidget
{
    Q_OBJECT

private:
    Ui::MainWindow* m_ui;
    QMap<QString, QString> ScriptOutputMap;
    QTimer* updateTimer;
    QString currentSelectedLabel;
    QString lastShownOutput;
    QString working_directory;

public:
    explicit ExecBox(QWidget *parent = nullptr, Ui::MainWindow* ui  = nullptr);

    QMap <QCheckBox*, QProcess*> CheckBoxProcessMap;
    QVector<QCheckBox* > checkBoxes;
    QVector<QString> commands;

    QMap<QString, QVector<parameter>> commandParameterMap;

    void ReadJSON();
    void ReadYAML();
    void writeInYAML(QString command, parameter param);
    void SetupUI(QCheckBox* checkbox);
    void StartSession(QProcess* process, const QString& cmd, const QString& label);
    void StopSession(QProcess* process, const QString& label);
    void createSpoiler(const QString command_label, QVector<parameter> parameters);
};

#endif // EXECBOX_H
