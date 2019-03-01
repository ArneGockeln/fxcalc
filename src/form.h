#pragma once

#include <QWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QLabel>
#include <QPushButton>

namespace poscalc {
	class Form: public QWidget {
		Q_OBJECT 

	public:
		Form(QWidget* parent = 0);

		QLineEdit* editAccountBalance();
		QLineEdit* editRiskPercent();
		QLineEdit* editSLPips();
		QLineEdit* editMarginRatio();
		QLineEdit* editUnits();
		QLineEdit* editLots();
		QLineEdit* editCommission();
		QLineEdit* editInstrumentRate();
		QLineEdit* editMarginInstrumentRate();
		QComboBox* cbInstrument();
		QComboBox* cbAccountCurrency();
		QLabel* labelResultRisk();
		QLabel* labelMarginRequired();
		QLabel* labelMarginInstrument();
		QLabel* labelPipValue();
		QLabel* labelInstrumentRate();
		QPushButton* btnRefreshRates();
		QPushButton* btnCopyUnits();
		QPushButton* btnCopyLots();

	private:
		QLineEdit* edit_account_balance_;
		QLineEdit* edit_risk_percent_;
		QLineEdit* edit_sl_pips_;
		QLineEdit* edit_margin_ratio_;
		QLineEdit* edit_units_;
		QLineEdit* edit_lots_;
		QLineEdit* edit_commission_;
		QLineEdit* edit_instrument_rate_;
		QLineEdit* edit_margin_instrument_rate_;
		QComboBox* cb_account_currency_;
		QComboBox* cb_instrument_;
		QLabel* label_result_risk_;
		QLabel* label_margin_required_;
		QLabel* label_pip_value_;
		QLabel* label_instrument_rate_;
		QLabel* label_margin_instrument_;
		QPushButton* btn_refresh_rates_;
		QPushButton* btn_units_clipboard_;
		QPushButton* btn_lots_clipboard_;

	};
};