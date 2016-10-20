
#include "config.h"

#include <iostream>
#include <cmath>

#include <QFileDialog>
#include <QMessageBox>
#include <QCloseEvent>
#include <QSettings>
#include <QtGlobal>
#include "mainwindow.h"
#include "ui_mainwindow.h"

namespace {

static const struct {
    char backend_name[16];
    char full_string[32];
} backendList[] = {
#ifdef HAVE_JACK
    { "jack", "JACK" },
#endif
#ifdef HAVE_PULSEAUDIO
    { "pulse", "PulseAudio" },
#endif
#ifdef HAVE_ALSA
    { "alsa", "ALSA" },
#endif
#ifdef HAVE_COREAUDIO
    { "core", "CoreAudio" },
#endif
#ifdef HAVE_OSS
    { "oss", "OSS" },
#endif
#ifdef HAVE_SOLARIS
    { "solaris", "Solaris" },
#endif
#ifdef HAVE_SNDIO
    { "sndio", "SoundIO" },
#endif
#ifdef HAVE_QSA
    { "qsa", "QSA" },
#endif
#ifdef HAVE_MMDEVAPI
    { "mmdevapi", "MMDevAPI" },
#endif
#ifdef HAVE_DSOUND
    { "dsound", "DirectSound" },
#endif
#ifdef HAVE_WINMM
    { "winmm", "Windows Multimedia" },
#endif
#ifdef HAVE_PORTAUDIO
    { "port", "PortAudio" },
#endif
#ifdef HAVE_OPENSL
    { "opensl", "OpenSL" },
#endif

    { "null", "Null Output" },
#ifdef HAVE_WAVE
    { "wave", "Wave Writer" },
#endif
    { "", "" }
};

static const struct NameValuePair {
    const char name[64];
    const char value[16];
} speakerModeList[] = {
    { "Autodetect", "" },
    { "Mono", "mono" },
    { "Stereo", "stereo" },
    { "Quadrophonic", "quad" },
    { "5.1 Surround (Side)", "surround51" },
    { "5.1 Surround (Rear)", "surround51rear" },
    { "6.1 Surround", "surround61" },
    { "7.1 Surround", "surround71" },

    { "Ambisonic, 1st Order", "ambi1" },
    { "Ambisonic, 2nd Order", "ambi2" },
    { "Ambisonic, 3rd Order", "ambi3" },

    { "", "" }
}, sampleTypeList[] = {
    { "Autodetect", "" },
    { "8-bit int", "int8" },
    { "8-bit uint", "uint8" },
    { "16-bit int", "int16" },
    { "16-bit uint", "uint16" },
    { "32-bit int", "int32" },
    { "32-bit uint", "uint32" },
    { "32-bit float", "float32" },

    { "", "" }
}, resamplerList[] = {
    { "Point", "point" },
    { "Linear", "linear" },
    { "Default (Linear)", "" },
    { "4-Point Sinc", "sinc4" },
    { "8-Point Sinc", "sinc8" },
    { "Band-limited Sinc", "bsinc" },

    { "", "" }
}, stereoModeList[] = {
    { "Autodetect", "" },
    { "Speakers", "speakers" },
    { "Headphones", "headphones" },

    { "", "" }
}, stereoPanList[] = {
    { "Default", "" },
    { "UHJ", "uhj" },
    { "Pair-Wise", "paired" },

    { "", "" }
}, ambiFormatList[] = {
    { "Default", "" },
    { "ACN + SN3D", "acn+sn3d" },
    { "ACN + N3D", "acn+n3d" },
    { "Furse-Malham", "fuma" },

    { "", "" }
};

static QString getDefaultConfigName()
{
#ifdef Q_OS_WIN32
    static const char fname[] = "alsoft.ini";
    QByteArray base = qgetenv("AppData");
#else
    static const char fname[] = "alsoft.conf";
    QByteArray base = qgetenv("XDG_CONFIG_HOME");
    if(base.isEmpty())
    {
        base = qgetenv("HOME");
        if(base.isEmpty() == false)
            base += "/.config";
    }
#endif
    if(base.isEmpty() == false)
        return base +'/'+ fname;
    return fname;
}

static QString getBaseDataPath()
{
#ifdef Q_OS_WIN32
    QByteArray base = qgetenv("AppData");
#else
    QByteArray base = qgetenv("XDG_DATA_HOME");
    if(base.isEmpty())
    {
        base = qgetenv("HOME");
        if(!base.isEmpty())
            base += "/.local/share";
    }
#endif
    return base;
}

static QStringList getAllDataPaths(QString append=QString())
{
    QStringList list;
    list.append(getBaseDataPath());
#ifdef Q_OS_WIN32
    // TODO: Common AppData path
#else
    QString paths = qgetenv("XDG_DATA_DIRS");
    if(paths.isEmpty())
        paths = "/usr/local/share/:/usr/share/";
    list += paths.split(QChar(':'), QString::SkipEmptyParts);
#endif
    QStringList::iterator iter = list.begin();
    while(iter != list.end())
    {
        if(iter->isEmpty())
            iter = list.erase(iter);
        else
        {
            iter->append(append);
            iter++;
        }
    }
    return list;
}

template<size_t N>
static QString getValueFromName(const NameValuePair (&list)[N], const QString &str)
{
    for(size_t i = 0;i < N-1;i++)
    {
        if(str == list[i].name)
            return list[i].value;
    }
    return QString();
}

template<size_t N>
static QString getNameFromValue(const NameValuePair (&list)[N], const QString &str)
{
    for(size_t i = 0;i < N-1;i++)
    {
        if(str == list[i].value)
            return list[i].name;
    }
    return QString();
}

}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    mPeriodSizeValidator(NULL),
    mPeriodCountValidator(NULL),
    mSourceCountValidator(NULL),
    mEffectSlotValidator(NULL),
    mSourceSendValidator(NULL),
    mSampleRateValidator(NULL),
    mJackBufferValidator(NULL),
    mNeedsSave(false)
{
    ui->setupUi(this);

    for(int i = 0;speakerModeList[i].name[0];i++)
        ui->channelConfigCombo->addItem(speakerModeList[i].name);
    ui->channelConfigCombo->adjustSize();
    for(int i = 0;sampleTypeList[i].name[0];i++)
        ui->sampleFormatCombo->addItem(sampleTypeList[i].name);
    ui->sampleFormatCombo->adjustSize();
    for(int i = 0;stereoModeList[i].name[0];i++)
        ui->stereoModeCombo->addItem(stereoModeList[i].name);
    ui->stereoModeCombo->adjustSize();
    for(int i = 0;stereoPanList[i].name[0];i++)
        ui->stereoPanningComboBox->addItem(stereoPanList[i].name);
    ui->stereoPanningComboBox->adjustSize();
    for(int i = 0;ambiFormatList[i].name[0];i++)
        ui->ambiFormatComboBox->addItem(ambiFormatList[i].name);
    ui->ambiFormatComboBox->adjustSize();

    int count;
    for(count = 0;resamplerList[count].name[0];count++) {
    }
    ui->resamplerSlider->setRange(0, count-1);

    ui->hrtfStateComboBox->adjustSize();

#if !defined(HAVE_NEON) && !defined(HAVE_SSE)
    ui->cpuExtDisabledLabel->move(ui->cpuExtDisabledLabel->x(), ui->cpuExtDisabledLabel->y() - 60);
#else
    ui->cpuExtDisabledLabel->setVisible(false);
#endif

#ifndef HAVE_NEON

#ifndef HAVE_SSE4_1
#ifndef HAVE_SSE3
#ifndef HAVE_SSE2
#ifndef HAVE_SSE
    ui->enableSSECheckBox->setVisible(false);
#endif /* !SSE */
    ui->enableSSE2CheckBox->setVisible(false);
#endif /* !SSE2 */
    ui->enableSSE3CheckBox->setVisible(false);
#endif /* !SSE3 */
    ui->enableSSE41CheckBox->setVisible(false);
#endif /* !SSE4.1 */
    ui->enableNeonCheckBox->setVisible(false);

#else /* !Neon */

#ifndef HAVE_SSE4_1
#ifndef HAVE_SSE3
#ifndef HAVE_SSE2
#ifndef HAVE_SSE
    ui->enableNeonCheckBox->move(ui->enableNeonCheckBox->x(), ui->enableNeonCheckBox->y() - 30);
    ui->enableSSECheckBox->setVisible(false);
#endif /* !SSE */
    ui->enableSSE2CheckBox->setVisible(false);
#endif /* !SSE2 */
    ui->enableSSE3CheckBox->setVisible(false);
#endif /* !SSE3 */
    ui->enableSSE41CheckBox->setVisible(false);
#endif /* !SSE4.1 */

#endif

    mPeriodSizeValidator = new QIntValidator(64, 8192, this);
    ui->periodSizeEdit->setValidator(mPeriodSizeValidator);
    mPeriodCountValidator = new QIntValidator(2, 16, this);
    ui->periodCountEdit->setValidator(mPeriodCountValidator);

    mSourceCountValidator = new QIntValidator(0, 4096, this);
    ui->srcCountLineEdit->setValidator(mSourceCountValidator);
    mEffectSlotValidator = new QIntValidator(0, 16, this);
    ui->effectSlotLineEdit->setValidator(mEffectSlotValidator);
    mSourceSendValidator = new QIntValidator(0, 4, this);
    ui->srcSendLineEdit->setValidator(mSourceSendValidator);
    mSampleRateValidator = new QIntValidator(8000, 192000, this);
    ui->sampleRateCombo->lineEdit()->setValidator(mSampleRateValidator);

    mJackBufferValidator = new QIntValidator(0, 8192, this);
    ui->jackBufferSizeLine->setValidator(mJackBufferValidator);

    connect(ui->actionLoad, SIGNAL(triggered()), this, SLOT(loadConfigFromFile()));
    connect(ui->actionSave_As, SIGNAL(triggered()), this, SLOT(saveConfigAsFile()));

    connect(ui->actionAbout, SIGNAL(triggered()), this, SLOT(showAboutPage()));

    connect(ui->closeCancelButton, SIGNAL(clicked()), this, SLOT(cancelCloseAction()));
    connect(ui->applyButton, SIGNAL(clicked()), this, SLOT(saveCurrentConfig()));

    connect(ui->channelConfigCombo, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(enableApplyButton()));
    connect(ui->sampleFormatCombo, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(enableApplyButton()));
    connect(ui->stereoModeCombo, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(enableApplyButton()));
    connect(ui->sampleRateCombo, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(enableApplyButton()));
    connect(ui->sampleRateCombo, SIGNAL(editTextChanged(const QString&)), this, SLOT(enableApplyButton()));

    connect(ui->resamplerSlider, SIGNAL(valueChanged(int)), this, SLOT(updateResamplerLabel(int)));

    connect(ui->periodSizeSlider, SIGNAL(valueChanged(int)), this, SLOT(updatePeriodSizeEdit(int)));
    connect(ui->periodSizeEdit, SIGNAL(editingFinished()), this, SLOT(updatePeriodSizeSlider()));
    connect(ui->periodCountSlider, SIGNAL(valueChanged(int)), this, SLOT(updatePeriodCountEdit(int)));
    connect(ui->periodCountEdit, SIGNAL(editingFinished()), this, SLOT(updatePeriodCountSlider()));

    connect(ui->stereoPanningComboBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(enableApplyButton()));
    connect(ui->ambiFormatComboBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(enableApplyButton()));

    connect(ui->decoderHQModeCheckBox, SIGNAL(stateChanged(int)), this, SLOT(toggleHqState(int)));
    connect(ui->decoderDistCompCheckBox, SIGNAL(stateChanged(int)), this, SLOT(enableApplyButton()));
    connect(ui->decoderQuadLineEdit, SIGNAL(textChanged(QString)), this, SLOT(enableApplyButton()));
    connect(ui->decoderQuadButton, SIGNAL(clicked()), this, SLOT(selectQuadDecoderFile()));
    connect(ui->decoder51LineEdit, SIGNAL(textChanged(QString)), this, SLOT(enableApplyButton()));
    connect(ui->decoder51Button, SIGNAL(clicked()), this, SLOT(select51DecoderFile()));
    connect(ui->decoder51RearLineEdit, SIGNAL(textChanged(QString)), this, SLOT(enableApplyButton()));
    connect(ui->decoder51RearButton, SIGNAL(clicked()), this, SLOT(select51RearDecoderFile()));
    connect(ui->decoder61LineEdit, SIGNAL(textChanged(QString)), this, SLOT(enableApplyButton()));
    connect(ui->decoder61Button, SIGNAL(clicked()), this, SLOT(select61DecoderFile()));
    connect(ui->decoder71LineEdit, SIGNAL(textChanged(QString)), this, SLOT(enableApplyButton()));
    connect(ui->decoder71Button, SIGNAL(clicked()), this, SLOT(select71DecoderFile()));

    connect(ui->preferredHrtfComboBox, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(enableApplyButton()));
    connect(ui->hrtfStateComboBox, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(enableApplyButton()));
    connect(ui->hrtfAddButton, SIGNAL(clicked()), this, SLOT(addHrtfFile()));
    connect(ui->hrtfRemoveButton, SIGNAL(clicked()), this, SLOT(removeHrtfFile()));
    connect(ui->hrtfFileList, SIGNAL(itemSelectionChanged()), this, SLOT(updateHrtfRemoveButton()));
    connect(ui->defaultHrtfPathsCheckBox, SIGNAL(stateChanged(int)), this, SLOT(enableApplyButton()));

    connect(ui->srcCountLineEdit, SIGNAL(editingFinished()), this, SLOT(enableApplyButton()));
    connect(ui->srcSendLineEdit, SIGNAL(editingFinished()), this, SLOT(enableApplyButton()));
    connect(ui->effectSlotLineEdit, SIGNAL(editingFinished()), this, SLOT(enableApplyButton()));

    connect(ui->enableSSECheckBox, SIGNAL(stateChanged(int)), this, SLOT(enableApplyButton()));
    connect(ui->enableSSE2CheckBox, SIGNAL(stateChanged(int)), this, SLOT(enableApplyButton()));
    connect(ui->enableSSE3CheckBox, SIGNAL(stateChanged(int)), this, SLOT(enableApplyButton()));
    connect(ui->enableSSE41CheckBox, SIGNAL(stateChanged(int)), this, SLOT(enableApplyButton()));
    connect(ui->enableNeonCheckBox, SIGNAL(stateChanged(int)), this, SLOT(enableApplyButton()));

    ui->enabledBackendList->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->enabledBackendList, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showEnabledBackendMenu(QPoint)));

    ui->disabledBackendList->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->disabledBackendList, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showDisabledBackendMenu(QPoint)));
    connect(ui->backendCheckBox, SIGNAL(stateChanged(int)), this, SLOT(enableApplyButton()));

    connect(ui->defaultReverbComboBox, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(enableApplyButton()));
    connect(ui->emulateEaxCheckBox, SIGNAL(stateChanged(int)), this, SLOT(enableApplyButton()));
    connect(ui->enableEaxReverbCheck, SIGNAL(stateChanged(int)), this, SLOT(enableApplyButton()));
    connect(ui->enableStdReverbCheck, SIGNAL(stateChanged(int)), this, SLOT(enableApplyButton()));
    connect(ui->enableChorusCheck, SIGNAL(stateChanged(int)), this, SLOT(enableApplyButton()));
    connect(ui->enableCompressorCheck, SIGNAL(stateChanged(int)), this, SLOT(enableApplyButton()));
    connect(ui->enableDistortionCheck, SIGNAL(stateChanged(int)), this, SLOT(enableApplyButton()));
    connect(ui->enableEchoCheck, SIGNAL(stateChanged(int)), this, SLOT(enableApplyButton()));
    connect(ui->enableEqualizerCheck, SIGNAL(stateChanged(int)), this, SLOT(enableApplyButton()));
    connect(ui->enableFlangerCheck, SIGNAL(stateChanged(int)), this, SLOT(enableApplyButton()));
    connect(ui->enableModulatorCheck, SIGNAL(stateChanged(int)), this, SLOT(enableApplyButton()));
    connect(ui->enableDedicatedCheck, SIGNAL(stateChanged(int)), this, SLOT(enableApplyButton()));

    connect(ui->pulseAutospawnCheckBox, SIGNAL(stateChanged(int)), this, SLOT(enableApplyButton()));
    connect(ui->pulseAllowMovesCheckBox, SIGNAL(stateChanged(int)), this, SLOT(enableApplyButton()));
    connect(ui->pulseFixRateCheckBox, SIGNAL(stateChanged(int)), this, SLOT(enableApplyButton()));

    connect(ui->jackAutospawnCheckBox, SIGNAL(stateChanged(int)), this, SLOT(enableApplyButton()));
    connect(ui->jackBufferSizeSlider, SIGNAL(valueChanged(int)), this, SLOT(updateJackBufferSizeEdit(int)));
    connect(ui->jackBufferSizeLine, SIGNAL(editingFinished()), this, SLOT(updateJackBufferSizeSlider()));

    connect(ui->alsaDefaultDeviceLine, SIGNAL(textChanged(QString)), this, SLOT(enableApplyButton()));
    connect(ui->alsaDefaultCaptureLine, SIGNAL(textChanged(QString)), this, SLOT(enableApplyButton()));
    connect(ui->alsaResamplerCheckBox, SIGNAL(stateChanged(int)), this, SLOT(enableApplyButton()));
    connect(ui->alsaMmapCheckBox, SIGNAL(stateChanged(int)), this, SLOT(enableApplyButton()));

    connect(ui->ossDefaultDeviceLine, SIGNAL(textChanged(QString)), this, SLOT(enableApplyButton()));
    connect(ui->ossPlaybackPushButton, SIGNAL(clicked(bool)), this, SLOT(selectOSSPlayback()));
    connect(ui->ossDefaultCaptureLine, SIGNAL(textChanged(QString)), this, SLOT(enableApplyButton()));
    connect(ui->ossCapturePushButton, SIGNAL(clicked(bool)), this, SLOT(selectOSSCapture()));

    connect(ui->solarisDefaultDeviceLine, SIGNAL(textChanged(QString)), this, SLOT(enableApplyButton()));
    connect(ui->solarisPlaybackPushButton, SIGNAL(clicked(bool)), this, SLOT(selectSolarisPlayback()));

    connect(ui->waveOutputLine, SIGNAL(textChanged(QString)), this, SLOT(enableApplyButton()));
    connect(ui->waveOutputButton, SIGNAL(clicked(bool)), this, SLOT(selectWaveOutput()));
    connect(ui->waveBFormatCheckBox, SIGNAL(stateChanged(int)), this, SLOT(enableApplyButton()));

    ui->backendListWidget->setCurrentRow(0);
    ui->tabWidget->setCurrentIndex(0);

    for(int i = 1;i < ui->backendListWidget->count();i++)
        ui->backendListWidget->setRowHidden(i, true);
    for(int i = 0;backendList[i].backend_name[0];i++)
    {
        QList<QListWidgetItem*> items = ui->backendListWidget->findItems(
            backendList[i].full_string, Qt::MatchFixedString
        );
        foreach(const QListWidgetItem *item, items)
            ui->backendListWidget->setItemHidden(item, false);
    }

    loadConfig(getDefaultConfigName());
}

