#include "parameter.h"

Parameter::Parameter(const QString& label, double min, double max, double initial)
    : label(label), min(min), max(max), initial(initial) {}
