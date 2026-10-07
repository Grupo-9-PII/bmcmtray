#pragma once

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTimer>
#include <QUrl>

class ReadinessChecker final : public QObject
{
    Q_OBJECT

public:
    explicit ReadinessChecker(QObject *parent = nullptr);

    void start(const QUrl &healthUrl, int timeoutMilliseconds = 30000);
    void stop();
    bool isChecking() const;

signals:
    void checking(const QString &message);
    void ready(const QUrl &url);
    void timedOut(const QUrl &url);

private:
    void sendRequest();
    void scheduleRetry();

    QNetworkAccessManager network_;
    QTimer retryTimer_;
    QTimer timeoutTimer_;
    QNetworkReply *reply_ = nullptr;
    QUrl healthUrl_;
    bool checking_ = false;
};
