#include "datasourcehistory.h"
#include "settings.h"

DataSourceHistory::DataSourceHistory(QObject *parent)
    : QObject(parent),
      m_currentIndex(-1)
{
    m_canGoBack = checkCanGoBack();
    m_canGoFwd = checkCanGoForward();
}

bool DataSourceHistory::canGoBack() const
{
    return m_canGoBack;
}

void DataSourceHistory::goBack()
{
    emit openDirectory(m_history[--m_currentIndex]);
    update();
}

bool DataSourceHistory::canGoForward() const
{
    return m_canGoFwd;
}

void DataSourceHistory::goForward()
{
    emit openDirectory(m_history[++m_currentIndex]);
    update();
}

void DataSourceHistory::track(const QString &path)
{
    if (canGoForward())
        m_history = m_history.mid(0, m_currentIndex + 1);

    m_history << path;
    m_currentIndex = m_history.count() - 1;
    update();
}

void DataSourceHistory::goTo(const QString &path)
{
    track(path);
    emit openDirectory(path);
    update();
}

void DataSourceHistory::goToWorkingDirectory()
{
    goTo(Settings::get()->getWorkingDir());
}

void DataSourceHistory::clear()
{
    m_currentIndex = -1;
    m_history.clear();
    update();
}

bool DataSourceHistory::checkCanGoBack() const
{
    return m_currentIndex > 0;
}

bool DataSourceHistory::checkCanGoForward() const
{
    return m_currentIndex != -1 && (m_currentIndex+1) < m_history.count();
}

void DataSourceHistory::update()
{
    bool back = checkCanGoBack();
    bool fwd = checkCanGoForward();

    if (back != m_canGoBack)
    {
        m_canGoBack = back;
        emit canGoBackChanged(back);
    }

    if (fwd != m_canGoFwd)
    {
        m_canGoFwd = fwd;
        emit canGoForwardChanged(fwd);
    }
}
