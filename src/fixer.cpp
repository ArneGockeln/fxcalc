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

#include "fixer.h"

#include <QDebug>
#include <QJsonObject>

namespace fxcalc {
	Fixer::Fixer(const QString api_url):
		m_api_url(api_url),	m_nw_reply(nullptr), m_nw_access_mgr( new QNetworkAccessManager( this ) ),
		m_buffer( new QByteArray ) {
			// connect network access manager to slot onResult
			QObject::connect( m_nw_access_mgr, SIGNAL(finished(QNetworkReply*)), this, SLOT(onResult(QNetworkReply*)));
	}

	// send latest request to fixer api
	void Fixer::latest(const QString instruments) {
		QString request = "latest";
		request.prepend( m_api_url );
		
		// only specific instruments
		if ( ! instruments.isEmpty() ) {
			request.append( "?symbols=" );
			request.append( instruments );	
			request.append( "&base=" );
		} else {
			request.append( "?base=" );
		}
		
		request.append( m_base_currency );
		
		const QUrl url( request );
		m_nw_reply = m_nw_access_mgr->get( QNetworkRequest( url ) );
	}

	// handle api reply
	void Fixer::onResult(QNetworkReply* reply) {
		if ( m_nw_reply->error() != QNetworkReply::NoError ) {
			qWarning() << "Network Error";
			return;
		}
		// read network reply into buffer
		m_buffer->append( reply->readAll() );
		// parse json document
		QJsonDocument doc = QJsonDocument::fromJson( *m_buffer );
		QJsonObject json  = doc.object();

		// on error
		if ( json.contains("error") ) {
			emit onError( json["error"].toString() );
			return;
		}

		// success
		emit onResponse(doc);
	}

	void Fixer::setBaseCurrency(const QString currency) {
		m_base_currency = currency;
	}

};