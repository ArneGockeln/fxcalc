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

#include "form.h"

#include <QGridLayout>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDoubleValidator>
#include <QIntValidator>

namespace fxcalc {
	Form::Form(QWidget* parent): QWidget(parent) {
		// create form fields
		edit_account_balance_        = new QLineEdit;
		edit_risk_percent_           = new QLineEdit;
		edit_sl_pips_                = new QLineEdit;
		edit_units_                  = new QLineEdit;
		edit_lots_                   = new QLineEdit;
		edit_margin_ratio_           = new QLineEdit;
		edit_commission_             = new QLineEdit;
		edit_instrument_rate_        = new QLineEdit;
		edit_margin_instrument_rate_ = new QLineEdit;
		cb_account_currency_         = new QComboBox;
		cb_instrument_               = new QComboBox;
		label_result_risk_           = new QLabel;
		label_pip_value_             = new QLabel;
		label_margin_required_       = new QLabel;
		label_instrument_rate_       = new QLabel(tr("Current ask"));
		label_margin_instrument_     = new QLabel;
		label_commission_            = new QLabel;		
		btn_units_clipboard_         = new QPushButton(tr("Copy"));
		btn_lots_clipboard_          = new QPushButton(tr("Copy"));

		// set alignment
		edit_account_balance_->setAlignment(Qt::AlignRight);
		edit_risk_percent_->setAlignment(Qt::AlignRight);
		edit_sl_pips_->setAlignment(Qt::AlignRight);
		edit_units_->setAlignment(Qt::AlignRight);
		edit_lots_->setAlignment(Qt::AlignRight);
		edit_margin_ratio_->setAlignment(Qt::AlignRight);
		edit_commission_->setAlignment(Qt::AlignRight);
		edit_instrument_rate_->setAlignment(Qt::AlignRight);
		edit_margin_instrument_rate_->setAlignment(Qt::AlignRight);

		label_result_risk_->setAlignment(Qt::AlignRight);
		label_pip_value_->setAlignment(Qt::AlignRight);
		label_margin_required_->setAlignment(Qt::AlignRight);
		label_commission_->setAlignment(Qt::AlignRight);

		edit_units_->setReadOnly(true);
		edit_lots_->setReadOnly(true);

		// set validators and field policies
		cb_account_currency_->setInsertPolicy( QComboBox::NoInsert );
		cb_instrument_->setInsertPolicy( QComboBox::NoInsert );

		auto balance_validator    = new QDoubleValidator(0, 999999999, 2, edit_account_balance_ );
		balance_validator->setNotation( QDoubleValidator::StandardNotation );
		balance_validator->setLocale(QLocale::c());

		auto commission_validator = new QDoubleValidator(0, 99999, 2, edit_commission_ );
		commission_validator->setNotation( QDoubleValidator::StandardNotation );
		commission_validator->setLocale(QLocale::c());

		auto risk_validator       = new QDoubleValidator(0, 100, 2, edit_risk_percent_ );
		risk_validator->setNotation( QDoubleValidator::StandardNotation );
		risk_validator->setLocale(QLocale::c());

		auto rate_validator       = new QDoubleValidator(0, 99999999, 5 );
		rate_validator->setNotation( QDoubleValidator::StandardNotation );
		rate_validator->setLocale(QLocale::c());

		edit_instrument_rate_->setValidator( rate_validator );

		auto pip_validator        = new QIntValidator(0, 999999 );
		edit_sl_pips_->setValidator( pip_validator );

		// create form labels
		QLabel* label_account_balance   = new QLabel(tr("Account Balance"));
		QLabel* label_account_currency  = new QLabel(tr("Account Denomination"));
		QLabel* label_risk_percent      = new QLabel(tr("Risk, %"));
		QLabel* label_pips              = new QLabel(tr("Stop loss, pips"));
		QLabel* label_instrument        = new QLabel(tr("Instrument"));
		QLabel* label_units             = new QLabel(tr("Units"));
		QLabel* label_lots              = new QLabel(tr("Lots"));
		QLabel* label_result_risk       = new QLabel(tr("Risk"));
		QLabel* label_result_profit     = new QLabel(tr("Profit"));
		QLabel* label_margin_required   = new QLabel(tr("Margin Required"));
		QLabel* label_margin_ratio      = new QLabel(tr("Margin Ratio n:1"));
		QLabel* label_pip_value         = new QLabel(tr("Pip Value"));
		QLabel* label_commission        = new QLabel(tr("Commission per 1k Lot"));
		QLabel* label_commissions       = new QLabel(tr("Commission open+close"));

		// create groups
		QGroupBox* group_inputs   = new QGroupBox;
		QGroupBox* group_pos_size = new QGroupBox(tr("Position Size"));
		QGroupBox* group_margin   = new QGroupBox(tr("Margin Requirements"));

		// create layouts
		QGridLayout* layout_inputs   = new QGridLayout;
		QGridLayout* layout_pos_size = new QGridLayout;
		QGridLayout* layout_margin   = new QGridLayout;
		layout_inputs->setColumnMinimumWidth(0, 150);
		layout_pos_size->setColumnMinimumWidth(0, 150);
		layout_margin->setColumnMinimumWidth(0, 150);

		// create clipboard layouts
		QHBoxLayout* layout_copy_units    = new QHBoxLayout;
		QHBoxLayout* layout_copy_lots     = new QHBoxLayout;
		QHBoxLayout* layout_pips          = new QHBoxLayout;

		// combine clipboard button with line edits
		layout_copy_units->addWidget(btn_units_clipboard_);
		layout_copy_units->addWidget(edit_units_);
		layout_copy_lots->addWidget(btn_lots_clipboard_);
		layout_copy_lots->addWidget(edit_lots_);

		// set group layouts
		group_inputs->setLayout( layout_inputs );
		group_pos_size->setLayout( layout_pos_size );
		group_margin->setLayout( layout_margin );

		// add form rows and columns
		// - inputs
		layout_inputs->addWidget(label_account_currency, 0, 0);
		layout_inputs->addWidget(cb_account_currency_, 0, 1);
		layout_inputs->addWidget(label_account_balance, 1, 0);
		layout_inputs->addWidget(edit_account_balance_, 1, 1);
		layout_inputs->addWidget(label_risk_percent, 2, 0);
		layout_inputs->addWidget(edit_risk_percent_, 2, 1);
		layout_inputs->addWidget(label_pips, 3, 0);
		layout_inputs->addWidget(edit_sl_pips_, 3, 1);
		layout_inputs->addWidget(label_commission, 4, 0);
		layout_inputs->addWidget(edit_commission_, 4, 1);
		layout_inputs->addWidget(label_instrument, 5, 0);
		layout_inputs->addWidget(cb_instrument_, 5, 1);
		layout_inputs->addWidget(label_instrument_rate_, 6, 0);
		layout_inputs->addWidget(edit_instrument_rate_, 6, 1);
		// - pos_size
		layout_pos_size->addWidget(label_pip_value, 0, 0);
		layout_pos_size->addWidget(label_pip_value_, 0, 1);
		layout_pos_size->addWidget(label_result_risk, 1, 0);
		layout_pos_size->addWidget(label_result_risk_, 1, 1);
		layout_pos_size->addWidget(label_commissions, 2, 0);
		layout_pos_size->addWidget(label_commission_, 2, 1);
		layout_pos_size->addWidget(label_units, 3, 0);
		layout_pos_size->addLayout(layout_copy_units, 3, 1);
		layout_pos_size->addWidget(label_lots, 4, 0);
		layout_pos_size->addLayout(layout_copy_lots, 4, 1);
		// - margins
		layout_margin->addWidget(label_margin_ratio, 0, 0);
		layout_margin->addWidget(edit_margin_ratio_, 0, 1);
		layout_margin->addWidget(label_margin_instrument_, 1, 0);
		layout_margin->addWidget(edit_margin_instrument_rate_, 1, 1);
		layout_margin->addWidget(label_margin_required, 2, 0);
		layout_margin->addWidget(label_margin_required_, 2, 1);

		// create main layout
		QVBoxLayout* layout_main = new QVBoxLayout;
		layout_main->addWidget( group_inputs );
		layout_main->addWidget( group_pos_size );
		layout_main->addWidget( group_margin );

		setLayout( layout_main );
	}

