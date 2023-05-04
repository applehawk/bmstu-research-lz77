#ifndef __ARCHPROJ_COMPRESSDLG_H__
#define __ARCHPROJ_COMPRESSDLG_H__

#include <QDialog>
#include <QFileDialog>
#include <QMutex>

class FileListWidget;
class QPushButton;
class QLineEdit;
class QComboBox;
class QLabel;
class QSlider;

class ExtensionMng;

class CompressDlg : public QDialog
{
	Q_OBJECT
private slots:
	void compressSlot();
	void pathEditSlot();

  void methodChoose(int idx);
  void sliderMoved(int value);
private:
  QSlider* m_qSlider;
	QPushButton* m_compress_Btn;
	QPushButton* m_cancel_Btn;
	QLineEdit* m_PathTo_Edit;
  QComboBox* m_CompressionMethod_Box;
  QComboBox* m_DictSize_Box;
  QComboBox* m_HashSize_Box;

  QLabel* m_HashSize_Label;

	ExtensionMng* m_archManager;
	//Файлы для сжатия
	FileListWidget* m_archFiles;
	quint64 m_totalBytesAll;
public:
	CompressDlg(QWidget* parent, FileListWidget* archFiles, ExtensionMng* archManager);
	~CompressDlg();
	void setupUI();
};

#endif
