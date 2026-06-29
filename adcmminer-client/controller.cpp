#include "controller.h"

Controller::Controller(const QString &path)
{
    m_timer = new QTimer(this);
    m_timer->setInterval(1'000);
    m_fileWatcherThread = new QThread(this);
    m_fileWatcherWorker = new FileWatcherWorker(path);
    m_fileWatcherWorker->moveToThread(m_fileWatcherThread);

    m_processingThread = new QThread(this);
    m_processingWorker = new ProcessingWorker();
    m_processingWorker->moveToThread(m_processingThread);

    connect(m_timer, &QTimer::timeout, m_fileWatcherWorker, &FileWatcherWorker::doWorkCheck);
    connect(this, &Controller::operatePath, m_fileWatcherWorker, &FileWatcherWorker::doWorkPath);
//    connect(m_fileWatcherWorker, &FileWatcherWorker::resultReadyCheck, this, &Controller::handleResultsReadyCheck);
    connect(m_fileWatcherThread, &QThread::finished, m_fileWatcherWorker, &QObject::deleteLater);
//    connect(this, &Controller::operateS, m_processingWorker, &ProcessingWorker::doWorkS);
    connect(this, &Controller::operateReset, m_processingWorker, &ProcessingWorker::doWorkReset);
    connect(m_processingWorker, &ProcessingWorker::resultReadyTimeCorrectedByAlpha, this, &Controller::handleResultsTimeCorrectedByAlpha);
    connect(m_processingWorker, &ProcessingWorker::resultReadyEnergyByAlpha, this, &Controller::handleResultsEnergyByAlpha);
    connect(m_processingWorker, &ProcessingWorker::resultReadyProcessing, this, &Controller::handleResultsProcessing);
    connect(m_processingThread, &QThread::finished, m_processingWorker, &QObject::deleteLater);

    connect(m_fileWatcherWorker, &FileWatcherWorker::resultReadyCheck, [this](const QString &message, const QString &path, const bool &isModified){
        emit handleResultsReadyCheck(message);
        if (isModified) {
            m_processingWorker->doWorkS(path);
        }
    });

    m_fileWatcherThread->start();
    m_processingThread->start();


}

Controller::~Controller()
{
    if (m_timer != nullptr) {
        m_timer->stop();
    }
    m_fileWatcherThread->quit();
    m_fileWatcherThread->wait();

    m_processingThread->quit();
    m_processingThread->wait();
}

void Controller::operateTimer(bool checked)
{
    if (m_timer != nullptr) {
        if (checked) {
            m_timer->start();
        } else {
            m_timer->stop();
        }
    }
}
