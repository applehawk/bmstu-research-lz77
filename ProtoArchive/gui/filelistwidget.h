#ifndef __ARCHPROJ_FILELISTWIDGET_H__
#define __ARCHPROJ_FILELISTWIDGET_H__

#include <QListWidget>
#include <QString>

class QFileIconProvider;

class FileListWidget : public QListWidget
{
	Q_OBJECT
private slots:
	void itemDeletePress ( QListWidgetItem * item );
private:
  QFileIconProvider* m_iconProvider;
public:
	FileListWidget(QWidget* parent = 0);
	~FileListWidget();

	void setupUI();

	void addFileToList(const QString& file);
	void dragEnterEvent(QDragEnterEvent* event);
	void dragMoveEvent(QDragMoveEvent* event);
	void dragLeaveEvent(QDragLeaveEvent* event);
	void dropEvent(QDropEvent* event);

	void filesStringList(QStringList* filesList);
	qint64 calcSizeOfFiles();
};

#endif
