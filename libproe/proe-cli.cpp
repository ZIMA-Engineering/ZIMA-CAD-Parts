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

#include "libproe.h"
#include <iostream>
#include <QFile>



void usage(char * name)
{
    QTextStream(stdout) << "Usage: " << name << " <file>" << endl;
}

int interact(attr_arr_t &attrs)
{
    int c, i;
    std::string input;
    QTextStream cout(stdout);

    cout << "Enter \"0\" to save, \"-1\" to abort, index number to change the attribute."
         << endl;

    i = 1;
    for (attr_iter_t it = attrs.begin(); it != attrs.end(); ++it, ++i)
        cout << i << ": " << it->name << " => " << it->value << "\n";


    while (1) {
        std::cin >> c;
        std::cin.get();
        switch (c) {
        case -1:
            return -1;
        case 0:
            return 0;
        default:
            if (c > (int) attrs.size()) {
                cout << "Index too high, try again." << "\n";
                continue;
            }

            cout << "Change name (leave empty to skip): ";
            std::getline(std::cin, input);
            if (input.length() > 0)
                attrs[c-1].name = QString(input.data());

            cout << "Change value (leave empty to skip): ";
            std::getline(std::cin, input);
            if (input.length() > 0)
                attrs[c-1].value = QString(input.data());

            i = 1;
            for (attr_iter_t it = attrs.begin(); it != attrs.end(); ++it, ++i)
                cout << i << ": " << it->name << " => " << it->value << "\n";
        }
    }
}

int main(int argc, char ** argv)
{
    if(argc < 2) {
        usage(argv[0]);
        return -1;
    }

    attr_arr_t attrs;
    QFile f (argv[1]);
    int rc;
    QTextStream cout(stdout);

    if(!f.open(QIODevice::ReadWrite)) {
        cout << "Cannot open file." << endl;
        return -2;
    }

    QTextStream ts(&f);

    rc = proe_get_attr(attrs, ts);
    if (rc) {
        cout << "Error reading attributes." << endl;
        rc = -3;
        goto end;
    }

    if (attrs.length() == 0) {
        cout << "No attributes found. Exiting." << endl;
        goto end;
    }

    rc = interact(attrs);
    if (rc == -1) {
        cout << "Aborted." << endl;
        rc = -4;
        goto end;
    }

    rc = proe_set_attr(attrs, ts);
    if (rc) {
        cout << "Error saving attributes." << endl;
        rc = -5;
        goto end;
    }

end:
    return rc;
}
