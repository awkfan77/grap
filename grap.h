#ifndef GRAP_H
#define GRAP_H
// This file is (c) 1998 Ted Faber (faber@lunabase.org) see COPYRIGHT
// for the full copyright and limitations of liabilities.

#ifndef DEFINES
#define DEFINES "/usr/share/grap/grap.defines"
#endif

#ifdef __GNUC__
// I worked from the online docs which use UnaryFunction
#define UnaryFunction unary_function
#endif

#ifndef STDC_HEADERS
extern "C" {
#ifndef __GNUC__
    int strlen(const char*);
    char *strcpy(char *, const char *);
    int strcmp(const char *, const char *);
#endif
    char *strcat(char *, const char *);
    char *strncpy(char *, const char *, const int);
    int strncmp(const char *, const char *, const int);
};
#endif

#ifndef HAVE_SNPRINTF
// This is the only signature that snprintf is called with in grap.
// It's concievable that someone could smash the stack here, but
// there' snot much privlege to gain, and I'd be impressed enough if
// you could print a float that caused a stack overflow and error that
// you can break the code.
inline int snprintf(char *s, int lim, const char *fmt, double d) {
    int tot;

#ifdef SPRINTF_NOT_INT
    // For some reason some old versions of SunOS have an sprintf that
    // doesn't return an int.  In this case, we can't bounds check at all.
    
    sprintf(s,fmt,d);
    tot = lim;
#else    
    if ( (tot = sprintf(s,fmt,d)) > lim ) {
	cerr << "Bad format to internal sprintf crashed the stack" << endl;
	abort();
    }
#endif

    return tot;
}
#endif

#include <stl.h>
#ifdef USE_STD_STRING
#include <string>
typedef string String;
#else
// need to include the full definition of the kludgy string class so
// that less<String> is defined when we have no hash map.
#include <string.h>
#include "grap_string.h"
#endif

class DisplayString;
class line;
class coord;
class macro;
class plot;
class tick;
class grid;
class circle;
class shiftdesc;


#ifndef HAVE_HASH_MAP
typedef less<String> Strcmp;

typedef map<String, double *, Strcmp> doubleDictionary;
typedef map<String, coord *, Strcmp> coordinateDictionary;
typedef map<String, line *, Strcmp> lineDictionary;
typedef map<String, macro *, Strcmp> macroDictionary;
#else
#include <hash_map>
// A functor for hashing strings - it is an adapter to get to the
// standard library char * hash function.
class Strhash : public unary_function<const String&, size_t> {
private:
    hash<const char *> h;
public:
    size_t operator()(const String& s) const {
	return h(s.c_str());
    }
};

typedef hash_map<String, double *, Strhash> doubleDictionary;
typedef hash_map<String, coord *, Strhash> coordinateDictionary;
typedef hash_map<String, line *, Strhash> lineDictionary;
typedef hash_map<String, macro *, Strhash> macroDictionary;
#endif
typedef list<plot *> plotSequence;
typedef list<double> doublelist;
typedef list<tick *> ticklist;
typedef list<grid *> gridlist;
typedef list<String *> linelist;
typedef list<String *> stringSequence;
typedef list<circle *> circleSequence;
typedef list<struct grap_buffer_state*> lexStack;
typedef list<DisplayString *> stringlist;
typedef list<shiftdesc *> shiftlist;

// number of functions taking 0,1,2 args.  The names of those
// functions are in grap_lex.l and the implementations in the
// jumptables in grap.y
const int NF0=1;
const int NF1=8;
const int NF2=3;

enum size { ht = 0, wid};

typedef struct {
    int op;
    double expr;
} bydesc;

// this is complex enough to need a constructor/destructor
class for_descriptor {
 public:
    double *loop_var;
    int dir;
    double limit;
    double by;
    int by_op;
    String *anything;
    for_descriptor() : 
	loop_var(0), dir(0), limit(0.0), by(0.0), by_op(0), anything(0) { }
    for_descriptor(double *lv, int d, double l, int b, int bo, String *a) :
	loop_var(lv), dir(d), limit(l), by(b), by_op(bo), anything(a) { }
    ~for_descriptor() {
	if ( anything) {
	    delete anything;
	    anything = 0;
	}
    }
};

typedef enum { GFILE=1,  GMACRO, GINTERNAL } grap_input;

class grap_buffer_state {
 public:
    struct yy_buffer_state *yy;
    for_descriptor *f;
    String *name;
    int line;
    int report_start;
    grap_input type;
    int tokenpos;
    grap_buffer_state() :
	yy(0), f(0), name(0), line(0), report_start(0), type(GFILE),
	tokenpos(0) { }
    grap_buffer_state(struct yy_buffer_state *yyb, for_descriptor *fo,
		      String *n, int l, int rs, grap_input t, int tp=0) :
	yy(yyb), f(fo), name(n), line(l), report_start(rs), type(t),
	tokenpos(tp) { }
    // this does *not* call yy_delete_buffer
    ~grap_buffer_state() {
        if ( f ) {
	    delete f;
	    f = 0;
	}
	if ( name ) {
	    delete name;
	    name = 0;
	}
    }
};


extern lexStack lexstack;

#define EPSILON	1e-6


#ifdef HAVE_RANDOM
#ifndef RANDOM_DECLARED
extern "C" {
    long random();
}
#endif
#else 
#ifdef HAVE_RAND
#define random rand
#ifndef RAND_DECLARED
extern "C" {
    long rand();
}
#endif
#endif 
#endif

#endif
