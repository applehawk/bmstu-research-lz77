#ifndef __ARCHPROJ_DECOMPRESSDLG_H__
#define __ARCHPROJ_DECOMPRESSDLG_H__

#include <QDialog>

class ExtensionMng;
class QLineEdit;

class DecompressDlg : public QDialog
{
	Q_OBJECT
private:
	ExtensionMng* m_archManager;
	QLineEdit* m_ArchFile_Edit;
	QLineEdit* m_DirUnpack_Edit;
private slots:
	void browseArchSlot();
	void browseDirSlot();
	void decompressSlot();
public:
	DecompressDlg(QWidget* parent, ExtensionMng* archManager);
	~DecompressDlg();
	void setupUI();
};

#endif