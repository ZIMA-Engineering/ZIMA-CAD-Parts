#include "serverswidget.h"
#include "serversview.h"
#include "settings.h"

#include <QtDebug>


ServersWidget::ServersWidget(QWidget *parent)
	: QWidget(parent)
{
	setupUi(this);

	serversToolBox->setStyleSheet(NavBar::loadStyle(":/styles/office2003gray.css"));

	splitter->setSizes(Settings::get()->ServersSplitterSizes);

    connect(splitter, SIGNAL(splitterMoved(int,int)), this, SLOT(splitterMoved(int,int)));
    connect(partsWidget, SIGNAL(changeSettings()), this, SLOT(settingsChanged()));
}

void ServersWidget::splitterMoved(int, int)
{
	Settings::get()->ServersSplitterSizes = splitter->sizes();
}

void ServersWidget::settingsChanged()
{
	if (Settings::get()->DataSourcesNeedsUpdate)
	{
		Settings::get()->DataSourcesNeedsUpdate = false;

		// Datasources
		// firstly delete all stuff used. Remember "the reset"
        while (serversToolBox->count())
        {
            serversToolBox->widget(0)->deleteLater();
            serversToolBox->removePage(0);
        }

		qApp->processEvents();

		// now setup all item==group again
        foreach(DataSource *ds, Settings::get()->DataSources)
		{
            ServersView *view = new ServersView(ds->rootPath, this);

            connect(view, SIGNAL(showSettings(SettingsDialog::Section)),
                    this, SIGNAL(showSettings(SettingsDialog::Section)));
            connect(view, SIGNAL(workingDirChanged()),
                    this, SIGNAL(workingDirChanged()));
            connect(view, SIGNAL(directorySelected(QString)),
                    partsWidget, SLOT(setDirectory(QString)));
            connect(view, SIGNAL(directorySelected(QString)),
                    this, SIGNAL(directorySelected(QString)));

            serversToolBox->addPage(view, ds->name, ds->icon);
		}

        serversToolBox->setVisibleRows(serversToolBox->count());

        goToWorkingDirectory();

	} // Settings::get()->DataSourcesNeedsUpdate

    partsWidget->settingsChanged();
}

void ServersWidget::expand(const QModelIndex & index)
{
	qobject_cast<QTreeView*>(serversToolBox->currentWidget())->expand(index);
}

QModelIndex ServersWidget::currentIndex()
{
	return qobject_cast<QTreeView*>(serversToolBox->currentWidget())->currentIndex();
}

void ServersWidget::setDirectory(const QString &path)
{
    for (int i = 0; i < serversToolBox->count(); ++i)
    {
        ServersView *w = qobject_cast<ServersView*>(serversToolBox->widget(i));
        if (w->navigateToDirectory(path))
        {
            serversToolBox->setCurrentIndex(i);
            partsWidget->setDirectory(path);
        }
    }
}

void ServersWidget::goToWorkingDirectory()
{
    setDirectory(Settings::get()->WorkingDir);
}
