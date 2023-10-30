#include "bufferlog-tag.h"
namespace ns3{

TypeId
BufferLogTag::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::BufferLogTag")
    .SetParent<Tag> ()
    .AddConstructor<BufferLogTag> ()
  ;
  return tid;
}
TypeId
BufferLogTag::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}
uint32_t
BufferLogTag::GetSerializedSize (void) const
{
  return 16;
}
void
BufferLogTag::Serialize (TagBuffer i) const
{
  i.WriteU32 (queueLength);
  i.WriteU32 (averageQueueLength);
  i.WriteU32 (OccupiedBuffer);
  i.WriteU32 (averageOccupiedBuffer);
}
void
BufferLogTag::Deserialize (TagBuffer i)
{
	queueLength = i.ReadU32();
	averageQueueLength = i.ReadU32();
	OccupiedBuffer = i.ReadU32();
	averageOccupiedBuffer = i.ReadU32();
}
void
BufferLogTag::Print (std::ostream &os) const
{
  os << "I dont care... :P " ;
}


void 
BufferLogTag::setQueueLength(uint32_t q){
	queueLength = q;
}
void 
BufferLogTag::setAverageQueueLength(uint32_t q){
	averageQueueLength = q;
}
void 
BufferLogTag::setOccupiedBuffer(uint32_t q){
	OccupiedBuffer = q;
}
void 
BufferLogTag::setAverageOccupiedBuffer(uint32_t q){
	averageOccupiedBuffer = q;
}

uint32_t 
BufferLogTag::getQueueLength(){
	return queueLength;
}
uint32_t 
BufferLogTag::getAverageQueueLength(){
	return averageQueueLength;
}
uint32_t 
BufferLogTag::getOccupiedBuffer(){
	return OccupiedBuffer;
}
uint32_t 
BufferLogTag::getAverageOccupiedBuffer(){
	return averageOccupiedBuffer;
}
/* Modification */
}