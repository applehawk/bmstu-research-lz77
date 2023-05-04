#include "statusdlg.h"
#include "extensionmng.h"

#include <QProgressBar>
#include <QBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QThread>
#include <QMessageBox>

StatusDlg::StatusDlg(QWidget* parent, bool bCompressStatus) : QDialog(parent), 
m_totalBytesAll(0), 
m_currTotalBytesAll(0)
{
	if(bCompressStatus)
		setWindowTitle(tr("Процесс архивации"));
	else
		setWindowTitle(tr("Процесс распаковки"));
}
StatusDlg::~StatusDlg() {}

void StatusDlg::setTotalBytesAll(quint64 totalBytesAll)
{
	m_totalBytesAll = totalBytesAll;
	bCanceled = false;
}

void StatusDlg::setupUI()
{
	QLabel* All_Label = new QLabel();
	All_Label->setText(tr("Статус операции:"));
	m_All_Progress = new QProgressBar();

	m_CurFile_Label = new QLabel();
	m_CurFile_Label->setText(tr("Файл:"));
	m_CurFile_Progress = new QProgressBar();

	Cancel_Btn = new QPushButton("Отмена");
	connect(Cancel_Btn, SIGNAL(clicked()),
							  this, SLOT(stopProcessSlot()));

	QHBoxLayout* HBtnBox = new QHBoxLayout();
	HBtnBox->addStretch();
	HBtnBox->addWidget(Cancel_Btn);
	QVBoxLayout* VMainBox = new QVBoxLayout();
	VMainBox->addWidget(All_Label);
	VMainBox->addWidget(m_All_Progress);
	VMainBox->addWidget(m_CurFile_Label);
  m_All_Progress->setValue(30);
  m_All_Progress->setMaximum(100);
  m_CurFile_Label->setText(tr("D:\\ProtoArchive\\test\\ProtoArchive.exe"));
	VMainBox->addWidget(m_CurFile_Progress);
  m_CurFile_Progress->setValue(30);
  m_CurFile_Progress->setMaximum(100);
	VMainBox->addLayout(HBtnBox);
	setFixedSize(360,130);

	setLayout(VMainBox);
}

void StatusDlg::setArchThread(ArchThread* processThread)
{
	m_processThread = processThread;
	//После завершения выполнения потока, удалить поток
	connect(processThread, SIGNAL(finished()), this, SLOT(stopArchThreadSlot()));
	connect(processThread, SIGNAL(changeStatus(ArchStatus*)),
		this, SLOT(statusChangeSlot(ArchStatus*)));
}

void StatusDlg::stopProcessSlot()
{
	bCanceled = true;
}

void StatusDlg::stopArchThreadSlot()
{
	m_CurFile_Label->setText(tr("Файл: "));
	setWindowTitle(tr("Процесс завершен!"));
	//m_processThread->terminate();//НеправильНо!...
	m_processThread->wait();
	delete m_processThread;
	hide();
}

void StatusDlg::statusChangeSlot(ArchStatus* status)
{
	if(status->isFail()) {
		QMessageBox::critical(NULL,"Ошибка!","Произошла ошибка при сжатии");
		emit stopArchThreadSlot();
		return;
	}
	if(status->isFinished())
	{
		//Если закончили сжимать очередной файл,
		//добавляем его размер к общему размеру
		m_currTotalBytesAll += status->processedBytes();
	}

	m_CurFile_Label->setText(tr("Файл: %1").arg(status->fileName()));
	m_CurFile_Progress->setRange(0,status->fileSize());
	m_CurFile_Progress->setValue(status->processedBytes());

	m_All_Progress->setValue(m_currTotalBytesAll+status->processedBytes());
	m_All_Progress->setRange(0,m_totalBytesAll);

	if(bCanceled)
	{
		status->setFlag(STAT_Canceled);
		return;
	}

	if(status->isCompression())
		setWindowTitle(tr("Сжатие файлов"));
	if(status->isDecompression())
		setWindowTitle(tr("Распаковка архива"));
}
