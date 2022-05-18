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
 */

#ifndef STATSMODE_H
#define STATSMODE_H

#include "common.h"
#include "mode.h"

namespace netanim
{


class NodeButton: public QPushButton
{
  Q_OBJECT
public:
  NodeButton (uint32_t nodeId);
private:
  uint32_t m_nodeId;
  void setNodeActive (bool active);
public slots:
  void buttonClickedSlot ();

};

class StatsMode: public Mode
{
  Q_OBJECT

  typedef enum
  {
    IPMAC = 0,
    Routing = 1,
    FlowMon = 2,
    CounterTables = 3
  } StatType_t;
public:

  // Getters
  static StatsMode * getInstance ();
  QWidget * getCentralWidget ();
  QString getTabName ();
  bool isNodeActive (uint32_t nodeId);
  QVector <uint32_t> stringToNodeVector (QString nodeString);
  QString nodeVectorToString (QVector<uint32_t> nodeVector);
  qreal getCurrentTime ();
  qreal getCurrentFontSize ();

  // Setters
  void setFocus (bool focus);
  void systemReset ();
  void setNodeActive (uint32_t nodeId, bool active);
  void showPopup (QString message);
  void setProgressBarRange (uint64_t rxCount);
  void setParsingCount (uint64_t parsingCount);





private:
  // state
  typedef enum
  {
    INIT,
    READY
  } StatsModeState_t;

  typedef std::vector<NodeButton *> NodeButtonVector_t;
  typedef std::map<uint32_t, bool> ActiveNodesMap_t;
  // Controls
  QWidget * m_centralWidget;
  QHBoxLayout * m_hLayout;
  QVBoxLayout * m_vLayout;
  QToolBar * m_nodeToolbar;
  QToolBar * m_topToolbar;
  QToolBar * m_bottomToolbar;
  QScrollArea * m_nodeToolbarScrollArea;
  QComboBox * m_statTypeComboBox;
  QPushButton * m_selectAllNodesButton;
  QPushButton * m_deselectAllNodesButton;
  QDialog * m_parsingXMLDialog;
  QToolButton * m_fileOpenButton;
  QProgressBar * m_parseProgressBar;
  QLabel * m_bottomStatusLabel;
  QLCDNumber * m_qLcdNumber;
  QSlider * m_simulationTimeSlider;
  QLabel * m_simTimeLabel;
  QLabel * m_fontSizeLabel;
  QSpinBox * m_fontSizeSpinBox;
  QPushButton * m_flowMonFileButton;
  QComboBox * m_counterTablesCombobox;
  QLineEdit * m_allowedNodesEdit;
  QLabel * m_allowedNodesLabel;
  QPushButton * m_showChartButton;





  NodeButtonVector_t m_nodeButtonVector;
  ActiveNodesMap_t m_activeNodes;

  // State
  uint64_t m_rtCount;
  StatType_t m_statType;
  int m_oldTimelineValue;
  double m_currentTime;
  qreal m_parsedMaxSimulationTime;
  qreal m_currentFontSize;
  bool m_showChart;
  StatsModeState_t m_state;


  StatsMode ();
  void init ();
  void initControls ();
  void initToolbars ();
  void initNodeToolbar ();
  void initTopToolbar ();
  void initBottomToolbar ();
  void addNodesToToolbar (bool zeroIndexed = true);
  bool parseRoutingXMLTraceFile (QString traceFileName);
  bool parseFlowMonXMLTraceFile (QString traceFileName);
  void showParsingXmlDialog (bool show);
  void routingPreParse ();
  void routingPostParse ();
  void flowMonPreParse ();
  void flowMonPostParse ();
  uint32_t getCurrentNodeCount ();
  void setMaxSimulationTime (double maxTime);
  void setMinSimulationTime (double minTime);
  void enableControlsForState ();
  void enableFlowMonControls (bool enable);
  void enableIpMacControls (bool enable);
  void enableCounterTables (bool enable);
  void enableRoutingStatsControls (bool enable);
  void setAvailableCounters ();




public slots:
  void testSlot ();

private slots:
  void clickRoutingTraceFileOpenSlot ();
  void selectAllNodesSlot ();
  void deselectAllNodesSlot ();
  void statTypeChangedSlot (int index);
  void updateTimelineSlot (int value);
  void fontSizeSlot (int value);
  void clickFlowMonTraceFileOpenSlot ();
  void allowedNodesChangedSlot (QString allowedNodes);
  void counterIndexChangedSlot (QString counterString);
  void showChartSlot ();




};

} // namespace netanim

#endif // STATSMODE_H
