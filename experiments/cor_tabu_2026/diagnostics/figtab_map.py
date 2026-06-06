import re
with open('/mnt/c/Users/diazhernan/CLionProjects/IJSP/Papers/COR_Tabu/main.aux') as fh:
    for line in fh:
        m = re.match(r'\\newlabel\{(tab[^}]+|fig[^}]+)\}\{\{([^}]+)\}\{(\d+)', line)
        if m:
            print(f'  {m.group(1):>25} = #{m.group(2):>4}  on page {m.group(3)}')
