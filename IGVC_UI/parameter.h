#ifndef PARAMETER_H
#define PARAMETER_H
#include <QString>

class parameter{
public:
    QString label;
    int min;
    int max;
    int initial;
    explicit parameter(const QString& label = "", int min = 0, int max = 100, int initial =  10);
};

#endif // PARAMETER_H
