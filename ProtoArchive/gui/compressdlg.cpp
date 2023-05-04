#include "compressdlg.h"
#include "filelistwidget.h"
#include "extensionmng.h"
#include "statusdlg.h"
#include "..\methods\archive_method.h"

#include <QPushButton>
#include <QBoxLayout>
#include <QLineEdit>
#include <QGroupBox>
#include <QLabel>
#include <QComboBox>

#include <QDataStream>
#include <QFile>
#include <QMessageBox>
#include <QFileDialog>

CompressDlg::CompressDlg(QWidget* parent, FileListWidget* archFiles, ExtensionMng* archManager) : 
QDialog(parent, Qt::Dialog), m_archFiles(archFiles), m_archManager(archManager) 
{
	m_totalBytesAll = archFiles->calcSizeOfFiles();
}

CompressDlg::~CompressDlg() {}

void CompressDlg::pathEditSlot()
{
	QString fileName = 
    QFileDialog::getSaveFileName(0, "Файл архива", QDir::current().absolutePath(), "*.paf");

	m_PathTo_Edit->setText(fileName);
}

QString sizeFormat(qint64 fileSize, bool bPowTwoCorrect=true)
{
	QString str;
	QStringList sizeNames;
	sizeNames << "Байт" << "Кб" << "Мб" << "Гб";
	int i=0;
	if(bPowTwoCorrect)
	{
		for(; fileSize>1024; ++i)
			fileSize>>=10;
	}
	str.append(sizeNames[i]);
	QString sizeStr = QString("%1").arg(fileSize);
	int slen = sizeStr.length();
	int sizeLenDiv3 = slen/3;
	for(int i=1; i<sizeLenDiv3; ++i)
		sizeStr.insert(slen-i*3,",");

	//Последний разделитель(число символов не кратно 3)
	//чтобы не вставлял разделитель спереди числа
	if(slen % 3 != 0 && slen > 3)
		sizeStr.insert(slen%3,",");
	str.prepend( sizeStr );
	return str;
}

void CompressDlg::methodChoose(int idx)
{
  switch(idx)
  {
    case 0: {
        m_HashSize_Box->setVisible(true);
        m_HashSize_Label->setVisible(true);
      }
      break;
    default: {
        m_HashSize_Box->setVisible(false);
        m_HashSize_Label->setVisible(false);
      }
  };
}

void CompressDlg::sliderMoved(int value)
{
  m_HashSize_Box->setCurrentIndex(10);
  m_DictSize_Box->setCurrentIndex((int)(value*2.5));
}

