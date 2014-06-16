#include "utils.h"
#include <QtCore>

int Utils::randInt(int low, int high)
{
    // Random number between low and high
    return qrand() % ((high + 1) - low) + low;
}

Utils::Utils()
{
}
