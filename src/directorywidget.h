#ifndef DIRECTORYWIDGET_H
#define DIRECTORYWIDGET_H

#include <QWidget>

#include "datasourcemodel.h"

class FileModel;
class FileFilterModel;
class ProductView;
class QWebEngineView;

namespace Ui {
class DirectoryWidget;
}


/*!
 * \brief The tab widget displaying "Parts" tabs.
 *
 */
class DirectoryWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DirectoryWidget(QWidget *parent = 0);
    ~DirectoryWidget();

public slots:
    void setDirectory(const QString &rootPath);
    void updateDirectory(const QString &rootPath);
    void openAboutPage();

    void settingsChanged();

signals:
    void changeSettings();
    void openPartDirectory(const QFileInfo &fi);

protected:
    void changeEvent(QEvent *event);

private:

    enum Tabs {
        TECH_SPECS,
        PARTS,
        TABS_COUNT
    };

    Ui::DirectoryWidget *ui;

    ProductView *m_productView;

    void loadIndexHtml(const QString &rootPath, QWebEngineView *webView, const QString &filterBase, bool hideIfNotFound);
    void editIndexFile(const QString &path);

private slots:
    void dirWebViewUrlLineEdit_returnPressed();
    void dirWebViewGoButton_clicked();
    void dirWebViewPinButton_clicked();
    void dirWebViewEditButton_clicked();
    void partsIndexUrlLineEdit_returnPressed();
    void partsIndexGoButton_clicked();
    void partsIndexPinButton_clicked();
    void partsIndexEditButton_clicked();
    void refreshButton_clicked();

    void dirWebView_urlChanged(const QUrl &url);
    void partsWebView_urlChanged(const QUrl &url);

    void previewInProductView(const QFileInfo &fi);

    void moveSelectedParts();
    void deleteSelectedParts();

    void adjustThumbColumnWidth(int width);

    void setFiltersDialog();
};

#endif // DIRECTORYWIDGET_H
