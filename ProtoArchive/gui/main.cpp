#include "archproj.h"
#include <QtGui/QApplication>
#include <QPushButton>
#include <QTranslator>
#include "langchooser.h"

int main(int argc, char *argv[])
{
        QApplication app(argc, argv);

        LangChooser chooser;
        chooser.setLocaleLanguage(&app);

        ArchProj w;

        w.setupUI();
        w.show();
        return app.exec();
}
