# Interaction colours and desktop integration

ZIMA-CAD-Parts uses the same interaction colours as ZIMA-CAD:

- Confirmed item selection and active tabs/commands: azure `#00D1FF`.
- Pointer hover on an available target: green `#4DD811`.
- Text on either highlight: black for contrast.

A selected tree/table row remains azure under the pointer. Hover does not
change selection. Active tabs and checkable buttons temporarily become green
under the pointer and return to azure on leave. Menus use green for their
temporary offered action. Disabled controls retain the platform's disabled
appearance. Generated directory tiles use the same green hover; external web
pages and CAD model colours are not restyled.

The shared `PartsInteraction::Style` wraps the platform-selected Qt style.
It changes item highlight painting without replacing tree branches, focus
indicators, indentation, scrolling, font metrics or selection behavior. Trees
enable mouse tracking and retain ordinary Qt delegates and selection models.
The datasource icon size remains 32 pixels through `setIconSize`, rather than
a tree stylesheet.

The navigation panel uses the desktop palette and inherited font. It no longer
forces a dark background, Arial, or a custom scrollbar. On Linux the installed
Qt platform-theme/style integration supplies the desktop appearance; this
application does not force Fusion or a Windows style. KDE/GNOME integration
still depends on the host's Qt plugins. Native Linux visual verification must
be performed on the Linux host; Windows verification cannot prove it.

These changes introduce no UI strings. All existing translation catalogs remain
applicable and are validated with the build.
