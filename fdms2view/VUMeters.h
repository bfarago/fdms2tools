#pragma once

// VUMeters Interface

class IVUMeters{
public:
	int m_aVU[10];
	virtual void DoReset()=0;
};
