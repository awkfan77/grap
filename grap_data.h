//-*-c++-*-
#ifndef GRAP_DATA_H
#define GRAP_DATA_H
// This file is (c) 1998 Ted Faber (faber@lunabase.org) see COPYRIGHT
// for the full copyright and limitations of liabilities.
#ifdef STDC_HEADERS
#include <string.h>
#endif
#include <ctype.h>
#include <string>
typedef string String;

inline string *dblString(double d, string *f=0) {
    const int sz = 64;
    char c[sz];
    string *s;

    if ( !f ) 
	snprintf(c,sz,"%g",d);
    else 
	snprintf(c,sz,f->c_str(),d);
    s = new string(c);
    return s;
}

inline void unquote(string *s) {
    int i;
    if ( (*s)[0] == '"' ) s->erase(0,1);
    i = s->length()-1;
    if ( (*s)[i] == '"' ) s->erase(i,1);
}

inline void quote(string *s) {
    s->insert((string::size_type) 0,1,'"');
    s->append(1,'"');
}

class grap_sprintf_String : public string {
public:
    grap_sprintf_String(const char *f=0) : string(f) {}
    grap_sprintf_String(const string& f) : string(f) {}
    grap_sprintf_String(const string *f) : string() {
	if (f) *this = *f;
    }
    void next_number(double d) {
	const int len=length()+64;
	char *c = new char [len];
	char *fmt = new char[len];
	char *f;
	char *cc;	// Save the start of the new string for deletion 
	int first = 1;

	copy(c, npos);
	c[length()] = '\0';
	cc = c;
	
	f = fmt;
	for(f=fmt; (*f = *c); c++,f++ ) {
	    if (*c == '%' ) {
		if ( first ) {
		    if ( c[1] == '%' ) {
			*++f = '%';
			*++f = '%';
			*++f = '%';
			c++;
		    }
		    else { first = 0; }
		} else { *++f = '%'; }
	    }
	}
	snprintf(c,len,fmt,d);
	*this = c;
	delete[] fmt;
	delete[] cc;
    }
    
    void finish_fmt() {
        // next_number has to double %% throughout the string.  This
        // method removes those doubled % s
	
	char *n = new char[length()+1];
	const char *a;
	char *b;

	for ( a = c_str(), b = n; (*b = *a); a++, b++) {
	    if ( *a == '%' && a[1] == '%' ) a++;
	}
	*this = n;
    }
};

/*  #endif */

class macro {
private:
#ifdef __GNUC__    
    // g++ knows this is a compile time constant, other compilers get
    // confused.  As a result non-GCC compilers allocate arg
    // dynamically.
    static const int numargs = 32;	// maximum number of arguments
#else
    static const int numargs;
#endif
public:
    int next_arg;			// the index into the next argument
    string *text;			// the text of the macro
#ifdef __GNUC__
    string *arg[numargs];		// the current argument values
#else
    string **arg;
#endif
    string *name;			// the name of the macro if it's in a
                                        // dictionary.
    macro(string *t=0, string *n =0) : next_arg(0), text(t), name(n) {
#ifndef __GNUC__
	arg = new string *[numargs];
#endif
	for ( int i = 0; i < numargs ; i++ )
	    arg[i] = 0;
    }
    ~macro() {
	if ( text ) {
	    delete text;
	    text = 0;
	}
	if ( name ) {
	    delete name;
	    name = 0;
	}
	for ( int i = 0; i < numargs ; i++ )
	    if ( arg[i] ) {
		delete arg[i];
		arg[i] = 0;
	    }
#ifndef __GNUC__
	delete arg;
#endif
	
    }
    int add_arg(string *s ) {
	if ( next_arg < numargs ) {
	    if ( arg[next_arg] ) delete arg[next_arg];
	    arg[next_arg++] = s;
	    return 1;
	}
	else return 0;
    }
    
    string *invoke() {
	string *s = new string;	// The expanded macro
	int argn =0;		// the number of the arg to interpolate
	int dig;		// Number of digits seen after this $
	int i=0;		// the current character offset
	const int lim=text->length();// number of characters to check

	while (i < lim ) {
	    switch ((*text)[i] ) {
		default:
		    *s += (*text)[i++];
		    break;
		case '$':
		    dig = 0;
		    i++;
		    while (isdigit((*text)[i])) {
			argn *= 10;
			argn += (*text)[i] - 0x30;
			i++; dig++;
		    }
		    if ( argn > 0 && argn <= numargs ) {
			if ( arg[argn-1] )
			    *s += *(arg[argn-1]);
		    }
		    argn = 0;
		    // If there was no digit after the $ leave the $
		    // untouched and leave i as the offset of the
		    // character immediately following the $
		    
		    if ( !dig ) {
			*s += '$';
		    }
		    break;
	    }
	}
	next_arg = 0;
	for ( i = 0 ; i < numargs; i ++ ) {
	    if ( arg[i] ) delete arg[i];
	    arg[i] =0;
	}
	return s;		
    }
};

class copydesc {
 public:
    typedef enum { fname, until} type;
    type t;
    string *s;
    copydesc() : t(fname), s(0) {} 
    ~copydesc() {
	if ( s ) {
	    delete s;
	    s = 0;
	}
    }
};

// we use this to make yacc happier
typedef pair<coord *, string *> coordid;
#endif
