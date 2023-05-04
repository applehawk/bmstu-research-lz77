#include "archproj.h"

#include "filelistwidget.h"
#include "extensionmng.h"

#include "compressdlg.h"
#include "decompressdlg.h"

#include "paf.h"

#include <QtGui>
#include <QActionGroup>
#include <QTextCodec>
#include <QDir>

#define RUS(str) codec1251->toUnicode(str)
QTextCodec * codec1251;


DirModel::DirModel(const QStringList &nameFilters, QDir::Filters filters,
									 QDir::SortFlags sort, QObject *parent) : QDirModel(nameFilters, filters, sort, parent)
{}

DirModel::DirModel(QObject *parent) : QDirModel(parent) {}
QVariant DirModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (orientation == Qt::Horizontal) {
		if (role != Qt::DisplayRole)
			return QVariant();
		switch (section) {
				case 0: return tr("Имя");
				case 1: return tr("Размер");
				case 2: return
#ifdef Q_OS_MAC
									tr("Тип", "Match OS X Finder");
#else
									tr("Тип", "All other platforms");
#endif
				case 3: return tr("Дата изменения");
				default: return QVariant();
		}
	}
	return QAbstractItemModel::headerData(section, orientation, role);
}

//ArchProj Main Window

ArchProj::ArchProj(QWidget *parent, Qt::WFlags flags)
	: QMainWindow(parent, flags), m_listView(NULL), m_treeView(NULL), m_dirModel(NULL)
{
	setWindowTitle("Archive Pro");
	codec1251 = QTextCodec::codecForName("Windows-1251");
	QTextCodec::setCodecForTr(codec1251);
	QTextCodec::setCodecForLocale(codec1251);
	QTextCodec::setCodecForCStrings(codec1251);

	m_archManager = new ExtensionMng();
	m_archManager->load_extensions_dir("\\exts");
  //Добавляем поддержку формата ProtoArchiveFormat 1.0
  m_archManager->add_extension(new PAF_ArchFormat());
}

ArchProj::~ArchProj() 
{
	m_archManager->unload_extensions();
	delete m_archManager;
} 

void ArchProj::showAboutDlg()
{
	QMessageBox::about(this, "О программе", tr(
		"<center><p>Программа для просмотра и создания архивов формата <b>PA(ProtoArchive)</b><br>"
		"с помощью собственной библиотеки сжатия данных <b><i>LZXLib</b></i></center></p><br>"
		"Назначение: Основное назначение программы ProtoArchive<br>"
		"заключается в оценке эффективности алгоритмов сжатия и<br>"
		"распаковки файлов различных форматов (.exe, .txt, .res…).<br>"
		"Программа может применяться как для исследования алгоритмов<br>"
		"сжатия, так и обычными пользователями для подготовки данных<br>"
		"к долговременному хранению, передачи их по сети Интернет."
		"<p><i>Разработчик: Василенко В.Ю.<br>"
		"Студент <b>МГТУ им. Н.Э.Баумана</b><br>"
		"Факультета \"Информатика и управление\" (ИУ)<br>"
		"Кафедры \"Вычислительные машины и сети\" (ИУ6)<br>"
		"Группы ИУ6-32</i></p>"));
}

void ArchProj::setupMenu()
{
	//Menu File
	QMenu* mnuFile = menuBar()->addMenu(tr("Файл"));
  //QAction* actOpen = mnuFile->addAction(tr("Открыть..."));
	//connect(actOpen, SIGNAL(triggered()), this, SLOT(OpenFileDlg()));
	mnuFile->addAction(m_exitAct);

	QMenu* mnuView = menuBar()->addMenu(tr("Вид"));
	mnuView->addActions(m_viewActions->actions());
  mnuView->addSeparator();
  //mnuView->addAction ( .. Формат вывода списка файлов для сжатия ... )

	QMenu* mnuTools = menuBar()->addMenu(tr("Инструменты"));
	mnuTools->addAction(m_addArchAct);
	mnuTools->addAction(m_compressAct);
	mnuTools->addAction(m_decompressAct);
	mnuTools->addAction(m_infoAct);
	mnuTools->addSeparator();
	mnuTools->addAction(m_plugsAct);

	QMenu* mnuLangs = mnuTools->addMenu(tr("Язык"));
  //mnuLangs->addActions(m_langsActions->actions());

	QMenu* mnuWindow = menuBar()->addMenu(tr("Окно"));
	mnuWindow->addAction(tr("Подготовка архива"));
	mnuWindow->addAction(tr("Файловая система"));
	//Menu Help
	QMenu* mnuHelp = menuBar()->addMenu("Помощь");
	mnuHelp->addAction(m_aboutAct);
	//popupFileList.About...
}

