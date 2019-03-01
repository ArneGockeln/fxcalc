#pragma once

#include <QMainWindow>
#include <QString>

#include <map>

#include "fixer.h"
#include "form.h"

#define FIXER_API_URL "https://api.exchangeratesapi.io/"

namespace poscalc {
class MainWindow: public QMainWindow {
	Q_OBJECT

public:
	MainWindow();

public slots:
	void calculate();
	void fetchExchangeRate();
	void onAccountCurrencyChange();

private:
	void initForm();
	void save();
	void load();

	bool use_custom_rate_;
	Form* form_;	
	Fixer* api_;
	std::map<QString, double> rates_;
	std::map<QString, int> currency_priority_;
};	
};
