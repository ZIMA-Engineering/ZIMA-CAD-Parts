#include <QGuiApplication>
#include <QImage>
#include <QPainter>
#include <QSvgRenderer>
#include <QDir>

// Export only from the vector master; do not resample an older application icon.
int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);
    if (argc != 3) return 1;
    QSvgRenderer renderer(QString::fromLocal8Bit(argv[1]));
    if (!renderer.isValid()) return 2;
    QDir output(QString::fromLocal8Bit(argv[2]));
    if (!output.mkpath(".")) return 3;
    for (int size : {16, 22, 24, 32, 48, 64, 128, 256, 512, 1024}) {
        QImage image(size, size, QImage::Format_ARGB32_Premultiplied);
        image.fill(Qt::transparent);
        QPainter painter(&image);
        renderer.render(&painter);
        painter.end();
        if (!image.save(output.filePath(QString::number(size) + ".png"))) return 4;
    }
    return 0;
}
