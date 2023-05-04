#ifndef __ARCHPROJ_LANGCHOOSER_H__
#define __ARCHPROJ_LANGCHOOSER_H__
#include <QString>
#include <QTranslator>
#include <QDialog>

class QCoreApplication;

class LangChooser //: public QDialog
{
private:
	QTranslator translator;
public:
	LangChooser();

	QStringList findQmFiles(const QString& dirStr);
	QString languageName(const QString& qmFile);

	bool setLocaleLanguage(QCoreApplication* app);
	void setLanguageFromName(const QString& langName);
};

#endif