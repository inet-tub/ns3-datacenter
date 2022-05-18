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

#include "netanim.h"

using namespace ns3;
using namespace netanim;

int main (int argc, char *argv[])
{
  ns3::LogComponentEnable ("AnimatorScene", ns3::LOG_LEVEL_ALL);
  ns3::LogComponentEnable ("AnimatorView", ns3::LOG_LEVEL_ALL);
  ns3::LogComponentEnable ("AnimNode", ns3::LOG_LEVEL_ALL);
  ns3::LogComponentEnable ("AnimPacket", ns3::LOG_LEVEL_ALL);
  ns3::LogComponentEnable ("AnimatorMode", ns3::LOG_LEVEL_ALL);
  ns3::LogComponentEnable ("ResizeableItem", ns3::LOG_LEVEL_ALL);
  ns3::LogComponentEnable ("Animxmlparser", ns3::LOG_LEVEL_ALL);
  ns3::LogComponentEnable ("AnimPropertyBroswer", ns3::LOG_LEVEL_ALL);
  //ns3::LogComponentEnable ("PacketsScene", ns3::LOG_LEVEL_ALL);
  //ns3::LogComponentEnable ("PacketsView", ns3::LOG_LEVEL_ALL);
  ns3::LogComponentEnable ("GraphPacket", ns3::LOG_LEVEL_ALL);
  ns3::LogComponentEnable ("CounterTablesScene", ns3::LOG_LEVEL_ALL);
  ns3::LogComponentEnable ("PacketsMode", ns3::LOG_LEVEL_ALL);
  //ns3::LogComponentEnable ("PacketsScene", ns3::LOG_LEVEL_ALL);


  QApplication app (argc, argv);
  app.setApplicationName ("NetAnim");
  app.setWindowIcon (QIcon (":/resources/netanim-logo.png"));
  NetAnim netAnim;
  return app.exec ();


}
