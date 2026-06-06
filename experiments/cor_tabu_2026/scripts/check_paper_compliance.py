#!/usr/bin/env python3
import re, os

P = "/mnt/c/Users/diazhernan/CLionProjects/IJSP/Papers/COR_Tabu/main.tex"
if not os.path.exists(P):
    P = "C:/Users/diazhernan/CLionProjects/IJSP/Papers/COR_Tabu/main.tex"
src = open(P, encoding="utf-8").read()

def strip_latex(s):
    s = re.sub(r"\\[a-zA-Z]+\{([^}]*)\}", r"\1", s)
    s = re.sub(r"\\[a-zA-Z]+", "", s)
    s = re.sub(r"[\\$\{\}_^]", "", s)
    s = re.sub(r"\s+", " ", s).strip()
    return s

print("=== Abstract ===")
m = re.search(r"\\begin\{abstract\}(.*?)\\end\{abstract\}", src, re.DOTALL)
if m:
    plain = strip_latex(m.group(1))
    words = plain.split()
    n = len(words)
    print(f"  words: {n}  ({'OK ≤250' if n <= 250 else 'OVER 250 — TRIM'})")

print()
print("=== Highlights (target: 3-5 bullets, ≤85 chars each) ===")
m = re.search(r"\\begin\{highlights\}(.*?)\\end\{highlights\}", src, re.DOTALL)
if m:
    items = re.split(r"\\item", m.group(1))
    for item in items:
        item = item.strip()
        if not item: continue
        plain = strip_latex(item)
        n = len(plain)
        flag = "OK" if n <= 85 else f"OVER (-{n-85})"
        print(f"  [{n:3d} chars] {flag}: {plain[:90]}")

print()
print("=== Reference style ===")
print(f"  Currently uses elsarticle-template-num (numbered references)")
print(f"  C&OR target: Harvard (author-year) per guide — VERIFY on journal page")

print()
print("=== Required declarations (presence check) ===")
checks = [
    ("Declaration of Competing Interest", r"(competing[\s\w]*interest|conflict of interest)"),
    ("CRediT / Author contributions",     r"(CRediT|author contributions|authorship contribution)"),
    ("Funding statement",                 r"(funding[\s\w]*statement|funded by|supported by)"),
    ("Data availability",                 r"(data availability)"),
    ("AI usage disclosure",               r"(generative AI|ChatGPT|Claude|large language model)"),
]
for name, pat in checks:
    n = len(re.findall(pat, src, re.IGNORECASE))
    print(f"  {name:35s}: {'present' if n else 'MISSING'}")
