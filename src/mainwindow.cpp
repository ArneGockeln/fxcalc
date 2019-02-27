#include "mainwindow.h"

#include <QClipboard>
#include <QStatusBar>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QDebug>
#include <QIntValidator>
#include <QDoubleValidator>
#include <QDesktopWidget>
#include <QApplication>
#include <QPushButton>
#include <QFile>
#include <QStringLiteral>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QInputDialog>
#include <QStandardPaths>
#include <QMessageBox>
#include <QTextStream>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QHBoxLayout>
#include <QFormLayout>

namespace poscalc {
	MainWindow::MainWindow(): m_api( new Fixer( FIXER_API_URL ) ) {
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

		setUnifiedTitleAndToolBarOnMac(true);

		statusBar()->showMessage( tr("Ready.") );
	}

	void MainWindow::initForm() {
		m_edit_account_balance = new QLineEdit;
		m_edit_risk_percent    = new QLineEdit;
		m_edit_sl_pips         = new QLineEdit;
		m_cb_account_currency  = new QComboBox;
		m_cb_instrument        = new QComboBox;

		m_cb_account_currency->setInsertPolicy( QComboBox::NoInsert );
		m_cb_instrument->setInsertPolicy( QComboBox::NoInsert );

		auto balanceValidator = new QDoubleValidator(0, 999999999, 2, this );
		balanceValidator->setNotation( QDoubleValidator::StandardNotation );
		m_edit_account_balance->setValidator( balanceValidator );

		auto riskValidator = new QDoubleValidator(0, 100, 2, this );
		riskValidator->setNotation( QDoubleValidator::StandardNotation );
		m_edit_risk_percent->setValidator( riskValidator );

		auto pipValidator = new QIntValidator(0, 9999, this );
		m_edit_sl_pips->setValidator( pipValidator );

		m_label_current_ask_price = new QLabel( QString::number( 0 ) );
		m_label_result_risk       = new QLabel( QString::number( 0 ) );
		m_edit_units              = new QLineEdit( QString::number( 0 ) );
		m_edit_lots               = new QLineEdit( QString::number( 0 ) );
		
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

		// refresh rates button
		m_btn_refresh_rates = new QPushButton(tr("Refresh"), this);

		// create form
		QFormLayout* layout = new QFormLayout;
		layout->addRow( tr("Account Currency"), m_cb_account_currency );
		layout->addRow( tr("Instrument"), m_cb_instrument );
		layout->addRow( "", m_btn_refresh_rates );
		layout->addRow( tr("Account Balance"), m_edit_account_balance );
		layout->addRow( tr("Risk Ratio, %"), m_edit_risk_percent );
		layout->addRow( tr("Stoploss, Pips"), m_edit_sl_pips );
		layout->addRow( tr("Current Ask"), m_label_current_ask_price );
		layout->addRow( tr("Risk"), m_label_result_risk );
		layout->addRow( tr("Units"), m_edit_units  );
		layout->addRow( tr("Lots"), m_edit_lots  );

		layout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);

		QWidget* main_widget = new QWidget;
		main_widget->setLayout( layout );
		
		setCentralWidget( main_widget );

		// set currency priority
		m_currency_priority.insert( std::pair<QString, int>("RUB", 310) );
		m_currency_priority.insert( std::pair<QString, int>("MXN", 320) );
		m_currency_priority.insert( std::pair<QString, int>("LTL", 330) );
		m_currency_priority.insert( std::pair<QString, int>("HRK", 340) );
		m_currency_priority.insert( std::pair<QString, int>("SEK", 350) );
		m_currency_priority.insert( std::pair<QString, int>("ZAR", 360) );
		m_currency_priority.insert( std::pair<QString, int>("NOK", 380) );
		m_currency_priority.insert( std::pair<QString, int>("LVL", 390) );
		m_currency_priority.insert( std::pair<QString, int>("HUF", 400) );
		m_currency_priority.insert( std::pair<QString, int>("HKD", 410) );
		m_currency_priority.insert( std::pair<QString, int>("CZK", 420) );
		m_currency_priority.insert( std::pair<QString, int>("PLN", 430) );
		m_currency_priority.insert( std::pair<QString, int>("DKK", 440) );
		m_currency_priority.insert( std::pair<QString, int>("SGD", 450) );
		m_currency_priority.insert( std::pair<QString, int>("CHF", 460) );
		m_currency_priority.insert( std::pair<QString, int>("CNH", 470) );
		m_currency_priority.insert( std::pair<QString, int>("CAD", 500) );
		m_currency_priority.insert( std::pair<QString, int>("USD", 600) );
		m_currency_priority.insert( std::pair<QString, int>("NZD", 700) );
		m_currency_priority.insert( std::pair<QString, int>("JPY", 300) );
		m_currency_priority.insert( std::pair<QString, int>("AUD", 800) );
		m_currency_priority.insert( std::pair<QString, int>("GBP", 900) );
		m_currency_priority.insert( std::pair<QString, int>("EUR", 1000) );

