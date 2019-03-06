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

#include "mainwindow.h"

#include <QDebug>

#include <QDesktopWidget>
#include <QClipboard>
#include <QStatusBar>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QApplication>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QStandardPaths>
#include <QMessageBox>
#include <QTextStream>
#include <QMenuBar>

#include <cmath>

namespace fxcalc {
	MainWindow::MainWindow(): api_( new Fixer( FIXER_API_URL ) ), use_custom_rate_(false), calc_mode_(CalcMode::NORMAL) {
		setWindowTitle( tr( "FX Calculator" ) );

		auto screenRect = QApplication::desktop()->screenGeometry();

		int wnd_width   = 480;
		int wnd_height  = 320;

		// center on screen
		int wnd_x       = ( screenRect.width() / 2 ) - ( wnd_width / 2 );
		int wnd_y       = ( screenRect.height() / 2 ) - ( wnd_height / 2 );
		setGeometry( wnd_x, wnd_y, wnd_width, wnd_height );

		// setup menu bar
		QMenuBar* bar = new QMenuBar;
		setMenuBar(bar);

		// set about dialog
		QAction* action_about = new QAction(tr("&About"), this);
		connect(action_about, &QAction::triggered, this, [this](){
			QMessageBox::about(this, tr("About FXCalc"), tr("This app is written by Arne Gockeln.\nMore about me https://arnegockeln.com.\nHappy trading."));
		});

		QMenu* file = menuBar()->addMenu(tr("&File"));
		file->addAction(action_about);

		// setup form
		initForm();

		// macos specific settings
		setUnifiedTitleAndToolBarOnMac(true);
	}

