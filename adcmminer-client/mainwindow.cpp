//#include <QtConcurrent>

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "calibration.h"
#include "datadelegate.h"

#include "piechartwidget.h"



MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_client{nullptr}
{
    ui->setupUi(this);
    m_settings = new Settings("settings.ini", this);
    m_path = m_settings->path();

    QMenu *fileMenu = new QMenu(tr("&File"), this);
    QAction *openAction = fileMenu->addAction(tr("&Open..."), this, &MainWindow::openFile);
    openAction->setShortcuts(QKeySequence::Open);
    QAction *quitAction = fileMenu->addAction(tr("E&xit"));
    quitAction->setShortcuts(QKeySequence::Quit);
    menuBar()->addMenu(fileMenu);


    connect(quitAction, &QAction::triggered, this, &MainWindow::close);


    m_mainWidget = new QWidget(this);
    m_mainLayout = new QGridLayout(m_mainWidget);
    setCentralWidget(m_mainWidget);

    QSplitter *splitterWidget = new QSplitter(m_mainWidget);
    m_mainLayout->addWidget(splitterWidget);

    m_widgetLeft = new QWidget(splitterWidget);

    m_gLleft = new QGridLayout(m_widgetLeft);
    m_pushButtonStartStop = new QPushButton(tr("Start"), m_widgetLeft);
    m_pushButtonStartStop->setCheckable(true);
    m_gLleft->addWidget(m_pushButtonStartStop);
    m_pushButtonReset = new QPushButton(tr("Reset"), m_widgetLeft);
    m_gLleft->addWidget(m_pushButtonReset);
    m_pushButtonConnect = new QPushButton(tr("Connect"), m_widgetLeft);
    m_gLleft->addWidget(m_pushButtonConnect);
    m_pushButtonTest = new QPushButton(tr("Test"), m_widgetLeft);
    m_gLleft->addWidget(m_pushButtonTest);
    QWidget *timeWidget = new QWidget(m_widgetLeft);
    QVBoxLayout *timeLayout = new QVBoxLayout(timeWidget);
    m_timeLabel = new QLabel(tr("Time, s"), m_widgetLeft);
    m_timeLabel->setAlignment(Qt::AlignCenter);
    timeLayout->addWidget(m_timeLabel);
    m_timeLineEdit = new QLineEdit(m_widgetLeft);
    m_timeLineEdit->setReadOnly(true);
    m_timeLineEdit->setAlignment(Qt::AlignCenter);
    timeLayout->addWidget(m_timeLineEdit);
    timeLayout->setAlignment(Qt::AlignBottom);
    m_gLleft->setRowStretch(2, 1);
    m_gLleft->addWidget(timeWidget);

    m_widgetLeft->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);

    m_widgetRight = new QWidget(splitterWidget);
    m_gLright = new QGridLayout(m_widgetRight);

    m_tabWidget = new QTabWidget(m_widgetRight);

    m_page_1 = new QWidget;
    m_tabWidget->addTab(m_page_1, tr("Time"));
    m_page_2 = new QWidget;
    m_tabWidget->addTab(m_page_2, tr("Energy"));
    m_page_3 = new QWidget;
    m_tabWidget->addTab(m_page_3, tr("Processing"));
    m_gLright->addWidget(m_tabWidget);

    splitterWidget->addWidget(m_widgetLeft);
    splitterWidget->addWidget(m_widgetRight);


//    m_dialog = nullptr;

//    m_statusMessageLabel = new QLabel(QString("<span style='color: yellow;'>%1</span>").arg(QChar(0x003F)));
//    m_statusMessageLabel = new QLabel(QString("<span></span>"));
    m_statusMessageLabel = new QLabel;
    m_statusMessageLabel->setTextFormat(Qt::RichText);
    m_statusMessageLabel->setText("<span></span>");
    statusBar()->addWidget(m_statusMessageLabel);
    m_controller = new Controller(m_path);

    m_serverStatusMessageLabel = new QLabel;
    m_serverStatusMessageLabel->setTextFormat(Qt::RichText);
    m_serverStatusMessageLabel->setText(QString("<span>%1</span>").arg(QString::fromUtf8(u8"\U0001F534")));
    statusBar()->addPermanentWidget(m_serverStatusMessageLabel);


    m_pushButtonStartStop->setStyleSheet(
        "QPushButton {"
        "   color: white;"
        "}"
        "QPushButton:checked {"
        "   background-color: red;"
        "}"
        "QPushButton:checked:hover {"
        "   background-color: coral;"
        "}"
        "QPushButton:!checked {"
        "   background-color: green;"
        "}"
        "QPushButton:!checked:hover {"
        "   background-color: lime;"
        "}"
    );


    connect(m_controller, &Controller::handleResultsReadyCheck, m_statusMessageLabel, &QLabel::setText);
    connect(m_controller, &Controller::handleResultsTimeCorrectedByAlpha, this, &MainWindow::newDataTimeCorrectedByAlpha);
    connect(m_controller, &Controller::handleResultsEnergyByAlpha, this, &MainWindow::newDataEnergyByAlpha);
    connect(m_controller, &Controller::handleResultsProcessing, this, &MainWindow::newDataProcessing);
    connect(m_pushButtonStartStop, &QPushButton::toggled, m_controller, &Controller::operateTimer);
    connect(m_pushButtonStartStop, &QPushButton::toggled, [this](bool checked){
        m_pushButtonStartStop->setText(checked ? tr("Stop") : tr("Start"));
    });
    connect(m_pushButtonReset, &QPushButton::clicked, m_controller, &Controller::operateReset);
    connect(m_pushButtonConnect, &QPushButton::clicked, this, [&](){
        const QString serverName = "ADCMMiner Server1";
        connectToServer(serverName);
    });

    connect(m_pushButtonTest, &QPushButton::clicked, this, [&](){
        TaskData task;
        task.deadline = QDateTime::currentDateTimeUtc();
        task.id = 1;
        task.title = "Task1";
        task.isCompleted = false;
        m_client->sendTask(task);
    });



    setupTimeCorrectedByAlpha();
    setupEnergyByAlpha();
    setupProcessing();


}

