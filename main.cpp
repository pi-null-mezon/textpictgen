#include <QApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>

#include <QDir>
#include <QFile>
#include <QImage>
#include <QPainter>
#include <QFileInfo>
#include <QPainterPath>
#include <QFontMetrics>
#include <QRandomGenerator>
#include <QLinearGradient>
#include <QRadialGradient>
#include <QDateTime>
#include <QUuid>

#include <QGraphicsBlurEffect>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>

QStringList fonts_names_list {"Times New Roman", "Arial"};

QImage blur(const QImage &src, const qreal radius=2.0f, int extent=0);

void makeColors(QRandomGenerator &rndgen, QColor &foreground, QColor &background, QColor &border, const int maxdev, bool randomswap=true)
{
    int base = 5 + rndgen.generateDouble() * 50;
    foreground = QColor(base + maxdev*rndgen.generateDouble(),
                        base + maxdev*rndgen.generateDouble(),
                        base + maxdev*rndgen.generateDouble());

    base = 150 + rndgen.generateDouble() * 50;
    background = QColor(base + maxdev*rndgen.generateDouble(),
                        base + maxdev*rndgen.generateDouble(),
                        base + maxdev*rndgen.generateDouble());

    if(randomswap && rndgen.generateDouble() > 0.5)
        std::swap(foreground,background);

    border = QColor(255*rndgen.generateDouble(),
                    255*rndgen.generateDouble(),
                    255*rndgen.generateDouble());

}

void makeBWColors(QRandomGenerator &rndgen, QColor &foreground, QColor &background, QColor &border, bool randomswap=true)
{
    int base = 5 + rndgen.generateDouble() * 50;
    foreground = QColor(base + 15*rndgen.generateDouble(),
                        base + 15*rndgen.generateDouble(),
                        base + 15*rndgen.generateDouble());

    base = 190 + rndgen.generateDouble() * 50;
    background = QColor(base + 15*rndgen.generateDouble(),
                        base + 15*rndgen.generateDouble(),
                        base + 15*rndgen.generateDouble());

    if(randomswap && rndgen.generateDouble() > 0.5)
        std::swap(foreground,background);

    border = foreground;
}


uchar clip2Uchar(float value)
{
    if(value > 254.0f)
        return 255;
    else if(value < 1.0f)
        return 0;
    return static_cast<uint>(value);
}

void disturbColors(QImage &img, QRandomGenerator &rndgen, int maxdev=11)
{
    const float r = 2*maxdev*(0.5-rndgen.generateDouble());
    const float g = 2*maxdev*(0.5-rndgen.generateDouble());
    const float b = 2*maxdev*(0.5-rndgen.generateDouble());
    int tmp;
    for(int y = 0; y < img.height(); ++y) {
        uchar *ptr = img.scanLine(y);
        for(int x = 0; x < img.width(); ++x) {
            tmp = 3*x;
            ptr[tmp] = clip2Uchar(ptr[tmp] + r);
            ptr[tmp+1] = clip2Uchar(ptr[tmp+1] + g);
            ptr[tmp+2] = clip2Uchar(ptr[tmp+2] + b);
        }
    }
}

void linearScale(QImage &img, const float alpha, const float betha=0)
{
    int tmp;
    for(int y = 0; y < img.height(); ++y) {
        uchar *ptr = img.scanLine(y);
        for(int x = 0; x < img.width(); ++x) {
            tmp = 3*x;
            ptr[tmp] = clip2Uchar(ptr[tmp]*alpha + betha);
            ptr[tmp+1] = clip2Uchar(ptr[tmp+1]*alpha + betha);
            ptr[tmp+2] = clip2Uchar(ptr[tmp+2]*alpha + betha);
        }
    }
}

void addRandomNoise(QImage &img, QRandomGenerator &rndgen, const int maxdev)
{
    int tmp;
    for(int y = 0; y < img.height(); ++y) {
        uchar *ptr = img.scanLine(y);
        for(int x = 0; x < img.width(); ++x) {
            tmp = 3*x;
            ptr[tmp] = clip2Uchar(ptr[tmp] + 2*(0.5 - rndgen.generateDouble())*maxdev);
            ptr[tmp+1] = clip2Uchar(ptr[tmp+1] + 2*(0.5 - rndgen.generateDouble())*maxdev);
            ptr[tmp+2] = clip2Uchar(ptr[tmp+2] + 2*(0.5 - rndgen.generateDouble())*maxdev);
        }
    }
}

