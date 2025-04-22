#include "parameter.h"

parameter::parameter(const QString& label, int min, int max, int initial)
    : label(label), min(min), max(max), initial(initial) {}