MainWindow::~MainWindow()
{
    m_settings->writeSettings();
    if (m_client) {
        m_client->disconnectFromServer();
    }
    delete ui;
}

void MainWindow::newDataTimeCorrectedByAlpha(const QMap<QString, QList<QPointF>> &data, const QMap<QString, QStringList> &text)
{
    if (data.size() != m_histChartWidgetsTimeCorrectedByAlpha.size()) {
        // TODO ?
    }
    auto j{0};
    for (auto i = data.cbegin(), end = data.cend(); i != end; ++i) {
        if (j < m_histChartWidgetsTimeCorrectedByAlpha.size()) {
            m_histChartWidgetsTimeCorrectedByAlpha.at(j)->setHeader(text[i.key()]);
            m_histChartWidgetsTimeCorrectedByAlpha.at(j)->setData(i.key(), i.value());
        }
        j++;
    }
}

void MainWindow::newDataEnergyByAlpha(const QMap<QString, QList<QPointF>> &data, const QMap<QString, QStringList> &text)
{
    if (data.size() != m_histChartWidgetsEnergyByAlpha.size()) {
        // TODO ?
    }
    auto j{0};
    for (auto i = data.cbegin(), end = data.cend(); i != end; ++i) {
        if (j < m_histChartWidgetsEnergyByAlpha.size()) {
            m_histChartWidgetsEnergyByAlpha.at(j)->setHeader(text[i.key()]);
            m_histChartWidgetsEnergyByAlpha.at(j)->setData(i.key(), i.value());
        }
        j++;
    }
}

void MainWindow::newDataProcessing(const QMap<QString, double> &data, double t, const QMap<QString, double> &countersA, const QMap<QString, double> &countersG)
{
    m_timeLineEdit->setText(QString::number(t, 'f', 2));
    m_pieChartWidget->setData(data);
    m_barChartWidgetCountersAlpha->setData(countersA);
    m_barChartWidgetCountersGamma->setData(countersG);
}

void MainWindow::setupTimeCorrectedByAlpha()
{
    m_page_1->setLayout(new QGridLayout());
    static_cast<QGridLayout*>(m_page_1->layout())->setSpacing(0);
    static_cast<QGridLayout*>(m_page_1->layout())->setContentsMargins(0, 0, 0, 0);
    m_histChartWidgetsTimeCorrectedByAlpha.resize(AppConstants::MAX_ALPHA_NUMBER);
    auto cd{static_cast<qsizetype>(std::ceil(std::sqrt(AppConstants::MAX_ALPHA_NUMBER)))};
    auto index{0};
    for (auto ir{0}; ir < cd; ++ir)
    {
        for (auto ic{0}; ic < cd; ++ic)
        {
            if (index < m_histChartWidgetsTimeCorrectedByAlpha.size())
            {
                m_histChartWidgetsTimeCorrectedByAlpha[index] = new HistChartWidget(-100.0, 100.0);
                static_cast<QGridLayout*>(m_page_1->layout())->addWidget(m_histChartWidgetsTimeCorrectedByAlpha.at(index), ir, ic);
                index++;
            }
        }
    }
    for (auto i{0}; i < cd; ++i)
    {
        static_cast<QGridLayout*>(m_page_1->layout())->setRowStretch(i, 1);
        static_cast<QGridLayout*>(m_page_1->layout())->setColumnStretch(i, 1);
    }
}

