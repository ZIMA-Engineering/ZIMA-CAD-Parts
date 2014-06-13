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
    void setPartsIndex(const QModelIndex &index);
    void techSpecsIndexOverwrite(Item *item);

signals:
    void changeSettings();

protected:
    void changeEvent(QEvent *event);
    
private:

    enum Tabs {
        TECH_SPECS,
        PARTS,
        DOWNLOADS,
        TABS_COUNT
    };

    Ui::ServerTabWidget *ui;

    FileModel *m_fileModel;
    FileFilterModel *m_proxyFileModel;
    ServersModel *m_serversModel;
    ProductView *m_productView;

    Item *lastPartsIndexItem;
    QUrl lastPartsIndex;
    QDateTime lastPartsIndexModTime;

    void viewHidePartsIndex(Item *item);

private slots:
    void techSpecUrlLineEdit_returnPressed();
    void techSpecGoButton_clicked();
    void techSpecPinButton_clicked();
    void partsIndexUrlLineEdit_returnPressed();
    void partsIndexGoButton_clicked();
    void partsIndexPinButton_clicked();

    void fileModel_requestColumnResize();
    void techSpec_urlChanged(const QUrl &url);
    void partsWebView_urlChanged(const QUrl &url);

    void previewInProductView(const QModelIndex &index);
    void partsTreeView_doubleClicked(const QModelIndex &index);

    void filesDeleted();
    void downloadButton();
    void updateClicked();
    void deleteSelectedParts();
    void partsIndexLoaded(const QModelIndex &index);

    void toggleDownload();
    void resumeDownload();
    void stopDownload();
    void adjustThumbColumnWidth(int width);
    void loadTechSpec(const QUrl &url);
    void partsIndexOverwrite(Item *item);

    void setFiltersDialog();
};

#endif // SERVERTABWIDGET_H
