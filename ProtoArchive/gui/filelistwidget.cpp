#include "filelistwidget.h"
#include <QtGui>

FileListWidget::FileListWidget(QWidget* parent) : QListWidget(parent) 
{
  m_iconProvider = new QFileIconProvider();
	setAcceptDrops(true);
}
FileListWidget::~FileListWidget()
{
  delete m_iconProvider;
}

void FileListWidget::addFileToList(const QString& file)
{
	QFileInfo infFile(file);
  //Не добавляем диски(корневые директории), директории и не существующие файлы
  if( infFile.isRoot() || !infFile.exists() || infFile.isDir())
		return;
	if( !findItems(file,Qt::MatchFixedString).isEmpty() ) {
		QMessageBox::warning(NULL, tr("Внимание!"), tr("Данный файл уже добавлен в список"));
		return;
	}
  QListWidgetItem* fileItem = new QListWidgetItem(
      m_iconProvider->icon(infFile),
      infFile.fileName() );

	fileItem->setData(Qt::UserRole, infFile.absoluteFilePath());
  addItem( fileItem );
}

void FileListWidget::setupUI()
{
	connect(this, SIGNAL(itemPressed(QListWidgetItem*)), this, SLOT(itemDeletePressed(QListWidgetItem*)));
}

void FileListWidget::itemDeletePress ( QListWidgetItem * item )
{
	removeItemWidget(item);
}

void FileListWidget::filesStringList(QStringList* filesList)
{
	if(!filesList)
		return;
	for(int i = 0; i<count(); ++i)
	{
		QListWidgetItem* fileItem = item(i);
		QVariant filePathVar = fileItem->data(Qt::UserRole);
		(*filesList) << filePathVar.toString();
	}
}

void FileListWidget::dragEnterEvent(QDragEnterEvent* event)
{
	event->acceptProposedAction();
}

void FileListWidget::dragMoveEvent(QDragMoveEvent *event)
{
	event->acceptProposedAction();
}

void FileListWidget::dragLeaveEvent(QDragLeaveEvent* event)
{
	event->accept();
}

void FileListWidget::dropEvent(QDropEvent* event)
{
	const QList<QUrl>& urls = event->mimeData()->urls();
	if(urls.isEmpty())
		return;
	QList<QUrl>::const_iterator iter, end = urls.end();
	for(iter = urls.begin(); iter != end; ++iter)
	{
		QString locFile = (*iter).toLocalFile();
		if(locFile.isEmpty())
			return;
		addFileToList( locFile );
	}
}

qint64 FileListWidget::calcSizeOfFiles()
{
	qint64 TotalSize=0;
	for(int i = 0; i<count(); ++i)
	{
		QListWidgetItem* fileItem = item(i);
		QVariant filePathVar = fileItem->data(Qt::UserRole);
		TotalSize += QFileInfo(filePathVar.toString()).size();
	}
	return TotalSize;
}
