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

//#include <iostream>
//#include <fstream>
//#include <vector>
//#include <boost/regex.hpp>

#include "libproe.h"

int proe_get_attr(attr_arr_t &attrs, QTextStream &s)
{
	QString str;
	QRegExp rgx(REGEX_DESC);
	qint64 pos;

	do {
		pos = s.pos();
		str = s.readLine();
        //qDebug() << "line" << pos << !str.isNull() << rgx.indexIn(str) << s.status();
        // TODO/FIXME: a hack for fixing infinite loop in eg. ZE0088-KIEKERT-FIAT_XUV-new_routing_of_fishmouth_sealing/ze0088-0000-0101.prt.11
        //if (pos > 10000) {
        //    qDebug() << "Infinite loop threshold reached. Skipping.";
        //    return 1;
       // }
	} while (!str.isNull() && rgx.indexIn(str) != 0 &&
             s.status() == QTextStream::Ok);
	
	s.seek(pos);
	
	rgx.setPattern(REGEX_ATTR);
	int len_n, len_v;
	int offset = 0;
	
	/* Data composition:
	 * <garbage><attribute mark><attribute name><\0><ATTR_SPLIT_STR>
	 * <attribute value><\0><some other stuff...>
	 */
	
	while ((offset = rgx.indexIn(str, offset)) != -1) {
		offset += rgx.matchedLength();
		len_n = str.indexOf(QChar('\0'), offset);// + rgx.matchedLength());
		len_v = str.indexOf(QChar('\0'), len_n + sizeof(ATTR_SPLIT_STR));
		len_v = len_v - len_n - sizeof(ATTR_SPLIT_STR);
		len_n -= offset;
		if (len_n > 0)
			attrs.append(attr_t(offset, str.mid(offset, len_n), 
			    str.mid(offset + len_n + sizeof(ATTR_SPLIT_STR), len_v)));
		
		offset += len_n + sizeof(ATTR_SPLIT_STR) + len_v;
	}
	
	return 0;
}

int proe_set_attr(attr_arr_t &attrs, QTextStream &s)
{
	
	for (attr_iter_t it = attrs.begin(); it != attrs.end(); ++it) {
		s.seek(it->name_offset);
		
		if (it->name_len > (size_t) it->name.length()) {
			s << it->name;
			for (size_t i = 0; i < it->name_len - it->name.length(); ++i)
				s << ' ';
		} else
			s << it->name.left(it->name_len);
		
		s.seek(it->name_offset + sizeof(ATTR_SPLIT_STR));
		
		if (it->value_len > (size_t) it->value.length()) {
			s << it->value;
			for (size_t i = 0; i < it->value_len - it->value.length(); ++i)
				s << ' ';
		} else
			s << it->value.left(it->value_len);
	}
	
	return 0;
}
