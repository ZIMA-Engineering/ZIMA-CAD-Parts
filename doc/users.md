users.ini
=========

`users.ini` is a configuration file that can be located only in the data
source's root `0000-index` directory. It is used by ZIMA-WEB-Parts (ZWP)
to grant access to configured set of users to specific parts.

Example file:

    ; `anonymous` is a special user that is used for users that are not logged
    ; in
    [anonymous]
    parts = stp

    [user1]
    password = mysecret
    parts = prt,igs,stl,stp

    [user2]
    password = mysecret
    parts = prt

Sections are user names, password is stored in key `password`. Key `parts`
configures a set of part types that the user has access to.
