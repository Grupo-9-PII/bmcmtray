#include "readinesschecker.h"

#include <QNetworkRequest>

ReadinessChecker::ReadinessChecker(QObject *parent)
    : QObject(parent)
{
    retryTimer_.setSingleShot(true);
    timeoutTimer_.setSingleShot(true);
    connect(&retryTimer_, &QTimer::timeout, this, &ReadinessChecker::sendRequest);
    connect(&timeoutTimer_, &QTimer::timeout, this, [this]
            {
        if (!checking_)
            return;

        checking_ = false;
        retryTimer_.stop();
        if (reply_) {
            QNetworkReply *reply = reply_;
            reply_ = nullptr;
            disconnect(reply, nullptr, this, nullptr);
            reply->abort();
            reply->deleteLater();
        }
        emit timedOut(healthUrl_); });
}

void ReadinessChecker::start(const QUrl &healthUrl, int timeoutMilliseconds)
{
    stop();
    healthUrl_ = healthUrl;
    if (!healthUrl_.isValid() || healthUrl_.host().isEmpty())
    {
        emit timedOut(healthUrl_);
        return;
    }

    checking_ = true;
    emit checking(QStringLiteral("Aguardando resposta de %1").arg(healthUrl_.toString()));
    timeoutTimer_.start(qMax(1000, timeoutMilliseconds));
    sendRequest();
}

void ReadinessChecker::stop()
{
    checking_ = false;
    retryTimer_.stop();
    timeoutTimer_.stop();
    if (reply_)
    {
        QNetworkReply *reply = reply_;
        reply_ = nullptr;
        disconnect(reply, nullptr, this, nullptr);
        reply->abort();
        reply->deleteLater();
    }
}

bool ReadinessChecker::isChecking() const
{
    return checking_;
}

void ReadinessChecker::sendRequest()
{
    if (!checking_ || reply_)
        return;

    QNetworkRequest request(healthUrl_);
    request.setTransferTimeout(2500);
    QNetworkReply *currentReply = network_.get(request);
    reply_ = currentReply;
    connect(currentReply, &QNetworkReply::finished, this, [this, currentReply]
            {
        const int statusCode = currentReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        const bool successful = currentReply->error() == QNetworkReply::NoError
                                && statusCode >= 200 && statusCode < 300;
        currentReply->deleteLater();
        if (reply_ == currentReply)
            reply_ = nullptr;

        if (!checking_)
            return;
        if (successful) {
            checking_ = false;
            retryTimer_.stop();
            timeoutTimer_.stop();
            emit ready(healthUrl_);
            return;
        }

        scheduleRetry(); });
}

void ReadinessChecker::scheduleRetry()
{
    if (checking_ && !retryTimer_.isActive())
        retryTimer_.start(1000);
}
