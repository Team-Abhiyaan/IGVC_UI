#ifndef PARAMETER_H
#define PARAMETER_H
#include <QString>

class Parameter{
public:
    QString label;
    double min;
    double max;
    double initial;
    explicit Parameter(const QString& label = "", double min = 0, double max = 100, double initial =  10);
};

#endif // PARAMETER_H
