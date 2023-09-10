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

#include "routingxmlparser.h"
#include "statsmode.h"
#include "animatormode.h"
#include "routingstatsscene.h"
#include "log.h"
#include <exception>

NS_LOG_COMPONENT_DEFINE("RoutingXmlParser");

namespace netanim
{

RoutingXmlparser::RoutingXmlparser (QString traceFileName):
  m_traceFileName (traceFileName),
  m_parsingComplete (false),
  m_reader (0),
  m_maxSimulationTime (0),
  m_minSimulationTime (0xFFFFFFFF),
  m_fileIsValid (true)
{
  m_version = 0;
  if (m_traceFileName == "")
    return;

  m_traceFile = new QFile (m_traceFileName);
  try
    {
      if ((m_traceFile->size () <= 0) || !m_traceFile->open (QIODevice::ReadOnly | QIODevice::Text))
        {
          m_fileIsValid = false;
          return;
        }
    }
  catch (std::exception& e)
    {
      NS_LOG_DEBUG ("Unable to load routing xml file:" << e.what ());
      m_fileIsValid = false;
    }

  //qDebug (m_traceFileName);
  QString allChars (m_traceFile->readAll ().constData ());
  allChars.replace ("\n", "&#13;&#10;");
  //qDebug (allChars);

  m_reader = new QXmlStreamReader (GET_ASCII (allChars));
}

RoutingXmlparser::~RoutingXmlparser ()
{
  if (m_traceFile)
    delete m_traceFile;
  if (m_reader)
    delete m_reader;
}

void
RoutingXmlparser::searchForVersion ()
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
RoutingXmlparser::getRtCount ()
{
  searchForVersion ();
  uint64_t count = 0;
  QFile * f = new QFile (m_traceFileName);
  if (f->open (QIODevice::ReadOnly | QIODevice::Text))
    {
      QString allContent = QString (f->readAll ());
      int j = 0;
      QString searchString = "rt t=";
      while ( (j = allContent.indexOf (searchString, j)) != -1)
        {
          ++j;
          ++count;
        }
      f->close ();
      delete f;
      //qDebug (QString::number (count));
      return count;
    }
  return count;
}

bool
RoutingXmlparser::isFileValid ()
{
  return m_fileIsValid;
}

bool
RoutingXmlparser::isParsingComplete ()
{
  return m_parsingComplete;
}


void
RoutingXmlparser::doParse ()
{
  // uint64_t parsedElementCount = 0;
  while (!isParsingComplete ())
    {
      if (AnimatorMode::getInstance ()->keepAppResponsive ())
        {
          //AnimatorMode::getInstance ()->setParsingCount (parsedElementCount);

        }
      RoutingParsedElement parsedElement = parseNext ();
      switch (parsedElement.type)
        {
        case RoutingParsedElement::XML_ANIM:
        {
          AnimatorMode::getInstance ()->setVersion (parsedElement.version);
          //qDebug (QString ("XML Version:") + QString::number (version));
          break;
        }
        case RoutingParsedElement::XML_RT:
        {
          RoutingStatsScene::getInstance ()->add (parsedElement.nodeId, parsedElement.updateTime, parsedElement.rt);
          // ++parsedElementCount;
          break;
        }
        case RoutingParsedElement::XML_RP:
        {
          RoutingStatsScene::getInstance ()->addRp (parsedElement.nodeId, parsedElement.destination, parsedElement.updateTime, parsedElement.rpes);
          // ++parsedElementCount;
          break;
        }
        case RoutingParsedElement::XML_INVALID:
        default:
        {
          //qDebug ("Invalid XML element");
        }
        } //switch
    } // while loop
}

RoutingParsedElement
RoutingXmlparser::parseNext ()
{
  RoutingParsedElement parsedElement;
  parsedElement.type = RoutingParsedElement::XML_INVALID;
  parsedElement.version = m_version;

  if (m_reader->atEnd () || m_reader->hasError ())
    {
      m_parsingComplete = true;
      m_traceFile->close ();
      return parsedElement;
    }
  //qDebug ("T" + m_reader->text ().toString ())  ;




  QXmlStreamReader::TokenType token =  m_reader->readNext ();
  if (token == QXmlStreamReader::StartDocument)
    return parsedElement;

  if (token == QXmlStreamReader::StartElement)
    {
      if (m_reader->name().toString() == "anim")
        {
          parsedElement = parseAnim ();
        }
      if (m_reader->name().toString() == "rt")
        {
          parsedElement = parseRt ();
        }
      if (m_reader->name().toString() == "rp")
        {
          parsedElement = parseRp ();
        }
    }

  if (m_reader->atEnd ())
    {
      m_parsingComplete = true;
      m_traceFile->close ();
    }
  return parsedElement;
}


RoutingParsedElement
RoutingXmlparser::parseAnim ()
{
  RoutingParsedElement parsedElement;
  parsedElement.type = RoutingParsedElement::XML_ANIM;
  parsedElement.version = m_version;
  QString v = m_reader->attributes ().value ("ver").toString ();
  if (!v.contains ("netanim-"))
    return parsedElement;
  v = v.replace ("netanim-","");
  m_version = v.toDouble ();
  parsedElement.version = m_version;
  //qDebug (QString::number (m_version));
  QString fileType = m_reader->attributes ().value ("filetype").toString ();
  if (fileType != "routing")
    {
      AnimatorMode::getInstance ()->showPopup ("filetype must be == routing. Invalid routing trace file?");
      NS_FATAL_ERROR ("Invalid routing trace file");
    }
  return parsedElement;
}

RoutingParsedElement
RoutingXmlparser::parseRt ()
{
  RoutingParsedElement parsedElement;

  parsedElement.type = RoutingParsedElement::XML_RT;
  parsedElement.nodeId = m_reader->attributes ().value ("id").toString ().toUInt ();
  parsedElement.rt = m_reader->attributes ().value ("info").toString ();
  parsedElement.updateTime = m_reader->attributes ().value ("t").toString ().toDouble ();
  m_minSimulationTime = qMin (m_minSimulationTime, parsedElement.updateTime);
  m_maxSimulationTime = std::max (m_maxSimulationTime,parsedElement.updateTime);
  return parsedElement;
}

RoutePathElement
RoutingXmlparser::parseRpe ()
{
  RoutePathElement elem;
  elem.nextHop = m_reader->attributes ().value ("nH").toString ();
  elem.nodeId = m_reader->attributes ().value ("n").toString ().toUInt ();
  //QDEBUG ("NH:" + elem.nextHop + " " + "N:" + QString::number (elem.nodeId));
  return elem;
}

RoutingParsedElement
RoutingXmlparser::parseRp ()
{
  RoutingParsedElement parsedElement;

  parsedElement.type = RoutingParsedElement::XML_RP;
  parsedElement.nodeId = m_reader->attributes ().value ("id").toString ().toUInt ();
  parsedElement.destination = m_reader->attributes ().value ("d").toString ();
  parsedElement.updateTime = m_reader->attributes ().value ("t").toString ().toDouble ();
  parsedElement.rpElementCount = m_reader->attributes ().value ("c").toString ().toUInt ();
  if (parsedElement.rpElementCount)
    {
      QXmlStreamReader::TokenType token = m_reader->readNext ();
      while (m_reader->name().toString() != "rp")
        {
          if (m_reader->name().toString() == "rpe" && (token != QXmlStreamReader::EndElement))
            {
              parsedElement.rpes.push_back (parseRpe ());
            }
          token = m_reader->readNext ();
        }

    }
  m_minSimulationTime = qMin (m_minSimulationTime, parsedElement.updateTime);
  m_maxSimulationTime = std::max (m_maxSimulationTime,parsedElement.updateTime);
  return parsedElement;
}


void
RoutingXmlparser::debugElement (RoutingParsedElement element)
{
  QString str = "Node Id:" + QString::number (element.nodeId) + " t:" + QString::number (element.updateTime);
  str += " Rt:" + element.rt;
}

double
RoutingXmlparser::getMaxSimulationTime ()
{
  return m_maxSimulationTime;
}

double
RoutingXmlparser::getMinSimulationTime ()
{
  return m_minSimulationTime;
}



} // namespace netanim

