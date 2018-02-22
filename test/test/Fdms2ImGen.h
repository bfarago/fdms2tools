#pragma once
#include <stdint.h>
#include <stdio.h>

class Fdms2ImGen
{
public:
	Fdms2ImGen();
	~Fdms2ImGen();
	int create(const char* afname);
	void close();
	void remove();
	const char* getFilename() { return m_fname; }
	size_t getSize() { return m_size; }
private:
	int addCatalog();
	int addProgram();
	int addGapFF();
	int addAudio();
	size_t addSameByte(int len, unsigned char bdata);
	size_t addDwBigendian(int len, unsigned long long dw);
	size_t addDwLittleendian(int len, unsigned long long dw);
	const char* m_fname;
	FILE * m_f;
	size_t	m_size;
	int m_maxsample;
};

