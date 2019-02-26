#include "mainwindow.h"

#include <QDebug>
#include <QIntValidator>
#include <QDoubleValidator>
#include <QDesktopWidget>
#include <QApplication>
#include <QFormLayout>
#include <QPushButton>
#include <QFile>
#include <QStringLiteral>
#include <QJsonDocument>
#include <QJsonObject>
#include <QInputDialog>
#include <QStandardPaths>
#include <QMessageBox>
#include <QTextStream>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>

namespace poscalc {
	MainWindow::MainWindow(): m_api(nullptr) {
		setWindowTitle( tr( "FX Position Calculator" ) );

		auto screenRect = QApplication::desktop()->screenGeometry();

		int wnd_width   = 480;
		int wnd_height  = 320;

		// center on screen
		int wnd_x       = ( screenRect.width() / 2 ) - ( wnd_width / 2 );
		int wnd_y       = ( screenRect.height() / 2 ) - ( wnd_height / 2 );

		setGeometry( wnd_x, wnd_y, wnd_width, wnd_height );

		// setup form
		initForm();

		// check if there is an fixer.io api key available
		// if not, ask user to provide a key. save it in run directory
		checkAPIKey();		
	}

	void MainWindow::checkAPIKey() {
		QString configLocation = QStandardPaths::writableLocation( QStandardPaths::AppConfigLocation );
		if ( configLocation.isEmpty() ) {
			configLocation.append( "poscalc" );
		}
		
		QFile loadFile( configLocation );

		if ( ! loadFile.open( QIODevice::ReadOnly ) ) {
			// popup model to enter a fixer.io api key
			bool ok;
			QString apikey = QInputDialog::getText(this, tr("Need API Key!"), tr("Fixer.io API Key:"), QLineEdit::Normal, "", &ok);

			if ( ok && ! apikey.isEmpty() ) {
				m_api_key = apikey;

				// init api
				m_api = new Fixer( FIXER_API_URL, apikey);

				// save key
				QFile saveFile( configLocation );
				if ( ! saveFile.open( QIODevice::WriteOnly ) ) {
					QMessageBox::critical(this, tr("Error"), tr("Couldn't open api key file for saving.") );
					return;
				}

				QJsonObject saveObject;
				saveObject["key"] = apikey;

				QJsonDocument saveDoc( saveObject );
				saveFile.write( saveDoc.toBinaryData() );
			}
			return;
		}

		QByteArray saveData = loadFile.readAll();
		QJsonDocument loadDoc( QJsonDocument::fromBinaryData( saveData ) );

		if ( ! loadDoc.isObject() ) {
			QMessageBox::critical(this, tr("Error"), "Couldn't load fixer.io API key." );
			return;
		}

		auto json = loadDoc.object();
		if ( json.contains( "key" ) ) {
			// get key
			m_api_key = json["key"].toString();

			// init api
			m_api = new Fixer( FIXER_API_URL, m_api_key);
		} else {
			QMessageBox::critical(this, tr("Error"), "Couldn't find fixer.io API key!" );
		}
	}

