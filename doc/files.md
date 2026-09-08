# File filters

Current Parts uses local exclusion rules in 0000-index/filters.ini.
It shows every file type, including hidden and system files, except the
reserved 0000-index directory and files excluded by local rules.

The former allow-list configuration in 0000-index/files.ini is no longer
read for filtering. Existing files.ini files are preserved and are not
rewritten or deleted. Rules are not inherited from parent folders or
metadata includes. Pro/E versions and ZIMA-CAD archive copies have separate
visibility settings in the Filters dialog for the current folder.

See [local filters, versions and previews](filters.md) for the current INI
format, defaults, examples and performance behavior.
