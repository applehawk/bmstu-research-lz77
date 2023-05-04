#include "langchooser.h"
#include <QDir>
#include <QLocale>
#include <QCoreApplication>

LangChooser::LangChooser()
{}

bool LangChooser::setLocaleLanguage(QCoreApplication* app)
{
	bool bLoad = translator.load("archproj_"+QLocale::system().name()+".qm", "langs");
	if(!bLoad)
		return false;
	if(app)
		app->installTranslator(&translator);

	return true;
}

QStringList LangChooser::findQmFiles(const QString& dirStr)
{
	QDir dir(dirStr);
	QStringList fileNames = dir.entryList(QStringList("*.qm"), QDir::Files,
		QDir::Name);
	QMutableStringListIterator i(fileNames);
	while (i.hasNext()) {
		i.next();
		i.setValue(dir.filePath(i.value()));
	}
	return fileNames;
}

QString LangChooser::languageName(const QString& qmFile)
{
	QTranslator translator;
	translator.load(qmFile);

	return translator.translate("LangChooser", "English");

}