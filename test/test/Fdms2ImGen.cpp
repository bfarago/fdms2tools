#include "Fdms2ImGen.h"
#include <stdlib.h>
#include <malloc.h>

Fdms2ImGen::Fdms2ImGen()
	:m_fname("test.img"), m_f(NULL), m_size(0), m_maxsample(44100 * 60)
{
}


Fdms2ImGen::~Fdms2ImGen()
{
	close(); //leave the file...
}

void Fdms2ImGen::close()
{
	if (m_f) {
		fclose(m_f);
		m_f = 0;
	}
}

int Fdms2ImGen::create(const char* afname)
{
	errno_t r=fopen_s(&m_f, afname, "w+b"); //  m_f=_fsopen(afilename, "wb+", _SH_DENYWR);
	if (r) {
		printf_s("Error: %s:%i, in function:%s. File can't be created:'%s'\n", __FILE__, __LINE__, __FUNCTION__, afname);
		return r;
	}
	m_size = 0;
	m_fname = afname;
	r |= addCatalog();
	r |= addCatalog();
	r |= addGapFF();
	r |= addAudio();
	close();
	return (int)r;
}
void Fdms2ImGen::remove()
{
	close();
	::remove(m_fname);
	m_size = 0;
}
size_t Fdms2ImGen::addSameByte(int len, unsigned char bdata)
{
	unsigned char* buf = (unsigned char*)malloc(len);
	for (int i = 0; i < len; i++) {
		buf[i] = bdata;
	}
	size_t size = fwrite(buf, 1, len, m_f);
	m_size += size;
	free(buf);
	return size;
}

size_t Fdms2ImGen::addDwBigendian(int len, unsigned long long dw)
{

	unsigned char buf[8];
#if 0
	unsigned long long b=dw;
	unsigned char* p = (unsigned char*)&b;
	if (len > 8)len = 8;
	for (int i = 0; i < len; i++) {
		buf[i] = p[len-1-i];
	}
#else
	int p = 0;
	if (len > 2) { //4
		buf[p++] = (dw >> 16) & 0xff;
		buf[p++] = (dw >> 24) & 0xff;
	}
	buf[p++] = dw & 0xff;
	buf[p++] = (dw >> 8) & 0xff;
	
#endif
	size_t size = fwrite(buf, 1, len, m_f);
	m_size += size;
	return size;
}
size_t Fdms2ImGen::addDwLittleendian(int len, unsigned long long dw)
{
	unsigned char buf[8];
	unsigned long long b = dw;
	unsigned char* p = (unsigned char*)&b;
	if (len > 8)len = 8;
	for (int i = 0; i < len; i++) {
		buf[i] = p[i];
	}
	size_t size = fwrite(buf, 1, len, m_f);
	m_size += size;
	return size;
}
int Fdms2ImGen::addCatalog()
{
	int r = 0;
	size_t size=0;
	size += addSameByte(0x200, 0xFF); //first unknow region
	size += addSameByte(0x100, 'X');  //X
	size += addSameByte(0x100, 0xFF); //unknow region, missing from reverse.txt
	size += addSameByte(0x400, 'A');  //A
	size += addSameByte(0x800, 'B');  //B
	size += addSameByte(4096, 'X'); //X
	size += addSameByte(8192, 'A'); //A
	size += addSameByte(8192, 'A'); //A
	size += addProgram();
	size += addProgram();
	size += addProgram();
	size += addProgram();
	size += addProgram();
	return r;
}
int Fdms2ImGen::addProgram()
{
	int r = 0;
	size_t size=0;
	size += addSameByte(4, 'T'); 
	size += addSameByte(1, 0x34); //MM
	size += addSameByte(1, 0x12); //HH
	size += addSameByte(1, ':');
	size += addSameByte(1, 0x56); //SS
	size += addSameByte(1, 0x11); //FF
	size += addSameByte(1, 0x24); //SF
	size += addSameByte(6, '?'); 
	size += addSameByte(1, 0x08); //click
	size += addSameByte(0x200-size, 'C'); //C

	size = addSameByte(0x600, 'X'); //X

	size = 0; //B block: index
	size += addSameByte(8, 'B');
	unsigned int startpos = 0x00050060;
	unsigned int length = startpos+ (m_maxsample * 16);

	size += addDwBigendian(4, startpos/512); //todo: startpos
	size += addDwBigendian(4, length/512); //todo: len
	size += addDwBigendian(4, 0x00000000); //stop
	size += addDwBigendian(4, 0x00000000); //
	size += addSameByte(0x800-size, 'B'); //B

	size = 0;
	size += addDwBigendian(2, 0x01); // 0001 bar

	size += addSameByte(1, 0x04); // 2/4
	size += addSameByte(1, 0x03);
	size += addDwBigendian(4, 0x00000000); //stop
	size = addSameByte(0x800-size, 'D'); //D

	size = 0;
	size += addSameByte(1, 0x01); // 0001 bar
	size += addSameByte(1, 0x00);
	size += addSameByte(1, 0x00);
	size += addSameByte(1, 0x01);
	size += addSameByte(1, 0x20);
	size += addSameByte(1, 0x01);
	size += addSameByte(1, 0x00);
	size += addSameByte(1, 0x00);
	size += addDwBigendian(4, 0x00000000); //stop
	size = addSameByte(0x800-size, 'E'); //E
	return r;
}
int Fdms2ImGen::addGapFF()
{
	int r = 0;
	size_t size;
	size = addSameByte(0x30000, 0xFF); //gap
	return r;
}
#include <math.h>
int Fdms2ImGen::addAudio()
{
#ifndef MAPPAGELENGTH
#define MAPPAGELENGTH 0x07FFFFFFUL
#endif
	int r = 0;
	size_t size = MAPPAGELENGTH - m_size;
	for (int sample=0; sample<m_maxsample/2; sample++){
		for (int ch=0; ch<8; ch++){
			short d = 0x7fff * (sin(6.28*(double)sample / 44 / (ch + 1)) *( (sample*4)%m_maxsample) /(double)m_maxsample);
			size += addDwLittleendian(2, d);
			size += addDwLittleendian(2, d);
		}
	}
	return r;
}
