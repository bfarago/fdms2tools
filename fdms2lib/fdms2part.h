/*Written by Barna Farago <brown@weblapja.com> 2006-2018
*/
#ifndef _FDMS2PART_H
#define _FDMS2PART_H

#include "fdms2common.h"
#include "fdms2pos.h"

/**fdms2part. Partition bookkeeper.
This class handles one partition particle by offset (physical) and logical pos,
and length informations.
*/
class fdms2part{
public:
 /**fdms2part default constructor.
 Creates a partition object as an empty partition.
 */
 fdms2part();

 /**fdms2part constructor.
 Same like setData() function.
 */
 fdms2part(t1_toffset PhisicalPos, fdms2pos len, fdms2pos& LogicalPos);

 /**setData().
 Set storable informations, and modify the third paramater by adding the
 length of this partition. (because we can skip the partion's area when
 iterating on disk.) 
 */
 void setData(t1_toffset PhisicalPos, fdms2pos len, fdms2pos& LogicalPos);

 /**getStartLength().
 The function takes back by the stored informations about this partition.
 */
 void getStartLength(t1_toffset &PhisicalPos, fdms2pos &len);
 
 /**dumpStr().
 It produce debug string to stdout about the stored informations. Debug purposes only.*/
 void dumpStr(char*& rpcStr);

private:
 t1_toffset m_PhisicalPos;
 fdms2pos   m_LogicalPos;
 fdms2pos   m_Length;
};

class fdms2parttable{
public:
 fdms2parttable();
 void kill();
 void init(int n);
 fdms2part& operator[](int i){return m_parts[i];}

 int m_length;
 fdms2part* m_parts;

};

#endif