	QLineEdit* Form::editAccountBalance() {
		return edit_account_balance_;
	}

	QLineEdit* Form::editRiskPercent() {
		return edit_risk_percent_;
	}

	QLineEdit* Form::editSLPips() {
		return edit_sl_pips_;
	}

	QLineEdit* Form::editUnits() {
		return edit_units_;
	}

	QLineEdit* Form::editLots() {
		return edit_lots_;
	}

	QLineEdit* Form::editMarginRatio() {
		return edit_margin_ratio_;
	}

	QLineEdit* Form::editCommission() {
		return edit_commission_;
	}

	QLineEdit* Form::editInstrumentRate() {
		return edit_instrument_rate_;
	}

	QLineEdit* Form::editMarginInstrumentRate() {
		return edit_margin_instrument_rate_;
	}

	QComboBox* Form::cbInstrument() {
		return cb_instrument_;
	}

	QComboBox* Form::cbAccountCurrency() {
		return cb_account_currency_;
	}

	QLabel* Form::labelResultRisk() {
		return label_result_risk_;
	}

	QLabel* Form::labelPipValue() {
		return label_pip_value_;
	}

	QLabel* Form::labelMarginRequired() {
		return label_margin_required_;
	}

	QLabel* Form::labelInstrumentRate() {
		return label_instrument_rate_;
	}

	QLabel* Form::labelMarginInstrument() {
		return label_margin_instrument_;
	}

	QLabel* Form::labelCommission() {
		return label_commission_;
	}

	QPushButton* Form::btnCopyUnits() {
		return btn_units_clipboard_;
	}

	QPushButton* Form::btnCopyLots() {
		return btn_lots_clipboard_;
	}

};