	void MainWindow::initForm() {
		m_edit_account_size   = new QLineEdit;
		m_edit_risk_percent   = new QLineEdit;
		m_edit_sl_pips        = new QLineEdit;
		m_cb_account_currency = new QComboBox;
		m_cb_instrument       = new QComboBox;

		m_cb_account_currency->setInsertPolicy( QComboBox::NoInsert );
		m_cb_instrument->setInsertPolicy( QComboBox::NoInsert );

		auto balanceValidator = new QDoubleValidator(0, 999999999, 2, this );
		balanceValidator->setNotation( QDoubleValidator::StandardNotation );
		m_edit_account_size->setValidator( balanceValidator );

		auto riskValidator = new QDoubleValidator(0, 100, 2, this );
		riskValidator->setNotation( QDoubleValidator::StandardNotation );
		m_edit_risk_percent->setValidator( riskValidator );

		auto pipValidator = new QIntValidator(0, 9999, this );
		m_edit_sl_pips->setValidator( pipValidator );

		m_label_current_ask_price = new QLabel( QString::number( 0 ) );
		m_label_result_risk       = new QLabel( QString::number( 0 ) );
		m_label_units             = new QLabel( QString::number( 0 ) );
		m_label_lots              = new QLabel( QString::number( 0 ) );
		
		// account currencies
		m_cb_account_currency->addItem( "AUD" );
		m_cb_account_currency->addItem( "CAD" );
		m_cb_account_currency->addItem( "CHF" );
		m_cb_account_currency->addItem( "EUR" );
		m_cb_account_currency->addItem( "GBP" );
		m_cb_account_currency->addItem( "JPY" );
		m_cb_account_currency->addItem( "NZD" );
		m_cb_account_currency->addItem( "USD" );
		m_cb_account_currency->setCurrentText( "EUR" );

		// load instruments from list
		QFile instrumentsFile(":/instruments.txt");
		if ( ! instrumentsFile.open( QIODevice::ReadOnly ) ) {
			QMessageBox::critical(this, tr("Error"), tr("Can't load instrument list!") );
		}

		QTextStream in(&instrumentsFile);
		while( ! in.atEnd() ) {
			QString instrument = in.readLine();
			m_cb_instrument->addItem( instrument );
		}
		instrumentsFile.close();

		// create form
		QFormLayout* layout = new QFormLayout;
		layout->addRow( tr("Account Currency"), m_cb_account_currency );
		layout->addRow( tr("Account Balance"), m_edit_account_size );
		layout->addRow( tr("Risk Ratio, %"), m_edit_risk_percent );
		layout->addRow( tr("Stoploss, Pips"), m_edit_sl_pips );
		layout->addRow( tr("Instrument"), m_cb_instrument );
		layout->addRow( tr("Current Ask"), m_label_current_ask_price );
		layout->addRow( tr("Risk"), m_label_result_risk );
		layout->addRow( tr("Units"), m_label_units );
		layout->addRow( tr("Lots"), m_label_lots );

		layout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);

		QWidget* main_widget = new QWidget;
		main_widget->setLayout( layout );
		
		setCentralWidget( main_widget );

		// connections
		QObject::connect( m_edit_account_size, SIGNAL(editingFinished()), this, SLOT(calculate()));
		QObject::connect( m_edit_risk_percent, SIGNAL(editingFinished()), this, SLOT(calculate()));
		QObject::connect( m_edit_sl_pips, SIGNAL(editingFinished()), this, SLOT(calculate()));
		QObject::connect( m_cb_account_currency, SIGNAL(currentTextChanged(QString)), this, SLOT(fetchExchangeRate()));
		QObject::connect( m_cb_instrument, SIGNAL(currentTextChanged(QString)), this, SLOT(calculate()));
	}

	/**
	 * If account currency is changed, fetch new exchange rate
	 * SLOT
	 */
	void MainWindow::fetchExchangeRate() {
		if ( m_api_key.isEmpty() ) {
			QMessageBox::critical(this, tr("Error"), tr("The API Key is empty! Can not fetch data!" ) );
			return;
		}

		if ( m_api == nullptr ) {
			return;
		}

		m_api->latest( m_cb_account_currency->currentText() );

		QObject::connect( m_api, &Fixer::response, [this](QJsonDocument doc) {
			qDebug() << "callback" << doc;
		});
	}

	/**
	 * Calculate on value change
	 * SLOT
	 */
	void MainWindow::calculate() {
		if ( m_api_key.isEmpty() ) return;
		if ( m_edit_risk_percent->text().isEmpty() ) return;
		if ( m_edit_account_size->text().isEmpty() ) return;
		if ( m_edit_sl_pips->text().isEmpty() ) return;


	}

};