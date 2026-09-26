# Native interaction appearance

ZIMA-CAD-Parts uses the platform Qt style and palette for tree/table selection,
hover, tabs, menus, buttons, focus, disabled controls and navigation. It does not
install an interaction proxy style or overwrite the application highlight palette.
The old azure/green widget overrides and loaded tab/navigation styles are removed.
Changing the desktop theme or accent updates existing widgets through Qt.

The rotated navigation title uses the palette button-text role. Generated local
directory tiles use browser system colors and support light/dark color schemes.
External pages, metadata thumbnails, brand artwork and CAD preview geometry
retain their own content. Selection, navigation and file operations are unchanged.

No UI strings change. Existing application catalogs remain applicable. Native
Linux desktop appearance must be verified on the Linux host; Windows checks do
not establish KDE/GNOME acceptance.
