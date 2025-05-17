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
#include <QScrollBar>
#include <QDebug>
#include <QTimer>
#include <QSaveFile>
#include <QListWidgetItem>
#include <QCoreApplication>
#include <QDebug>
#include <QPushButton>
#include <yaml-cpp/yaml.h>
#include <QDir>
#include <iostream>
#include <fstream>
#include <iomanip>



//#include <typeinfo> //remove it if not used

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
    QString JSONpath, YAMLpath;

    QMap<QString, double> lastParamValues;

public:
    explicit ExecBox(QWidget *parent = nullptr, Ui::MainWindow* ui  = nullptr);

    QMap <QCheckBox*, QProcess*> CheckBoxProcessMap;
    QVector<QCheckBox* > checkBoxes;
    QMap <QString, QCheckBox*> label_checkbox_map;
    QVector<QString> commands;
    QMap<QString, QVector<Parameter>> commandParameterMap;
    QPushButton *set_default, *select_default;
    QVector<QCheckBox*> defaults;
    QStringList list_of_labels_of_defaults;

    void createConfigDir(QString configDirPath);
    void ReadJSON();
    void ReadYAML();
    void writeInYAML(QString command, Parameter param);
    void add_default_buttons();
    void SetupUI(QCheckBox* checkbox);
    void StartSession(QProcess* process, const QString& cmd, const QString& label);
    void StopSession(QProcess* process, const QString& label);
    double getOtherParamValue(QString command_label, QString other_param_name);
    void createSpoiler(const QString command_label, QVector<Parameter> parameters);
};

#endif // EXECBOX_H