MainWindow::~MainWindow()
{
    delete ui;
    delete mPeriodSizeValidator;
    delete mPeriodCountValidator;
    delete mSourceCountValidator;
    delete mEffectSlotValidator;
    delete mSourceSendValidator;
    delete mSampleRateValidator;
    delete mJackBufferValidator;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if(!mNeedsSave)
        event->accept();
    else
    {
        QMessageBox::StandardButton btn = QMessageBox::warning(this,
            tr("Apply changes?"), tr("Save changes before quitting?"),
            QMessageBox::Save | QMessageBox::No | QMessageBox::Cancel
        );
        if(btn == QMessageBox::Save)
            saveCurrentConfig();
        if(btn == QMessageBox::Cancel)
            event->ignore();
        else
            event->accept();
    }
}

void MainWindow::cancelCloseAction()
{
    mNeedsSave = false;
    close();
}


void MainWindow::showAboutPage()
{
    QMessageBox::information(this, tr("About"),
        tr("OpenAL Soft Configuration Utility.\nBuilt for OpenAL Soft library version ")+(ALSOFT_VERSION ".")
    );
}


QStringList MainWindow::collectHrtfs()
{
    QStringList ret;

    for(int i = 0;i < ui->hrtfFileList->count();i++)
    {
        QDir dir(ui->hrtfFileList->item(i)->text());
        QStringList fnames = dir.entryList(QDir::Files | QDir::Readable, QDir::Name);
        foreach(const QString &fname, fnames)
        {
            if(!fname.endsWith(".mhr", Qt::CaseInsensitive))
                continue;

            QString name = fname.left(fname.length()-4);
            if(!ret.contains(name))
                ret.push_back(name);
            else
            {
                size_t i = 1;
                do {
                    QString s = name+" #"+QString::number(i);
                    if(!ret.contains(s))
                    {
                        ret.push_back(s);
                        break;
                    }
                    ++i;
                } while(1);
            }
        }
    }

    if(ui->defaultHrtfPathsCheckBox->isChecked())
    {
        QStringList paths = getAllDataPaths("/openal/hrtf");
        foreach(const QString &name, paths)
        {
            QDir dir(name);
            QStringList fnames = dir.entryList(QDir::Files | QDir::Readable, QDir::Name);
            foreach(const QString &fname, fnames)
            {
                if(!fname.endsWith(".mhr", Qt::CaseInsensitive))
                    continue;

                QString name = fname.left(fname.length()-4);
                if(!ret.contains(name))
                    ret.push_back(name);
                else
                {
                    size_t i = 1;
                    do {
                        QString s = name+" #"+QString::number(i);
                        if(!ret.contains(s))
                        {
                            ret.push_back(s);
                            break;
                        }
                        ++i;
                    } while(1);
                }
            }
        }
    }
    return ret;
}