void CompressDlg::setupUI()
{
  QVBoxLayout* Main_Lay = new QVBoxLayout();
  setLayout(Main_Lay);
  setWindowTitle( tr("Сжатие...") );

  //Задаем Слой Пути к Файлу
  QHBoxLayout* Path_Lay = new QHBoxLayout();

  m_PathTo_Edit = new QLineEdit();
  //Предлагаем имя архива по первому файлу в списке
  //и текущий путь для архива
  QString firstFileName = m_archFiles->item(0)->text();
  //Предусматриваем ситуацию, когда имя файла содержится в суффиксе(тоесть начинается с точки)
  QString baseName = QFileInfo(firstFileName).baseName();
  QString suffixName = QFileInfo(firstFileName).suffix();
  m_PathTo_Edit->setText(
      QDir::current().absoluteFilePath(
          (baseName.isEmpty()?suffixName:baseName) + "." + "paf"
          ) );

  QPushButton* BrowseDir_Btn = new QPushButton();
  BrowseDir_Btn->setText(tr("Путь..."));
  connect(BrowseDir_Btn, SIGNAL(clicked()), this, SLOT(pathEditSlot()));

  Path_Lay->addWidget(m_PathTo_Edit);
  Path_Lay->addWidget(BrowseDir_Btn);
  ///////////////////////////////////
  //Центральный слой
  QHBoxLayout* Central_Lay = new QHBoxLayout();
  //Слой для установки параметров
  QVBoxLayout* Params_Lay = new QVBoxLayout();

  QGroupBox* Params_Group = new QGroupBox();
  Params_Group->setTitle(tr("Параметры:"));
  m_CompressionMethod_Box = new QComboBox();
  connect(m_CompressionMethod_Box, SIGNAL(activated(int)), this, SLOT(methodChoose(int)));
  m_CompressionMethod_Box->addItem("LZX");
  m_CompressionMethod_Box->addItem("SuffixLZ");
  m_CompressionMethod_Box->addItem("ZLib");

  QLabel* DictSize_Label = new QLabel(tr("Размер словаря:"));
  DictSize_Label->setToolTip(tr("Больший размер словаря\n"
                          "позволяет повысить эффективность сжатия,\n"
                          "но скорость быстро падает пропорционально\n"
                          "велечине размера словаря"));
  m_DictSize_Box = new QComboBox();
  for(int i = 0; i<12; ++i)
    m_DictSize_Box->addItem( QString("%1 Кб").arg(2<<i) );

  m_HashSize_Box = new QComboBox();
  for(int i = 0; i<12; ++i)
    m_HashSize_Box->addItem( QString("%1 Кб").arg(2<<i) );

  m_HashSize_Label = new QLabel(tr("Размер хэш-таблицы:"));
  //m_HashSize_Label->setToolTip;

  Params_Lay->addWidget( new QLabel(tr("Метод сжатия:")) );
  Params_Lay->addWidget(m_CompressionMethod_Box);
  Params_Lay->addSpacerItem( new QSpacerItem( 120, 20,QSizePolicy::Fixed, QSizePolicy::Expanding) );
  Params_Lay->addWidget( DictSize_Label );
  Params_Lay->addWidget(m_DictSize_Box);
  Params_Lay->addWidget( m_HashSize_Label );
  Params_Lay->addWidget(m_HashSize_Box);
  Params_Group->setLayout(Params_Lay);
  Central_Lay->addWidget(Params_Group);
  /////////////////////////////////////
  //Информационный слой
  QVBoxLayout* Info_Lay = new QVBoxLayout();
  QGroupBox* InfoCompress_Group = new QGroupBox();
  InfoCompress_Group->setTitle(tr("Информация:"));

  QLabel* Size_Label = new QLabel(tr("Размер файлов: %1").
                                  arg(sizeFormat(m_totalBytesAll)), this);
  Size_Label->setToolTip(tr("%1")
                         .arg(sizeFormat(m_totalBytesAll,false)));
  Info_Lay->addWidget(Size_Label);

  Info_Lay->addWidget(
          new QLabel(tr("Кол-во: %1").arg(m_archFiles->count()))
          );
  Info_Lay->addSpacerItem( new QSpacerItem( 120, 20,QSizePolicy::Fixed, QSizePolicy::Expanding) );
  InfoCompress_Group->setLayout(Info_Lay);

  Central_Lay->addWidget(InfoCompress_Group);
  //Слой управления
  QHBoxLayout* Buttons_Lay = new QHBoxLayout();
  m_compress_Btn = new QPushButton();
  m_compress_Btn->setText(tr("Сжать"));
  m_compress_Btn->setDefault(true);
  connect(m_compress_Btn, SIGNAL(clicked()), this, SLOT(compressSlot()));

  m_cancel_Btn = new QPushButton();
  m_cancel_Btn->setText(tr("Отмена"));
  connect(m_cancel_Btn, SIGNAL(clicked()), this, SLOT(close()));
  Buttons_Lay->addStretch(0);
  Buttons_Lay->addWidget(m_compress_Btn);
  Buttons_Lay->addWidget(m_cancel_Btn);
  //////////////////////////////////////
  //Слой для управления качеством сжатия
  QGroupBox* qualityCompress = new QGroupBox();
  qualityCompress->setTitle( tr("Качество сжатия:") );
  QVBoxLayout* qVLay = new QVBoxLayout();
  QHBoxLayout* qSliderLay = new QHBoxLayout();
  m_qSlider = new QSlider();
  connect(m_qSlider, SIGNAL(sliderMoved(int)), this, SLOT(sliderMoved(int)));
  //Подписываю не очень универсально тики на слайдере, как красиво подписать не придумал
  //(стандартные средства Qt не позволяют это просто сделать)
  //можно получать позицию тиков по стилю QStyle::sliderPositionFromValue

  qSliderLay->addSpacing(20);
  qSliderLay->addWidget(m_qSlider);
  qSliderLay->addSpacing(25);

  m_qSlider->setOrientation(Qt::Horizontal);
  m_qSlider->setRange(0, 4);
  m_qSlider->setTickInterval(1);
  m_qSlider->setTickPosition(QSlider::TicksAbove);


  QGridLayout* textTickLay = new QGridLayout();
  textTickLay->setSizeConstraint(QLayout::SetMaximumSize);

  textTickLay->addWidget( new QLabel(tr("Мгновенно")), 1, 1, Qt::AlignLeft);
  textTickLay->addWidget( new QLabel(tr("Быстро")), 1, 2, Qt::AlignLeft );
  textTickLay->addWidget( new QLabel(tr("Баланс")), 1, 3, Qt::AlignCenter);
  textTickLay->addWidget( new QLabel(tr("Сильно")), 1, 4, Qt::AlignRight);
  textTickLay->addWidget( new QLabel(tr("Сверхсильно")), 1, 5, Qt::AlignRight );


  qVLay->addLayout(qSliderLay);
  qVLay->addLayout(textTickLay);
  qualityCompress->setLayout(qVLay);

  Main_Lay->addLayout(Path_Lay);
  Main_Lay->addLayout(Central_Lay);
  Main_Lay->addWidget(qualityCompress);
  Main_Lay->addLayout(Buttons_Lay);
}


