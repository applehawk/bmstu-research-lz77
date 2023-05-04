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
				case 0: return tr("���");
				case 1: return tr("������");
				case 2: return
#ifdef Q_OS_MAC
									tr("���", "Match OS X Finder");
#else
									tr("���", "All other platforms");
#endif
				case 3: return tr("���� ���������");
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
  //��������� ��������� ������� ProtoArchiveFormat 1.0
  m_archManager->add_extension(new PAF_ArchFormat());
}

ArchProj::~ArchProj() 
{
	m_archManager->unload_extensions();
	delete m_archManager;
} 

void ArchProj::showAboutDlg()
{
	QMessageBox::about(this, "� ���������", tr(
		"<center><p>��������� ��� ��������� � �������� ������� ������� <b>PA(ProtoArchive)</b><br>"
		"� ������� ����������� ���������� ������ ������ <b><i>LZXLib</b></i></center></p><br>"
		"����������: �������� ���������� ��������� ProtoArchive<br>"
		"����������� � ������ ������������� ���������� ������ �<br>"
		"���������� ������ ��������� �������� (.exe, .txt, .res�).<br>"
		"��������� ����� ����������� ��� ��� ������������ ����������<br>"
		"������, ��� � �������� �������������� ��� ���������� ������<br>"
		"� ��������������� ��������, �������� �� �� ���� ��������."
		"<p><i>�����������: ��������� �.�.<br>"
		"������� <b>���� ��. �.�.�������</b><br>"
		"���������� \"����������� � ����������\" (��)<br>"
		"������� \"�������������� ������ � ����\" (��6)<br>"
		"������ ��6-32</i></p>"));
}

void ArchProj::setupMenu()
{
	//Menu File
	QMenu* mnuFile = menuBar()->addMenu(tr("����"));
  //QAction* actOpen = mnuFile->addAction(tr("�������..."));
	//connect(actOpen, SIGNAL(triggered()), this, SLOT(OpenFileDlg()));
	mnuFile->addAction(m_exitAct);

	QMenu* mnuView = menuBar()->addMenu(tr("���"));
	mnuView->addActions(m_viewActions->actions());
  mnuView->addSeparator();
  //mnuView->addAction ( .. ������ ������ ������ ������ ��� ������ ... )

	QMenu* mnuTools = menuBar()->addMenu(tr("�����������"));
	mnuTools->addAction(m_addArchAct);
	mnuTools->addAction(m_compressAct);
	mnuTools->addAction(m_decompressAct);
	mnuTools->addAction(m_infoAct);
	mnuTools->addSeparator();
	mnuTools->addAction(m_plugsAct);

	QMenu* mnuLangs = mnuTools->addMenu(tr("����"));
  //mnuLangs->addActions(m_langsActions->actions());

	QMenu* mnuWindow = menuBar()->addMenu(tr("����"));
	mnuWindow->addAction(tr("���������� ������"));
	mnuWindow->addAction(tr("�������� �������"));
	//Menu Help
	QMenu* mnuHelp = menuBar()->addMenu("������");
	mnuHelp->addAction(m_aboutAct);
	//popupFileList.About...
}

void ArchProj::createToolBars()
{
	m_fileToolBar = addToolBar(tr("�����������"));
	m_fileToolBar->setIconSize(QSize(64,64));
	m_fileToolBar->addAction(m_addArchAct);
	m_fileToolBar->addAction(m_compressAct);
	m_fileToolBar->addAction(m_decompressAct);
	m_fileToolBar->addAction(m_infoAct);
	m_fileToolBar->addAction(m_aboutAct);
	m_fileToolBar->addAction(m_exitAct);
}

//������� ���� �� �������� ������ ���������� ���������� ��� � ������ ���������
void ArchProj::doubleTreeClicked(const QModelIndex& index)
{
	//�� ������� ���
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
	ArchFiles_Dock->setWindowTitle(tr("����� ��� ������"));
	ArchFiles_Dock->setAllowedAreas(Qt::AllDockWidgetAreas);

	LookFiles_Dock = new QDockWidget(this);
	LookFiles_Dock->setWindowTitle(tr("��������"));
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
	statusBar->showMessage(tr("�����"));
	setStatusBar(statusBar);
}

