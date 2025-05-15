
#ifndef SPOILER_H
#define SPOILER_H
#include <QFrame>
#include <QGridLayout>
#include <QParallelAnimationGroup>
#include <QScrollArea>
#include <QDebug>
#include <QFile>
#include <QToolButton>
#include <QWidget>

class Spoiler : public QWidget {
    Q_OBJECT
private:
    QGridLayout mainLayout;
    QFrame headerLine;
    QParallelAnimationGroup toggleAnimation;
    QScrollArea contentArea;
    int animationDuration{300};
public:
    QToolButton playButton;
    explicit Spoiler(const QString & title = "", const int animationDuration = 300, QWidget *parent = nullptr);
    void setContentLayout(QLayout & contentLayout);
    QToolButton toggleButton;
};
#endif
