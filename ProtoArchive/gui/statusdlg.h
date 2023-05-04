#ifndef __ARCHPROJ_STATUSDLG_H__
#define __ARCHPROJ_STATUSDLG_H__

#include <QDialog>

class QProgressBar;
class QLabel;
class QPushButton;
class QThread;

class ArchStatus;
class ArchThread;
//������ ������� �������� � ����������
class StatusDlg : public QDialog
{
	Q_OBJECT
private:
	QPushButton* m_Cancel_Btn;
	QProgressBar* m_All_Progress;
	QProgressBar* m_CurFile_Progress;
	QLabel* m_CurFile_Label;
	QPushButton* Cancel_Btn;
	//����� �������� ���������(������,����������)
	ArchThread* m_processThread;

	quint64 m_totalBytesAll; //������ ���� �������������� ������
	quint64 m_currTotalBytesAll; //�������(�����������) ������ ������������ ������
	volatile bool bCanceled;
signals:
	void statusChangeSignal(ArchStatus* status);
private slots:
	void stopArchThreadSlot(); //���������� ����� ����� ����������
	void stopProcessSlot(); //����������, ����� ������������ ����� ������ ��������
	void statusChangeSlot(ArchStatus* status); //����������, ����� �������� ������
public:
	StatusDlg(QWidget* parent, bool bCompressStatus);
	~StatusDlg();
	//���������� ����� ���������
	//��� ���������� ��� ������
	void setArchThread(ArchThread* processThread);
	//���������� ���-�� ���� ������
	void setTotalBytesAll(quint64 totalBytesAll);

	void setupUI();
};

#endif