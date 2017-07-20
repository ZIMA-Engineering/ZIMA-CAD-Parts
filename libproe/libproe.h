/*            DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE
                    Version 2, December 2004

 Copyright (C) 2004 Sam Hocevar <sam@hocevar.net>

 Everyone is permitted to copy and distribute verbatim or modified
 copies of this license document, and changing it is allowed as long
 as the name is changed.

            DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE
   TERMS AND CONDITIONS FOR COPYING, DISTRIBUTION AND MODIFICATION

  0. You just DO WHAT THE FUCK YOU WANT TO.
 
*/

#ifndef __LIBPROE_H__
#define __LIBPROE_H__

#include <QtCore>

#define REGEX_DESC "^description\\x00"
#define REGEX_ATTR "\\xf6\\x18\\xf6\\xf2\\xf7\\x0d\\xe3|\\xe1\\xe1\\xe1\\xe3"

#define ATTR_SPLIT_STR "\\x27\\x88\\x00\\xe3\\x33"

struct attr_t {
public:
	attr_t(size_t n_off, QString n, QString v) : 
	       name_offset(n_off), name(n), value(v)
	{
		name_len = name.length();
		value_len = value.length();
	}
	
public:
	size_t name_offset;
	size_t name_len, value_len;
	QString name;
	QString value;

};

typedef QList<struct attr_t> attr_arr_t;
typedef QList<struct attr_t>::iterator attr_iter_t;

int proe_get_attr(attr_arr_t &attrs, QTextStream &);
int proe_set_attr(attr_arr_t &attrs, QTextStream &);


#endif

