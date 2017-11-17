#include "datasourcewidget.h"
#include "datasourceview.h"
#include "settings.h"

#include <QtDebug>


DataSourceWidget::DataSourceWidget(QWidget *parent)
	: QWidget(parent)
{
	setupUi(this);

	dsList->setStyleSheet(NavBar::loadStyle(":/styles/office2003gray.css"));

	splitter->setSizes(Settings::get()->ServersSplitterSizes);

	connect(splitter, SIGNAL(splitterMoved(int,int)), this, SLOT(splitterMoved(int,int)));
	connect(dirWidget, SIGNAL(changeSettings()), this, SLOT(settingsChanged()));
}

void DataSourceWidget::splitterMoved(int, int)
{
	Settings::get()->ServersSplitterSizes = splitter->sizes();
}

void DataSourceWidget::handleOpenPartDirectory(const QFileInfo &fi)
{
	setDirectory(fi.absoluteFilePath());
}

void DataSourceWidget::settingsChanged()
{
	if (Settings::get()->DataSourcesNeedsUpdate)
	{
		Settings::get()->DataSourcesNeedsUpdate = false;

		// Datasources
		// firstly delete all stuff used. Remember "the reset"
		while (dsList->count())
		{
			dsList->widget(0)->deleteLater();
			dsList->removePage(0);
		}

		qApp->processEvents();

		// now setup all item==group again
		foreach(DataSource *ds, Settings::get()->DataSources)
		{
			DataSourceView *view = new DataSourceView(ds->rootPath, this);

			connect(view, SIGNAL(showSettings(SettingsDialog::Section)),
			        this, SIGNAL(showSettings(SettingsDialog::Section)));
			connect(view, SIGNAL(workingDirChanged()),
			        this, SIGNAL(workingDirChanged()));
			connect(view, SIGNAL(directorySelected(QString)),
					dirWidget, SLOT(setDirectory(QString)));
			connect(dirWidget, SIGNAL(openPartDirectory(QFileInfo)),
					this, SLOT(handleOpenPartDirectory(QFileInfo)));
			connect(view, SIGNAL(directorySelected(QString)),
			        this, SIGNAL(directorySelected(QString)));
			connect(view, SIGNAL(directoryChanged(QString)),
					dirWidget, SLOT(updateDirectory(QString)));

			dsList->addPage(view, ds->name, ds->icon);
		}

		dsList->setVisibleRows(dsList->count());

		goToWorkingDirectory();

	} // Settings::get()->DataSourcesNeedsUpdate

	dirWidget->settingsChanged();
}

void DataSourceWidget::expand(const QModelIndex & index)
{
	qobject_cast<QTreeView*>(dsList->currentWidget())->expand(index);
}

QModelIndex DataSourceWidget::currentIndex()
{
	return qobject_cast<QTreeView*>(dsList->currentWidget())->currentIndex();
}

void DataSourceWidget::setDirectory(const QString &path)
{
	for (int i = 0; i < dsList->count(); ++i)
	{
		DataSourceView *w = qobject_cast<DataSourceView*>(dsList->widget(i));
		if (w->navigateToDirectory(path))
		{
			dsList->setCurrentIndex(i);
			dirWidget->setDirectory(path);
		}
	}
}

void DataSourceWidget::goToWorkingDirectory()
{
    setDirectory(Settings::get()->getWorkingDir());
}
