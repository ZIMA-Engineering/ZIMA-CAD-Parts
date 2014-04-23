#ifndef SERVERTABWIDGET_H
#define SERVERTABWIDGET_H

#include <QWidget>

#include "serversmodel.h"

class FileModel;
class FileFilterModel;
class ProductView;


namespace Ui {
class ServerTabWidget;
}

class ServerTabWidget : public QWidget
{
    Q_OBJECT
    
public:
    explicit ServerTabWidget(ServersModel *serversModel, QWidget *parent = 0);
    ~ServerTabWidget();

public slots:
    void settingsChanged();

protected:
    void changeEvent(QEvent *event);
    
private:
    Ui::ServerTabWidget *ui;

    FileModel *m_fileModel;
    FileFilterModel *m_proxyFileModel;
    ServersModel *m_serversModel;
    ProductView *m_productView;

private slots:
    void techSpecUrlLineEdit_returnPressed();
    void techSpecGoButton_clicked();
    void techSpecPinButton_clicked();
    void partsIndexUrlLineEdit_returnPressed();
    void partsIndexGoButton_clicked();
    void partsIndexPinButton_clicked();

    void fileModel_requestColumnResize();

    void previewInProductView(const QModelIndex &index);
    void tree_doubleClicked(const QModelIndex &index);
};

#endif // SERVERTABWIDGET_H
