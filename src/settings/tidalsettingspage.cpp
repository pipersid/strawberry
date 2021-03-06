/*
 * Strawberry Music Player
 * Copyright 2018, Jonas Kvinge <jonas@jkvinge.net>
 *
 * Strawberry is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Strawberry is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Strawberry.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "config.h"

#include <QObject>
#include <QString>
#include <QSettings>
#include <QMessageBox>
#include <QEvent>

#include "tidalsettingspage.h"
#include "ui_tidalsettingspage.h"
#include "core/application.h"
#include "core/iconloader.h"
#include "internet/internetservices.h"
#include "tidal/tidalservice.h"

const char *TidalSettingsPage::kSettingsGroup = "Tidal";

TidalSettingsPage::TidalSettingsPage(SettingsDialog *parent)
    : SettingsPage(parent),
      ui_(new Ui::TidalSettingsPage),
      service_(dialog()->app()->internet_services()->Service<TidalService>()) {

  ui_->setupUi(this);
  setWindowIcon(IconLoader::Load("tidal"));

  connect(ui_->button_login, SIGNAL(clicked()), SLOT(LoginClicked()));
  connect(ui_->login_state, SIGNAL(LogoutClicked()), SLOT(LogoutClicked()));

  connect(this, SIGNAL(Login(QString, QString)), service_, SLOT(SendLogin(QString, QString)));

  connect(service_, SIGNAL(LoginFailure(QString)), SLOT(LoginFailure(QString)));
  connect(service_, SIGNAL(LoginSuccess()), SLOT(LoginSuccess()));

  dialog()->installEventFilter(this);

  ui_->combobox_quality->addItem("Low", "LOW");
  ui_->combobox_quality->addItem("High", "HIGH");
  ui_->combobox_quality->addItem("Lossless", "LOSSLESS");

  ui_->combobox_streamurl->addItem("HTTP", "http");
  ui_->combobox_streamurl->addItem("HTTPS", "https");

  ui_->combobox_coversize->addItem("160x160", "160x160");
  ui_->combobox_coversize->addItem("320x320", "320x320");
  ui_->combobox_coversize->addItem("640x640", "640x640");
  ui_->combobox_coversize->addItem("750x750", "750x750");
  ui_->combobox_coversize->addItem("1280x1280", "1280x1280");

}

TidalSettingsPage::~TidalSettingsPage() { delete ui_; }

void TidalSettingsPage::Load() {

  QSettings s;

  s.beginGroup(kSettingsGroup);
  ui_->checkbox_enable->setChecked(s.value("enabled", false).toBool());
  ui_->username->setText(s.value("username").toString());
  QByteArray password = s.value("password").toByteArray();
  if (password.isEmpty()) ui_->password->clear();
  else ui_->password->setText(QString::fromUtf8(QByteArray::fromBase64(password)));
  dialog()->ComboBoxLoadFromSettings(s, ui_->combobox_quality, "quality", "HIGH");
  ui_->spinbox_searchdelay->setValue(s.value("searchdelay", 1500).toInt());
  ui_->spinbox_artistssearchlimit->setValue(s.value("artistssearchlimit", 5).toInt());
  ui_->spinbox_albumssearchlimit->setValue(s.value("albumssearchlimit", 100).toInt());
  ui_->spinbox_songssearchlimit->setValue(s.value("songssearchlimit", 100).toInt());
  ui_->checkbox_fetchalbums->setChecked(s.value("fetchalbums", false).toBool());
  dialog()->ComboBoxLoadFromSettings(s, ui_->combobox_coversize, "coversize", "320x320");
  dialog()->ComboBoxLoadFromSettings(s, ui_->combobox_streamurl, "streamurl", "http");
  s.endGroup();

  if (service_->authenticated()) ui_->login_state->SetLoggedIn(LoginStateWidget::LoggedIn);

}

void TidalSettingsPage::Save() {

  QSettings s;
  s.beginGroup(kSettingsGroup);
  s.setValue("enabled", ui_->checkbox_enable->isChecked());
  s.setValue("username", ui_->username->text());
  s.setValue("password", QString::fromUtf8(ui_->password->text().toUtf8().toBase64()));
  s.setValue("quality", ui_->combobox_quality->itemData(ui_->combobox_quality->currentIndex()));
  s.setValue("searchdelay", ui_->spinbox_searchdelay->value());
  s.setValue("artistssearchlimit", ui_->spinbox_artistssearchlimit->value());
  s.setValue("albumssearchlimit", ui_->spinbox_albumssearchlimit->value());
  s.setValue("songssearchlimit", ui_->spinbox_songssearchlimit->value());
  s.setValue("fetchalbums", ui_->checkbox_fetchalbums->isChecked());
  s.setValue("coversize", ui_->combobox_coversize->itemData(ui_->combobox_coversize->currentIndex()));
  s.setValue("streamurl", ui_->combobox_streamurl->itemData(ui_->combobox_streamurl->currentIndex()));
  s.endGroup();

  service_->ReloadSettings();

}

void TidalSettingsPage::LoginClicked() {
  emit Login(ui_->username->text(), ui_->password->text());
  ui_->button_login->setEnabled(false);
}

bool TidalSettingsPage::eventFilter(QObject *object, QEvent *event) {

  if (object == dialog() && event->type() == QEvent::Enter) {
    ui_->button_login->setEnabled(true);
    return false;
  }

  return SettingsPage::eventFilter(object, event);
}

void TidalSettingsPage::LogoutClicked() {
  service_->Logout();
  ui_->button_login->setEnabled(true);
  ui_->login_state->SetLoggedIn(LoginStateWidget::LoggedOut);
}

void TidalSettingsPage::LoginSuccess() {
  if (!this->isVisible()) return;
  ui_->login_state->SetLoggedIn(LoginStateWidget::LoggedIn);
  ui_->button_login->setEnabled(false);
}

void TidalSettingsPage::LoginFailure(QString failure_reason) {
  if (!this->isVisible()) return;
  QMessageBox::warning(this, tr("Authentication failed"), failure_reason);
}
