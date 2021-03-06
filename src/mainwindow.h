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

#pragma once

#include <QMainWindow>
#include <QString>

#include <map>

#include "form.h"

namespace fxcalc {
class MainWindow: public QMainWindow {
	Q_OBJECT

public:
	enum CalcMode {
		NORMAL = 0,
		TP_RATE,
		TP_PIPS
	};

	MainWindow();

public slots:
	void calculate();

private:
	void initForm();
	void save();
	void load();

	CalcMode calc_mode_;
	Form* form_;
	std::map<QString, int> currency_priority_;
};	
};
