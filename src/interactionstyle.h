#pragma once

#include <QAbstractItemView>
#include <QApplication>
#include <QPainter>
#include <QProxyStyle>
#include <QStyleOptionViewItem>

// Keep the platform's style and metrics; only interaction colours are ours.
namespace PartsInteraction
{
inline QColor selection() { return QColor("#00D1FF"); }
inline QColor hover() { return QColor("#4DD811"); }

class Style final : public QProxyStyle
{
public:
    explicit Style(QStyle *platform) : QProxyStyle(platform) {}

    void polish(QWidget *widget) override
    {
        QProxyStyle::polish(widget);
        if (auto *view = qobject_cast<QAbstractItemView *>(widget))
        {
            view->setMouseTracking(true);
            view->viewport()->setMouseTracking(true);
        }
    }

    void drawPrimitive(PrimitiveElement element, const QStyleOption *option,
                       QPainter *painter, const QWidget *widget = nullptr) const override
    {
        if (element == PE_PanelItemViewItem && option->state.testFlag(State_Enabled)
                && option->state.testFlag(State_Selected))
        {
            painter->fillRect(option->rect, option->palette.brush(QPalette::Highlight));
            return;
        }
        QProxyStyle::drawPrimitive(element, option, painter, widget);
    }

    void drawControl(ControlElement element, const QStyleOption *option,
                     QPainter *painter, const QWidget *widget = nullptr) const override
    {
        if (element == CE_ItemViewItem)
        {
            if (const auto *item = qstyleoption_cast<const QStyleOptionViewItem *>(option))
            {
                auto state = *item;
                const bool selected = state.state.testFlag(State_Selected);
                if (state.state.testFlag(State_Enabled)
                        && (selected || state.state.testFlag(State_MouseOver)))
                {
                    state.palette.setColor(QPalette::Highlight, selected ? selection() : hover());
                    state.palette.setColor(QPalette::HighlightedText, Qt::black);
                    state.state |= State_Selected;
                    state.state &= ~State_MouseOver;
                    QProxyStyle::drawControl(element, &state, painter, widget);
                    return;
                }
            }
        }
        QProxyStyle::drawControl(element, option, painter, widget);
    }
};

inline void install(QApplication &application)
{
    const auto font = application.font();
    auto palette = application.palette();
    application.setStyle(new Style(application.style()));
    application.setFont(font);
    for (auto group : {QPalette::Active, QPalette::Inactive})
    {
        palette.setColor(group, QPalette::Highlight, selection());
        palette.setColor(group, QPalette::HighlightedText, Qt::black);
    }
    application.setPalette(palette);
    // Do not style item views, branches, scroll bars or fonts: Linux supplies
    // those through its Qt platform theme. Menus use transient hover colour.
    application.setStyleSheet(QStringLiteral(
        "QPushButton:checked:enabled, QToolButton:checked:enabled {"
        "background-color: #00D1FF; color: black; border: 1px solid #00D1FF; }"
        "QPushButton:hover:enabled, QToolButton:hover:enabled {"
        "background-color: #4DD811; color: black; border: 1px solid #4DD811; }"
        "QMenu::item:selected:enabled, QMenuBar::item:selected:enabled {"
        "background-color: #4DD811; color: black; }"));
}
}
