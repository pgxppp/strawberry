/***************************************************************************
 *   Copyright (C) 2003-2005 by Mark Kretschmann <markey@web.de>           *
 *   Copyright (C) 2005 by Jakub Stachowski <qbast@go2.pl>                 *
 *   Copyright (C) 2006 Paul Cifarelli <paul@cifarelli.net>       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Steet, Fifth Floor, Boston, MA  02111-1307, USA.          *
 ***************************************************************************/

#ifndef GSTENGINE_H
#define GSTENGINE_H

#include "config.h"

#include <memory>
#include <stdbool.h>

#include <gst/gst.h>

#include <QtGlobal>
#include <QObject>
#include <QFuture>
#include <QByteArray>
#include <QList>
#include <QVariant>
#include <QString>
#include <QUrl>
#include <QTimer>
#include <QTimerEvent>

#include "core/timeconstants.h"
#include "engine_fwd.h"
#include "enginebase.h"
#include "gstbufferconsumer.h"

class TaskManager;
class GstEnginePipeline;

#ifdef Q_OS_DARWIN
struct _GTlsDatabase;
typedef struct _GTlsDatabase GTlsDatabase;
#endif

/**
 * @class GstEngine
 * @short GStreamer engine plugin
 * @author Mark Kretschmann <markey@web.de>
 */
class GstEngine : public Engine::Base, public GstBufferConsumer {
  Q_OBJECT

 public:
  GstEngine(TaskManager *task_manager);
  ~GstEngine();

  static const char *kAutoSink;
  static const char *kALSASink;
  static const char *kOSSSink;
  static const char *kOSS4Sink;
  static const char *kJackAudioSink;
  static const char *kPulseSink;
  static const char *kA2DPSink;
  static const char *kAVDTPSink;

  bool Init();
  void EnsureInitialised() { initialising_.waitForFinished(); }
  void InitialiseGStreamer();
  void SetEnvironment();
  
  OutputDetailsList GetOutputsList() const;

  qint64 position_nanosec() const;
  qint64 length_nanosec() const;
  Engine::State state() const;
  const Engine::Scope &scope(int chunk_length);

  static bool ALSADeviceSupport(const QString &name);
  static bool PulseDeviceSupport(const QString &name);

  GstElement *CreateElement(const QString &factoryName, GstElement *bin = 0, bool fatal = true, bool showerror = true);

  // BufferConsumer
  void ConsumeBuffer(GstBuffer *buffer, int pipeline_id);
  
  bool IsEqualizerEnabled() { return equalizer_enabled_; }

 public slots:
  void StartPreloading(const QUrl &url, bool force_stop_at_end, qint64 beginning_nanosec, qint64 end_nanosec);
  bool Load(const QUrl &, Engine::TrackChangeFlags change, bool force_stop_at_end, quint64 beginning_nanosec, qint64 end_nanosec);
  bool Play(quint64 offset_nanosec);
  void Stop(bool stop_after = false);
  void Pause();
  void Unpause();
  void Seek(quint64 offset_nanosec);

  /** Set whether equalizer is enabled */
  void SetEqualizerEnabled(bool);

  /** Set equalizer preamp and gains, range -100..100. Gains are 10 values. */
  void SetEqualizerParameters(int preamp, const QList<int> &bandGains);

  /** Set Stereo balance, range -1.0f..1.0f */
  void SetStereoBalance(float value);

  void ReloadSettings();

  void AddBufferConsumer(GstBufferConsumer *consumer);
  void RemoveBufferConsumer(GstBufferConsumer *consumer);

#ifdef Q_OS_DARWIN
  GTlsDatabase *tls_database() const { return tls_database_; }
#endif

 protected:
  void SetVolumeSW(uint percent);
  void timerEvent(QTimerEvent*);

 private slots:
  void EndOfStreamReached(int pipeline_id, bool has_next_track);
  void HandlePipelineError(int pipeline_id, const QString &message, int domain, int error_code);
  void NewMetaData(int pipeline_id, const Engine::SimpleMetaBundle &bundle);
  void AddBufferToScope(GstBuffer *buf, int pipeline_id);
  void FadeoutFinished();
  void FadeoutPauseFinished();
  void SeekNow();
  void PlayDone(QFuture<GstStateChangeReturn> future, const quint64, const int);

  void BufferingStarted();
  void BufferingProgress(int percent);
  void BufferingFinished();

 private:
  PluginDetailsList GetPluginList(const QString &classname) const;

  void StartFadeout();
  void StartFadeoutPause();

  void StartTimers();
  void StopTimers();

  std::shared_ptr<GstEnginePipeline> CreatePipeline();
  std::shared_ptr<GstEnginePipeline> CreatePipeline(const QUrl &url, qint64 end_nanosec);
  std::shared_ptr<GstEnginePipeline> CreatePipeline(const QByteArray &url, qint64 end_nanosec);

  void UpdateScope(int chunk_length);
  
  QByteArray FixupUrl(const QUrl &url);

 private:
  static const qint64 kTimerIntervalNanosec = 1000  *kNsecPerMsec;  // 1s
  static const qint64 kPreloadGapNanosec = 3000  *kNsecPerMsec;     // 3s
  static const qint64 kSeekDelayNanosec = 100  *kNsecPerMsec;       // 100msec

  TaskManager *task_manager_;
  int buffering_task_id_;

  QFuture<void> initialising_;

  QString sink_;
  QVariant device_;

  std::shared_ptr<GstEnginePipeline> current_pipeline_;
  std::shared_ptr<GstEnginePipeline> fadeout_pipeline_;
  std::shared_ptr<GstEnginePipeline> fadeout_pause_pipeline_;
  QUrl preloaded_url_;

  QList<GstBufferConsumer*> buffer_consumers_;

  GstBuffer *latest_buffer_;

  bool equalizer_enabled_;
  int equalizer_preamp_;
  QList<int> equalizer_gains_;
  float stereo_balance_;

  bool rg_enabled_;
  int rg_mode_;
  float rg_preamp_;
  bool rg_compression_;

  qint64 buffer_duration_nanosec_;

  int buffer_min_fill_;

  bool mono_playback_;

  mutable bool can_decode_success_;
  mutable bool can_decode_last_;

  // Hack to stop seeks happening too often
  QTimer *seek_timer_;
  bool waiting_to_seek_;
  quint64 seek_pos_;

  int timer_id_;
  int next_element_id_;

  bool is_fading_out_to_pause_;
  bool has_faded_out_;

  int scope_chunk_;
  bool have_new_buffer_;
  int scope_chunks_;

};

#endif /* GSTENGINE_H */