void ArchProj::createToolBars()
{
	m_fileToolBar = addToolBar(tr("Инструменты"));
	m_fileToolBar->setIconSize(QSize(64,64));
	m_fileToolBar->addAction(m_addArchAct);
	m_fileToolBar->addAction(m_compressAct);
	m_fileToolBar->addAction(m_decompressAct);
	m_fileToolBar->addAction(m_infoAct);
	m_fileToolBar->addAction(m_aboutAct);
	m_fileToolBar->addAction(m_exitAct);
}

//Двойной клик по элементу дерева инициирует добавление его в список упавковки
void ArchProj::doubleTreeClicked(const QModelIndex& index)
{
	//Не рабочий код
	/*if(!m_dirModel->isDir(index))
	{
		archList->addFileToList( fsModel->filePath(index) );
	}*/
}

void ArchProj::setupUI()
{
	createActions();
	setupMenu();
	createToolBars();

	changeNoViewer(true);

	ArchFiles_Dock = new QDockWidget(this);
	ArchFiles_Dock->setWindowTitle(tr("Файлы для сжатия"));
	ArchFiles_Dock->setAllowedAreas(Qt::AllDockWidgetAreas);

	LookFiles_Dock = new QDockWidget(this);
	LookFiles_Dock->setWindowTitle(tr("Просмотр"));
	LookFiles_Dock->setAllowedAreas(Qt::AllDockWidgetAreas);
	LookFiles_Dock->setVisible(false);

	addDockWidget(Qt::RightDockWidgetArea, ArchFiles_Dock);
	addDockWidget(Qt::LeftDockWidgetArea, LookFiles_Dock);
	
	m_archList = new FileListWidget(this);
	m_archList->clear();
	m_archList->setViewMode(QListWidget::IconMode);
	//m_archList->setDragDropOverwriteMode(true);
	
	ArchFiles_Dock->setWidget(m_archList);
	
	QStatusBar* statusBar = new QStatusBar(this);
	statusBar->showMessage(tr("Готов"));
	setStatusBar(statusBar);
}

void ArchProj::showCompressDlg(bool bChecked)
{
	if(m_archList->count() == 0)
	{
		QMessageBox::warning(this, tr("Предупреждение: Диалог Сжатия"),
			tr("Ваш список файлов для сжатия пуст!"));
	} else 
	{
		CompressDlg* compressDlg = new CompressDlg(this, m_archList, m_archManager);
		compressDlg->setupUI();
		compressDlg->setModal(true);
		compressDlg->show();
	}
}

void ArchProj::showDecompressDlg(bool bChecked)
{
	//Создаем диалог распаковки
	DecompressDlg* decompressDlg = new DecompressDlg(this, m_archManager);
	decompressDlg->setupUI();
	decompressDlg->setModal(true);
	decompressDlg->show();
}

void ArchProj::showInfoDlg(bool bChecked)
{
	QMessageBox::information(this, "Информация об архиве", "Информационный диалог");
}

void ArchProj::showPluginsDlg(bool bChecked)
{
	QMessageBox::information(this, "Plugins", "Plugin dialog");
}

void ArchProj::keyPressEvent( QKeyEvent * event )
{
	if(event->key() == Qt::Key_Delete && m_archList->hasFocus())
	{
		QList<QListWidgetItem*> selItems = m_archList->selectedItems();
		if(!selItems.isEmpty()) {
			m_archList->removeItemWidget(selItems.last());
			m_archList->setItemHidden(selItems.last(),true);
			delete selItems.last();
		}
	}
}

void ArchProj::changeNoViewer(bool bChecked)
{
	if(!bChecked)
		return;
	if(m_treeView) {
		m_treeView->setVisible(false);
		delete m_treeView;
		m_treeView = NULL;
	}
	if(m_listView) {
		m_listView->setVisible(false);
		delete m_listView;
		m_listView = NULL;
	}
	//Для экономии памяти удаляем модель файловой системы
	if(m_dirModel) {
		delete m_dirModel;
		m_dirModel = NULL;
	}
	setCentralWidget(NULL);
}