void MainWindow::loadConfigFromFile()
{
    QString fname = QFileDialog::getOpenFileName(this, tr("Select Files"));
    if(fname.isEmpty() == false)
        loadConfig(fname);
}

void MainWindow::loadConfig(const QString &fname)
{
    QSettings settings(fname, QSettings::IniFormat);

    QString sampletype = settings.value("sample-type").toString();
    ui->sampleFormatCombo->setCurrentIndex(0);
    if(sampletype.isEmpty() == false)
    {
        QString str = getNameFromValue(sampleTypeList, sampletype);
        if(!str.isEmpty())
        {
            int j = ui->sampleFormatCombo->findText(str);
            if(j > 0) ui->sampleFormatCombo->setCurrentIndex(j);
        }
    }

    QString channelconfig = settings.value("channels").toString();
    ui->channelConfigCombo->setCurrentIndex(0);
    if(channelconfig.isEmpty() == false)
    {
        QString str = getNameFromValue(speakerModeList, channelconfig);
        if(!str.isEmpty())
        {
            int j = ui->channelConfigCombo->findText(str);
            if(j > 0) ui->channelConfigCombo->setCurrentIndex(j);
        }
    }

    QString srate = settings.value("frequency").toString();
    if(srate.isEmpty())
        ui->sampleRateCombo->setCurrentIndex(0);
    else
    {
        ui->sampleRateCombo->lineEdit()->clear();
        ui->sampleRateCombo->lineEdit()->insert(srate);
    }

    ui->srcCountLineEdit->clear();
    ui->srcCountLineEdit->insert(settings.value("sources").toString());
    ui->effectSlotLineEdit->clear();
    ui->effectSlotLineEdit->insert(settings.value("slots").toString());
    ui->srcSendLineEdit->clear();
    ui->srcSendLineEdit->insert(settings.value("sends").toString());

    QString resampler = settings.value("resampler").toString().trimmed();
    ui->resamplerSlider->setValue(0);
    /* The "cubic" resampler is no longer supported. It's been replaced by
     * "sinc4". */
    if(resampler == "cubic")
        resampler = "sinc4";
    for(int i = 0;resamplerList[i].name[0];i++)
    {
        if(resampler == resamplerList[i].value)
        {
            ui->resamplerSlider->setValue(i);
            break;
        }
    }

    QString stereomode = settings.value("stereo-mode").toString().trimmed();
    ui->stereoModeCombo->setCurrentIndex(0);
    if(stereomode.isEmpty() == false)
    {
        QString str = getNameFromValue(stereoModeList, stereomode);
        if(!str.isEmpty())
        {
            int j = ui->stereoModeCombo->findText(str);
            if(j > 0) ui->stereoModeCombo->setCurrentIndex(j);
        }
    }

    int periodsize = settings.value("period_size").toInt();
    ui->periodSizeEdit->clear();
    if(periodsize >= 64)
    {
        ui->periodSizeEdit->insert(QString::number(periodsize));
        updatePeriodSizeSlider();
    }

    int periodcount = settings.value("periods").toInt();
    ui->periodCountEdit->clear();
    if(periodcount >= 2)
    {
        ui->periodCountEdit->insert(QString::number(periodcount));
        updatePeriodCountSlider();
    }

    QString stereopan = settings.value("stereo-panning").toString();
    ui->stereoPanningComboBox->setCurrentIndex(0);
    if(stereopan.isEmpty() == false)
    {
        QString str = getNameFromValue(stereoPanList, stereopan);
        if(!str.isEmpty())
        {
            int j = ui->stereoPanningComboBox->findText(str);
            if(j > 0) ui->stereoPanningComboBox->setCurrentIndex(j);
        }
    }

    QString ambiformat = settings.value("ambi-format").toString();
    ui->ambiFormatComboBox->setCurrentIndex(0);
    if(ambiformat.isEmpty() == false)
    {
        QString str = getNameFromValue(ambiFormatList, ambiformat);
        if(!str.isEmpty())
        {
            int j = ui->ambiFormatComboBox->findText(str);
            if(j > 0) ui->ambiFormatComboBox->setCurrentIndex(j);
        }
    }

    bool hqmode = settings.value("decoder/hq-mode", false).toBool();
    ui->decoderHQModeCheckBox->setChecked(hqmode);
    bool distcomp = settings.value("decoder/distance-comp", true).toBool();
    ui->decoderDistCompCheckBox->setChecked(distcomp);
    ui->decoderDistCompCheckBox->setEnabled(hqmode);

    ui->decoderQuadLineEdit->setText(settings.value("decoder/quad").toString());
    ui->decoder51LineEdit->setText(settings.value("decoder/surround51").toString());
    ui->decoder51RearLineEdit->setText(settings.value("decoder/surround51rear").toString());
    ui->decoder61LineEdit->setText(settings.value("decoder/surround61").toString());
    ui->decoder71LineEdit->setText(settings.value("decoder/surround71").toString());

    QStringList disabledCpuExts = settings.value("disable-cpu-exts").toStringList();
    if(disabledCpuExts.size() == 1)
        disabledCpuExts = disabledCpuExts[0].split(QChar(','));
    std::transform(disabledCpuExts.begin(), disabledCpuExts.end(),
                   disabledCpuExts.begin(), std::mem_fun_ref(&QString::trimmed));
    ui->enableSSECheckBox->setChecked(!disabledCpuExts.contains("sse", Qt::CaseInsensitive));
    ui->enableSSE2CheckBox->setChecked(!disabledCpuExts.contains("sse2", Qt::CaseInsensitive));
    ui->enableSSE3CheckBox->setChecked(!disabledCpuExts.contains("sse3", Qt::CaseInsensitive));
    ui->enableSSE41CheckBox->setChecked(!disabledCpuExts.contains("sse4.1", Qt::CaseInsensitive));
    ui->enableNeonCheckBox->setChecked(!disabledCpuExts.contains("neon", Qt::CaseInsensitive));

    QStringList hrtf_paths = settings.value("hrtf-paths").toStringList();
    if(hrtf_paths.size() == 1)
        hrtf_paths = hrtf_paths[0].split(QChar(','));
    std::transform(hrtf_paths.begin(), hrtf_paths.end(),
                   hrtf_paths.begin(), std::mem_fun_ref(&QString::trimmed));
    if(!hrtf_paths.empty() && !hrtf_paths.back().isEmpty())
        ui->defaultHrtfPathsCheckBox->setCheckState(Qt::Unchecked);
    else
    {
        hrtf_paths.removeAll(QString());
        ui->defaultHrtfPathsCheckBox->setCheckState(Qt::Checked);
    }
    hrtf_paths.removeDuplicates();
    ui->hrtfFileList->clear();
    ui->hrtfFileList->addItems(hrtf_paths);
    updateHrtfRemoveButton();

    QString hrtfstate = settings.value("hrtf").toString().toLower();
    if(hrtfstate == "true")
        ui->hrtfStateComboBox->setCurrentIndex(1);
    else if(hrtfstate == "false")
        ui->hrtfStateComboBox->setCurrentIndex(2);
    else
        ui->hrtfStateComboBox->setCurrentIndex(0);

    ui->preferredHrtfComboBox->clear();
    ui->preferredHrtfComboBox->addItem("- Any -");
    if(ui->defaultHrtfPathsCheckBox->isChecked())
    {
        QStringList hrtfs = collectHrtfs();
        foreach(const QString &name, hrtfs)
            ui->preferredHrtfComboBox->addItem(name);
    }

    QString defaulthrtf = settings.value("default-hrtf").toString();
    ui->preferredHrtfComboBox->setCurrentIndex(0);
    if(defaulthrtf.isEmpty() == false)
    {
        int i = ui->preferredHrtfComboBox->findText(defaulthrtf);
        if(i > 0)
            ui->preferredHrtfComboBox->setCurrentIndex(i);
        else
        {
            i = ui->preferredHrtfComboBox->count();
            ui->preferredHrtfComboBox->addItem(defaulthrtf);
            ui->preferredHrtfComboBox->setCurrentIndex(i);
        }
    }
    ui->preferredHrtfComboBox->adjustSize();

    ui->enabledBackendList->clear();
    ui->disabledBackendList->clear();
    QStringList drivers = settings.value("drivers").toStringList();
    if(drivers.size() == 0)
        ui->backendCheckBox->setChecked(true);
    else
    {
        if(drivers.size() == 1)
            drivers = drivers[0].split(QChar(','));
        std::transform(drivers.begin(), drivers.end(),
                       drivers.begin(), std::mem_fun_ref(&QString::trimmed));

        bool lastWasEmpty = false;
        foreach(const QString &backend, drivers)
        {
            lastWasEmpty = backend.isEmpty();
            if(lastWasEmpty) continue;

            if(!backend.startsWith(QChar('-')))
                for(int j = 0;backendList[j].backend_name[0];j++)
                {
                    if(backend == backendList[j].backend_name)
                    {
                        ui->enabledBackendList->addItem(backendList[j].full_string);
                        break;
                    }
                }
            else if(backend.size() > 1)
            {
                QStringRef backendref = backend.rightRef(backend.size()-1);
                for(int j = 0;backendList[j].backend_name[0];j++)
                {
                    if(backendref == backendList[j].backend_name)
                    {
                        ui->disabledBackendList->addItem(backendList[j].full_string);
                        break;
                    }
                }
            }
        }
        ui->backendCheckBox->setChecked(lastWasEmpty);
    }

    QString defaultreverb = settings.value("default-reverb").toString().toLower();
    ui->defaultReverbComboBox->setCurrentIndex(0);
    if(defaultreverb.isEmpty() == false)
    {
        for(int i = 0;i < ui->defaultReverbComboBox->count();i++)
        {
            if(defaultreverb.compare(ui->defaultReverbComboBox->itemText(i).toLower()) == 0)
            {
                ui->defaultReverbComboBox->setCurrentIndex(i);
                break;
            }
        }
    }

    ui->emulateEaxCheckBox->setChecked(settings.value("reverb/emulate-eax", false).toBool());

    QStringList excludefx = settings.value("excludefx").toStringList();
    if(excludefx.size() == 1)
        excludefx = excludefx[0].split(QChar(','));
    std::transform(excludefx.begin(), excludefx.end(),
                   excludefx.begin(), std::mem_fun_ref(&QString::trimmed));
    ui->enableEaxReverbCheck->setChecked(!excludefx.contains("eaxreverb", Qt::CaseInsensitive));
    ui->enableStdReverbCheck->setChecked(!excludefx.contains("reverb", Qt::CaseInsensitive));
    ui->enableChorusCheck->setChecked(!excludefx.contains("chorus", Qt::CaseInsensitive));
    ui->enableCompressorCheck->setChecked(!excludefx.contains("compressor", Qt::CaseInsensitive));
    ui->enableDistortionCheck->setChecked(!excludefx.contains("distortion", Qt::CaseInsensitive));
    ui->enableEchoCheck->setChecked(!excludefx.contains("echo", Qt::CaseInsensitive));
    ui->enableEqualizerCheck->setChecked(!excludefx.contains("equalizer", Qt::CaseInsensitive));
    ui->enableFlangerCheck->setChecked(!excludefx.contains("flanger", Qt::CaseInsensitive));
    ui->enableModulatorCheck->setChecked(!excludefx.contains("modulator", Qt::CaseInsensitive));
    ui->enableDedicatedCheck->setChecked(!excludefx.contains("dedicated", Qt::CaseInsensitive));

    ui->pulseAutospawnCheckBox->setChecked(settings.value("pulse/spawn-server", true).toBool());
    ui->pulseAllowMovesCheckBox->setChecked(settings.value("pulse/allow-moves", false).toBool());
    ui->pulseFixRateCheckBox->setChecked(settings.value("pulse/fix-rate", false).toBool());

    ui->jackAutospawnCheckBox->setChecked(settings.value("jack/spawn-server", false).toBool());
    ui->jackBufferSizeLine->setText(settings.value("jack/buffer-size", QString()).toString());
    updateJackBufferSizeSlider();

    ui->alsaDefaultDeviceLine->setText(settings.value("alsa/device", QString()).toString());
    ui->alsaDefaultCaptureLine->setText(settings.value("alsa/capture", QString()).toString());
    ui->alsaResamplerCheckBox->setChecked(settings.value("alsa/allow-resampler", false).toBool());
    ui->alsaMmapCheckBox->setChecked(settings.value("alsa/mmap", true).toBool());

    ui->ossDefaultDeviceLine->setText(settings.value("oss/device", QString()).toString());
    ui->ossDefaultCaptureLine->setText(settings.value("oss/capture", QString()).toString());

    ui->solarisDefaultDeviceLine->setText(settings.value("solaris/device", QString()).toString());

    ui->waveOutputLine->setText(settings.value("wave/file", QString()).toString());
    ui->waveBFormatCheckBox->setChecked(settings.value("wave/bformat", false).toBool());

    ui->applyButton->setEnabled(false);
    ui->closeCancelButton->setText(tr("Close"));
    mNeedsSave = false;
}

