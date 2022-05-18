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

#include "packetsmode.h"
#include "packetsview.h"
#include "packetsscene.h"
#include "animatormode.h"

#define TIME_EDIT_WIDTH 150
#define TIME_EDIT_MASK "dddd.ddddddddd"
#define ALLOWED_NODES_WITH 300
#define ALLOWED_NODES "0:1:2:3:4:5:6:7:8:9"
#define REGEX_EDIT_WIDTH 300

NS_LOG_COMPONENT_DEFINE ("PacketsMode");
namespace netanim {
PacketsMode * pPacketsMode = 0;

PacketsMode::PacketsMode ():
  m_fromTime (0),
  m_toTime (0),
  m_showGrid (false)
{
  m_mainToolBar = new QToolBar;
  m_filterToolBar = new QToolBar;
  m_testButton = new QToolButton;
  m_zoomInButton = new QToolButton;
  m_zoomInButton->setText ("Zoom In");  
  connect (m_zoomInButton, SIGNAL(clicked()), this, SLOT(zoomInSlot()));
  m_zoomInButton->setToolTip ("Zoom in");
  m_zoomInButton->setIcon (QIcon (":/resources/animator_zoomin.svg"));


  m_zoomOutButton = new QToolButton;
  m_zoomOutButton->setText ("Zoom Out");
  connect (m_zoomOutButton, SIGNAL(clicked()), this, SLOT(zoomOutSlot()));
  m_zoomOutButton->setToolTip ("Zoom Out");
  m_zoomOutButton->setIcon (QIcon (":/resources/animator_zoomout.svg"));


  m_testButton->setText ("test");
  m_showGridLinesButton = new QToolButton;  
  m_showGridLinesButton->setIcon (QIcon (":/resources/animator_grid.svg"));
  m_showGridLinesButton->setCheckable (true);
  m_showGridLinesButton->setChecked (false);
  connect (m_showGridLinesButton, SIGNAL (clicked ()), this, SLOT (showGridLinesSlot ()));



  m_fromTimeLabel = new QLabel ("From Time");
  m_fromTimeEdit = new QLineEdit;
  QDoubleValidator * doubleValidator = new QDoubleValidator (0);
  doubleValidator->setDecimals (10);
  doubleValidator->setNotation (QDoubleValidator::StandardNotation);
  m_fromTimeEdit->setValidator (doubleValidator);
  //m_fromTimeEdit->setInputMask (TIME_EDIT_MASK);
  m_fromTimeEdit->setMaximumWidth (TIME_EDIT_WIDTH);

  m_toTimeLabel = new QLabel ("To Time");
  m_toTimeEdit = new QLineEdit;
  m_toTimeEdit->setValidator (doubleValidator);
  //m_toTimeEdit->setInputMask (TIME_EDIT_MASK);
  m_toTimeEdit->setMaximumWidth (TIME_EDIT_WIDTH);

  m_allowedNodesLabel = new QLabel ("Show Nodes");
  m_allowedNodesEdit = new QLineEdit;
  m_allowedNodesEdit->setMaximumWidth (ALLOWED_NODES_WITH);


  m_showPacketsTableButton = new QToolButton;
  m_showPacketsTableButton->setIcon (QIcon (":/resources/animator_packetstats.svg"));
  m_showPacketsTableButton->setToolTip ("Packet table");
  m_showPacketsTableButton->setCheckable (true);
  m_showPacketsTableButton->setChecked (true);
  connect (m_showPacketsTableButton, SIGNAL (clicked ()), this, SLOT (showPacketTableSlot()));

  m_showGraphButton = new QPushButton ("Show Graph");
  m_showGraphButton->setCheckable (true);
  m_showGraphButton->setChecked (true);

  connect (m_testButton, SIGNAL(clicked()), this, SLOT(testSlot()));
  //m_mainToolBar->addWidget (m_testButton);
  m_mainToolBar->addWidget (m_zoomInButton);
  m_mainToolBar->addWidget (m_zoomOutButton);
  m_mainToolBar->addSeparator ();
  m_mainToolBar->addWidget (m_showGridLinesButton);
  m_mainToolBar->addWidget (m_fromTimeLabel);
  m_mainToolBar->addWidget (m_fromTimeEdit);
  m_mainToolBar->addWidget (m_toTimeLabel);
  m_mainToolBar->addWidget (m_toTimeEdit);
  m_mainToolBar->addWidget (m_allowedNodesLabel);
  m_mainToolBar->addWidget (m_allowedNodesEdit);
  m_mainToolBar->addWidget (m_showPacketsTableButton);
  m_mainToolBar->addWidget (m_showGraphButton);

  m_wifiFilterCb = new QCheckBox ("Wifi");
  m_pppFilterCb = new QCheckBox ("Ppp");
  m_ipv4FilterCb = new QCheckBox ("Ipv4");
  m_ipv6FilterCb = new QCheckBox ("Ipv6");
  m_arpFilterCb = new QCheckBox ("Arp");
  m_tcpFilterCb = new QCheckBox ("Tcp");
  m_udpFilterCb = new QCheckBox ("Udp");
  m_aodvFilterCb = new QCheckBox ("Aodv");
  m_icmpFilterCb = new QCheckBox  ("Icmp");
  m_ethernetFilterCb = new QCheckBox ("Ethernet");
  m_olsrFilterCb = new QCheckBox ("Olsr");


  m_regexFilterLabel = new QLabel ("Regex on meta data");
  m_regexFilterEdit = new QLineEdit (".*");
  m_regexFilterEdit->setMaximumWidth (REGEX_EDIT_WIDTH);

  m_packetsTable = new Table;
  QStringList packetTableHeaders;
  packetTableHeaders << "From Id"
                     << "To Id"
                     << "Tx"
                     << "Meta";
  m_packetsTable->setHeaderList (packetTableHeaders);

  m_submitButton = new QPushButton ("Submit");


  m_filterToolBar->addWidget (m_tcpFilterCb);
  m_filterToolBar->addWidget (m_udpFilterCb);
  m_filterToolBar->addWidget (m_ipv4FilterCb);
  m_filterToolBar->addWidget (m_ipv6FilterCb);
  m_filterToolBar->addWidget (m_icmpFilterCb);
  m_filterToolBar->addWidget (m_wifiFilterCb);
  m_filterToolBar->addWidget (m_ethernetFilterCb);
  m_filterToolBar->addWidget (m_pppFilterCb);
  m_filterToolBar->addWidget (m_aodvFilterCb);
  m_filterToolBar->addWidget (m_olsrFilterCb);
  m_filterToolBar->addWidget (m_arpFilterCb);
  m_filterToolBar->addSeparator ();
  m_filterToolBar->addWidget (m_regexFilterLabel);
  m_filterToolBar->addWidget (m_regexFilterEdit);
  m_filterToolBar->addWidget (m_submitButton);

  m_mainSplitter = new QSplitter;
  m_vLayout = new QVBoxLayout;
  m_vLayout->addWidget (m_mainToolBar);
  m_vLayout->addWidget (m_filterToolBar);

  m_mainSplitter->addWidget (PacketsView::getInstance ());
  m_mainSplitter->addWidget (m_packetsTable);

  m_vLayout->addWidget (m_mainSplitter);

  m_centralWidget = new QWidget;
  m_centralWidget->setLayout (m_vLayout);


  setToTime (0.0);
  setFromTime (0.0);
  setAllowedNodes (ALLOWED_NODES);
  connect (m_fromTimeEdit, SIGNAL(textChanged(QString)), this, SLOT (fromTimeChangedSlot(QString)));
  connect (m_toTimeEdit, SIGNAL(textChanged(QString)), this, SLOT (toTimeChangedSlot(QString)));
  connect (m_allowedNodesEdit, SIGNAL(textChanged(QString)), this, SLOT (allowedNodesChangedSlot(QString)));

  connect (m_wifiFilterCb, SIGNAL(clicked()), this, SLOT(filterClickedSlot()));
  connect (m_tcpFilterCb, SIGNAL(clicked()), this, SLOT(filterClickedSlot()));
  connect (m_udpFilterCb, SIGNAL(clicked()), this, SLOT(filterClickedSlot()));
  connect (m_ipv4FilterCb, SIGNAL(clicked()), this, SLOT(filterClickedSlot()));
  connect (m_ipv6FilterCb, SIGNAL(clicked()), this, SLOT(filterClickedSlot()));
  connect (m_icmpFilterCb, SIGNAL(clicked()), this, SLOT(filterClickedSlot()));
  connect (m_wifiFilterCb, SIGNAL(clicked()), this, SLOT(filterClickedSlot()));
  connect (m_ethernetFilterCb, SIGNAL(clicked()), this, SLOT(filterClickedSlot()));
  connect (m_pppFilterCb, SIGNAL(clicked()), this, SLOT(filterClickedSlot()));
  connect (m_aodvFilterCb, SIGNAL(clicked()), this, SLOT(filterClickedSlot()));
  connect (m_olsrFilterCb, SIGNAL(clicked()), this, SLOT(filterClickedSlot()));
  connect (m_arpFilterCb, SIGNAL(clicked()), this, SLOT(filterClickedSlot()));
  connect (m_regexFilterEdit, SIGNAL(textEdited(QString)), this, SLOT(regexFilterSlot(QString)));
  connect (m_submitButton, SIGNAL(clicked()), this, SLOT (submitFilterClickedSlot()));
  connect (m_showGraphButton, SIGNAL(clicked()), this, SLOT(showGraphClickedSlot()));
}

QString
PacketsMode::getTabName ()
{
  return "Packets";
}

void
PacketsMode::testSlot ()
{
}


void
PacketsMode::zoomInSlot ()
{
  PacketsView::getInstance ()->zoomIn ();
}

Table *
PacketsMode::getTable ()
{
  return m_packetsTable;
}


void
PacketsMode::zoomOutSlot ()
{
  PacketsView::getInstance ()->zoomOut ();
}

void
PacketsMode::setFocus (bool focus)
{
  //focus?qDebug (QString ("Animator Focus")):qDebug (QString ("Animator lost Focus"));
  Q_UNUSED (focus);
  if (focus)
    {
      qreal lastPacketTime = AnimatorMode::getInstance ()->getLastPacketEventTime ();
      if (lastPacketTime < 0)
        return;
      uint32_t nodeCount = AnimNodeMgr::getInstance ()->getCount ();
      if (nodeCount == 0)
        return;
      m_allowedNodes.clear ();
      for (uint32_t i = 0; i < nodeCount; ++i)
        {
          //if (i>10)
          //  break;
          m_allowedNodes.push_back (i);
        }
      setAllowedNodes (nodeVectorToString (m_allowedNodes));
      //m_allowedNodes.clear ();
      qreal thousandthPacketTime = AnimatorMode::getInstance ()->getThousandthPacketTime ();
      if (thousandthPacketTime < 0)
        m_toTime = lastPacketTime;
      else
        m_toTime = thousandthPacketTime;
      m_fromTime = AnimatorMode::getInstance ()->getFirstPacketTime ();
      m_toTimeEdit->setText (QString::number (m_toTime, 'g', 6));
      m_fromTimeEdit->setText (QString::number (m_fromTime, 'g', 6));

      //setAllowedNodes (m_allowedNodes);
      PacketsScene::getInstance ()->redraw(m_fromTime, m_toTime, m_allowedNodes, m_showGrid);
      PacketsView::getInstance ()->horizontalScrollBar ()->setValue (-100);
    }

}

QWidget *
PacketsMode::getCentralWidget ()
{
  return m_centralWidget;
}
PacketsMode *
PacketsMode::getInstance ()
{
  if (!pPacketsMode)
    {
      pPacketsMode = new PacketsMode;
    }
  return pPacketsMode;
}


void
PacketsMode::setFromTime (qreal fromTime)
{
  m_fromTime = fromTime;
  //m_fromTimeEdit->setText (QString::number (fromTime, 'g', 6));
  //PacketsScene::getInstance ()->redraw(m_fromTime, m_toTime, m_allowedNodes, m_showGrid);
  PacketsView::getInstance ()->horizontalScrollBar ()->setValue (-100);

}

void
PacketsMode::setToTime (qreal toTime)
{
  m_toTime = toTime;
  //m_toTimeEdit->setText (QString::number (toTime, 'g', 6));
  //PacketsScene::getInstance ()->redraw(m_fromTime, m_toTime, m_allowedNodes, m_showGrid);
  PacketsView::getInstance ()->horizontalScrollBar ()->setValue (-100);

}


QVector <uint32_t>
PacketsMode::stringToNodeVector (QString nodeString)
{
  QStringList nodes = nodeString.split (":", QString::SkipEmptyParts);
  QVector <uint32_t> v;
  foreach (QString s ,nodes)
    {
      v.push_back (s.toUInt ());
    }
  return v;
}


QString
PacketsMode::nodeVectorToString (QVector<uint32_t> nodeVector)
{
  QString s;
  for (int i = 0; i < nodeVector.size (); ++i)
    {
      s += QString::number (nodeVector[i]) + ":";
    }
  return s;
}

void
PacketsMode::setAllowedNodes (QString allowedNodesString)
{
  QVector <uint32_t> nodes = stringToNodeVector (allowedNodesString);
  m_allowedNodesEdit->setText (allowedNodesString);
  m_allowedNodes = nodes;
  PacketsScene::getInstance ()->redraw(m_fromTime, m_toTime, m_allowedNodes, m_showGrid);
  PacketsView::getInstance ()->horizontalScrollBar ()->setValue (-100);

}

void
PacketsMode::showPopup (QString msg)
{
  QMessageBox msgBox;
  msgBox.setText (msg);
  msgBox.exec ();
}

void
PacketsMode::fromTimeChangedSlot (QString fromTimeText)
{
  qreal temp = fromTimeText.toDouble ();
  m_fromTime = temp;
  setFromTime (m_fromTime);

}

void
PacketsMode::toTimeChangedSlot (QString toTimeText)
{
  qreal temp = toTimeText.toDouble ();
  m_toTime = temp;
  setToTime (m_toTime);

}

void
PacketsMode::showPacketTableSlot ()
{
  m_packetsTable->setVisible (m_showPacketsTableButton->isChecked ());
}

void
PacketsMode::submitFilterClickedSlot ()
{
  PacketsScene::getInstance ()->redraw(m_fromTime, m_toTime, m_allowedNodes, m_showGrid);
}

void
PacketsMode::showGraphClickedSlot ()
{
  PacketsScene::getInstance ()->showGraph (m_showGraphButton->isChecked ());
}


void
PacketsMode::filterClickedSlot ()
{
  int ft = AnimPacket::ALL;
  ft |= m_wifiFilterCb->isChecked () ? AnimPacket::WIFI: AnimPacket::ALL;
  ft |= m_arpFilterCb->isChecked () ? AnimPacket::ARP: AnimPacket::ALL;
  ft |= m_aodvFilterCb->isChecked () ? AnimPacket::AODV: AnimPacket::ALL;
  ft |= m_olsrFilterCb->isChecked () ? AnimPacket::OLSR: AnimPacket::ALL;
  ft |= m_pppFilterCb->isChecked () ? AnimPacket::PPP: AnimPacket::ALL;
  ft |= m_tcpFilterCb->isChecked () ? AnimPacket::TCP: AnimPacket::ALL;
  ft |= m_udpFilterCb->isChecked () ? AnimPacket::UDP: AnimPacket::ALL;
  ft |= m_icmpFilterCb->isChecked () ? AnimPacket::ICMP: AnimPacket::ALL;
  ft |= m_ipv4FilterCb->isChecked () ? AnimPacket::IPV4: AnimPacket::ALL;
  ft |= m_ipv6FilterCb->isChecked () ? AnimPacket::IPV6: AnimPacket::ALL;
  ft |= m_pppFilterCb->isChecked () ? AnimPacket::PPP: AnimPacket::ALL;
  ft |= m_ethernetFilterCb->isChecked () ? AnimPacket::ETHERNET: AnimPacket::ALL;
  PacketsScene::getInstance ()->setFilter (ft);

}

void
PacketsMode::regexFilterSlot (QString reg)
{
  //NS_LOG_DEBUG ("Regex");
  PacketsScene::getInstance ()->setRegexFilter (reg);
}

void
PacketsMode::allowedNodesChangedSlot (QString allowedNodes)
{
  QVector <uint32_t> nodes = stringToNodeVector (allowedNodes);
  if (nodes.empty ())
    {
      showPopup ("unable to parse node list");
      setAllowedNodes (ALLOWED_NODES);
      return;
    }
  setAllowedNodes (allowedNodes);

}

void
PacketsMode::showGridLinesSlot ()
{
  m_showGrid = m_showGridLinesButton->isChecked ();
  PacketsScene::getInstance ()->redraw(m_fromTime, m_toTime, m_allowedNodes, m_showGrid);
  PacketsView::getInstance ()->horizontalScrollBar ()->setValue (-100);

}


}
