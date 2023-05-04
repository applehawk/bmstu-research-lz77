#ifndef __ARCHPROJ_MAINWNDW_H__
#define __ARCHPROJ_MAINWNDW_H__

#include <QtGui/QMainWindow>
#include <QAbstractItemModel>
#include <QDirModel>
#include <QList>
#include <QVariant>
#include <QToolBar>
#include <QListWidget>
#include "langchooser.h"

class QActionGroup;
class QTreeView;
class QListView;
class QDirModel;
class FileListWidget;

class ExtensionMng;

class DirModel : public QDirModel
{
	Q_OBJECT
public:
	DirModel(const QStringList &nameFilters, QDir::Filters filters,
		QDir::SortFlags sort, QObject *parent = 0);
	explicit DirModel(QObject *parent = 0);

	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
};

class ArchProj : public QMainWindow
{
	Q_OBJECT
private:
	QToolBar* m_fileToolBar;

	QDockWidget* ArchFiles_Dock;
	QDockWidget* LookFiles_Dock;

	QTreeView* m_treeView; //� ���� ������
	QListView* m_listView; //� ���� ������

	QDirModel* m_dirModel; //�������� ������ ��� TreeView

	QActionGroup* m_viewActions;
	
	FileListWidget* m_archList; //������ ������ ��� ��������
	QMenu* m_popupFileListMenu;

	QAction* m_compressAct;
	QAction* m_decompressAct;
	QAction* m_infoAct;
	QAction* m_aboutAct;
	QAction* m_exitAct;
	QAction* m_plugsAct;
	QAction* m_addArchAct;

	QActionGroup* m_langsActions; //������������ ����� �������
	LangChooser m_chooser;

	ExtensionMng* m_archManager;
private slots:
	void showCompressDlg(bool bChecked);
	void showDecompressDlg(bool bChecked);
	void showInfoDlg(bool bChecked);
	void showPluginsDlg(bool bChecked);
	void showAboutDlg();

	void doubleTreeClicked(const QModelIndex& index);

	void changeListViewer(bool bChecked);
	void changeTreeViewer(bool bChecked);
	void changeNoViewer(bool bChecked);
private:
	void keyPressEvent ( QKeyEvent * event );
public:
	ArchProj(QWidget *parent = 0, Qt::WFlags flags = 0);
	~ArchProj();

	void setupMenu();
	void setupUI();
	void createToolBars();
	void createActions();

	bool setLanguage(const QString& langName);
private:
};

#endif // ARCHPROJ_H