void MainWindow::saveCurrentConfig()
{
    saveConfig(getDefaultConfigName());
    ui->applyButton->setEnabled(false);
    ui->closeCancelButton->setText(tr("Close"));
    mNeedsSave = false;
    QMessageBox::information(this, tr("Information"),
                             tr("Applications using OpenAL need to be restarted for changes to take effect."));
}

void MainWindow::saveConfigAsFile()
{
    QString fname = QFileDialog::getOpenFileName(this, tr("Select Files"));
    if(fname.isEmpty() == false)
    {
        saveConfig(fname);
        ui->applyButton->setEnabled(false);
        mNeedsSave = false;
    }
}

void MainWindow::saveConfig(const QString &fname) const
{
    QSettings settings(fname, QSettings::IniFormat);

    /* HACK: Compound any stringlist values into a comma-separated string. */
    QStringList allkeys = settings.allKeys();
    foreach(const QString &key, allkeys)
    {
        QStringList vals = settings.value(key).toStringList();
        if(vals.size() > 1)
            settings.setValue(key, vals.join(QChar(',')));
    }

    settings.setValue("sample-type", getValueFromName(sampleTypeList, ui->sampleFormatCombo->currentText()));
    settings.setValue("channels", getValueFromName(speakerModeList, ui->channelConfigCombo->currentText()));

    uint rate = ui->sampleRateCombo->currentText().toUInt();
    if(!(rate > 0))
        settings.setValue("frequency", QString());
    else
        settings.setValue("frequency", rate);

    settings.setValue("period_size", ui->periodSizeEdit->text());
    settings.setValue("periods", ui->periodCountEdit->text());

    settings.setValue("sources", ui->srcCountLineEdit->text());
    settings.setValue("slots", ui->effectSlotLineEdit->text());

    settings.setValue("resampler", resamplerList[ui->resamplerSlider->value()].value);

    settings.setValue("stereo-mode", getValueFromName(stereoModeList, ui->stereoModeCombo->currentText()));
    settings.setValue("stereo-panning", getValueFromName(stereoPanList, ui->stereoPanningComboBox->currentText()));
    settings.setValue("ambi-format", getValueFromName(ambiFormatList, ui->ambiFormatComboBox->currentText()));

    settings.setValue("decoder/hq-mode",
        ui->decoderHQModeCheckBox->isChecked() ? QString("true") : QString(/*"false"*/)
    );
    settings.setValue("decoder/distance-comp",
        ui->decoderDistCompCheckBox->isChecked() ? QString(/*"true"*/) : QString("false")
    );

    settings.setValue("decoder/quad", ui->decoderQuadLineEdit->text());
    settings.setValue("decoder/surround51", ui->decoder51LineEdit->text());
    settings.setValue("decoder/surround51rear", ui->decoder51RearLineEdit->text());
    settings.setValue("decoder/surround61", ui->decoder61LineEdit->text());
    settings.setValue("decoder/surround71", ui->decoder71LineEdit->text());

    QStringList strlist;
    if(!ui->enableSSECheckBox->isChecked())
        strlist.append("sse");
    if(!ui->enableSSE2CheckBox->isChecked())
        strlist.append("sse2");
    if(!ui->enableSSE3CheckBox->isChecked())
        strlist.append("sse3");
    if(!ui->enableSSE41CheckBox->isChecked())
        strlist.append("sse4.1");
    if(!ui->enableNeonCheckBox->isChecked())
        strlist.append("neon");
    settings.setValue("disable-cpu-exts", strlist.join(QChar(',')));

    if(ui->hrtfStateComboBox->currentIndex() == 1)
        settings.setValue("hrtf", "true");
    else if(ui->hrtfStateComboBox->currentIndex() == 2)
        settings.setValue("hrtf", "false");
    else
        settings.setValue("hrtf", QString());

    if(ui->preferredHrtfComboBox->currentIndex() == 0)
        settings.setValue("default-hrtf", QString());
    else
    {
        QString str = ui->preferredHrtfComboBox->currentText();
        settings.setValue("default-hrtf", str);
    }

    strlist.clear();
    for(int i = 0;i < ui->hrtfFileList->count();i++)
        strlist.append(ui->hrtfFileList->item(i)->text());
    if(!strlist.empty() && ui->defaultHrtfPathsCheckBox->isChecked())
        strlist.append(QString());
    settings.setValue("hrtf-paths", strlist.join(QChar(',')));

    strlist.clear();
    for(int i = 0;i < ui->enabledBackendList->count();i++)
    {
        QString label = ui->enabledBackendList->item(i)->text();
        for(int j = 0;backendList[j].backend_name[0];j++)
        {
            if(label == backendList[j].full_string)
            {
                strlist.append(backendList[j].backend_name);
                break;
            }
        }
    }
    for(int i = 0;i < ui->disabledBackendList->count();i++)
    {
        QString label = ui->disabledBackendList->item(i)->text();
        for(int j = 0;backendList[j].backend_name[0];j++)
        {
            if(label == backendList[j].full_string)
            {
                strlist.append(QChar('-')+QString(backendList[j].backend_name));
                break;
            }
        }
    }
    if(strlist.size() == 0 && !ui->backendCheckBox->isChecked())
        strlist.append("-all");
    else if(ui->backendCheckBox->isChecked())
        strlist.append(QString());
    settings.setValue("drivers", strlist.join(QChar(',')));

    // TODO: Remove check when we can properly match global values.
    if(ui->defaultReverbComboBox->currentIndex() == 0)
        settings.setValue("default-reverb", QString());
    else
    {
        QString str = ui->defaultReverbComboBox->currentText().toLower();
        settings.setValue("default-reverb", str);
    }

    if(ui->emulateEaxCheckBox->isChecked())
        settings.setValue("reverb/emulate-eax", "true");
    else
        settings.remove("reverb/emulate-eax"/*, "false"*/);

    strlist.clear();
    if(!ui->enableEaxReverbCheck->isChecked())
        strlist.append("eaxreverb");
    if(!ui->enableStdReverbCheck->isChecked())
        strlist.append("reverb");
    if(!ui->enableChorusCheck->isChecked())
        strlist.append("chorus");
    if(!ui->enableDistortionCheck->isChecked())
        strlist.append("distortion");
    if(!ui->enableCompressorCheck->isChecked())
        strlist.append("compressor");
    if(!ui->enableEchoCheck->isChecked())
        strlist.append("echo");
    if(!ui->enableEqualizerCheck->isChecked())
        strlist.append("equalizer");
    if(!ui->enableFlangerCheck->isChecked())
        strlist.append("flanger");
    if(!ui->enableModulatorCheck->isChecked())
        strlist.append("modulator");
    if(!ui->enableDedicatedCheck->isChecked())
        strlist.append("dedicated");
    settings.setValue("excludefx", strlist.join(QChar(',')));

    settings.setValue("pulse/spawn-server",
        ui->pulseAutospawnCheckBox->isChecked() ? QString(/*"true"*/) : QString("false")
    );
    settings.setValue("pulse/allow-moves",
        ui->pulseAllowMovesCheckBox->isChecked() ? QString("true") : QString(/*"false"*/)
    );
    settings.setValue("pulse/fix-rate",
        ui->pulseFixRateCheckBox->isChecked() ? QString("true") : QString(/*"false"*/)
    );

    settings.setValue("jack/spawn-server",
        ui->jackAutospawnCheckBox->isChecked() ? QString("true") : QString(/*"false"*/)
    );
    settings.setValue("jack/buffer-size", ui->jackBufferSizeLine->text());

    settings.setValue("alsa/device", ui->alsaDefaultDeviceLine->text());
    settings.setValue("alsa/capture", ui->alsaDefaultCaptureLine->text());
    settings.setValue("alsa/allow-resampler",
        ui->alsaResamplerCheckBox->isChecked() ? QString("true") : QString(/*"false"*/)
    );
    settings.setValue("alsa/mmap",
        ui->alsaMmapCheckBox->isChecked() ? QString(/*"true"*/) : QString("false")
    );

    settings.setValue("oss/device", ui->ossDefaultDeviceLine->text());
    settings.setValue("oss/capture", ui->ossDefaultCaptureLine->text());

    settings.setValue("solaris/device", ui->solarisDefaultDeviceLine->text());

    settings.setValue("wave/file", ui->waveOutputLine->text());
    settings.setValue("wave/bformat",
        ui->waveBFormatCheckBox->isChecked() ? QString("true") : QString(/*"false"*/)
    );

    /* Remove empty keys
     * FIXME: Should only remove keys whose value matches the globally-specified value.
     */
    allkeys = settings.allKeys();
    foreach(const QString &key, allkeys)
    {
        QString str = settings.value(key).toString();
        if(str == QString())
            settings.remove(key);
    }
}


