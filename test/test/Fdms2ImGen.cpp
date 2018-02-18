#include "Fdms2ImGen.h"
#include <stdlib.h>
#include <malloc.h>

Fdms2ImGen::Fdms2ImGen()
	:m_fname("test.img"), m_f(NULL), m_size(0)
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
	size_t size;
	size = addSameByte(0x200, 'C'); //C
	size = addSameByte(0x600, 'X'); //X
	size = addSameByte(0x800, 'B'); //B
	size = addSameByte(0x800, 'D'); //D
	size = addSameByte(0x800, 'E'); //E
	return r;
}
int Fdms2ImGen::addGapFF()
{
	int r = 0;
	size_t size;
	size = addSameByte(0x30000, 0xFF); //gap
	return r;
}
int Fdms2ImGen::addAudio()
{
	int r = 0;
	size_t size;
	size = addSameByte(0x100, 0x00); //audio
	return r;
}