	void MainWindow::initForm() {

		// create currency priority
		currency_priority_.insert( std::pair<QString, int>("RUB", 310) );
		currency_priority_.insert( std::pair<QString, int>("MXN", 320) );
		currency_priority_.insert( std::pair<QString, int>("LTL", 330) );
		currency_priority_.insert( std::pair<QString, int>("HRK", 340) );
		currency_priority_.insert( std::pair<QString, int>("SEK", 350) );
		currency_priority_.insert( std::pair<QString, int>("ZAR", 360) );
		currency_priority_.insert( std::pair<QString, int>("NOK", 380) );
		currency_priority_.insert( std::pair<QString, int>("LVL", 390) );
		currency_priority_.insert( std::pair<QString, int>("HUF", 400) );
		currency_priority_.insert( std::pair<QString, int>("HKD", 410) );
		currency_priority_.insert( std::pair<QString, int>("CZK", 420) );
		currency_priority_.insert( std::pair<QString, int>("PLN", 430) );
		currency_priority_.insert( std::pair<QString, int>("DKK", 440) );
		currency_priority_.insert( std::pair<QString, int>("SGD", 450) );
		currency_priority_.insert( std::pair<QString, int>("CHF", 460) );
		currency_priority_.insert( std::pair<QString, int>("CNH", 470) );
		currency_priority_.insert( std::pair<QString, int>("CAD", 500) );
		currency_priority_.insert( std::pair<QString, int>("USD", 600) );
		currency_priority_.insert( std::pair<QString, int>("NZD", 700) );
		currency_priority_.insert( std::pair<QString, int>("JPY", 300) );
		currency_priority_.insert( std::pair<QString, int>("AUD", 800) );
		currency_priority_.insert( std::pair<QString, int>("GBP", 900) );
		currency_priority_.insert( std::pair<QString, int>("EUR", 1000) );

		// create form
		form_ = new Form;

		// add account currencies
		form_->cbAccountCurrency()->addItem( "AUD" );
		form_->cbAccountCurrency()->addItem( "CAD" );
		form_->cbAccountCurrency()->addItem( "CHF" );
		form_->cbAccountCurrency()->addItem( "EUR" );
		form_->cbAccountCurrency()->addItem( "GBP" );
		form_->cbAccountCurrency()->addItem( "JPY" );
		form_->cbAccountCurrency()->addItem( "NZD" );
		form_->cbAccountCurrency()->addItem( "USD" );
		form_->cbAccountCurrency()->setCurrentText( "EUR" );

		// load instruments from list
		QFile instrumentsFile(":/instruments.txt");
		if ( ! instrumentsFile.open( QIODevice::ReadOnly ) ) {
			QMessageBox::critical(this, tr("Error"), tr("Can't load instrument list!") );
		}

		QTextStream in(&instrumentsFile);
		while( ! in.atEnd() ) {
			QString instrument = in.readLine();
			form_->cbInstrument()->addItem( instrument );
		}
		instrumentsFile.close();

		// load settings from file
		load();

		// set central widget
		setCentralWidget(form_);

		// connections
		connect( form_->editAccountBalance(), &QLineEdit::editingFinished, this, &MainWindow::calculate );
		connect( form_->editRiskPercent(), &QLineEdit::editingFinished, this, &MainWindow::calculate );
		connect( form_->editContractSize(), &QLineEdit::editingFinished, this, &MainWindow::calculate );
		connect( form_->editSLPips(), &QLineEdit::editingFinished, this, &MainWindow::calculate );
		connect( form_->editTPPips(), &QLineEdit::editingFinished, [this]() {
			calc_mode_ = CalcMode::TP_PIPS;
			calculate();
		});
		connect( form_->editMarginRatio(), &QLineEdit::editingFinished, this, &MainWindow::calculate );
		connect( form_->editCommission(), &QLineEdit::editingFinished, this, &MainWindow::calculate );
		connect( form_->editInstrumentRate(), &QLineEdit::editingFinished, [this](){
			// calculate with custom exchange rate
			use_custom_rate_ = true;
			calculate();
		});
		connect( form_->editMarginInstrumentRate(), &QLineEdit::editingFinished, [this]() {
			// calculate with custom exchange rate
			use_custom_rate_ = true;
			calculate();
		});
		connect( form_->cbInstrument(), &QComboBox::currentTextChanged, [this]() {
			use_custom_rate_ = false;
			calculate();
		});
		connect( form_->cbAccountCurrency(), &QComboBox::currentTextChanged, this, &MainWindow::onAccountCurrencyChange );
		connect( form_->btnRefreshRates(), &QPushButton::clicked, this, &MainWindow::onAccountCurrencyChange );

		connect( form_->btnCopyUnits(), &QPushButton::clicked, [this]() {
			// copy units to clipboard
			auto clipboard = QGuiApplication::clipboard();
			if ( clipboard != nullptr ) {
				clipboard->setText( form_->editUnits()->text() );

				statusBar()->showMessage( tr("Units copied to clipboard."), 3000);
			}
		});
		connect( form_->btnCopyLots(), &QPushButton::clicked, [this]() {
			// copy lots to clipboard
			auto clipboard = QGuiApplication::clipboard();
			if ( clipboard != nullptr ) {
				clipboard->setText( form_->editLots()->text() );

				statusBar()->showMessage( tr("Lots copied to clipboard."), 3000);
			}
		});
	}
	