void MainWindow::enableApplyButton()
{
    if(!mNeedsSave)
        ui->applyButton->setEnabled(true);
    mNeedsSave = true;
    ui->closeCancelButton->setText(tr("Cancel"));
}


void MainWindow::updateResamplerLabel(int num)
{
    ui->resamplerLabel->setText(resamplerList[num].name);
    enableApplyButton();
}


void MainWindow::updatePeriodSizeEdit(int size)
{
    ui->periodSizeEdit->clear();
    if(size >= 64)
    {
        size = (size+32)&~0x3f;
        ui->periodSizeEdit->insert(QString::number(size));
    }
    enableApplyButton();
}

void MainWindow::updatePeriodSizeSlider()
{
    int pos = ui->periodSizeEdit->text().toInt();
    if(pos >= 64)
    {
        if(pos > 8192)
            pos = 8192;
        ui->periodSizeSlider->setSliderPosition(pos);
    }
    enableApplyButton();
}

void MainWindow::updatePeriodCountEdit(int count)
{
    ui->periodCountEdit->clear();
    if(count >= 2)
        ui->periodCountEdit->insert(QString::number(count));
    enableApplyButton();
}

void MainWindow::updatePeriodCountSlider()
{
    int pos = ui->periodCountEdit->text().toInt();
    if(pos < 2)
        pos = 0;
    else if(pos > 16)
        pos = 16;
    ui->periodCountSlider->setSliderPosition(pos);
    enableApplyButton();
}


