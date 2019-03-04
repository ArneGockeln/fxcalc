// License
// FXCalc is free software: you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// FXCalc is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.
// 
// You should have received a copy of the GNU General Public License
// along with FXCalc. If not, see <http://www.gnu.org/licenses/>.

#pragma once

#include <QJsonDocument>
#include <QByteArray>
#include <QNetworkRequest>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QString>

namespace fxcalc {
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