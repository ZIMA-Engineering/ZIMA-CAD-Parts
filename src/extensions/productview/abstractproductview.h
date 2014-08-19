#ifndef ABSTRACTPRODUCTVIEW_H
#define ABSTRACTPRODUCTVIEW_H

#include <QWidget>

#include "file.h"

/**
 * @brief The abstract base for all "file preview" processing
 *
 * So called "product view" is a handler for file type displayed
 * in FileModel.
 * Any file type must inherit from AbstractProductView and then
 * it has to register itself in ProductView constructor as
 * template <class T> void addProviders()
 */
class AbstractProductView : public QWidget
{
	Q_OBJECT
public:
	explicit AbstractProductView(QWidget *parent = 0);

    //! Returns title for current product view
	virtual QString title() = 0;
    //! Returns a list with file types available in current product type
    virtual FileTypeList canHandle() = 0;
    //! Display the file
    virtual bool handle(FileMetadata *f) = 0;

signals:

public slots:

};

#endif // ABSTRACTPRODUCTVIEW_H
