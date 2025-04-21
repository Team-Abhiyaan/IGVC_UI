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

    void ReadFile();
    void SetupUI(QCheckBox* checkbox);
    void StartSession(QProcess* process, const QString& cmd, const QString& label);
    void StopSession(QProcess* process, const QString& label);
    void createSpoiler(const QString label);
};

#endif // EXECBOX_H
