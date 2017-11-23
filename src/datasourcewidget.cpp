#include "datasourcewidget.h"
#include "datasourceview.h"
#include "settings.h"
#include "datasourcehistory.h"

#include <QtDebug>


DataSourceWidget::DataSourceWidget(const QString &dir, QWidget *parent)
	: QWidget(parent)
{
	setupUi(this);

	dsList->setStyleSheet(NavBar::loadStyle(":/styles/office2003gray.css"));

	splitter->setSizes(Settings::get()->ServersSplitterSizes);

	connect(splitter, SIGNAL(splitterMoved(int,int)), this, SLOT(splitterMoved(int,int)));
	connect(dirWidget, SIGNAL(changeSettings()), this, SLOT(settingsChanged()));

	m_history = new DataSourceHistory(this);

	connect(m_history, SIGNAL(openDirectory(QString)),
			this, SLOT(setDirectory(QString)));

	setupDataSources(dir);
}

void DataSourceWidget::splitterMoved(int, int)
{
	Settings::get()->ServersSplitterSizes = splitter->sizes();
}

void DataSourceWidget::handleOpenPartDirectory(const QFileInfo &fi)
{
	setDirectory(fi.absoluteFilePath());
}

void DataSourceWidget::announceDirectoryChange(const QString &dir)
{
	m_currentDir = dir;
	emit directoryChanged(this, dir);
}

void DataSourceWidget::settingsChanged()
{
	// Data sources
	if (Settings::get()->DataSourcesNeedsUpdate)
	{
		// firstly delete all stuff used. Remember "the reset"
		while (dsList->count())
		{
			dsList->widget(0)->deleteLater();
			dsList->removePage(0);
		}

		qApp->processEvents();

		// now setup all item==group again
		setupDataSources(m_currentDir);
	}

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
			m_currentDir = path;
		}
	}
}

DataSourceHistory *DataSourceWidget::history()
{
	return m_history;
}

QString DataSourceWidget::currentDir() const
{
	return m_currentDir;
}

void DataSourceWidget::goToWorkingDirectory()
{
	QString wdir = Settings::get()->getWorkingDir();
	setDirectory(wdir);
	m_history->track(wdir);
}

void DataSourceWidget::setupDataSources(const QString &dir)
{
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
				m_history, SLOT(track(QString)));
		connect(view, SIGNAL(directorySelected(QString)),
				this, SLOT(announceDirectoryChange(QString)));
		connect(view, SIGNAL(directoryChanged(QString)),
				dirWidget, SLOT(updateDirectory(QString)));
		connect(view, SIGNAL(openInANewTabRequested(QString)),
				this, SIGNAL(openInANewTabRequested(QString)));

		dsList->addPage(view, ds->name, ds->icon);
	}

	dsList->setVisibleRows(dsList->count());

	setDirectory(dir);
	m_history->track(dir);
}