		// restore values from config file
		load();

		// connections
		QObject::connect( m_edit_account_balance, SIGNAL(editingFinished()), this, SLOT(onAccountBalanceChange()));
		QObject::connect( m_edit_risk_percent, SIGNAL(editingFinished()), this, SLOT(onRiskChange()));
		QObject::connect( m_edit_sl_pips, SIGNAL(editingFinished()), this, SLOT(onSLChange()));
		QObject::connect( m_cb_account_currency, SIGNAL(currentTextChanged(QString)), this, SLOT(onAccountCurrencyChange()));
		QObject::connect( m_cb_instrument, SIGNAL(currentTextChanged(QString)), this, SLOT(onInstrumentChange()));
		QObject::connect( m_btn_refresh_rates, SIGNAL(clicked()), this, SLOT(onAccountCurrencyChange()));
	}

	/**
	 * If account currency is changed, fetch new exchange rate
	 * SLOT
	 */
	void MainWindow::fetchExchangeRate() {
		if ( m_api == nullptr ) {
			QMessageBox::critical(this, tr("Error"), tr("The API is not initialized.") );
			return;
		}

		statusBar()->showMessage( tr("Sending request for exchange rates...") );

		// send latest request
		m_api->latest();

		// handle json response
		QObject::connect( m_api, &Fixer::onResponse, [this](QJsonDocument doc) {
			auto json = doc.object();
			auto base = ( json.contains("base") ? json["base"].toString() : m_cb_account_currency->currentText() );
			if ( json.contains( "rates" ) && json["rates"].isObject() ) {
				auto rates = json["rates"].toObject();
				foreach( const QString& key, rates.keys() ) {
					auto value = rates.value( key );
					// try to insert
					auto insert = m_rates.insert( std::pair<QString, double>( key, value.toDouble() ) );
					if ( ! insert.second ) {
						// failed, try to update
						m_rates[ key ] = value.toDouble();
					}
				}

				statusBar()->showMessage( tr("Exchange rates updated.") );

				// update
				calculate();
			} else {
				QMessageBox::information(this, tr("Info"), tr("No rates found.") );
			}
		});
		// error response
		QObject::connect( m_api, &Fixer::onError, [this](const QString msg) {
			statusBar()->showMessage( tr("API Error."), 5000 );
			QMessageBox::critical(this, tr("Error"), msg);
		});
	}

	/**
	 * Calculate on value change
	 * SLOT
	 */
	void MainWindow::calculate() {
		// save values to json file
		save();

		statusBar()->showMessage(tr("Calculating..."));

		if ( m_edit_risk_percent->text().isEmpty() ) return;
		if ( m_edit_account_balance->text().isEmpty() ) return;
		if ( m_edit_sl_pips->text().isEmpty() ) return;
		if ( m_rates.size() == 0 ) return;

		bool ok(false);
		double account_size = m_edit_account_balance->text().toDouble(&ok);
		if ( ! ok ) {
			QMessageBox::information(this, tr("Info"), tr("Couldn't convert balance to double."));
			return;
		}

		ok = false;
		double risk_percent = m_edit_risk_percent->text().toDouble(&ok);
		if ( ! ok ) {
			QMessageBox::information(this, tr("Info"), tr("Couldn't convert risk to double."));
			return;
		}

		ok = false;
		int sl_pips = m_edit_sl_pips->text().toInt(&ok);
		if ( ! ok ) {
			QMessageBox::information(this, tr("Info"), tr("Couldn't convert pips to integer."));
			return;
		}

		double current_price = 1;
		double unit_costs    = 0.0001;
		QString currency     = m_cb_account_currency->currentText(); 
		QString second_currency;
		double risk          = ( risk_percent * account_size ) / 100;

		QRegularExpression re(".{3}$");
		QRegularExpressionMatch match = re.match( m_cb_instrument->currentText() );
		if ( match.hasMatch() ) {
			second_currency = match.captured(0);
		}

		// find rate for the second currency
		if ( second_currency != currency ) {
			auto it = m_rates.find( second_currency );
			if ( it != m_rates.end() ) {
				current_price = it->second;
			}
		}

		QString firstAffCur;
		QString secondAffCur;
		// get currency priority
		if ( m_currency_priority[currency] > m_currency_priority[second_currency] || m_currency_priority[second_currency] == 0 ) {
			firstAffCur = currency;
			secondAffCur = second_currency;
		} else {
			firstAffCur = second_currency;
			secondAffCur = currency;
		}

		QString stype = ( currency == firstAffCur ? " Ask price" : " Bid price" );
		
		if ( ! secondAffCur.isEmpty() ) {
			if ( stype == " Ask price" ) {
				if ( secondAffCur == "JPY" ) unit_costs = unit_costs / ( current_price / 100 );
				else unit_costs = unit_costs / current_price;
			} else {
				if ( secondAffCur == "JPY" ) unit_costs = unit_costs * current_price / 100;
				else unit_costs = unit_costs * current_price;
			}
		}

		double result = risk / sl_pips / unit_costs;

		// set label with current price
		m_label_current_ask_price->setText( QString::number( current_price ) );
		// set label with money risk
		m_label_result_risk->setText( QString::number( risk, 'f', 2 ) + " " + currency );
		// set label units
		m_edit_units->setText( QString::number( result, 'f', 0 ) );
		// set label lots
		m_edit_lots->setText( QString::number( ( result / 100000 ), 'f', 2 ) );

		statusBar()->showMessage(tr("Ready."));
	}

	void MainWindow::onRiskChange() {
		calculate();
	}

	void MainWindow::onSLChange() {
		calculate();
	}

	void MainWindow::onAccountCurrencyChange() {
		if ( m_api == nullptr ) return;

		// set base currency
		m_api->setBaseCurrency( m_cb_account_currency->currentText() );

		// fetch exchange rate
		fetchExchangeRate();
	}

	void MainWindow::onAccountBalanceChange() {
		calculate();
	}

	void MainWindow::onInstrumentChange() {
		calculate();
	}

	// save form data to file
	void MainWindow::save() {
		QString configLocation = QStandardPaths::writableLocation( QStandardPaths::AppConfigLocation );
		if ( configLocation.isEmpty() ) {
			configLocation.append( "poscalc" );
		}
		
		QFile saveFile( configLocation );
		if ( ! saveFile.open( QIODevice::WriteOnly ) ) {
			QMessageBox::critical(this, tr("Error"), tr("Couldn't save settings file.") );
			saveFile.close();
			return;
		}

		QJsonObject json;
		json["balance"] = m_edit_account_balance->text();
		json["risk"]    = m_edit_risk_percent->text();
		json["slpips"]  = m_edit_sl_pips->text();
		json["currency"] = m_cb_account_currency->currentText();
		json["instrument"] = m_cb_instrument->currentText();

		QJsonArray rates;
		for( auto& pair : m_rates ) {
			QJsonObject json_pair;
			json_pair[ pair.first ] = pair.second;

			rates.append( json_pair );
		}

		json["rates"] = rates;

		QJsonDocument doc(json);

		saveFile.write( doc.toJson() );
	}

	// load form data from file
	void MainWindow::load() {
		QString configLocation = QStandardPaths::writableLocation( QStandardPaths::AppConfigLocation );
		if ( configLocation.isEmpty() ) {
			configLocation.append( "poscalc" );
		}

		QFile loadFile( configLocation );
		if ( ! loadFile.open( QIODevice::ReadOnly ) ) {
			qWarning() << "Couldn't open" << configLocation;
			return;
		}

		auto data = loadFile.readAll();
		QJsonDocument doc( QJsonDocument::fromJson( data ) );

		QJsonObject json = doc.object();
		if ( json.contains("balance") ) {
			m_edit_account_balance->setText( json["balance"].toString() );
		}
		if ( json.contains("risk") ) {
			m_edit_risk_percent->setText( json["risk"].toString() );
		}
		if ( json.contains("slpips") ) {
			m_edit_sl_pips->setText( json["slpips"].toString() );
		}
		if ( json.contains("currency") ) {
			m_cb_account_currency->setCurrentText( json["currency"].toString() );
		}
		if( json.contains("instrument") ) {
			m_cb_instrument->setCurrentText( json["instrument"].toString() );
		}
		if ( json.contains("rates") ) {
			m_rates.clear();
			auto rates = json["rates"].toArray();
			for( int i = 0; i < rates.size(); ++i ) {
				QJsonObject rate = rates[i].toObject();
				auto key = rate.keys()[0];
				auto value = rate.value( key );

				auto insert = m_rates.insert( std::pair<QString, double>( key, value.toDouble() ) );
				if ( ! insert.second ) {
					// update
					m_rates[ key ] = value.toDouble();
				}

				// update current ask price
				if ( key == m_cb_account_currency->currentText() ) {
					m_label_current_ask_price->setText( value.toString() );
				}
			}
		}

		calculate();
	}

};