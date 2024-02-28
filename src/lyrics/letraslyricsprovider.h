/*
 * Strawberry Music Player
 * Copyright 2024, Jonas Kvinge <jonas@jkvinge.net>
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

#ifndef LETRASLYRICSPROVIDER_H
#define LETRASLYRICSPROVIDER_H

#include <QtGlobal>
#include <QObject>
#include <QList>
#include <QVariant>
#include <QString>
#include <QUrl>

#include "core/shared_ptr.h"
#include "core/networkaccessmanager.h"
#include "htmllyricsprovider.h"
#include "lyricssearchrequest.h"

class LetrasLyricsProvider : public HtmlLyricsProvider {
  Q_OBJECT

 public:
  explicit LetrasLyricsProvider(SharedPtr<NetworkAccessManager> network, QObject *parent = nullptr);

 protected:
  QUrl Url(const LyricsSearchRequest &request) override;

 private:
  QString StringFixup(const QString &text);

 private:
  static const char kUrl[];
  static const char kStartTag[];
  static const char kEndTag[];
  static const char kLyricsStart[];
};

#endif  // LETRASLYRICSPROVIDER_H
