/*
 * Copyright (C) 2004-2008 Geometer Plus <contact@geometerplus.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include <QtGui/QApplication>
#include <QtGui/QDesktopWidget>
#include <QtGui/QLayout>
#include <QtGui/QPushButton>
#include <QtGui/QButtonGroup>
#include <QtGui/QResizeEvent>

#include <ZLDialogManager.h>

#include "ZLQtOptionsDialog.h"
#include "ZLQtDialogContent.h"
#include "ZLQtUtil.h"

ZLQtOptionsDialog::ZLQtOptionsDialog(const ZLResource &resource, shared_ptr<ZLRunnable> applyAction, bool showApplyButton) : QDialog(qApp->activeWindow()), ZLDesktopOptionsDialog(resource, applyAction) {
	setModal(true);
	setWindowTitle(::qtString(caption()));
	QVBoxLayout *layout = new QVBoxLayout(this);

	myTabWidget = new QTabWidget(this);
	layout->addWidget(myTabWidget);

	QWidget *group = new QWidget(this);
	layout->addWidget(group);
	QGridLayout *buttonLayout = new QGridLayout(group);
	buttonLayout->setColumnStretch(0, 3);

	QPushButton *okButton = new QPushButton(group);
	okButton->setText(::qtButtonName(ZLDialogManager::OK_BUTTON));
	buttonLayout->addWidget(okButton, 0, 1);
	connect(okButton, SIGNAL(clicked()), this, SLOT(accept()));

	QPushButton *cancelButton = new QPushButton(group);
	cancelButton->setText(::qtButtonName(ZLDialogManager::CANCEL_BUTTON));
	buttonLayout->addWidget(cancelButton, 0, 2);
	connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));

	if (showApplyButton) {
		QPushButton *applyButton = new QPushButton(group);
		applyButton->setText(::qtButtonName(ZLDialogManager::APPLY_BUTTON));
		buttonLayout->addWidget(applyButton, 0, 3);
		connect(applyButton, SIGNAL(clicked()), this, SLOT(apply()));
	}

	if (parent() == 0) {
		QDesktopWidget *desktop = qApp->desktop();
		if (desktop != 0) {
			move((desktop->width() - width()) / 2, (desktop->height() - height()) / 2);
		}
	}
}

void ZLQtOptionsDialog::apply() {
	ZLOptionsDialog::accept();
}

ZLDialogContent &ZLQtOptionsDialog::createTab(const ZLResourceKey &key) {
	ZLQtDialogContent *tab = new ZLQtDialogContent(new QWidget(myTabWidget), tabResource(key));
	myTabWidget->addTab(tab->widget(), ::qtString(tab->displayName()));
	myTabs.push_back(tab);
	return *tab;
}

const std::string &ZLQtOptionsDialog::selectedTabKey() const {
	return myTabs[myTabWidget->currentIndex()]->key();
}

void ZLQtOptionsDialog::selectTab(const ZLResourceKey &key) {
	for (std::vector<shared_ptr<ZLDialogContent> >::const_iterator it = myTabs.begin(); it != myTabs.end(); ++it) {
		if ((*it)->key() == key.Name) {
			myTabWidget->setCurrentWidget(((ZLQtDialogContent&)**it).widget());
			break;
		}
	}
}

bool ZLQtOptionsDialog::runInternal() {
	for (std::vector<shared_ptr<ZLDialogContent> >::iterator it = myTabs.begin(); it != myTabs.end(); ++it) {
		((ZLQtDialogContent&)**it).close();
	}
	return exec() == QDialog::Accepted;
}