void MainWindow::toggleHqState(int state)
{
    ui->decoderDistCompCheckBox->setEnabled(state);
    enableApplyButton();
}

void MainWindow::selectQuadDecoderFile()
{ selectDecoderFile(ui->decoderQuadLineEdit, "Select Quadrophonic Decoder");}
void MainWindow::select51DecoderFile()
{ selectDecoderFile(ui->decoder51LineEdit, "Select 5.1 Surround (Side) Decoder");}
void MainWindow::select51RearDecoderFile()
{ selectDecoderFile(ui->decoder51RearLineEdit, "Select 5.1 Surround (Rear) Decoder");}
void MainWindow::select61DecoderFile()
{ selectDecoderFile(ui->decoder61LineEdit, "Select 6.1 Surround Decoder");}
void MainWindow::select71DecoderFile()
{ selectDecoderFile(ui->decoder71LineEdit, "Select 7.1 Surround Decoder");}
void MainWindow::selectDecoderFile(QLineEdit *line, const char *caption)
{
    QString dir = line->text();
    if(dir.isEmpty() || QDir::isRelativePath(dir))
    {
        QStringList paths = getAllDataPaths("/openal/presets");
        while(!paths.isEmpty())
        {
            if(QDir(paths.last()).exists())
            {
                dir = paths.last();
                break;
            }
            paths.removeLast();
        }
    }
    QString fname = QFileDialog::getOpenFileName(this, tr(caption),
        dir, tr("AmbDec Files (*.ambdec);;All Files (*.*)")
    );
    if(!fname.isEmpty())
    {
        line->setText(fname);
        enableApplyButton();
    }
}


