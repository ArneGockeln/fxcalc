#pragma once

#include <QMainWindow>
#include <QWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QLabel>

#include <map>

#include "fixer.h"

#define FIXER_API_URL "https://api.exchangeratesapi.io/"

namespace poscalc {
class MainWindow: public QMainWindow {
	Q_OBJECT

public:
	MainWindow();

public slots:
	void calculate();
	void fetchExchangeRate();

	void onRiskChange();
	void onSLChange();
	void onAccountCurrencyChange();
	void onAccountBalanceChange();
	void onInstrumentChange();

private:
	void initForm();
	void save();
	void load();

	QLineEdit* m_edit_account_balance;
	QLineEdit* m_edit_risk_percent;
	QLineEdit* m_edit_sl_pips;
	QComboBox* m_cb_account_currency;
	QComboBox* m_cb_instrument;
	QLabel* m_label_current_ask_price;
	QLabel* m_label_result_risk;
	QLineEdit* m_edit_units;
	QLineEdit* m_edit_lots;
	QPushButton* m_btn_refresh_rates;
	Fixer* m_api;
	std::map<QString, double> m_rates;
	std::map<QString, int> m_currency_priority;
};	
};
