"""Validate complete catalogs and placeholders. Run from any directory."""
from collections import Counter
from pathlib import Path
import re
import xml.etree.ElementTree as ET

root = Path(__file__).resolve().parents[1]
reference = None
for locale in ("cs_CZ", "de_DE", "fr_FR", "ru_RU"):
    tree = ET.parse(root / "locale" / ("zima-cad-parts_" + locale + ".ts"))
    assert tree.getroot().get("language") == locale
    keys = set()
    for context in tree.findall("context"):
        for message in context.findall("message"):
            source = message.findtext("source") or ""
            translation = message.find("translation")
            assert translation is not None, source
            assert translation.get("type") not in ("unfinished", "vanished", "obsolete"), source
            forms = translation.findall("numerusform")
            if message.get("numerus") == "yes":
                assert len(forms) == (3 if locale in ("cs_CZ", "ru_RU") else 2), (locale, source)
            texts = [f.text or "" for f in forms] if forms else [translation.text or ""]
            for text in texts:
                assert text.strip(), (locale, source)
                assert Counter(re.findall(r"%[Ln]?\d+|%n", source)) == Counter(re.findall(r"%[Ln]?\d+|%n", text)), (locale, source)
                assert source.count("\n") == text.count("\n"), (locale, source)
                assert re.findall(r"<[^>]+>", source) == re.findall(r"<[^>]+>", text), (locale, source)
            keys.add((context.findtext("name"), source, message.findtext("comment")))
    if reference is None:
        reference = keys
    assert keys == reference, locale
    print(f"{locale}: {len(keys)} complete translations; placeholders and markup OK")
