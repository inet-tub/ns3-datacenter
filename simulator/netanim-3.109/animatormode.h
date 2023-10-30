/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: John Abraham <john.abraham.in@gmail.com>
 * Contributions: Eugene Kalishenko <ydginster@gmail.com> (Open Source and Linux Laboratory http://dev.osll.ru/)
 * 		  Dmitrii Shakshin <d.shakshin@gmail.com> (Open Source and Linux Laboratory http://dev.osll.ru/)
 *        Makhtar Diouf <makhtar.diouf@gmail.com>
 */

#ifndef AnimatorPlugin_H
#define AnimatorPlugin_H

#include "common.h"
#include "animatorconstants.h"
#include "animatorscene.h"
#include "animatorview.h"
#include "mode.h"
#include "timevalue.h"
#include "animevent.h"
#include "QtTreePropertyBrowser"

namespace netanim
{


typedef struct {
  QString fileName;
  qreal x;
  qreal y;
  qreal scaleX;
  qreal scaleY;
  qreal opacity;

} BackgroudImageProperties_t;

class AnimatorMode: public Mode
{
  Q_OBJECT

public:
  // Getters

  static AnimatorMode * getInstance ();
  QWidget * getCentralWidget ();
  QString getTabName ();
  qreal getCurrentNodeSize ();
  QGraphicsPixmapItem * getBackground ();
  TimeValue<AnimEvent *>* getEvents ();
  qreal getLastPacketEventTime ();
  qreal getThousandthPacketTime ();
  qreal getFirstPacketTime ();

  // Setters

  void setParsingCount (uint64_t parsingCount);
  void setVersion (double version);
  void setWPacketDetected ();
  void setFocus (bool focus);
  void setCurrentTime (qreal currentTime);
  void addAnimEvent (qreal t, AnimEvent *);
  void setNodeSize (AnimNode * animNode, qreal size);
  void setNodePos (AnimNode * animNode, qreal x, qreal y);
  void setNodeResource (AnimNode * animNode, uint32_t resourceId);
  void setNodeSysId (AnimNode * animNode, uint32_t sysId);
  void setShowNodeTrajectory (AnimNode * animNode);
  void setBackgroundImageProperties (BackgroudImageProperties_t prop);
  BackgroudImageProperties_t getBackgroundProperties ();

  // Actions

  bool keepAppResponsive ();
  void showPopup (QString msg);
  void externalPauseEvent ();
  void start ();
  void openPropertyBroswer ();

private:

  // state
  typedef enum
  {
    APP_INIT,
    APP_START,
    SYSTEM_RESET_IN_PROGRESS,
    SYSTEM_RESET_COMPLETE,
    PLAYING,
    PAUSING,
    SIMULATION_COMPLETE
  } AnimatorModeState_t;
  double m_version;
  bool m_playing;
  AnimatorModeState_t m_state;
  QTimer * m_updateRateTimer;
  double m_currentTime;
  qreal m_currentZoomFactor;
  bool m_showWiressCircles;
  double m_updateRates[UPDATE_RATE_SLIDER_MAX];
  double m_currentUpdateRate;
  double m_parsedMaxSimulationTime;
  int m_oldTimelineValue;
  QVector <QWidget *> m_toolButtonVector;
  QElapsedTimer m_appResponsiveTimer;
  bool m_simulationCompleted;
  uint64_t m_rxCount;
  TimeValue<AnimEvent *> m_events;
  bool m_showPacketMetaInfo;
  QString m_traceFileName;
  bool m_showPackets;
  bool m_fastForwarding;
  qreal m_lastPacketEventTime;
  qreal m_firstPacketEventTime;
  std::map <AnimPacket *, AnimPacket *> m_wiredPacketsToAnimate;
  std::map <AnimPacket *, AnimPacket *> m_wirelessPacketsToAnimate;
  qreal m_thousandthPacketTime;
  qreal m_pauseAtTime;
  bool m_pauseAtTimeTriggered;
  BackgroudImageProperties_t m_backgroundImageProperties;
  QPointF m_minPoint;
  QPointF m_maxPoint;
  bool m_backgroundExists;




  //controls
  QVBoxLayout * m_vLayout;
  QLabel * m_gridLinesLabel;
  QLabel * m_nodeSizeLabel;
  QToolButton * m_gridButton;
  QToolButton * m_batteryCapacityButton;
  QSpinBox * m_gridLinesSpinBox;
  QComboBox * m_nodeSizeComboBox;
  QToolButton * m_testButton;
  QToolButton * m_showIpButton;
  QToolButton * m_showMacButton;
  QToolButton * m_showNodeIdButton;
  QToolButton * m_showNodeSysIdButton;
  QToolButton * m_playButton;
  QToolButton * m_fileOpenButton;
  QToolButton * m_reloadFileButton;
  QToolButton * m_zoomInButton;
  QToolButton * m_zoomOutButton;
  QToolButton * m_showWirelessCirclesButton;
  QSlider * m_updateRateSlider;
  QLabel * m_fastRateLabel;
  QLabel * m_slowRateLabel;
  QLCDNumber * m_qLcdNumber;
  QWidget * m_centralWidget;
  QDialog * m_parsingXMLDialog;
  QDialog * m_transientDialog;
  QToolBar * m_topToolBar;
  QToolButton * m_packetStatsButton;
  QSplitter * m_mainSplitter;
  QToolButton * m_nodeTrajectoryButton;
  QLabel * m_timelineSliderLabel;
  QToolBar * m_verticalToolbar;
  QLabel * m_pktFilterFromLabel;
  QComboBox * m_pktFilterFromComboBox;
  QLabel * m_pktFilterToLabel;
  QComboBox * m_pktFilterToComboBox;
  QToolButton * m_blockPacketsButton;
  QToolBar * m_bottomToolbar;
  QLabel * m_bottomStatusLabel;
  QToolButton * m_resetButton;
  QToolButton * m_showMetaButton;
  QProgressBar * m_parseProgressBar;
  QSlider * m_simulationTimeSlider;
  QToolButton * m_showRoutePathButton;
  QToolButton * m_showPropertiesButton;
  QParallelAnimationGroup * m_buttonAnimationGroup;
  QLabel * m_pauseAtLabel;
  QLineEdit * m_pauseAtEdit;
  QToolButton * m_stepButton;
  QToolButton * m_mousePositionButton;



  //functions
  AnimatorMode ();
  bool parseXMLTraceFile (QString traceFileName);
  void setLabelStyleSheet ();
  void initUpdateRate ();
  void enableAllToolButtons (bool show);
  qreal nodeSizeStringToValue (QString nodeSize);
  void systemReset ();
  void preParse ();
  void postParse ();
  void initToolbars ();
  void initLabels ();
  void initControls ();
  void setTopToolbarWidgets ();
  void setVerticalToolbarWidgets ();
  void setBottomToolbarWidgets ();
  void setToolButtonVector ();
  void setControlDefaults ();
  bool checkSimulationCompleted ();
  void doSimulationCompleted ();
  void timerCleanup ();
  void showParsingXmlDialog (bool show);
  void showTransientDialog (bool show, QString msg = "");
  void setProgressBarRange (uint64_t rxCount);
  void init ();
  void showAnimatorView (bool show);
  void showPackets (bool show);
  void setMaxSimulationTime (double maxTime);
  void resetBackground ();
  void displayPacket (qreal t);
  void dispatchEvents ();
  void setSimulationCompleted ();
  void purgeWiredPackets (bool sysReset = false);
  void purgeWirelessPackets ();
  void purgeAnimatedNodes ();
  void fastForward (qreal t);
  void reset ();
  QPropertyAnimation * getButtonAnimation (QToolButton * toolButton);
  void initPropertyBrowser ();
  void removeWiredPacket (AnimPacket * animPacket);


private slots:
  void testSlot ();
  void clickTraceFileOpenSlot ();
  void reloadFileSlot ();
  void clickZoomInSlot ();
  void clickZoomOutSlot ();
  void clickResetSlot ();
  void clickPlaySlot ();
  void simulationSliderPressedSlot ();
  void updateTimelineSlot (int value);
  void updateTimelineSlot ();
  void updateRateTimeoutSlot ();
  void updateGridLinesSlot (int value);
  void updateNodeSizeSlot (QString value);
  void updateUpdateRateSlot (int);
  void showGridLinesSlot ();
  void showNodeIdSlot ();
  void showNodeSysIdSlot ();
  void showMetaSlot ();
  void showPacketSlot ();
  void showWirelessCirclesSlot ();
  void showPacketStatsSlot ();
  void showNodePositionStatsSlot ();
  void showIpSlot ();
  void showMacSlot ();
  void showRoutePathSlot ();
  void showBatteryCapacitySlot ();
  void buttonAnimationGroupFinishedSlot ();
  void showPropertiesSlot ();
  void pauseAtTimeSlot ();
  void stepSlot ();
  void enableMousePositionSlot ();
};


} // namespace netanim

#endif // AnimatorPlugin_H