int main(int argc, char *argv[])
{
#ifdef Q_OS_WIN
    setlocale(LC_CTYPE,"rus")
#endif

    QCommandLineParser cmdparser;
    cmdparser.setApplicationDescription("This application was designed to generate pictures with text phrases. Output could be used for CTC learning.");
    cmdparser.addHelpOption();
    cmdparser.addVersionOption();

    QCommandLineOption phrase_opt(QStringList()     << "text" << "t", "Input text phrase", "some text");
    cmdparser.addOption(phrase_opt);
    QCommandLineOption outdir_opt(QStringList()     << "outdir" << "o", "Output directory, where subdirectory with samples and markup file will be stored", "./", "./");
    cmdparser.addOption(outdir_opt);
    QCommandLineOption markupfilename_opt(QStringList() << "markupfilename", "Markup filename", "gt.txt", "gt.txt");
    cmdparser.addOption(markupfilename_opt);
    QCommandLineOption samples_opt(QStringList()    << "samples" << "s", "Samples to generate", "samples", "2");
    cmdparser.addOption(samples_opt);
    QCommandLineOption seed_opt(QStringList()       << "seed", "Seed value for random generator, current time if value == 0 ", "0", "0");
    cmdparser.addOption(seed_opt);
    QCommandLineOption extension_opt(QStringList() << "extension", "Picture codec", "jpg,png", "jpg");
    cmdparser.addOption(extension_opt);
    QCommandLineOption maxangle_opt(QStringList() << "angledev", "Max rotation angle in degrees", "0..90", "3");
    cmdparser.addOption(maxangle_opt);
    QCommandLineOption maxcolordev_opt(QStringList() << "colordev", "Max color deviation", "0..55", "55");
    cmdparser.addOption(maxcolordev_opt);
    QCommandLineOption noisedev_opt(QStringList() << "noisedev", "Max noise devialtion", "0..55", "13");
    cmdparser.addOption(noisedev_opt);

    QApplication app(argc, argv);
    cmdparser.process(app);

    if(!cmdparser.isSet(phrase_opt)) {
        qInfo("You have not specify any phrase, so, there is nothing to generate! Abort...");
        return 1;
    }

    QDir odir(cmdparser.value(outdir_opt));
    if(!odir.exists())
        odir.mkpath(odir.absolutePath());
    QDir pictsubdir(odir.absolutePath().append("/%1").arg(APP_NAME));
    if(!pictsubdir.exists())
        pictsubdir.mkpath(pictsubdir.absolutePath());

    QFileInfo markupfileinfo(odir.absolutePath().append("/%1").arg(cmdparser.value(markupfilename_opt)));
    QFile markupfile(markupfileinfo.absoluteFilePath());
    if(markupfileinfo.exists()) {
        qInfo("Markup file has been found in output diectory >> new lines will be appended to existed ones");
        if(!markupfile.open(QIODevice::WriteOnly | QIODevice::Append)) {
            qInfo("Can not open file '%s'! Abort...", markupfile.fileName().toUtf8().constData());
            return 2;
        }
    } else {
        qInfo("Markup file will be created in output diectory");
        if(!markupfile.open(QIODevice::WriteOnly)) {
            qInfo("Can not open file '%s'! Abort...", markupfile.fileName().toUtf8().constData());
            return 3;
        }
    }

    quint32 seedval = cmdparser.value(seed_opt).toUInt();
    if(seedval == 0)
        seedval = static_cast<quint32>(QDateTime::currentMSecsSinceEpoch());
    qInfo("Seed value: %u", seedval);
    QRandomGenerator rndgen(seedval);


    QString phrase = cmdparser.value(phrase_opt);
    qInfo("'%s' phrase under processing", phrase.toUtf8().constData());

    QColor fcolor, bcolor, border_color;
    for(uint i = 0; i < cmdparser.value(samples_opt).toUInt(); ++i) {
        const QString uuid = QUuid::createUuid().toString();

        const int border = 1 + (rndgen.generate() % 7);

        QFont font(fonts_names_list.at(rndgen.generate() % fonts_names_list.size()), 8 + 16*rndgen.generateDouble());

        QFontMetrics fm(font);
        const int width = qRound(fm.horizontalAdvance(cmdparser.value(phrase_opt))*1.05) + 2*border;
        const int height = qRound(fm.height() / 1.7f) + 2*border;

        QImage qimg(width, height, QImage::Format_RGB888);

        QPainter painter(&qimg);
        painter.setRenderHints(QPainter::Antialiasing);

        if(rndgen.generateDouble() > 0.33) {
            makeBWColors(rndgen,fcolor,bcolor,border_color,false);
            painter.fillRect(QRect(0,0,width,height),bcolor);
        } else {
            makeColors(rndgen,fcolor,bcolor,border_color,cmdparser.value(maxcolordev_opt).toInt(),false);
            painter.fillRect(QRect(0,0,width,height),bcolor);
            switch (rndgen.generate() % 6) {
                case 0: {
                    QLinearGradient grad(QPointF(rndgen.generate() % width, -width), QPointF(rndgen.generate() % width, height+width));
                    if(rndgen.generateDouble() > 0.5) {
                        grad.setColorAt(0,fcolor);
                        grad.setColorAt(1,bcolor);
                    } else {
                        grad.setColorAt(0,bcolor);
                        grad.setColorAt(1,fcolor);
                    }
                    painter.fillRect(QRect(0,0,width,height),grad);
                } break;
                case 1: {
                    QLinearGradient grad(QPointF(-width, rndgen.generate() % height), QPointF(width+width, rndgen.generate() % height));
                    if(rndgen.generateDouble() > 0.5) {
                        grad.setColorAt(0,fcolor);
                        grad.setColorAt(1,bcolor);
                    } else {
                        grad.setColorAt(0,bcolor);
                        grad.setColorAt(1,fcolor);
                    }
                    painter.fillRect(QRect(0,0,width,height),grad);
                } break;
                case 2: {
                    QRadialGradient grad(rndgen.generate() % width, rndgen.generate() % height, rndgen.generate() % 3*width);
                    grad.setColorAt(0,bcolor);
                    grad.setColorAt(1,fcolor);
                    painter.fillRect(QRect(0,0,width,height),grad);
                };
            }
        }

        painter.setBrush(fcolor);
        if(rndgen.generateDouble() > 0.5)
            painter.setPen(QPen(border_color,rndgen.generateDouble()));
        else
            painter.setPen(QPen(fcolor,rndgen.generateDouble()));

        painter.translate(QPointF(width/2.0f,height/2.0f));
        painter.rotate((2*rndgen.generateDouble() - 1) * cmdparser.value(maxangle_opt).toDouble());
        painter.translate(QPointF(-width/2.0f,-height/2.0f));
        painter.translate(QPointF(width*0.05*rndgen.generateDouble(),height*0.1*(0.5 - rndgen.generateDouble())));
        painter.scale(1+0.05*rndgen.generateDouble(),1+0.05*rndgen.generateDouble());

        QPainterPath path;
        path.addText(QPointF(border, height - border),font,phrase);
        painter.drawPath(path);       

        linearScale(qimg,0.5 + 1.0*rndgen.generateDouble());

        disturbColors(qimg,rndgen);

        addRandomNoise(qimg,rndgen,rndgen.generateDouble()*cmdparser.value(noisedev_opt).toInt());

        const QString filename = QString("%1.%2").arg(uuid,cmdparser.value(extension_opt));

        if(font.pointSizeF() > 11 && rndgen.generateDouble() > 0.4) {
            QImage _tmpimg = blur(qimg, (1.0 + 2.0*rndgen.generateDouble()));
            _tmpimg.save(QString("%1/%2").arg(pictsubdir.absolutePath(),filename));
        } else
            qimg.save(QString("%1/%2").arg(pictsubdir.absolutePath(),filename));

        markupfile.write(QString("%1/%2\t%3\n").arg(APP_NAME,filename,phrase).toUtf8());
    }

    qInfo("Done");
    return 0;
}

// https://stackoverflow.com/questions/14915129/qimage-transform-using-qgraphicsblureffect
QImage blur(const QImage &src, const qreal radius, int extent)
{
    if(src.isNull())
        return QImage();

    QGraphicsScene scene;

    QGraphicsPixmapItem item;

    item.setPixmap(QPixmap::fromImage(src));

    QGraphicsBlurEffect effect;
    effect.setBlurRadius(radius);

    item.setGraphicsEffect(&effect);
    scene.addItem(&item);

    QImage res(src.size()+QSize(extent*2, extent*2), QImage::Format_ARGB32);
    res.fill(Qt::transparent);

    QPainter painter(&res);
    scene.render(&painter, QRectF(), QRectF( -extent, -extent, src.width()+extent*2, src.height()+extent*2 ) );

    return res;
}
