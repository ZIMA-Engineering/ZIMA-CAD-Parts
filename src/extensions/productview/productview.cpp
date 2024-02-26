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

#include "productview.h"
#include "settings.h"

#include "proeproductview.h"
#include "dxfproductview.h"
#include "pdfproductview.h"
#include "imageproductview.h"

#include "filemodel.h"


ProductView::ProductView(QWidget *parent) :
    QObject(parent),
    m_current(0)
{
    addProviders<ProEProductView>();
    addProviders<DxfProductView>();
    addProviders<ImageProductView>();
#ifdef HAVE_POPPLER
    addProviders<PDFProductView>();
#endif
    //failbackProvider = new FailbackProductView(this);
    //failbackProvider->hide();
//	failbackProvider = 0;
}

ProductView::~ProductView()
{
    saveSettings();
    providers.clear();
}

void ProductView::saveSettings()
{
    if (m_current)
    {
        Settings::get()->ExtensionsProductViewGeometry = m_current->saveGeometry();
        Settings::get()->ExtensionsProductViewPosition = m_current->pos();
    }
}

void ProductView::hide()
{
    if (!m_current)
        return;

    saveSettings();
    m_current->hide();
}

void ProductView::setFile(FileMetadata* f)
{
    if (!providers.contains(f->type))
    {
        if (m_current)
            m_current->hide();
        m_current = 0;
        return;
    }

    if (m_current && m_current != providers.value(f->type))
    {
        saveSettings();
        m_current->hide();
    }

    m_current = providers.value(f->type);
    m_current->restoreGeometry(Settings::get()->ExtensionsProductViewGeometry);
    QPoint pt = Settings::get()->ExtensionsProductViewPosition;
    if (!pt.isNull())
        m_current->move(pt);

    m_current->setWindowTitle(f->fileInfo.baseName() + " " + m_current->title());
    m_current->handle(f);
    m_current->show();
}
