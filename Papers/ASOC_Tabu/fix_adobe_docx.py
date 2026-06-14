#!/usr/bin/env python3
"""Repair the Adobe PDF->Word conversion of the blinded manuscript.

Adobe's OCR misread every 'm' as 'rn' (936 cases) and turned en-dashes into
U+FFFD. We hold the correct text (from the LaTeX source via pandoc), so we
build a corrupted->correct dictionary from the real vocabulary and replace
ONLY exact known corrupted tokens (never a blind rn->m, which would break
legitimate words like 'return' or 'modern'). The .docx XML is edited in place
and re-zipped.
"""
import re
import shutil
import zipfile

SRC_DOCX = "submission_ASOC_blind/main_blind.docx"
OUT_DOCX = "submission_ASOC_blind/main_blind_fixed.docx"
CORRECT = "correct_text.txt"

# 1. Correct vocabulary (case-sensitive) from the LaTeX-derived text
text = open(CORRECT, encoding="utf-8").read()
vocab = set(re.findall(r"[A-Za-z]+", text))

# 2. corrupted -> correct, skipping ambiguous forms that are themselves words
fixes = {}
for w in vocab:
    if "m" in w:
        corrupt = w.replace("m", "rn")
        if corrupt != w and corrupt not in vocab:
            fixes[corrupt] = w
# Longer corrupted tokens first (avoid partial overlaps)
ordered = sorted(fixes.items(), key=lambda kv: -len(kv[0]))

# 3. Patch the document XML
shutil.copy(SRC_DOCX, OUT_DOCX)
with zipfile.ZipFile(SRC_DOCX) as z:
    names = z.namelist()
    data = {n: z.read(n) for n in names}

xml = data["word/document.xml"].decode("utf-8")
before = xml

n_word = 0
for corrupt, correct in ordered:
    pat = r"(?<![A-Za-z])" + re.escape(corrupt) + r"(?![A-Za-z])"
    xml, k = re.subn(pat, correct, xml)
    n_word += k

# 4. Baked-in hyphenation from PDF line breaks ("Ef-fective" -> "Effective").
# Only join when the merged form is a real word AND the hyphenated form is NOT
# a legitimate compound in the source (so "best-case", "ABC-based" are kept).
hyfix = 0
for a, b in set(re.findall(r"([A-Za-z]+)-([A-Za-z]+)", xml)):
    joined = a + b
    # plain hyphenation artifact -> word; or hyphenation+m->rn combined
    target = None
    if joined in vocab and (a + "-" + b) not in vocab:
        target = joined
    elif joined in fixes:            # e.g. op-tirnisation -> optimisation
        target = fixes[joined]
    if target:
        pat = r"(?<![A-Za-z])" + re.escape(a + "-" + b) + r"(?![A-Za-z])"
        xml, k = re.subn(pat, target, xml)
        hyfix += k

# 4b. Citation brackets: Adobe read "[" as "l" and "]" as "j"  ("l12j" -> "[12]")
xml, n_brk = re.subn(r"(?<![A-Za-z])l(\d[\d,\s]*?)j(?![A-Za-z])", r"[\1]", xml)

# 4c. Explicit capital-letter OCR confusions (E->B, W->V, I->l, f->J) for the
# specific non-word tokens found; conservative, avoids real words like "Bold".
explicit = {
    "fBABC": "fEABC", "JEABC": "fEABC", "Bxtreme": "Extreme",
    "Bxtra": "Extra", "Bvery": "Every", "Vilcoxon": "Wilcoxon",
    "Vhen": "When", "Iast": "last", "Optirnization": "Optimization",
}
n_cap = 0
for bad, good in explicit.items():
    pat = r"(?<![A-Za-z])" + re.escape(bad) + r"(?![A-Za-z])"
    xml, k = re.subn(pat, good, xml)
    n_cap += k

# 5. Broken dashes: U+FFFD -> en-dash (the source used '--' = en-dash)
n_dash = xml.count("�")
xml = xml.replace("�", "–")

data["word/document.xml"] = xml.encode("utf-8")

# 5. Re-zip
with zipfile.ZipFile(OUT_DOCX, "w", zipfile.ZIP_DEFLATED) as z:
    for n in names:
        z.writestr(n, data[n])

print("dictionary size:", len(fixes))
print("m->rn replacements applied:", n_word)
print("hyphenation joins applied:", hyfix)
print("citation brackets fixed:", n_brk)
print("dashes fixed:", n_dash)
print("changed:", before != xml)

# Diagnostic: tokens still holding 'rn' that are neither real words nor fixed
import collections
leftovers = collections.Counter()
for tok in re.findall(r"[A-Za-z]+rn[A-Za-z]*|[A-Za-z]*rn[A-Za-z]+", xml):
    if tok not in vocab and tok.replace("rn", "m") in vocab:
        leftovers[tok] += 1
if leftovers:
    print("POSSIBLE residual m->rn tokens:", dict(leftovers.most_common(15)))
else:
    print("no residual m->rn tokens detected")
