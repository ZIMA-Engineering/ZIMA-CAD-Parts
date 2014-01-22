#ifndef ABSTRACTPRODUCTVIEW_H
#define ABSTRACTPRODUCTVIEW_H

#include <QWidget>

#include "../../item.h"

class AbstractProductView : public QWidget
{
    Q_OBJECT
public:
    explicit AbstractProductView(QWidget *parent = 0);

    virtual QString title() = 0;
    virtual QList<File::FileTypes> canHandle() = 0;
    virtual bool handle(File *f) = 0;

signals:

public slots:

};

#endif // ABSTRACTPRODUCTVIEW_H
