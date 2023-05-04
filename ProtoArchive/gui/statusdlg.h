#ifndef __ARCHPROJ_STATUSDLG_H__
#define __ARCHPROJ_STATUSDLG_H__

#include <QDialog>

class QProgressBar;
class QLabel;
class QPushButton;
class QThread;

class ArchStatus;
class ArchThread;
//Диалог статуса упаковки и распаковки
class StatusDlg : public QDialog
{
	Q_OBJECT
private:
	QPushButton* m_Cancel_Btn;
	QProgressBar* m_All_Progress;
	QProgressBar* m_CurFile_Progress;
	QLabel* m_CurFile_Label;
	QPushButton* Cancel_Btn;
	//Поток процесса обработки(сжатия,распаковки)
	ArchThread* m_processThread;

	quint64 m_totalBytesAll; //размер всех обрабатываемых файлов
	quint64 m_currTotalBytesAll; //текущий(накопленный) размер обработанных файлов
	volatile bool bCanceled;
signals:
	void statusChangeSignal(ArchStatus* status);
private slots:
	void stopArchThreadSlot(); //вызывается когда поток завершился
	void stopProcessSlot(); //вызывается, когда пользователь нажал отмену процесса
	void statusChangeSlot(ArchStatus* status); //вызывается, когда сменился статус
public:
	StatusDlg(QWidget* parent, bool bCompressStatus);
	~StatusDlg();
	//Установить поток архивации
	//для реализации его отмены
	void setArchThread(ArchThread* processThread);
	//Установить кол-во всех байтов
	void setTotalBytesAll(quint64 totalBytesAll);

	void setupUI();
};

#endif