#include "fixer.h"

#include <QDebug>

namespace poscalc {
	Fixer::Fixer(const QString api_url, const QString api_key):
		m_api_url(api_url),	m_api_key(api_key),	m_nw_reply(nullptr), m_nw_access_mgr( new QNetworkAccessManager( this ) ),
		m_buffer( new QByteArray ) {
			// connect network access manager to slot onResult
			QObject::connect( m_nw_access_mgr, SIGNAL(finished(QNetworkReply*)), this, SLOT(onResult(QNetworkReply*)));
	}

	// send latest request to fixer api
	void Fixer::latest(const QString instruments) {
		QString request = "latest?access_key=";
		request.prepend( m_api_url );
		request.append( m_api_key );
		request.append( "&format=1&symbols=" );
		request.append( instruments );
		
		const QUrl url( request );
		m_nw_reply = m_nw_access_mgr->get( QNetworkRequest( url ) );
	}

	void Fixer::onResult(QNetworkReply* reply) {
		if ( m_nw_reply->error() != QNetworkReply::NoError ) {
			qWarning() << "Network Error";
			return;
		}
		// read network reply into buffer
		m_buffer->append( reply->readAll() );
		// parse json document
		QJsonDocument doc = QJsonDocument::fromJson( *m_buffer );
		// emit signal
		emit response(doc);
	}

};