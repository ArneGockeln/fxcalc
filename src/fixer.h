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
	Fixer(const QString api_url, const QString api_key);
	void latest(const QString instruments);

signals:
	void response(QJsonDocument json);

public slots:
	void onResult(QNetworkReply* reply);

private:
	QNetworkReply* m_nw_reply;
	QNetworkAccessManager* m_nw_access_mgr;
	QByteArray* m_buffer;
	QString m_api_key;
	QString m_api_url;

};
};