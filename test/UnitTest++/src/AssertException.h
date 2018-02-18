#ifndef UNITTEST_ASSERTEXCEPTION_H
#define UNITTEST_ASSERTEXCEPTION_H

#include <exception>


namespace UnitTest {
#define MAXFILENAMELEN (256)
#define MAXDESCRIPTIONLEN (512)

class AssertException : public std::exception
{
public:
    AssertException(char const* description, char const* filename, int lineNumber);
    virtual ~AssertException() throw();

    virtual char const* what() const throw();

    char const* Filename() const;
    int LineNumber() const;

private:
    char m_description[MAXDESCRIPTIONLEN];
    char m_filename[MAXFILENAMELEN];
    int m_lineNumber;
};

}

#endif