void MainWindow::updateJackBufferSizeEdit(int size)
{
    ui->jackBufferSizeLine->clear();
    if(size > 0)
        ui->jackBufferSizeLine->insert(QString::number(1<<size));
    enableApplyButton();
}

void MainWindow::updateJackBufferSizeSlider()
{
    int value = ui->jackBufferSizeLine->text().toInt();
    int pos = (int)floor(log2(value) + 0.5);
    ui->jackBufferSizeSlider->setSliderPosition(pos);
    enableApplyButton();
}


void MainWindow::addHrtfFile()
{
    QString path = QFileDialog::getExistingDirectory(this, tr("Select HRTF Path"));
    if(path.isEmpty() == false && !getAllDataPaths("/openal/hrtf").contains(path))
    {
        ui->hrtfFileList->addItem(path);
        enableApplyButton();
    }
}

void MainWindow::removeHrtfFile()
{
    QList<QListWidgetItem*> selected = ui->hrtfFileList->selectedItems();
    if(!selected.isEmpty())
    {
        foreach(QListWidgetItem *item, selected)
            delete item;
        enableApplyButton();
    }
}

void MainWindow::updateHrtfRemoveButton()
{
    ui->hrtfRemoveButton->setEnabled(ui->hrtfFileList->selectedItems().size() != 0);
}

