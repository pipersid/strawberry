/*
 * Strawberry Music Player
 * This file was part of Clementine.
 * Copyright 2010, David Sansome <me@davidsansome.com>
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

#ifndef ORGANISEDIALOG_H
#define ORGANISEDIALOG_H

#include "config.h"

#include <memory>
#include <stdbool.h>

#include <QtGlobal>
#include <QObject>
#include <QWidget>
#include <QDialog>
#include <QAbstractItemModel>
#include <QFuture>
#include <QList>
#include <QSize>
#include <QString>
#include <QStringList>
#include <QUrl>
#include <QtEvents>

#include "core/song.h"
#include "organise.h"
#include "organiseformat.h"

class QResizeEvent;
class QShowEvent;

class TaskManager;
class OrganiseErrorDialog;
class Ui_OrganiseDialog;

class OrganiseDialog : public QDialog {
  Q_OBJECT

 public:
  OrganiseDialog(TaskManager *task_manager, QWidget *parent = nullptr);
  ~OrganiseDialog();

  static const char *kDefaultFormat;
  static const char *kSettingsGroup;

  QSize sizeHint() const;

  void SetDestinationModel(QAbstractItemModel *model, bool devices = false);

  // These functions return true if any songs were actually added to the dialog.
  // SetSongs returns immediately, SetUrls and SetFilenames load the songs in the background.
  bool SetSongs(const SongList &songs);
  bool SetUrls(const QList<QUrl> &urls);
  bool SetFilenames(const QStringList &filenames);

  void SetCopy(bool copy);

 signals:
  void FileCopied(int);

 public slots:
  void accept();

 protected:
  void showEvent(QShowEvent *);
  void resizeEvent(QResizeEvent *);

 private slots:
  void Reset();

  void InsertTag(const QString &tag);
  void UpdatePreviews();

  void OrganiseFinished(const QStringList files_with_errors, const QStringList log);

 private:
  SongList LoadSongsBlocking(const QStringList &filenames);
  void SetLoadingSongs(bool loading);

  static Organise::NewSongInfoList ComputeNewSongsFilenames(const SongList &songs, const OrganiseFormat &format);

  Ui_OrganiseDialog *ui_;
  TaskManager *task_manager_;

  OrganiseFormat format_;

  QFuture<SongList> songs_future_;
  SongList songs_;
  Organise::NewSongInfoList new_songs_info_;
  quint64 total_size_;

  std::unique_ptr<OrganiseErrorDialog> error_dialog_;

  bool resized_by_user_;
};

#endif  // ORGANISEDIALOG_H
