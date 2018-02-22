/* Written by Barna Farago <brown@weblapja.com> 2006-2018
*/
#pragma once

// VUMeters Interface

class IVUMeters{
public:
	int m_aVU[10];
	virtual void DoReset()=0;
};
