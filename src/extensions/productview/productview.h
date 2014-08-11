/*
  ZIMA-CAD-Parts
  http://www.zima-construction.cz/software/ZIMA-Parts

  Copyright (C) 2011-2012 Jakub Skokan <aither@havefun.cz>

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef PRODUCTVIEW_H
#define PRODUCTVIEW_H

#include "../../zima-cad-parts.h"

#include <QWidget>
#include "ui_productview.h"
#include "abstractproductview.h"
#include "failbackproductview.h"

class FileMetadata;

namespace Ui {
class ProductView;
}

/**
 * @brief The user interface for Product Views
 * @see AbstractProductView
 *
 * Each product view must register itself in the ProductView constructor
 * to be used.
 */
class ProductView : public QDialog
{
	Q_OBJECT

public:
	explicit ProductView(QWidget *parent = 0);
	~ProductView();

    //! Returns true if the one of registered product views can handle given file type
	bool canHandle(File::FileType t);

public slots:
    /** \brief Set the file which will be displayed
     */
    void setFile(FileMetadata* f);

protected:
	void hideEvent(QHideEvent *e);
	void showEvent(QShowEvent *e);

private:
	Ui::ProductView *ui;
	QHash<File::FileType, AbstractProductView*> providers;
	AbstractProductView *currentProvider;
	FailbackProductView *failbackProvider;

    //! The main registration function
	template <class T> void addProviders()
	{
		T *provider = new T(this);
		provider->hide();
		foreach(File::FileType i, provider->canHandle())
            providers[i] = provider;
	}

	void saveSettings();
};

#endif // PRODUCTVIEW_H
