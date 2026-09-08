#ifndef APPLICATIONLANGUAGE_H
#define APPLICATIONLANGUAGE_H
#include <QApplication>
#include <QLibraryInfo>
#include <QLocale>
#include <QTranslator>

// Both translators live as long as the application. English removes the previous
// catalog too, so switching back never leaves texts in the previous language.
inline void applyApplicationLanguage(const QString &language)
{
    static QTranslator *app = new QTranslator(qApp);
    static QTranslator *qt = new QTranslator(qApp);
    qApp->removeTranslator(app);
    qApp->removeTranslator(qt);
    QLocale::setDefault(QLocale(language));
    const QString base = QCoreApplication::applicationDirPath();
    const QString qtName = "qtbase_" + language.left(2);
    if (qt->load("qt_" + language.left(2), base + "/translations") ||
        qt->load(qtName, base + "/translations") ||
        qt->load(qtName, QLibraryInfo::path(QLibraryInfo::TranslationsPath)))
        qApp->installTranslator(qt);
    const QString name = "zima-cad-parts_" + language;
    if (app->load(":/i18n/" + name) || app->load(name, base) ||
        app->load(name, base + "/locale"))
        qApp->installTranslator(app);
}
#endif