void MainWindow::showEnabledBackendMenu(QPoint pt)
{
    QMap<QAction*,QString> actionMap;

    pt = ui->enabledBackendList->mapToGlobal(pt);

    QMenu ctxmenu;
    QAction *removeAction = ctxmenu.addAction(QIcon::fromTheme("list-remove"), "Remove");
    if(ui->enabledBackendList->selectedItems().size() == 0)
        removeAction->setEnabled(false);
    ctxmenu.addSeparator();
    for(size_t i = 0;backendList[i].backend_name[0];i++)
    {
        QString backend = backendList[i].full_string;
        QAction *action = ctxmenu.addAction(QString("Add ")+backend);
        actionMap[action] = backend;
        if(ui->enabledBackendList->findItems(backend, Qt::MatchFixedString).size() != 0 ||
           ui->disabledBackendList->findItems(backend, Qt::MatchFixedString).size() != 0)
            action->setEnabled(false);
    }

    QAction *gotAction = ctxmenu.exec(pt);
    if(gotAction == removeAction)
    {
        QList<QListWidgetItem*> selected = ui->enabledBackendList->selectedItems();
        foreach(QListWidgetItem *item, selected)
            delete item;
        enableApplyButton();
    }
    else if(gotAction != NULL)
    {
        QMap<QAction*,QString>::const_iterator iter = actionMap.find(gotAction);
        if(iter != actionMap.end())
            ui->enabledBackendList->addItem(iter.value());
        enableApplyButton();
    }
}

void MainWindow::showDisabledBackendMenu(QPoint pt)
{
    QMap<QAction*,QString> actionMap;

    pt = ui->disabledBackendList->mapToGlobal(pt);

    QMenu ctxmenu;
    QAction *removeAction = ctxmenu.addAction(QIcon::fromTheme("list-remove"), "Remove");
    if(ui->disabledBackendList->selectedItems().size() == 0)
        removeAction->setEnabled(false);
    ctxmenu.addSeparator();
    for(size_t i = 0;backendList[i].backend_name[0];i++)
    {
        QString backend = backendList[i].full_string;
        QAction *action = ctxmenu.addAction(QString("Add ")+backend);
        actionMap[action] = backend;
        if(ui->disabledBackendList->findItems(backend, Qt::MatchFixedString).size() != 0 ||
           ui->enabledBackendList->findItems(backend, Qt::MatchFixedString).size() != 0)
            action->setEnabled(false);
    }

    QAction *gotAction = ctxmenu.exec(pt);
    if(gotAction == removeAction)
    {
        QList<QListWidgetItem*> selected = ui->disabledBackendList->selectedItems();
        foreach(QListWidgetItem *item, selected)
            delete item;
        enableApplyButton();
    }
    else if(gotAction != NULL)
    {
        QMap<QAction*,QString>::const_iterator iter = actionMap.find(gotAction);
        if(iter != actionMap.end())
            ui->disabledBackendList->addItem(iter.value());
        enableApplyButton();
    }
}

void MainWindow::selectOSSPlayback()
{
    QString current = ui->ossDefaultDeviceLine->text();
    if(current.isEmpty()) current = ui->ossDefaultDeviceLine->placeholderText();
    QString fname = QFileDialog::getOpenFileName(this, tr("Select Playback Device"), current);
    if(!fname.isEmpty())
    {
        ui->ossDefaultDeviceLine->setText(fname);
        enableApplyButton();
    }
}

void MainWindow::selectOSSCapture()
{
    QString current = ui->ossDefaultCaptureLine->text();
    if(current.isEmpty()) current = ui->ossDefaultCaptureLine->placeholderText();
    QString fname = QFileDialog::getOpenFileName(this, tr("Select Capture Device"), current);
    if(!fname.isEmpty())
    {
        ui->ossDefaultCaptureLine->setText(fname);
        enableApplyButton();
    }
}

void MainWindow::selectSolarisPlayback()
{
    QString current = ui->solarisDefaultDeviceLine->text();
    if(current.isEmpty()) current = ui->solarisDefaultDeviceLine->placeholderText();
    QString fname = QFileDialog::getOpenFileName(this, tr("Select Playback Device"), current);
    if(!fname.isEmpty())
    {
        ui->solarisDefaultDeviceLine->setText(fname);
        enableApplyButton();
    }
}

void MainWindow::selectWaveOutput()
{
    QString fname = QFileDialog::getSaveFileName(this, tr("Select Wave File Output"),
        ui->waveOutputLine->text(), tr("Wave Files (*.wav *.amb);;All Files (*.*)")
    );
    if(!fname.isEmpty())
    {
        ui->waveOutputLine->setText(fname);
        enableApplyButton();
    }
}
