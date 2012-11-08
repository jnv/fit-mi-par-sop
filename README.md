# Úloha SOP: součet podmnožin

## Architektura virtuálního paralelního počítače

Ve všech ulohách uvažujeme paralelní počítač s distribuovanou pamětí s p procesory očíslovanými 0, 1, 2, …, p-1, propojenými úplnou propojovací sítí (tzv. topologie úplný graf (UG)), kdy pro všechny dvojice různých čísel i a j, procesor i může přímo poslat zprávu procesoru j.

## Velikost vstupní instance

Ve všech úlohách volte rozsah vstupních dat tak, aby sekvenční výpočet na 1 procesoru trval nejméně jednotky minut a nejvýše 20 minut. Vstupní parametry úlohy se zadávají jako parametry příkazové řádky a vstupní data se načítají ze souboru!

## Strategie dělení zásobníku a hledání dárce

Na třetím cvičení si každá dvojice k zadané úloze vybere jednu strategii dělení zásobníku (R-ADZ, D-ADZ, D-R-ADZ) a jednu strategii hledání dárce (ACZ-AHD, GCZ-AHD, NV-AHD). Výběr je třeba zdůvodnit cvičícímu a na konci semestru do zprávy.

# Zadání

## Vstupní data:

- n = přirozené číslo představující mohutnost množiny S, n>19.
- S = množina n různých přirozených čísel (s1,..,sn)
- c = přirozené číslo, c >= mini si
- a = přirozené číslo, 1 < a =< n/10

## Úkol:

Nalezení a neprázdných vzájemně disjunktních podmnožin T množiny S takových, že součet prvků všech a podmnožin T je největší možný a současně součty prvků jednotlivých podmnožin jsou menší než c.

## Výstup algoritmu:

Konstatovaní, zda a podmnožin T existuje. V případě pozitivní odpovědi výpis prvků a podmnožin množiny T a jejich součtů.

## Sekvenční algoritmus:

Sekvenční algoritmus je typu BB-DFS s omezenou hloubkou stromu stavů na n. Řešení nemusí existovat.
Cena, kterou maximalizujeme, je součet prvků a podmnožin množiny T. Algoritmus končí, když je cena rovna horní mezi nebo když prohledá celý stavový prostor.

**Dolní mez** není známa.

**Horní mez** je a*(c-1) (počet podmnožin je a a maximální součet podmnožiny je c-1).

## Paralelní algoritmus:

Paralelní algoritmus je typu G-PBB-DFS-D.