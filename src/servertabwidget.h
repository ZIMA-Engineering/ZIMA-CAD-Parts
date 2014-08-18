#ifndef SERVERTABWIDGET_H
#define SERVERTABWIDGET_H

#include <QWidget>

#include "serversmodel.h"

class FileModel;
class FileFilterModel;
class ProductView;
class QWebView;

namespace Ui {
class ServerTabWidget;
}


/*!
 * \brief The tab widget displaying "Parts" tabs.
 *
 */
class ServerTabWidget : public QWidget
{
	Q_OBJECT

public:
    explicit ServerTabWidget(QWidget *parent = 0);
	~ServerTabWidget();

public slots:
    void setDirectory(const QString &rootPath);

	void settingsChanged();

signals:
	void changeSettings();

protected:
	void changeEvent(QEvent *event);

private:

	enum Tabs {
	    TECH_SPECS,
	    PARTS,
	    TABS_COUNT
	};

	Ui::ServerTabWidget *ui;

	ProductView *m_productView;

    void loadIndexHtml(const QString &rootPath, QWebView *webView, const QString &filterBase, bool hideIfNotFound);

private slots:
	void techSpecUrlLineEdit_returnPressed();
	void techSpecGoButton_clicked();
	void techSpecPinButton_clicked();
	void partsIndexUrlLineEdit_returnPressed();
	void partsIndexGoButton_clicked();
	void partsIndexPinButton_clicked();

	void techSpec_urlChanged(const QUrl &url);
	void partsWebView_urlChanged(const QUrl &url);

	void previewInProductView(const QModelIndex &index);
	void partsTreeView_doubleClicked(const QModelIndex &index);

	void filesDeleted();
	void deleteSelectedParts();

	void adjustThumbColumnWidth(int width);

	void setFiltersDialog();
};

#endif // SERVERTABWIDGET_H
