# Native ZIMA-CAD metadata fixtures

These documents were created and saved with the native ZIMA-CAD CLI from
commit `9dc04b5e`. The Part uses INI format 41 and the Assembly format 33.
They exercise actual serialized parameters, Czech column-name aliases,
localized/shared values, punctuation, zero quantity and calculated mass.

The integration test copies the fixtures to a temporary directory and imports
them through the production reader in Czech and English. It does not require
ZIMA-CAD or OCCT to read the metadata.
