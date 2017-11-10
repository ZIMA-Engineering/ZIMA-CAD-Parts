#ifndef SERVERTABWIDGET_H
#define SERVERTABWIDGET_H

#include <QWidget>

#include "serversmodel.h"

class FileModel;
class FileFilterModel;
class ProductView;
class QWebEngineView;

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
	void updateDirectory(const QString &rootPath);

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

	Ui::ServerTabWidget *ui;

	ProductView *m_productView;

	void loadIndexHtml(const QString &rootPath, QWebEngineView *webView, const QString &filterBase, bool hideIfNotFound);
	void editIndexFile(const QString &path);

private slots:
	void techSpecUrlLineEdit_returnPressed();
	void techSpecGoButton_clicked();
	void techSpecPinButton_clicked();
	void techSpecEditButton_clicked();
	void partsIndexUrlLineEdit_returnPressed();
	void partsIndexGoButton_clicked();
	void partsIndexPinButton_clicked();
	void partsIndexEditButton_clicked();
	void refreshButton_clicked();

	void techSpec_urlChanged(const QUrl &url);
	void partsWebView_urlChanged(const QUrl &url);

	void previewInProductView(const QFileInfo &fi);

	void deleteSelectedParts();

	void adjustThumbColumnWidth(int width);

	void setFiltersDialog();
};

#endif // SERVERTABWIDGET_H
