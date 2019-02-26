#pragma once

#include <QMainWindow>
#include <QWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QLabel>

#include "fixer.h"

#define FIXER_API_URL "http://data.fixer.io/api/"

namespace poscalc {
class MainWindow: public QMainWindow {
	Q_OBJECT

public:
	MainWindow();

public slots:
	void calculate();
	void fetchExchangeRate();

private:
	void checkAPIKey();
	void initForm();

	QLineEdit* m_edit_account_size;
	QLineEdit* m_edit_risk_percent;
	QLineEdit* m_edit_sl_pips;
	QComboBox* m_cb_account_currency;
	QComboBox* m_cb_instrument;
	QLabel* m_label_current_ask_price;
	QLabel* m_label_result_risk;
	QLabel* m_label_units;
	QLabel* m_label_lots;
	QString m_api_key;
	Fixer* m_api;
};	
};
