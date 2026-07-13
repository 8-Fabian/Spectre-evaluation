# Spectre-evaluation
Evaluation of Spectre attack

## Projekt bauen
```
cmake -B build
cd build
make
```
Python venv erstellen
```
python3 -m venv evaluation_venv
source evaluation_venv/bin/activate
pip install -r requirements.txt
```
## Projekt ausführen
Mit python Angreifer starten
```
python3 evaluation.py
```
Einen victim process starten
```
taskset -c <prozessor_kern> ./spectre_victim
```
Prozess mit Mitigation starten
```
taskset -c <prozessor_kern> ./spectre_mitigated
```
