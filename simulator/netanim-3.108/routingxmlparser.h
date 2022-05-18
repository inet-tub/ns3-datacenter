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
#ifndef ROUTINGXMLPARSER_H
#define ROUTINGXMLPARSER_H

#include "common.h"

namespace netanim
{

typedef struct
{
  uint32_t nodeId;
  QString nextHop;
} RoutePathElement;

typedef std::vector <RoutePathElement> RoutePathElementsVector_t;
struct RoutingParsedElement
{
  enum RoutingParsedElementType
  {
    XML_INVALID,
    XML_ANIM,
    XML_RT,
    XML_RP,
    XML_RPE
  };
  RoutingParsedElementType type;

  // Anim

  double version;


  // Update time

  double updateTime;

  // Node

  uint32_t nodeId;

  // Routing table

  QString rt;

  // Route Path

  uint32_t rpElementCount;
  RoutePathElementsVector_t rpes;
  QString destination;

};


class RoutingXmlparser
{
public:
  RoutingXmlparser (QString traceFileName);
  ~RoutingXmlparser ();
  RoutingParsedElement parseNext ();
  bool isParsingComplete ();
  double getMaxSimulationTime ();
  double getMinSimulationTime ();
  bool isFileValid ();
  uint64_t getRtCount ();
  void doParse ();


private:
  QString m_traceFileName;
  bool m_parsingComplete;
  QXmlStreamReader * m_reader;
  QFile * m_traceFile;
  double m_maxSimulationTime;
  double m_minSimulationTime;
  bool m_fileIsValid;
  double m_version;
  RoutingParsedElement parseAnim ();
  RoutingParsedElement parseRt ();
  RoutingParsedElement parseRp ();
  RoutePathElement parseRpe ();
  void parseGeneric (RoutingParsedElement &);

  void searchForVersion ();
  void debugElement (RoutingParsedElement element);
};




}  // namespace netanim

#endif // ROUTINGXMLPARSER_H
