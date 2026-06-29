#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "constants.h"
#include "settings.h"
#include "piechartwidget.h"
#include "histchartwidget.h"
#include "barchartwidget.h"

#include "controller.h"

#include "client.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;


private slots:
    void newDataTimeCorrectedByAlpha(const QMap<QString, QList<QPointF>> &, const QMap<QString, QStringList> &text);
    void newDataEnergyByAlpha(const QMap<QString, QList<QPointF>> &data, const QMap<QString, QStringList> &text);
    void newDataProcessing(const QMap<QString, double> &data, double t, const QMap<QString, double> &countersA, const QMap<QString, double> &countersG);

    void setupTimeCorrectedByAlpha();
    void setupEnergyByAlpha();
    void setupProcessing();

    void openFile();

    void connectToServer(const QString &serverName);
    // client signals handlers
    void serverConnected();
    void serverDisconnected();
    void serverConnectionError(const QString &error);
    void serverStatusReceived(const QJsonObject &status);

private:
    Ui::MainWindow *ui;
    Settings *m_settings;
    QString m_path;
    QLabel *m_statusMessageLabel;

    QPushButton *m_pushButtonStartStop;
    QPushButton *m_pushButtonReset;

    Controller *m_controller;

    QTabWidget *m_tabWidget;

    QWidget *m_page_1;
    QWidget *m_page_2;
    QWidget *m_page_3;

    QWidget *m_mainWidget;
    QGridLayout *m_mainLayout;

    QWidget *m_widgetLeft;
    QWidget *m_widgetRight;

    QGridLayout *m_gLleft;
    QGridLayout *m_gLright;
    QDialog *m_dialog;

    PieChartWidget *m_pieChartWidget;
    QList<HistChartWidget *> m_histChartWidgetsTimeCorrectedByAlpha;
    QList<HistChartWidget *> m_histChartWidgetsEnergyByAlpha;

    QLabel *m_timeLabel;
    QLineEdit *m_timeLineEdit;

    BarChartWidget *m_barChartWidgetCountersAlpha;
    BarChartWidget *m_barChartWidgetCountersGamma;
    Client *m_client;

    QLabel *m_serverStatusMessageLabel;
    QPushButton *m_pushButtonConnect;
};
#endif // MAINWINDOW_H