void ArchProj::showCompressDlg(bool bChecked)
{
	if(m_archList->count() == 0)
	{
		QMessageBox::warning(this, tr("��������������: ������ ������"),
			tr("��� ������ ������ ��� ������ ����!"));
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
	//������� ������ ����������
	DecompressDlg* decompressDlg = new DecompressDlg(this, m_archManager);
	decompressDlg->setupUI();
	decompressDlg->setModal(true);
	decompressDlg->show();
}

void ArchProj::showInfoDlg(bool bChecked)
{
	QMessageBox::information(this, "���������� �� ������", "�������������� ������");
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
	//��� �������� ������ ������� ������ �������� �������
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

	m_treeView->setAnimated(false);// �������� �� �������� � ����������
	//m_treeView->setSortingEnabled(true); 
	//m_treeView->setWindowTitle(tr("�������� �������"));
	m_treeView->setDragDropMode(QAbstractItemView::InternalMove);
	//m_treeView->show();
	connect(m_treeView, SIGNAL(doubleClicked(const QModelIndex&)), 
	this, SLOT(doubleTreeClicked(const QModelIndex&)));
	setCentralWidget(m_treeView);
}

void ArchProj::createActions()
{
	//���������� ������ �������� ��� ������������ ����
	m_viewActions = new QActionGroup(this);
	QAction* treeViewAct = m_viewActions->addAction(tr("�����������"));
	QAction* listViewAct = m_viewActions->addAction(tr("������"));
	QAction* noViewAct = m_viewActions->addAction(tr("��� ���������"));
	listViewAct->setCheckable(true);
	treeViewAct->setCheckable(true);
	noViewAct->setCheckable(true);
	connect(listViewAct, SIGNAL(toggled(bool)), this, SLOT( changeListViewer(bool) ));
	connect(treeViewAct, SIGNAL(toggled(bool)), this, SLOT( changeTreeViewer(bool) ));
	connect(noViewAct, SIGNAL(toggled(bool)), this, SLOT( changeNoViewer(bool) ));

	//������������� �� ���������
	noViewAct->setChecked(true);

	//���������� ������ �������� ��� ������������ ������
  /* ��������� ������!
  m_langsActions = new QActionGroup(this);
	QStringList qmFiles = m_chooser.findQmFiles("langs");

	QStringList::iterator iter,end = qmFiles.end();
	for(iter = qmFiles.begin(); iter != end; ++iter)
	{
		QString language = m_chooser.languageName(*iter);
		QAction* actLang = m_langsActions->addAction(language);
  }*/

	//�������� ������� �����
	m_addArchAct = new QAction( tr("��������..."), this);
	m_addArchAct->setShortcut( QKeySequence::fromString("A") );
	m_addArchAct->setStatusTip( tr("�������� ���� � �����") );


	m_compressAct = new QAction( tr("������..."), this);
	m_compressAct->setShortcut( QKeySequence::fromString("C") );
	m_compressAct->setStatusTip( tr("�������� ������ ������") );
	connect(m_compressAct, SIGNAL(triggered(bool)), this, SLOT(showCompressDlg(bool)));

	m_decompressAct = new QAction( tr("����������...") , this);
	m_decompressAct->setShortcut( QKeySequence::fromString("D") );
	m_decompressAct->setStatusTip( tr("������� ������ ����������") );
	connect(m_decompressAct, SIGNAL(triggered(bool)), this, SLOT(showDecompressDlg(bool)));

	m_plugsAct = new QAction( tr("������") ,this);
	m_plugsAct->setShortcut( QKeySequence::fromString("Ctrl+P") );
	m_plugsAct->setStatusTip( tr("�������� ������ ��� ���������� ��������") );
	connect(m_plugsAct, SIGNAL(triggered(bool)), this, SLOT(showPluginsDlg(bool)));

	m_infoAct = new QAction( tr("����������"), this);
	m_infoAct->setShortcut( QKeySequence::fromString("Ctrl+I") );
	m_infoAct->setStatusTip( tr("�������� ���������� � ������ �����") );
	connect(m_infoAct, SIGNAL(triggered(bool)), this, SLOT(showInfoDlg(bool)));

	m_aboutAct = new QAction( tr("���������"), this);
	m_aboutAct->setShortcut( QKeySequence::fromString("Ctrl+A") );
	m_aboutAct->setStatusTip( tr("� ��������� � �� �������������") );
	connect(m_aboutAct, SIGNAL(triggered()), this, SLOT(showAboutDlg()));
	
	m_exitAct = new QAction( tr("�����"), this);
	m_exitAct->setShortcut( QKeySequence::fromString("Ctrl+C") );
	m_exitAct->setStatusTip( tr("����� �� ����������...") );
	connect(m_exitAct, SIGNAL(triggered()), this, SLOT(close()));
}
