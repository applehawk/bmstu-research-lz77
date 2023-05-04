#include "decompressdlg.h"
#include "extensionmng.h"
#include "statusdlg.h"

#include <QLineEdit>
#include <QPushButton>
#include <QLayout>
#include <QGroupBox>
#include <QFileDialog>
#include <QMessageBox>

DecompressDlg::DecompressDlg(QWidget* parent, ExtensionMng* archManager):
	m_archManager(archManager) 
{
	setWindowTitle(tr("���������� ������"));
}
DecompressDlg::~DecompressDlg() {}


void DecompressDlg::browseArchSlot()
{
	QString archPath = QFileDialog::getOpenFileName(NULL, 
    tr("����������: ���� ������"), QDir::currentPath(), "*.paf");
	m_ArchFile_Edit->setText(archPath);
  //��� ���������� ��������� ���������� ��� ��������� �����
  m_DirUnpack_Edit->setText( QFileInfo(archPath).absoluteDir().absolutePath() );
}
void DecompressDlg::browseDirSlot()
{
	QString dirPath = QFileDialog::getExistingDirectory(NULL, 
		tr("����������: ����������"), QDir::currentPath());
	m_DirUnpack_Edit->setText(dirPath);
}

void DecompressDlg::setupUI()
{
  QVBoxLayout* Main_Lay = new QVBoxLayout();
  QHBoxLayout* Arch_Lay = new QHBoxLayout();
  QHBoxLayout* Dir_Lay = new QHBoxLayout();

  QGroupBox* fileArchGroup = new QGroupBox(tr("�������� �����"));
  m_ArchFile_Edit = new QLineEdit();
  QPushButton* BrowseArch_Btn = new QPushButton();
	connect(BrowseArch_Btn,SIGNAL(clicked()), this, SLOT(browseArchSlot()));
	BrowseArch_Btn->setText(tr("�����..."));

  QGroupBox* dirDestGroup = new QGroupBox(tr("���������� ��� ����������"));
  m_DirUnpack_Edit = new QLineEdit();
  QPushButton* BrowseDir_Btn = new QPushButton();
	connect(BrowseDir_Btn,SIGNAL(clicked()), this, SLOT(browseDirSlot()));
	BrowseDir_Btn->setText(tr("����..."));

	Arch_Lay->addWidget(m_ArchFile_Edit);
	Arch_Lay->addWidget(BrowseArch_Btn);
	fileArchGroup->setLayout(Arch_Lay);

	Dir_Lay->addWidget(m_DirUnpack_Edit);
	Dir_Lay->addWidget(BrowseDir_Btn);
	dirDestGroup->setLayout(Dir_Lay);

  QHBoxLayout* controlBtnsLay = new QHBoxLayout();

  QPushButton* cancelBtn = new QPushButton(tr("������"));
	connect(cancelBtn, SIGNAL(clicked()), this, SLOT(close()));
  QPushButton* decompressBtn = new QPushButton(tr("������"));
	connect(decompressBtn, SIGNAL(clicked()), this, SLOT(decompressSlot()));

	controlBtnsLay->addStretch(10);
	controlBtnsLay->addWidget(cancelBtn);
	controlBtnsLay->addWidget(decompressBtn);
	Main_Lay->addWidget(fileArchGroup);
	Main_Lay->addWidget(dirDestGroup);
	Main_Lay->addLayout(controlBtnsLay);
	setLayout(Main_Lay);
}

void DecompressDlg::decompressSlot()
{
	if(!QFileInfo(m_ArchFile_Edit->text()).exists() 
		|| !QDir(m_DirUnpack_Edit->text()).exists())
	{
		QMessageBox::warning(this, tr("�����������: ��������������!"),
			tr("������ ���������������� ���� ����������"
			"��� �������������� ����������"), QMessageBox::Ok);
		return;
	}

	//������� ������ �������
	StatusDlg* statusDlg = new StatusDlg(parentWidget(), false);
	statusDlg->setupUI();
	statusDlg->show();
	statusDlg->setTotalBytesAll(QFileInfo(m_ArchFile_Edit->text()).size());

  ArchThread* archThread = m_archManager->createArchThread("paf", LZX);
  if(archThread == NULL)
  {
    QMessageBox::warning(this, "������ ����������",
                         "����������� ������ ������\n���������� �� ��������������!");
    return;
  }

	statusDlg->setArchThread(archThread);
  archThread->startDecompressProcess(m_ArchFile_Edit->text(), m_DirUnpack_Edit->text());

	close();
}
