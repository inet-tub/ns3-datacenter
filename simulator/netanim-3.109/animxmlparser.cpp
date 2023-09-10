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
 * 		          Dmitrii Shakshin <d.shakshin@gmail.com> (Open Source and Linux Laboratory http://dev.osll.ru/)
 *                Makhtar Diouf <makhtar.diouf@gmail.com>
 */

#include "common.h"
#include "animxmlparser.h"
#include "animatormode.h"
#include "animresource.h"
#include "animnode.h"
#include <QtDebug>
#include <exception>

namespace netanim
{

NS_LOG_COMPONENT_DEFINE ("Animxmlparser");

Animxmlparser::Animxmlparser (QString traceFileName):
  m_traceFileName (traceFileName),
  m_parsingComplete (false),
  m_reader (0),
  m_maxSimulationTime (0),
  m_fileIsValid (true),
  m_lastPacketEventTime (-1),
  m_thousandThPacketTime (-1),
  m_firstPacketTime (65535),
  m_minNodeX (0),
  m_minNodeY (0),
  m_maxNodeX (0),
  m_maxNodeY (0)
{
  m_version = 0;
  if (m_traceFileName == "")
    return;

  try
    {
      m_traceFile = new QFile (m_traceFileName);
      if ((m_traceFile->size () <= 0) || !m_traceFile->open (QIODevice::ReadOnly | QIODevice::Text))
        {
          //qDebug (QString ("Critical:Trace file is invalid"));
          m_fileIsValid = false;
          return;
        }
      //qDebug (m_traceFileName);
      m_reader = new QXmlStreamReader (m_traceFile);
     }
  catch (std::exception &e)
    {
      NS_LOG_DEBUG ("Error opening trace file: " << e.what ());
      m_fileIsValid = false;
    }
}

Animxmlparser::~Animxmlparser ()
{
  if (m_traceFile)
    delete m_traceFile;
  if (m_reader)
    delete m_reader;
}

void
Animxmlparser::searchForVersion ()
{
  QFile * f = new QFile (m_traceFileName);
  if (f->open (QIODevice::ReadOnly | QIODevice::Text))
    {
      QString firstLine = QString (f->readLine ());
      int startIndex = 0;
      int endIndex = 0;
      QString versionField = VERSION_FIELD_DEFAULT;
      startIndex = firstLine.indexOf (versionField);
      endIndex = firstLine.lastIndexOf ("\"");
      if ((startIndex != -1) && (endIndex > startIndex))
        {
          int adjustedStartIndex = startIndex + versionField.length ();
          QString v = firstLine.mid (adjustedStartIndex, endIndex-adjustedStartIndex);
          m_version = v.toDouble ();
        }
      f->close ();
      delete f;
    }
}

uint64_t
Animxmlparser::getRxCount ()
{
  searchForVersion ();
  uint64_t count = 1;
  QFile * f = new QFile (m_traceFileName);
  if (f->open (QIODevice::ReadOnly | QIODevice::Text))
    {
      QString allContent = QString (f->readAll ());
      int j = 0;
      QString searchString = " toId=";
      if (m_version >= 3.102)
        searchString = " tId";

      while ( (j = allContent.indexOf (searchString, j)) != -1)
        {
          ++j;
          ++count;
        }
      f->close ();
      delete f;
      //qDebug (QString::number (count));
    }
  return qMax (count, (uint64_t)1);
}

bool
Animxmlparser::isFileValid ()
{
  return m_fileIsValid;
}

bool
Animxmlparser::isParsingComplete ()
{
  return m_parsingComplete;
}

qreal
Animxmlparser::getLastPacketEventTime ()
{
  return m_lastPacketEventTime;
}

qreal
Animxmlparser::getFirstPacketTime ()
{
  return m_firstPacketTime;
}

QPointF
Animxmlparser::getMinPoint ()
{
  return QPointF (m_minNodeX, m_minNodeY);
}

QPointF
Animxmlparser::getMaxPoint ()
{
  return QPointF (m_maxNodeX, m_maxNodeY);
}

qreal
Animxmlparser::getThousandthPacketTime ()
{
  return m_thousandThPacketTime;
}

void
Animxmlparser::doParse ()
{
  uint64_t parsedElementCount = 0;
  AnimatorMode * pAnimatorMode = AnimatorMode::getInstance ();
  while (!isParsingComplete ())
    {
      if (AnimatorMode::getInstance ()->keepAppResponsive ())
        {
          AnimatorMode::getInstance ()->setParsingCount (parsedElementCount);

        }
      ParsedElement parsedElement = parseNext ();
      switch (parsedElement.type)
        {
        case XML_ANIM:
        {
          AnimatorMode::getInstance ()->setVersion (parsedElement.version);
          //qDebug (QString ("XML Version:") + QString::number (version));
          break;
        }
        case XML_NODE:
        {
            m_minNodeX = qMin (m_minNodeX, parsedElement.node_x);
            m_minNodeY = qMin (m_minNodeY, parsedElement.node_y);
            m_maxNodeX = qMax (m_maxNodeX, parsedElement.node_x);
            m_maxNodeY = qMax (m_maxNodeY, parsedElement.node_y);
          AnimNodeAddEvent * ev = new AnimNodeAddEvent (parsedElement.nodeId,
              parsedElement.nodeSysId,
              parsedElement.node_x,
              parsedElement.node_y,
              parsedElement.nodeDescription,
              parsedElement.node_r,
              parsedElement.node_g,
              parsedElement.node_b);
          pAnimatorMode->addAnimEvent (0, ev);
          AnimNodeMgr::getInstance ()->addAPosition (parsedElement.nodeId, 0, QPointF (parsedElement.node_x,
                                                                                    parsedElement.node_y));
          break;
        }
        case XML_PACKET_TX_REF:
        {
          m_packetRefs[parsedElement.uid] = parsedElement;
          break;
        }
        case XML_WPACKET_RX_REF:
        {
          ParsedElement & ref = m_packetRefs[parsedElement.uid];
          parsedElement.packetrx_fromId = ref.packetrx_fromId;
          parsedElement.packetrx_fbTx = ref.packetrx_fbTx;
          parsedElement.packetrx_lbTx = ref.packetrx_lbTx;
          parsedElement.meta_info = ref.meta_info;
          break;
        }
        case XML_WPACKET_RX:
        case XML_PACKET_RX:
        {
          m_firstPacketTime = qMin (m_firstPacketTime, parsedElement.packetrx_fbTx);
          if (parsedElement.packetrx_fromId == parsedElement.packetrx_toId)
            break;
          uint8_t numWirelessSlots = 3;
          AnimPacketEvent * ev = new AnimPacketEvent (parsedElement.packetrx_fromId,
              parsedElement.packetrx_toId,
              parsedElement.packetrx_fbTx,
              parsedElement.packetrx_fbRx,
              parsedElement.packetrx_lbTx,
              parsedElement.packetrx_lbRx,
              parsedElement.isWpacket,
              parsedElement.meta_info,
              numWirelessSlots);
          pAnimatorMode->addAnimEvent (parsedElement.packetrx_fbTx, ev);
          ++parsedElementCount;
          m_lastPacketEventTime = parsedElement.packetrx_fbRx;
          if (parsedElementCount == 50)
            m_thousandThPacketTime = parsedElement.packetrx_fbRx;

          if (!parsedElement.isWpacket)
            {
              qreal fullDuration = parsedElement.packetrx_fbRx - parsedElement.packetrx_fbTx;
              uint32_t numSlots = WIRED_PACKET_SLOTS;
              qreal step = fullDuration/numSlots;
              for (uint32_t i = 1; i <= numSlots; ++i)
                {
                  qreal point = parsedElement.packetrx_fbTx + (i * step);
                  //NS_LOG_DEBUG ("Point:" << point);
                  pAnimatorMode->addAnimEvent (point, new AnimWiredPacketUpdateEvent ());
                }
            }

          //NS_LOG_DEBUG ("Packet Last Time:" << m_lastPacketEventTime);
          break;
        }
        case XML_LINK:
        {
          //AnimLinkMgr::getInstance ()->add (parsedElement.link_fromId, parsedElement.link_toId);
          AnimLinkAddEvent * ev = new AnimLinkAddEvent (parsedElement.link_fromId,
              parsedElement.link_toId,
              parsedElement.linkDescription,
              parsedElement.fromNodeDescription,
              parsedElement.toNodeDescription);
          pAnimatorMode->addAnimEvent (0, ev);
          break;
        }
        case XML_NONP2P_LINK:
        {
          AnimLinkAddEvent * ev = new AnimLinkAddEvent (parsedElement.link_fromId,
              parsedElement.link_toId,
              parsedElement.linkDescription,
              parsedElement.fromNodeDescription,
              parsedElement.toNodeDescription,
              false);
          pAnimatorMode->addAnimEvent (0, ev);
          break;


        }
        case XML_LINKUPDATE:
        {
          AnimLinkUpdateEvent * ev = new AnimLinkUpdateEvent (parsedElement.link_fromId,
              parsedElement.link_toId,
              parsedElement.linkDescription);
          pAnimatorMode->addAnimEvent (parsedElement.updateTime, ev);
          break;
        }
        case XML_BACKGROUNDIMAGE:
        {
          BackgroudImageProperties_t bgProp;
          bgProp.fileName = parsedElement.fileName;
          bgProp.x = parsedElement.x;
          bgProp.y = parsedElement.y;
          bgProp.scaleX = parsedElement.scaleX;
          bgProp.scaleY = parsedElement.scaleY;
          bgProp.opacity = parsedElement.opacity;
          AnimatorMode::getInstance ()->setBackgroundImageProperties (bgProp);
          break;
        }

        case XML_RESOURCE:
        {
          AnimResourceManager::getInstance ()->add (parsedElement.resourceId, parsedElement.resourcePath);
          break;
        }
        case XML_IP:
        {
          AnimIpEvent * ev = new AnimIpEvent (parsedElement.nodeId, parsedElement.ipAddresses);
          pAnimatorMode->addAnimEvent (0, ev);
          break;
        }
        case XML_IPV6:
        {
          AnimIpv6Event * ev = new AnimIpv6Event (parsedElement.nodeId, parsedElement.ipv6Addresses);
          pAnimatorMode->addAnimEvent (0, ev);
          break;
        }
        case XML_CREATE_NODE_COUNTER:
        {
            AnimCreateNodeCounterEvent * ev = 0;
            if (parsedElement.nodeCounterType == ParsedElement::UINT32_COUNTER)
              ev = new AnimCreateNodeCounterEvent (parsedElement.nodeCounterId, parsedElement.nodeCounterName, AnimCreateNodeCounterEvent::UINT32_COUNTER);
            if (parsedElement.nodeCounterType == ParsedElement::DOUBLE_COUNTER)
              ev = new AnimCreateNodeCounterEvent (parsedElement.nodeCounterId, parsedElement.nodeCounterName, AnimCreateNodeCounterEvent::DOUBLE_COUNTER);
            if (ev)
              {
                pAnimatorMode->addAnimEvent (0, ev);
              }
            break;
        }
        case XML_NODECOUNTER_UPDATE:
        {
            AnimNodeCounterUpdateEvent * ev = new AnimNodeCounterUpdateEvent (parsedElement.nodeCounterId,
                                                                              parsedElement.nodeId,
                                                                              parsedElement.nodeCounterValue);
            pAnimatorMode->addAnimEvent (parsedElement.updateTime, ev);
            break;
        }
        case XML_NODEUPDATE:
        {
          if (parsedElement.nodeUpdateType == ParsedElement::POSITION)
            {
              AnimNodePositionUpdateEvent * ev = new AnimNodePositionUpdateEvent (parsedElement.nodeId,
                  parsedElement.node_x,
                  parsedElement.node_y);
              pAnimatorMode->addAnimEvent (parsedElement.updateTime, ev);
              AnimNodeMgr::getInstance ()->addAPosition (parsedElement.nodeId, parsedElement.updateTime, QPointF (parsedElement.node_x,
                                                                                        parsedElement.node_y));
              m_minNodeX = qMin (m_minNodeX, parsedElement.node_x);
              m_minNodeY = qMin (m_minNodeY, parsedElement.node_y);
              m_maxNodeX = qMax (m_maxNodeX, parsedElement.node_x);
              m_maxNodeY = qMax (m_maxNodeY, parsedElement.node_y);

            }
          if (parsedElement.nodeUpdateType == ParsedElement::COLOR)
            {
              AnimNodeColorUpdateEvent * ev = new AnimNodeColorUpdateEvent (parsedElement.nodeId,
                  parsedElement.node_r,
                  parsedElement.node_g,
                  parsedElement.node_b);

              pAnimatorMode->addAnimEvent (parsedElement.updateTime, ev);
            }
          if (parsedElement.nodeUpdateType == ParsedElement::DESCRIPTION)
            {
              AnimNodeDescriptionUpdateEvent * ev = new AnimNodeDescriptionUpdateEvent (parsedElement.nodeId,
                  parsedElement.nodeDescription);
              pAnimatorMode->addAnimEvent (parsedElement.updateTime, ev);

            }
          if (parsedElement.nodeUpdateType == ParsedElement::SIZE)
            {
              AnimNodeSizeUpdateEvent * ev = new AnimNodeSizeUpdateEvent (parsedElement.nodeId,
                  parsedElement.node_width,
                  parsedElement.node_height);
              pAnimatorMode->addAnimEvent (parsedElement.updateTime, ev);

            }
          if (parsedElement.nodeUpdateType == ParsedElement::IMAGE)
            {
              AnimNodeImageUpdateEvent * ev = new AnimNodeImageUpdateEvent (parsedElement.nodeId,
                  parsedElement.resourceId);
              pAnimatorMode->addAnimEvent (parsedElement.updateTime, ev);
            }
          if (parsedElement.nodeUpdateType == ParsedElement::SYSTEM_ID)
            {
              AnimNodeSysIdUpdateEvent * ev = new AnimNodeSysIdUpdateEvent (parsedElement.nodeId,
                                parsedElement.nodeSysId);
              pAnimatorMode->addAnimEvent (parsedElement.updateTime, ev);
            }
          break;

        }
        case XML_INVALID:
        default:
        {
          //qDebug ("Invalid XML element");
        }
        } //switch
    } // while loop
}

ParsedElement
Animxmlparser::parseNext ()
{
  ParsedElement parsedElement;
  parsedElement.type = XML_INVALID;
  parsedElement.version = m_version;
  parsedElement.isWpacket = false;

  if (m_reader->atEnd () || m_reader->hasError ())
    {
      m_parsingComplete = true;
      m_traceFile->close ();
      return parsedElement;
    }



  QXmlStreamReader::TokenType token =  m_reader->readNext ();
  if (token == QXmlStreamReader::StartDocument)
    return parsedElement;

  if (token == QXmlStreamReader::StartElement)
    {
      if (m_reader->name().toString() == "anim")
        {
          parsedElement = parseAnim ();
        }
      if (m_reader->name().toString() == "topology")
        {
          parsedElement = parseTopology ();
        }
      if (m_reader->name().toString() == "node")
        {
          parsedElement = parseNode ();
        }
      if (m_reader->name().toString() == "ip")
        {
          parsedElement = parseIpv4 ();
        }
      if (m_reader->name().toString() == "ipv6")
        {
          parsedElement = parseIpv6 ();
        }
      if (m_reader->name().toString() == "packet")
        {
          parsedElement = parsePacket ();
        }
      if (m_reader->name().toString() == "p")
        {
          parsedElement = parseP ();
        }
      if (m_reader->name().toString() == "wp")
        {
          parsedElement = parseWp ();
        }
      if (m_reader->name().toString() == "wpacket")
        {
          parsedElement = parseWPacket ();
        }
      if (m_reader->name().toString() == "link")
        {
          parsedElement = parseLink ();
        }
      if (m_reader->name().toString() == "nonp2plinkproperties")
        {
          parsedElement = parseNonP2pLink ();
        }
      if (m_reader->name().toString() == "linkupdate")
        {
          parsedElement = parseLinkUpdate ();
        }
      if (m_reader->name().toString() == "nu")
        {
          parsedElement = parseNodeUpdate ();
        }
      if (m_reader->name().toString() == "res")
        {
          parsedElement = parseResource ();
        }
      if (m_reader->name().toString() == "bg")
        {
          parsedElement = parseBackground ();
        }
      if (m_reader->name().toString() == "ncs")
        {
          parsedElement = parseCreateNodeCounter ();
        }
      if (m_reader->name().toString() == "nc")
        {
          parsedElement = parseNodeCounterUpdate ();
        }
      if (m_reader->name().toString() == "pr")
        {
          parsedElement = parsePacketTxRef ();
        }
      if (m_reader->name().toString() == "wpr")
        {
          parsedElement = parseWPacketRxRef ();
        }
      //qDebug (m_reader->name ().toString ());
    }

  if (m_reader->atEnd ())
    {
      m_parsingComplete = true;
      m_traceFile->close ();
    }
  return parsedElement;
}


ParsedElement
Animxmlparser::parseAnim ()
{
  ParsedElement parsedElement;
  parsedElement.type = XML_ANIM;
  parsedElement.version = m_version;
  QString v = m_reader->attributes ().value ("ver").toString ();
  if (!v.contains ("netanim-"))
    return parsedElement;
  v = v.replace ("netanim-","");
  m_version = v.toDouble ();
  if (m_version < ANIM_MIN_VERSION)
    {
      AnimatorMode::getInstance ()->showPopup ("This XML format is not supported. Minimum Version:" + QString::number (ANIM_MIN_VERSION));
      NS_FATAL_ERROR ("This XML format is not supported. Minimum Version:" << ANIM_MIN_VERSION);
    }
  parsedElement.version = m_version;
  //qDebug (QString::number (m_version));
  QString fileType = m_reader->attributes ().value ("filetype").toString ();
  if (fileType != "animation")
    {
      AnimatorMode::getInstance ()->showPopup ("filetype must be == animation. Invalid animation trace file?");
      NS_FATAL_ERROR ("Invalid animation trace file");
    }
  return parsedElement;
}

ParsedElement
Animxmlparser::parseTopology ()
{
  ParsedElement parsedElement;
  parsedElement.type = XML_TOPOLOGY;
  parsedElement.topo_width = m_reader->attributes ().value ("maxX").toString ().toDouble ();
  parsedElement.topo_height = m_reader->attributes ().value ("maxY").toString ().toDouble ();
  return parsedElement;

}

ParsedElement
Animxmlparser::parseLink ()
{
  ParsedElement parsedElement;
  parsedElement.type = XML_LINK;
  parsedElement.link_fromId = m_reader->attributes ().value ("fromId").toString ().toUInt ();
  parsedElement.link_toId = m_reader->attributes ().value ("toId").toString ().toDouble ();
  parsedElement.fromNodeDescription = m_reader->attributes ().value ("fd").toString ();
  parsedElement.toNodeDescription = m_reader->attributes ().value ("td").toString ();
  parsedElement.linkDescription = m_reader->attributes ().value ("ld").toString ();
  return parsedElement;

}

ParsedElement
Animxmlparser::parseBackground ()
{
  ParsedElement parsedElement;
  parsedElement.type = XML_BACKGROUNDIMAGE;
  parsedElement.fileName = m_reader->attributes ().value ("f").toString ();
  parsedElement.x = m_reader->attributes ().value ("x").toString ().toDouble ();
  parsedElement.y = m_reader->attributes ().value ("y").toString ().toDouble ();
  parsedElement.scaleX = m_reader->attributes ().value ("sx").toString ().toDouble ();
  parsedElement.scaleY = m_reader->attributes ().value ("sy").toString ().toDouble ();
  parsedElement.opacity = m_reader->attributes ().value ("o").toString ().toDouble ();
  return parsedElement;
}

ParsedElement
Animxmlparser::parseNonP2pLink ()
{
  ParsedElement parsedElement;
  parsedElement.type = XML_NONP2P_LINK;
  parsedElement.link_fromId = m_reader->attributes ().value ("id").toString ().toUInt ();
  parsedElement.fromNodeDescription = m_reader->attributes ().value ("ipAddress").toString ();
  return parsedElement;
}

ParsedElement
Animxmlparser::parseLinkUpdate ()
{
  ParsedElement parsedElement;
  parsedElement.type = XML_LINKUPDATE;
  parsedElement.link_fromId = m_reader->attributes ().value ("fromId").toString ().toUInt ();
  parsedElement.link_toId = m_reader->attributes ().value ("toId").toString ().toDouble ();
  parsedElement.linkDescription = m_reader->attributes ().value ("ld").toString ();
  parsedElement.updateTime = m_reader->attributes ().value ("t").toString ().toDouble ();
  setMaxSimulationTime (parsedElement.updateTime);
  return parsedElement;

}

ParsedElement
Animxmlparser::parsePacketTxRef ()
{
  ParsedElement parsedElement;
  parsedElement.type = XML_PACKET_TX_REF;
  parsedElement.uid = m_reader->attributes ().value ("uId").toString ().toLong ();
  parsedElement.packetrx_fromId = m_reader->attributes ().value ("fId").toString ().toUInt ();
  parsedElement.packetrx_fbTx = m_reader->attributes ().value ("fbTx").toString ().toDouble ();
  parsedElement.packetrx_lbTx = m_reader->attributes ().value ("lbTx").toString ().toDouble ();
  setMaxSimulationTime (parsedElement.packetrx_lbTx);
  parsedElement.meta_info = m_reader->attributes ().value ("meta-info").toString ();
  if (parsedElement.meta_info == "")
    {
      parsedElement.meta_info = "null";
    }
  return parsedElement;
}

ParsedElement
Animxmlparser::parseWPacketRxRef ()
{
  ParsedElement parsedElement;
  parsedElement.type = XML_WPACKET_RX_REF;
  parsedElement.isWpacket = true;
  parsedElement.uid = m_reader->attributes ().value ("uId").toString ().toLong ();
  parsedElement.packetrx_toId = m_reader->attributes ().value ("tId").toString ().toUInt ();
  parsedElement.packetrx_fbRx = m_reader->attributes ().value ("fbRx").toString ().toDouble ();
  parsedElement.packetrx_lbRx = m_reader->attributes ().value ("lbRx").toString ().toDouble ();
  setMaxSimulationTime (parsedElement.packetrx_lbRx);
  return parsedElement;
}


ParsedElement
Animxmlparser::parseNode ()
{
  ParsedElement parsedElement;
  parsedElement.type = XML_NODE;
  parsedElement.nodeId = m_reader->attributes ().value ("id").toString ().toUInt ();
  parsedElement.nodeSysId = m_reader->attributes ().value ("sysId").toString ().toUInt ();
  parsedElement.node_x = m_reader->attributes ().value ("locX").toString ().toDouble ();
  parsedElement.node_y = m_reader->attributes ().value ("locY").toString ().toDouble ();
  parsedElement.node_batteryCapacity = m_reader->attributes ().value ("rc").toString ().toDouble ();
  parsedElement.nodeDescription = m_reader->attributes ().value ("descr").toString ();
  parsedElement.node_r = m_reader->attributes ().value ("r").toString ().toUInt ();
  parsedElement.node_g = m_reader->attributes ().value ("g").toString ().toUInt ();
  parsedElement.node_b = m_reader->attributes ().value ("b").toString ().toUInt ();
  parsedElement.hasColorUpdate = !m_reader->attributes ().value ("r").isEmpty ();
  parsedElement.hasBattery = !m_reader->attributes ().value ("rc").isEmpty ();
  return parsedElement;
}

ParsedElement
Animxmlparser::parseNodeUpdate ()
{
  ParsedElement parsedElement;
  parsedElement.type = XML_NODEUPDATE;
  QString nodeUpdateString = m_reader->attributes ().value ("p").toString ();
  if (nodeUpdateString == "p")
    parsedElement.nodeUpdateType = ParsedElement::POSITION;
  if (nodeUpdateString == "c")
    parsedElement.nodeUpdateType = ParsedElement::COLOR;
  if (nodeUpdateString == "d")
    parsedElement.nodeUpdateType = ParsedElement::DESCRIPTION;
  if (nodeUpdateString == "s")
    parsedElement.nodeUpdateType = ParsedElement::SIZE;
  if (nodeUpdateString == "i")
    parsedElement.nodeUpdateType = ParsedElement::IMAGE;
  if (nodeUpdateString == "y")
    parsedElement.nodeUpdateType = ParsedElement::SYSTEM_ID;
  parsedElement.updateTime = m_reader->attributes ().value ("t").toString ().toDouble ();
  setMaxSimulationTime (parsedElement.updateTime);
  parsedElement.nodeId = m_reader->attributes ().value ("id").toString ().toUInt ();

  switch (parsedElement.nodeUpdateType)
    {
    case ParsedElement::POSITION:
      parsedElement.node_x = m_reader->attributes ().value ("x").toString ().toDouble ();
      parsedElement.node_y = m_reader->attributes ().value ("y").toString ().toDouble ();
      break;
    case ParsedElement::COLOR:
      parsedElement.node_r = m_reader->attributes ().value ("r").toString ().toUInt ();
      parsedElement.node_g = m_reader->attributes ().value ("g").toString ().toUInt ();
      parsedElement.node_b = m_reader->attributes ().value ("b").toString ().toUInt ();
      break;
    case ParsedElement::DESCRIPTION:
      parsedElement.nodeDescription = m_reader->attributes ().value ("descr").toString ();

      break;
    case ParsedElement::SIZE:
      parsedElement.node_width = m_reader->attributes ().value ("w").toString ().toDouble ();
      parsedElement.node_height = m_reader->attributes ().value ("h").toString ().toDouble ();
      break;

    case ParsedElement::IMAGE:
      parsedElement.resourceId = m_reader->attributes ().value ("rid").toString ().toUInt ();
      break;

    case ParsedElement::SYSTEM_ID:
      parsedElement.nodeSysId = m_reader->attributes ().value ("sysId").toString ().toUInt ();
      break;
    }

  return parsedElement;
}

ParsedElement
Animxmlparser::parseNodeCounterUpdate ()
{
  ParsedElement parsedElement;
  parsedElement.type = XML_NODECOUNTER_UPDATE;
  parsedElement.nodeCounterId = m_reader->attributes ().value ("c").toString ().toUInt ();
  parsedElement.nodeId = m_reader->attributes ().value ("i").toString ().toUInt ();
  parsedElement.updateTime = m_reader->attributes ().value ("t").toString ().toDouble ();
  parsedElement.nodeCounterValue = m_reader->attributes ().value ("v").toString ().toDouble ();
  setMaxSimulationTime (parsedElement.updateTime);
  return parsedElement;
}


ParsedElement
Animxmlparser::parseCreateNodeCounter ()
{
  ParsedElement parsedElement;
  parsedElement.type = XML_CREATE_NODE_COUNTER;
  parsedElement.nodeCounterId = m_reader->attributes ().value ("ncId").toString ().toUInt ();
  parsedElement.nodeCounterName = m_reader->attributes ().value ("n").toString ();
  QString counterType = m_reader->attributes ().value ("t").toString ();
  if (counterType == "UINT32")
    parsedElement.nodeCounterType = ParsedElement::UINT32_COUNTER;
  if (counterType == "DOUBLE")
    parsedElement.nodeCounterType = ParsedElement::DOUBLE_COUNTER;
  return parsedElement;
}

ParsedElement
Animxmlparser::parseResource ()
{
  ParsedElement parsedElement;
  parsedElement.type = XML_RESOURCE;
  parsedElement.resourceId = m_reader->attributes ().value ("rid").toString ().toUInt ();
  parsedElement.resourcePath = m_reader->attributes ().value ("p").toString ();
  return parsedElement;
}

void
Animxmlparser::parseGeneric (ParsedElement & parsedElement)
{
  parsedElement.packetrx_fromId = m_reader->attributes ().value ("fId").toString ().toUInt ();
  parsedElement.packetrx_fbTx = m_reader->attributes ().value ("fbTx").toString ().toDouble ();
  parsedElement.packetrx_lbTx = m_reader->attributes ().value ("lbTx").toString ().toDouble ();
  setMaxSimulationTime (parsedElement.packetrx_lbTx);
  parsedElement.packetrx_toId = m_reader->attributes ().value ("tId").toString ().toUInt ();
  parsedElement.packetrx_fbRx = m_reader->attributes ().value ("fbRx").toString ().toDouble ();
  parsedElement.packetrx_lbRx = m_reader->attributes ().value ("lbRx").toString ().toDouble ();
  if (!parsedElement.packetrx_lbRx && parsedElement.packetrx_fbRx)
    {
      parsedElement.packetrx_lbRx = parsedElement.packetrx_fbRx;
    }
  setMaxSimulationTime (parsedElement.packetrx_lbRx);
  parsedElement.meta_info = m_reader->attributes ().value ("meta-info").toString ();
  if (parsedElement.meta_info == "")
    {
      parsedElement.meta_info = "null";
    }
}

ParsedElement
Animxmlparser::parseIpv4 ()
{
  ParsedElement parsedElement;
  parsedElement.type = XML_IP;
  parsedElement.nodeId = m_reader->attributes ().value ("n").toString ().toUInt ();
  while (m_reader->readNextStartElement ())
  {
      QString address = m_reader->name ().toString ();
      parsedElement.ipAddresses.push_back(m_reader->readElementText ());
  }
  return parsedElement;
}

ParsedElement
Animxmlparser::parseIpv6 ()
{
  ParsedElement parsedElement;
  parsedElement.type = XML_IPV6;
  parsedElement.nodeId = m_reader->attributes ().value ("n").toString ().toUInt ();
  while (m_reader->readNextStartElement ())
  {
      QString address = m_reader->name ().toString ();
      parsedElement.ipv6Addresses.push_back(m_reader->readElementText ());
  }
  return parsedElement;
}

ParsedElement
Animxmlparser::parseP ()
{
  ParsedElement parsedElement;
  parsedElement.isWpacket = false;
  parsedElement.type = XML_PACKET_RX;
  parseGeneric (parsedElement);
  return parsedElement;
}

ParsedElement
Animxmlparser::parseWp ()
{
  ParsedElement parsedElement;
  parsedElement.type = XML_WPACKET_RX;
  parsedElement.isWpacket = true;
  parseGeneric (parsedElement);
  return parsedElement;
}

ParsedElement
Animxmlparser::parsePacket ()
{
  ParsedElement parsedElement;
  parsedElement.type = XML_PACKET_RX;
  parsedElement.packetrx_fromId = m_reader->attributes ().value ("fromId").toString ().toUInt ();
  parsedElement.packetrx_fbTx = m_reader->attributes ().value ("fbTx").toString ().toDouble ();
  parsedElement.packetrx_lbTx = m_reader->attributes ().value ("lbTx").toString ().toDouble ();
  parsedElement.meta_info = "null";
  setMaxSimulationTime (parsedElement.packetrx_lbTx);
  while (m_reader->name().toString() != "rx")
    m_reader->readNext ();

  if (m_reader->atEnd () || m_reader->hasError ())
    {
      m_parsingComplete = true;
      m_traceFile->close ();
      return parsedElement;
    }

  parsedElement.packetrx_toId = m_reader->attributes ().value ("toId").toString ().toUInt ();
  parsedElement.packetrx_fbRx = m_reader->attributes ().value ("fbRx").toString ().toDouble ();
  parsedElement.packetrx_lbRx = m_reader->attributes ().value ("lbRx").toString ().toDouble ();
  setMaxSimulationTime (parsedElement.packetrx_lbRx);

  while (m_reader->name().toString() == "rx")
    m_reader->readNext ();
  if (m_reader->name().toString() == "packet")
    return parsedElement;
  m_reader->readNext ();
  if (m_reader->name().toString() != "meta")
    return parsedElement;
  parsedElement.meta_info = m_reader->attributes ().value ("info").toString ();
  //qDebug (parsedElement.meta_info);
  return parsedElement;

}

ParsedElement
Animxmlparser::parseWPacket ()
{

  ParsedElement parsedElement;
  parsedElement.type = XML_WPACKET_RX;
  parsedElement.packetrx_fromId = m_reader->attributes ().value ("fromId").toString ().toUInt ();
  parsedElement.packetrx_fbTx = m_reader->attributes ().value ("fbTx").toString ().toDouble ();
  parsedElement.packetrx_lbTx = m_reader->attributes ().value ("lbTx").toString ().toDouble ();
  parsedElement.meta_info = "null";
  setMaxSimulationTime (parsedElement.packetrx_lbTx);
  while (m_reader->name().toString() != "rx")
    m_reader->readNext ();
  if (m_reader->atEnd () || m_reader->hasError ())
    {
      m_parsingComplete = true;
      m_traceFile->close ();
      return parsedElement;
    }

  //qDebug (m_reader->name ().toString ()+"parseWpacket");
  parsedElement.packetrx_toId = m_reader->attributes ().value ("toId").toString ().toUInt ();
  parsedElement.packetrx_fbRx = m_reader->attributes ().value ("fbRx").toString ().toDouble ();
  parsedElement.packetrx_lbRx = m_reader->attributes ().value ("lbRx").toString ().toDouble ();
  setMaxSimulationTime (parsedElement.packetrx_lbRx);
  while (m_reader->name().toString() == "rx")
    m_reader->readNext ();
  if (m_reader->name().toString() == "wpacket")
    return parsedElement;
  m_reader->readNext ();
  if (m_reader->name().toString() != "meta")
    return parsedElement;
  parsedElement.meta_info = m_reader->attributes ().value ("info").toString ();
  //qDebug (parsedElement.meta_info);
  return parsedElement;

}

void
Animxmlparser::setMaxSimulationTime (qreal t)
{
  m_maxSimulationTime = std::max (m_maxSimulationTime, t);
}

double
Animxmlparser::getMaxSimulationTime ()
{
  return m_maxSimulationTime;
}


} // namespace netanim