	/**
	 * If account currency is changed, fetch new exchange rate
	 * SLOT
	 */
	void MainWindow::fetchExchangeRate() {
		if ( api_ == nullptr ) {
			QMessageBox::critical(this, tr("Error"), tr("The API is not initialized.") );
			return;
		}

		// update status bar
		statusBar()->showMessage( tr("Sending request for exchange rates...") );

		// send latest request
		api_->latest();

		// handle json response
		QObject::connect( api_, &Fixer::onResponse, [this](QJsonDocument doc) {
			auto json = doc.object();
			auto base = ( json.contains("base") ? json["base"].toString() : form_->cbAccountCurrency()->currentText() );
			if ( json.contains( "rates" ) && json["rates"].isObject() ) {
				auto rates = json["rates"].toObject();
				foreach( const QString& key, rates.keys() ) {
					auto value = rates.value( key );
					// try to insert
					auto insert = rates_.insert( std::pair<QString, double>( key, value.toDouble() ) );
					if ( ! insert.second ) {
						// failed, try to update
						rates_[ key ] = value.toDouble();
					}
				}

				statusBar()->showMessage( tr("Exchange rates updated."), 5000 );

				// reset use custom rate
				use_custom_rate_ = false;

				// update
				calculate();
			} else {
				QMessageBox::information(this, tr("Info"), tr("No rates found.") );
			}
		});
		// error response
		QObject::connect( api_, &Fixer::onError, [this](const QString msg) {
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

		if ( form_->editRiskPercent()->text().isEmpty() ) return;
		if ( form_->editAccountBalance()->text().isEmpty() ) return;
		if ( form_->editSLPips()->text().isEmpty() ) return;
		if ( rates_.size() == 0 ) return;

		//
		// ------------ CONVERT DOUBLE and INTEGER VALUES
		// 

		bool ok(false);
		double account_size = QLocale::system().toDouble( form_->editAccountBalance()->text(), &ok );
		if ( ! ok ) {
			statusBar()->showMessage(tr("Couldn't convert balance to double."), 3000);
			return;
		}

		ok = false;
		double risk_percent = QLocale::system().toDouble( form_->editRiskPercent()->text(), &ok );
		if ( ! ok ) {
			statusBar()->showMessage(tr("Couldn't convert risk to double."), 3000);
			return;
		}

		ok = false;
		int sl_pips = QLocale::system().toInt( form_->editSLPips()->text(), &ok );
		if ( ! ok ) {
			statusBar()->showMessage(tr("Couldn't convert pips to integer."), 3000);
			return;
		}

		double commissions = 0;
		if ( ! form_->editCommission()->text().isEmpty() ) {
			ok = false;
			commissions = QLocale::system().toDouble( form_->editCommission()->text(), &ok );
			if ( ! ok ) {
				statusBar()->showMessage(tr("Couldn't convert commission to double."), 3000);
			}
		}

		int margin_ratio = 0;
		if ( ! form_->editMarginRatio()->text().isEmpty() ) {
			ok = false;
			margin_ratio = QLocale::system().toInt( form_->editMarginRatio()->text(), &ok );
			if ( ! ok ) {
				statusBar()->showMessage(tr("Couldn't convert margin ratio to int."), 3000);
				margin_ratio = 0;
			}
		}

		int tp_pips = 0;
		if ( ! form_->editTPPips()->text().isEmpty() ) {
			ok = false;
			tp_pips = QLocale::system().toInt( form_->editTPPips()->text(), &ok );
			if ( ! ok ) {
				statusBar()->showMessage(tr("Couldn't convert take profit pips to int."), 3000);
			}
		}

		int contract_size = 100000;
		if ( ! form_->editContractSize()->text().isEmpty() ) {
			ok = false;
			contract_size = QLocale::system().toInt( form_->editContractSize()->text(), &ok );
			if ( ! ok ) {
				statusBar()->showMessage(tr("Couldn't convert contract size to int."), 3000 );
				return;
			}
		}
		
		//
		// ----------------------- DEFAULT VALUES
		// 
		double tp_rate           = 0;
		double current_price     = 1;
		double point_size        = 0.0001;
		double unit_costs        = 0.0001;
		QString account_currency = form_->cbAccountCurrency()->currentText();
		int account_precision    = 5;
		if ( account_currency == "JPY" ) {
			account_precision = 3;
		}
		int instrument_precision = 5;

		// base(EUR)/quote(USD) = EUR/USD
		QString base_currency;
		QString quote_currency;
		
		// calculate risk in account currency
		double risk = ( risk_percent * account_size ) / 100;

		// get base and quote currency from pair by using regular expression
		// split currency pair in two parts like: EURUSD => (EUR), (USD)
		QRegularExpression re("(?<base>.{3})(?<quote>.{3})");
		QRegularExpressionMatch match = re.match( form_->cbInstrument()->currentText() );
		if ( match.hasMatch() ) {
			base_currency  = match.captured("base");
			quote_currency = match.captured("quote");
		}

		if ( quote_currency == "JPY" ) {
			instrument_precision = 3;
			point_size           = 0.001;
		}

		// find rate for the second currency
		if ( quote_currency != account_currency ) {

			form_->labelInstrumentRate()->setText(tr("Current ask ") + account_currency + quote_currency );

			if ( ! use_custom_rate_ ) {
				auto it = rates_.find( quote_currency );
				if ( it != rates_.end() ) {
					current_price = it->second;

					// set line edit with current price
					form_->editInstrumentRate()->setText( QLocale::system().toString( current_price, 'f', instrument_precision ) );
				}	
			} else {
				ok = false;
				if ( ! form_->editInstrumentRate()->text().isEmpty() ) {
					current_price = QLocale::system().toDouble( form_->editInstrumentRate()->text(), &ok );
					if ( ! ok ) {
						statusBar()->showMessage(tr("Couldn't convert custom exchange rate to double."), 3000 );
						return;
					}
				}
			}
		}

		QString base_aff_currency;
		QString quote_aff_currency;
		QString stype;
		if ( account_currency != quote_currency ) {
			if ( currency_priority_[account_currency] > currency_priority_[quote_currency] || currency_priority_[quote_currency] == 0 ) {
				base_aff_currency = account_currency;
				quote_aff_currency = quote_currency;
			} else {
				base_aff_currency = quote_currency;
				quote_aff_currency = account_currency;
			}

			if ( account_currency == base_aff_currency ) {
				stype = "Ask";
			} else {
				stype = "Bid";
			}
		}		
		
		if ( ! quote_aff_currency.isEmpty() ) {
			if ( stype == "Ask" ) {
				if ( quote_aff_currency == "JPY" ) {
					unit_costs = unit_costs / ( current_price / 100 );
				} else {
					unit_costs = unit_costs / current_price;
				}
			} else {
				if ( quote_aff_currency == "JPY" ) {
					unit_costs = unit_costs * current_price / 100;
				} else {
					unit_costs = unit_costs * current_price;
				}
			}
		}

		double units = risk / sl_pips / unit_costs;

		// calculate margin requirements
		// get price for margin calc
		double margin_price = 1;
		double margin       = 0;
		if ( base_currency != account_currency ) {
			if ( ! use_custom_rate_ ) {
				auto it = rates_.find( base_currency );
				if ( it != rates_.end() ) {
					margin_price = 1 / it->second;
				}
			} else {
				ok = false;
				if ( ! form_->editMarginInstrumentRate()->text().isEmpty() ) {
					margin_price = QLocale::system().toDouble( form_->editMarginInstrumentRate()->text(), &ok );
					if ( ! ok ) {
						statusBar()->showMessage( tr("Couldn't convert custom margin rate to double."), 3000);
						return;
					}
				}
			}
		}

		margin = ( margin_price * units ) / margin_ratio;

		// calculate lots
		double lots = ( units / contract_size );

		// calculate profit
		// Rate of Exit * Profit in Points * Lot Size of Exit * Point Value of Exit
		double profit = 0;

		if ( calc_mode_ == CalcMode::TP_PIPS ) {
			tp_rate = current_price + ( point_size * tp_pips );	
		} else if ( calc_mode_ == CalcMode::TP_RATE ) {
			tp_pips = std::abs( current_price - tp_rate ) * ( 1 / point_size );
		}

		profit = tp_pips * ( unit_costs * contract_size );

		// calculate commissions for entry and exit
		if ( commissions > 0 ) {
			commissions = commissions * ( ( contract_size * current_price ) / 100000 ) * 2;
		}

		//
		// --------------- UPDATE UI
		// 

		// set label margin instrument
		form_->labelMarginInstrument()->setText( tr("Current ask ") + base_currency + account_currency );		
		// set unit costs
		form_->labelPipValue()->setText( QLocale::system().toString( unit_costs * contract_size, 'f', 2 ) + " " + account_currency );
		// set label with money risk
		form_->labelResultRisk()->setText( QLocale::system().toString( risk, 'f', 2 ) + " " + account_currency );
		// set label with money profit
		form_->labelResultProfit()->setText( QLocale::system().toString( profit, 'f', 2 ) + " " + account_currency );
		// set label margin requirements
		form_->labelMarginRequired()->setText( QLocale::system().toString( margin, 'f', 2 ) + " " + account_currency );
		// set label for commissions
		form_->labelCommission()->setText( QLocale::system().toString( commissions, 'f', 2 ) + " " + account_currency );

		// set tp pips
		form_->editTPPips()->setText( QString::number( tp_pips ) );
		// set label units
		form_->editUnits()->setText( QString::number( units, 'f', 0 ) );
		// set label lots
		form_->editLots()->setText( QLocale::system().toString( lots, 'f', 3 ) );
		// set edit for margin instrument rate
		form_->editMarginInstrumentRate()->setText( QLocale::system().toString( margin_price, 'f', account_precision ) );
		
		// update statusbar
		statusBar()->clearMessage();
		// rest calc mode
		calc_mode_ = CalcMode::NORMAL;
	}

	// slot
	void MainWindow::onAccountCurrencyChange() {
		if ( api_ == nullptr ) return;

		// set base currency
		api_->setBaseCurrency( form_->cbAccountCurrency()->currentText() );

		// fetch exchange rate
		fetchExchangeRate();
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
		json["balance"]      = form_->editAccountBalance()->text();
		json["risk"]         = form_->editRiskPercent()->text();
		json["contractsize"] = form_->editContractSize()->text();
		json["slpips"]       = form_->editSLPips()->text();
		json["tppips"]       = form_->editTPPips()->text();
		json["commission"]   = form_->editCommission()->text();
		json["marginratio"]  = form_->editMarginRatio()->text();
		json["currency"]     = form_->cbAccountCurrency()->currentText();
		json["instrument"]   = form_->cbInstrument()->currentText();

		QJsonArray rates;
		for( auto& pair : rates_ ) {
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

		// defaults
		form_->editContractSize()->setText( QStringLiteral("100000") );

		auto data = loadFile.readAll();
		QJsonDocument doc( QJsonDocument::fromJson( data ) );

		QJsonObject json = doc.object();
		if ( json.contains("balance") ) {
			form_->editAccountBalance()->setText( json["balance"].toString() );
		}
		if ( json.contains("risk") ) {
			form_->editRiskPercent()->setText( json["risk"].toString() );
		}
		if ( json.contains("contractsize") ) {
			form_->editContractSize()->setText( json["contractsize"].toString() );
		}
		if ( json.contains("slpips") ) {
			form_->editSLPips()->setText( json["slpips"].toString() );
		}
		if ( json.contains("tppips") ) {
			form_->editTPPips()->setText( json["tppips"].toString() );
		}
		if ( json.contains("commission") ) {
			form_->editCommission()->setText( json["commission"].toString() );
		}
		if ( json.contains("marginratio") ) {
			form_->editMarginRatio()->setText( json["marginratio"].toString() );
		}
		if ( json.contains("currency") ) {
			form_->cbAccountCurrency()->setCurrentText( json["currency"].toString() );
		}
		if( json.contains("instrument") ) {
			form_->cbInstrument()->setCurrentText( json["instrument"].toString() );
		}
		if ( json.contains("rates") ) {
			rates_.clear();
			auto rates = json["rates"].toArray();
			for( int i = 0; i < rates.size(); ++i ) {
				QJsonObject rate = rates[i].toObject();
				auto key = rate.keys()[0];
				auto value = rate.value( key );

				auto insert = rates_.insert( std::pair<QString, double>( key, value.toDouble() ) );
				if ( ! insert.second ) {
					// update
					rates_[ key ] = value.toDouble();
				}

				// update current ask price
				if ( key == form_->cbAccountCurrency()->currentText() ) {
					form_->editInstrumentRate()->setText( value.toString() );
				}
			}
		}

		calculate();
	}

};