void ArchProj::changeListViewer(bool bChecked)
{
	if(!bChecked)
		return;
	setCentralWidget(NULL);
	if(m_treeView) {
		m_treeView->setVisible(false);
		delete m_treeView;
		m_treeView = NULL;
	}
	if(!m_dirModel)
		m_dirModel = new DirModel();

	m_listView = new QListView(this);
	m_listView->setDragDropMode(QAbstractItemView::InternalMove);
	m_listView->setModel(m_dirModel);
	m_listView->setResizeMode(QListView::Adjust);
	m_listView->setLayoutMode(QListView::Batched);
	m_listView->setMovement(QListView::Snap);
	m_listView->setWrapping(false);
	m_listView->setBatchSize(100);
	m_listView->setRootIndex( m_dirModel->index( QDir::currentPath() ));
	setCentralWidget(m_listView);
}
void ArchProj::changeTreeViewer(bool bChecked)
{
	if(!bChecked)
		return;
	setCentralWidget(NULL);
	if(m_listView) {
		m_listView->setVisible(false);
		delete m_listView;
		m_listView = NULL;
	}
	if(!m_dirModel)
		m_dirModel = new DirModel();

	m_treeView = new QTreeView(this);
	m_treeView->setModel(m_dirModel);

	m_treeView->setAnimated(false);// Повисает на анимации и сортировке
	//m_treeView->setSortingEnabled(true); 
	//m_treeView->setWindowTitle(tr("Файловая система"));
	m_treeView->setDragDropMode(QAbstractItemView::InternalMove);
	//m_treeView->show();
	connect(m_treeView, SIGNAL(doubleClicked(const QModelIndex&)), 
	this, SLOT(doubleTreeClicked(const QModelIndex&)));
	setCentralWidget(m_treeView);
}

void ArchProj::createActions()
{
	//Подготовка группы действий для переключения вида
	m_viewActions = new QActionGroup(this);
	QAction* treeViewAct = m_viewActions->addAction(tr("Древовидный"));
	QAction* listViewAct = m_viewActions->addAction(tr("Список"));
	QAction* noViewAct = m_viewActions->addAction(tr("Без просмотра"));
	listViewAct->setCheckable(true);
	treeViewAct->setCheckable(true);
	noViewAct->setCheckable(true);
	connect(listViewAct, SIGNAL(toggled(bool)), this, SLOT( changeListViewer(bool) ));
	connect(treeViewAct, SIGNAL(toggled(bool)), this, SLOT( changeTreeViewer(bool) ));
	connect(noViewAct, SIGNAL(toggled(bool)), this, SLOT( changeNoViewer(bool) ));

	//Устанавливаем по умолчанию
	noViewAct->setChecked(true);

	//Подготовка группы действий для переключения языков
  /* Поддержка языков!
  m_langsActions = new QActionGroup(this);
	QStringList qmFiles = m_chooser.findQmFiles("langs");

	QStringList::iterator iter,end = qmFiles.end();
	for(iter = qmFiles.begin(); iter != end; ++iter)
	{
		QString language = m_chooser.languageName(*iter);
		QAction* actLang = m_langsActions->addAction(language);
  }*/

	//Действия главной формы
	m_addArchAct = new QAction( tr("Добавить..."), this);
	m_addArchAct->setShortcut( QKeySequence::fromString("A") );
	m_addArchAct->setStatusTip( tr("Добавить файл в архив") );


	m_compressAct = new QAction( tr("Сжатие..."), this);
	m_compressAct->setShortcut( QKeySequence::fromString("C") );
	m_compressAct->setStatusTip( tr("Показать диалог сжатия") );
	connect(m_compressAct, SIGNAL(triggered(bool)), this, SLOT(showCompressDlg(bool)));

	m_decompressAct = new QAction( tr("Извлечение...") , this);
	m_decompressAct->setShortcut( QKeySequence::fromString("D") );
	m_decompressAct->setStatusTip( tr("Покзать диалог извлечения") );
	connect(m_decompressAct, SIGNAL(triggered(bool)), this, SLOT(showDecompressDlg(bool)));

	m_plugsAct = new QAction( tr("Модули") ,this);
	m_plugsAct->setShortcut( QKeySequence::fromString("Ctrl+P") );
	m_plugsAct->setStatusTip( tr("Показать диалог для управления модулями") );
	connect(m_plugsAct, SIGNAL(triggered(bool)), this, SLOT(showPluginsDlg(bool)));

	m_infoAct = new QAction( tr("Информация"), this);
	m_infoAct->setShortcut( QKeySequence::fromString("Ctrl+I") );
	m_infoAct->setStatusTip( tr("Показать информацию о сжатом файле") );
	connect(m_infoAct, SIGNAL(triggered(bool)), this, SLOT(showInfoDlg(bool)));

	m_aboutAct = new QAction( tr("Программа"), this);
	m_aboutAct->setShortcut( QKeySequence::fromString("Ctrl+A") );
	m_aboutAct->setStatusTip( tr("О программе и ее разработчиках") );
	connect(m_aboutAct, SIGNAL(triggered()), this, SLOT(showAboutDlg()));
	
	m_exitAct = new QAction( tr("Выход"), this);
	m_exitAct->setShortcut( QKeySequence::fromString("Ctrl+C") );
	m_exitAct->setStatusTip( tr("Выход из приложения...") );
	connect(m_exitAct, SIGNAL(triggered()), this, SLOT(close()));
}