void CompressDlg::compressSlot()
{
	if(!m_archFiles)
		QMessageBox::critical(this, tr("Critical error"), tr("File list not access"));
  if(!QFileInfo( m_PathTo_Edit->text() ).absoluteDir().exists())
  {
    QMessageBox::warning(this, "Диалог сжатия",
                "Путь к файлу архива указан не корректно!");
    return;
  }
	QStringList files;
	m_archFiles->filesStringList(&files);

  MethodType type;

  //Задаем выбранный метод
  switch(m_CompressionMethod_Box->currentIndex())
  {
    case(0): type = LZX;  break;
    case(1): type = LZ_SUFFIX; break;
    case(2): type = ZLib; break;
    default: type = LZX;
  };
  QString archFmtName = QFileInfo( m_PathTo_Edit->text() ).suffix();
  ArchThread* archThread = m_archManager->createArchThread(archFmtName, type);
  if(archThread == NULL)
  {
    QMessageBox::warning(this, "Диалог сжатия",
                         "Запрошенный формат архива\nпрограммой не поддерживается!");
    return;
  }
  ArchiveMethod* method = archThread->archMethod();
  MethodParams& params = method->params();

  params.setParam("WindowBits", m_DictSize_Box->currentIndex()+11);
  params.setParam("HashBits", m_HashSize_Box->currentIndex()+11);
  if(m_qSlider->value() == 3)
    params.setParam("MaxDepth", 500);
  if(m_qSlider->value() == 4)
    params.setParam("MaxDepth", -1);

    //Создаем диалог статуса
  StatusDlg* statusDlg = new StatusDlg(parentWidget(), true);
  statusDlg->setupUI();
  statusDlg->show();
  statusDlg->setTotalBytesAll(m_totalBytesAll);

  //присоединяем к диалогу статуса потока сжатия
  statusDlg->setArchThread(archThread);
  archThread->startCompressProcess(files,m_PathTo_Edit->text());

	close();
}

