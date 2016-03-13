#ifndef LOGGING_H
#define LOGGING_H

#include <QDebug>

#define qEnter() qDebug() << "Entering" << Q_FUNC_INFO
#define _(x) #x << "=" << x
#define qReturn(x) qDebug() << "Leaving" << Q_FUNC_INFO; return x;

#endif // LOGGING_H