void MainWindow::setupEnergyByAlpha()
{
    if (m_page_2->layout()) {
        delete m_page_2->layout();
    }
    for (auto i{0}; i < m_histChartWidgetsEnergyByAlpha.size(); ++i) {
        m_histChartWidgetsEnergyByAlpha.at(i)->deleteLater();
    }
    m_page_2->setLayout(new QGridLayout());
    static_cast<QGridLayout*>(m_page_2->layout())->setSpacing(0);
    static_cast<QGridLayout*>(m_page_2->layout())->setContentsMargins(0, 0, 0, 0);
    m_histChartWidgetsEnergyByAlpha.resize(AppConstants::MAX_ALPHA_NUMBER);
    auto cd{static_cast<qsizetype>(std::ceil(std::sqrt(AppConstants::MAX_ALPHA_NUMBER)))};
    auto index{0};
    for (auto ir{0}; ir < cd; ++ir) {
        for (auto ic{0}; ic < cd; ++ic) {
            if (index < m_histChartWidgetsEnergyByAlpha.size()) {
                m_histChartWidgetsEnergyByAlpha[index] = new HistChartWidget(0.0, 8'000.0);
                static_cast<QGridLayout*>(m_page_2->layout())->addWidget(m_histChartWidgetsEnergyByAlpha.at(index), ir, ic);
                index++;
            }
        }
    }
    for (auto i{0}; i < cd; ++i) {
        static_cast<QGridLayout*>(m_page_2->layout())->setRowStretch(i, 1);
        static_cast<QGridLayout*>(m_page_2->layout())->setColumnStretch(i, 1);
    }
}

void MainWindow::setupProcessing()
{
    QGridLayout *gl = new QGridLayout(m_page_3);
    m_page_3->setLayout(gl);
    m_pieChartWidget = new PieChartWidget(m_page_3);
    gl->addWidget(m_pieChartWidget, 0, 0);
    m_barChartWidgetCountersAlpha = new BarChartWidget(tr("A"), m_page_3);
    gl->addWidget(m_barChartWidgetCountersAlpha, 0, 1);
    m_barChartWidgetCountersGamma = new BarChartWidget(tr("G"), m_page_3);
    gl->addWidget(m_barChartWidgetCountersGamma, 1, 1);
}


void MainWindow::openFile() {
    QString fileName = QFileDialog::getOpenFileName(this,
        tr("Open File"),
        "",
        tr("ADCM Files (*.dat);;All Files (*.*)"));

    if (fileName.isEmpty()) {
        return;
    }
    m_path = fileName;
    m_controller->operatePath(m_path);
    m_settings->setPath(m_path);
}

void MainWindow::connectToServer(const QString &serverName)
{
    if (m_client) {
        m_client->disconnectFromServer();
        return;
    }
    if (!m_client) {
        m_client = new Client(this);
        connect(m_client, &Client::connected,
                this, &MainWindow::serverConnected);
        connect(m_client, &Client::disconnected,
                this, &MainWindow::serverDisconnected);
        connect(m_client, &Client::connectionError,
                this, &MainWindow::serverConnectionError);
//        connect(m_client, &Client::serverStatusReceived,
//                this, &MainWindow::serverStatusReceived);
        connect(m_client, &Client::resultReceived,
                this, [](ResultData resultData){
            qDebug() << "Клиент успешно принял структуру TaskData:";
            qDebug() << "ID:" << resultData.id;
            qDebug() << "Title:" << resultData.title;
            qDebug() << "Deadline:" << resultData.deadline.toString();
            qDebug() << "Status (Completed):" << resultData.isCompleted;
        });
        connect(m_client, &Client::complexDataReceived, this, &MainWindow::newDataEnergyByAlpha);
        m_client->connectToServer(serverName);
    }
}

void MainWindow::serverConnected()
{
    qDebug() << "serverConnected";
    m_serverStatusMessageLabel->setText(QString("%1").arg(QString::fromUtf8(u8"\U0001F7E2")));
}

void MainWindow::serverDisconnected()
{
    QString r;
    r.append(QString::fromUtf8(u8"\U0001F534"));
    r.append(QString::fromUtf8(u8"\U0001F503"));
    m_serverStatusMessageLabel->setText(r);
}

void MainWindow::serverConnectionError(const QString &error)
{
    QString r;
    r.append(QString("%1 %2").arg(QString::fromUtf8(u8"\U0001F534")).arg(error));
    r.append(QString::fromUtf8(u8"\U0001F503"));
    m_serverStatusMessageLabel->setText(r);
}

void MainWindow::serverStatusReceived(const QJsonObject &status)
{
    QString statusStr = QString("Server Status:\n"
                                   "  Connections: %1\n"
                                   "  Active Calculations: %2\n"
                                   "  Thread Pool: %3")
                           .arg(status["active_connections"].toInt())
                           .arg(status["active_calculations"].toInt())
                           .arg(status["thread_pool_size"].toInt());
    qDebug() << statusStr;
}
