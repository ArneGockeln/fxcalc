#pragma once

#include <QJsonDocument>
#include <QByteArray>
#include <QNetworkRequest>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QString>

namespace poscalc {
class Fixer: public QObject {
	Q_OBJECT

public:
	Fixer(const QString api_url);
	void latest(const QString instruments = "");
	void setBaseCurrency(const QString currency);

signals:
	void onResponse(QJsonDocument json);
	void onError(const QString msg);

private slots:
	void onResult(QNetworkReply* reply);

private:
	QNetworkReply* m_nw_reply;
	QNetworkAccessManager* m_nw_access_mgr;
	QByteArray* m_buffer;
	QString m_api_url;
	QString m_base_currency;
};
};