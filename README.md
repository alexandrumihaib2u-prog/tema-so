# Tema SO – Comunicare între procese prin memorie partajată

Două procese independente numără împreună de la 1 la 1000 într-un obiect mapat în memorie.
Accesul la memorie este protejat de un semafor cu nume.

Algoritm (fiecare proces):
1. rezervă semaforul
2. citește numărul din memorie
3. dă cu banul (`random(2)`); cât timp cade 2, scrie în memorie următorul număr
4. eliberează semaforul

Semaforul e necesar pentru că memoria e „teren neutru”: fără el, ambele procese ar putea
citi aceeași valoare și ar scrie numere duplicate sau în altă ordine.

## Rulare
- **Windows:** `cd windows && start.bat` (deschide două ferestre = două procese)
- **Linux:** `cd linux && ./start.sh`

Fiecare linie afișează PID-ul procesului care a scris numărul, deci se vede cum alternează.
